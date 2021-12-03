#include "Utilities/strtools.h"

#include "MemoryLifetimeOwner.h"

#include "uuid/uuid.h" //requires apt-get install uuid-dev
#include <stdlib.h>    //malloc
#include <ctype.h>
using namespace std;

const char *newUUID() {
  char *sUUID = MM_MALLOC_FOR_RETURN(37);
  uuid_t newUUID;
  uuid_generate(newUUID);
  uuid_unparse(newUUID, sUUID);
  return sUUID;
}

char *itoa(int n, const char *prefix, const char *suffix) {
  //caller frees result
  int absn;
  char *buffer, *pos;
  int len;
  int digits      = 0;
  int prefix_len  = (prefix ? strlen(prefix) : 0);
  int suffix_len  = (suffix ? strlen(suffix) : 0);
  int is_negative = (n < 0);

  //calculate size
  absn = abs(n);
  do {digits++;} while ((absn /= 10) > 0);
  len    = prefix_len + is_negative + digits + suffix_len;
  buffer = MM_MALLOC_FOR_RETURN(len + 1);
  
  //fill content
  if (prefix_len) strcpy(buffer, prefix);
  absn  = abs(n);
  pos   = buffer + len - suffix_len;
  *pos-- = '\0'; //zero terminate
  do {*pos-- = (absn % 10) + '0';} while ((absn /= 10) > 0);
  if (is_negative) *pos-- = '-';
  pos++;
  if (suffix_len) strcpy(buffer + len - suffix_len, suffix); //including zero terminator

  return buffer;
}

char *utoa(unsigned int n) {
  return itoa(n);
}

char *ultoa(unsigned long n) {
  return utoa(n);
}

const char *strchrs(const char *sString, const char *sChrs) {
  char c;
  const char *sPos   = sString;
  const char *sFound = 0;

  if (sString && sChrs) {
    while (!sFound && (c = *sPos)) {
      sFound = strchr(sChrs, c);
      sPos++;
    }
  }

  return sFound ? sPos - 1: 0;
}

size_t strcount(const char *x, const char y, size_t l) {
  size_t z = 0;
  char c;

  if (l) {
    while (l-- && (c = *x++)) if (c == y) z++;
  } else {
    while (c = *x++) if (c == y) z++;
  }

  return z;
}

const char *strnchr(const char *sString, const char cChr, size_t iLen) {
  if (!sString) return (0);

  while (iLen--) {
    if (*sString == cChr) return sString;
    sString++;
  }
  return (0);
}

const char *strnrchr(const char *s, const char c, size_t n) {
  const char *f = 0;

  while (n-- > 0 && *s) {
    if (c == *s) f = s;
    s++;
  }

  return f;
}

char *strnstr(const char *s, const char *find, size_t slen) {
  char c, sc;
  size_t len;

  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
        if ((sc = *s++) == '\0' || slen-- < 1) return (NULL);
      } while (sc != c);
      if (len > slen) return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}

const char *strpad(const char *sIn, const char cPad, size_t iPad) {
  //IF DIFFERENT: caller frees result 
  size_t iCur, iDiff;
  char *sRet = 0;
  
  if (sIn && iPad) {
    iCur = strlen(sIn);
    if (iCur < iPad) {
      iDiff = (iPad - iCur);
      sRet  = MM_MALLOC_FOR_RETURN(iPad + 1);
      memcpy(sRet + (iDiff), sIn, iCur);
      memset(sRet, cPad, (iDiff));
      sRet[iPad] = 0;
    }
  }

  return (sRet ? sRet : sIn);
}

char *strend(char *s) {
  return (s ? s + strlen(s) : NULL);
}

char *strquote(const char *s) {
  /* caller frees result */
  char *ret = NULL;
  size_t len;
  
  if (s) {
    len = strlen(s);
    ret = MM_MALLOC(len + 2 + 1);
    *ret = '\'';
    strcpy(ret + 1, s);
    ret[len + 1] = '\'';
    ret[len + 2] = '\0';
  }
  return ret;
}

const char *strrstr(const char *x, const char *y) {
  int m = strlen(x);
  int n = strlen(y);
  char *X = MM_MALLOC(m+1);
  char *Y = MM_MALLOC(n+1);
  int i;
  for (i=0; i<m; i++) X[m-1-i] = x[i]; X[m] = 0;
  for (i=0; i<n; i++) Y[n-1-i] = y[i]; Y[n] = 0;
  const char *Z = strstr(X,Y);
  if (Z) {
    int ro = Z-X;
    int lo = ro+n-1;
    int ol = m-1-lo;
    Z = x+ol;
  }
  
  //free up
  MM_FREE(X); 
  MM_FREE(Y);
  
  return Z;
}

bool strisspace(const char *x) {
  while (isspace(*x)) x++;
  return !*x;
}

const char *strreplace(const char *x, const char f, const char r) {
  
  char c, *yp, *y = 0;
  
  if (x) {
    yp = y = MM_MALLOC_FOR_RETURN(strlen(x) + 1);
    while (c = *x) {
      if (c == f) *yp = r;
      else        *yp = c;
      yp++;
      x++;
    }
    *yp = 0;
  }
      
  return y;
}

vector<string> split(string stString, const char *sRegex, REGEX_FLAGS iFlags) {
  return split(stString.c_str(), sRegex, iFlags);
}

vector<string> split(const char *sString, const char *sRegex, REGEX_FLAGS iFlags) {
  rpattern_c rgx1(sRegex, iFlags, MODE_DEFAULT); //compile the regex
  split_results *pmResults = new split_results;
  rgx1.split(sString, *pmResults);
  return pmResults->strings();
}

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t ilen) throw() {
  char *sDup = MM_MALLOC_FOR_RETURN(ilen + 1);
  memcpy(sDup, s, ilen);
  sDup[ilen] = 0;
  return sDup;
}
#endif
