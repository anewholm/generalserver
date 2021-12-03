//platform agnostic file
#include "StringVector.h"

namespace general_server {
  typename StringVector::iterator StringVector::find(const char *sKey) {
    typename StringVector::iterator iItem;
    if (sKey) {
      for (iItem = begin(); iItem != end(); iItem++) if (*iItem && (sKey == *iItem || !strcmp(sKey, *iItem))) break;
    } else iItem = end();
    return iItem;
  }
  
  typename StringVector::const_iterator StringVector::find_const(const char *sKey) const {
    typename StringVector::const_iterator iItem;
    if (sKey) {
      for (iItem = begin(); iItem != end(); iItem++) if (*iItem && (sKey == *iItem || !strcmp(sKey, *iItem))) break;
    } else iItem = end();
    return iItem;
  }

  void StringVector::push_back_unique(const char *sKey) {
    if (find(sKey) == end()) push_back(sKey);
  }

  void StringVector::insert_unique(const StringVector::const_iterator iStart, const StringVector::const_iterator iEnd, const StringVector::iterator iWhere) {
    typename StringVector::const_iterator iNewItem;
    for (iNewItem = iStart; iNewItem != iEnd; iNewItem++) {
      if (find(*iNewItem) == end()) insert(iWhere, *iNewItem);
    }
  }
  
  void StringVector::insert_unique(const StringVector::const_iterator iStart, const StringVector::const_iterator iEnd) {
    typename StringVector::const_iterator iNewItem;
    for (iNewItem = iStart; iNewItem != iEnd; iNewItem++) push_back_unique(*iNewItem);
  }

  const char *StringVector::toString(const char *sDelimiter) const {
    string stString;
    typename StringVector::const_iterator iItem;
    for (iItem = begin(); iItem != end(); iItem++) {
      if (sDelimiter && stString.size()) stString += sDelimiter;
      stString += *iItem;
    }
    return MM_STRDUP(stString.c_str());
  }
}
