#ifndef _ITOA_C
#define _ITOA_C

#define INT_DIGITS 19   /* enough for 64 bit integer */
#include "define.h"
#include <cstring>
#include <vector>

using namespace std;

#include "Utilities/regexpr2.h"
//GRETA: http://easyethical.org/opensource/spider/regexp%20c++/greta2.htm
using namespace regex;


const char *newUUID();

char *itoa(int n, const char *sPrefix = 0, const char *sSuffix = 0);
char *utoa(unsigned int n);
char *ultoa(unsigned long n);

const char *strchrs(const char *sString, const char *sChrs);
size_t strcount(const char *x, const char y, size_t l);
const char *strnchr(const char *sString, const char cChr, size_t iLen);
const char *strnrchr(const char *s, const char c, size_t n);
char *strnstr(const char *s, const char *find, size_t slen);
const char *strpad(const char *sIn, const char cPad, size_t iPad);
char *strquote(const char *s);
const char *strrstr(const char *x, const char *y);
bool strisspace(const char *x);
const char *strreplace(const char *x, const char f, const char r);
char *strend(char *s);
vector<string> split(string stString, const char *sRegex = "[\\s,;]+", REGEX_FLAGS iFlags = SINGLELINE);
vector<string> split(const char *sString, const char *sRegex = "[\\s,;]+", REGEX_FLAGS iFlags = SINGLELINE);
#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t ilen) throw();
#endif

#endif
