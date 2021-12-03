//platform agnostic file
#ifndef _STRINGVECTOR_H
#define _STRINGVECTOR_H

#include <vector>
#include <string.h>
#include <string>
#include "MemoryLifetimeOwner.h"
using namespace std;

namespace general_server {
  class StringVector: public vector<const char*> {
  public:
    typename StringVector::iterator find(const char *sKey);
    typename StringVector::const_iterator find_const(const char *sKey) const;
    void push_back_unique(const char *sKey);
    void insert_unique(const StringVector::const_iterator iStart, const StringVector::const_iterator iEnd, const StringVector::iterator iWhere);
    void insert_unique(const StringVector::const_iterator iStart, const StringVector::const_iterator iEnd);
    const char *toString(const char *sDelimiter = ",") const;
  };
}

#endif
