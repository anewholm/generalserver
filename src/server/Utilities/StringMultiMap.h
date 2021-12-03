//platform agnostic file
#ifndef _STRINGMULTIMAP_H
#define _STRINGMULTIMAP_H

#include "StringMap.h"
#include "MemoryLifetimeOwner.h"

#include <map>
#include <iostream>
#include <string.h>
#include <sstream>
using namespace std;

namespace general_server {
  template<class T, class C = LessFctr> class StringMultiMap: public multimap<const char*, T, C> {
    //typedef const char *(T::*FTOSTRING)(); //can't do this because T is a pointer, not a class

  public:
    typename StringMultiMap<T,C>::iterator insert(const char *sKey, T obj);
    const char *toString(           const bool bIgnoreBlanks = false) const;
    const char *toStringKeys(       const char *sDelimeter = ", ", const bool bIgnoreBlanks = false) const;
    const char *toStringFromObjects(const bool bIgnoreBlanks = false) const;
    //const char *toStringFromObjects(FTOSTRING fToString, const bool bIgnoreBlanks = false) const;
    void elements_free_delete();
    void elements_free();
  };

  //################################################ same compilation unit functions ###################################
  template<class T, class C> typename StringMultiMap<T,C>::iterator StringMultiMap<T,C>::insert(const char *sKey, T obj) {
    typename StringMultiMap<T,C>::iterator iInserted = multimap<const char*, T, C>::insert(pair<const char*, T>(sKey, obj));
    return iInserted;
  }

  template<class T, class C> const char *StringMultiMap<T,C>::toStringKeys(const char *sDelimeter, const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMultiMap::const_iterator iItem;
    string st;
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      if (!bIgnoreBlanks || iItem->first) {
        if (iItem != StringMultiMap::begin()) st += sDelimeter;

        //first is always a const char*
        if      (!iItem->first)  st += "<0x0>";
        else if (!*iItem->first) st += "<blank string>";
        else                     st += iItem->first;
      }
    }
    return MM_STRDUP(st.c_str());
  }

  template<class T, class C> const char *StringMultiMap<T,C>::toString(const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMultiMap::const_iterator iItem;
    stringstream st;
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      if (!bIgnoreBlanks || iItem->first) {
        //first is always a const char*
        if      (!iItem->first)  st << "<0x0>";
        else if (!*iItem->first) st << "<blank string>";
        else                     st << iItem->first;

        st << " = ";

        //using explicit conversions
        if (iItem->second) st << (T) iItem->second;
        else               st << "<0x0>";
        st << "\n";
      }
    }
    return MM_STRDUP(st.str().c_str());
  }

  template<class T, class C> const char *StringMultiMap<T,C>::toStringFromObjects(const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMultiMap::const_iterator iItem;
    string st;
    const char *sDesc;
    
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      if (!bIgnoreBlanks || iItem->first) {
        //first is always a const char*
        if      (!iItem->first)  st += "<0x0>";
        else if (!*iItem->first) st += "<blank string>";
        else                     st += iItem->first;

        st += " = ";

        //explicit object conversion
        if (iItem->second) {
          if (sDesc = iItem->second->toString()) {
            if (*sDesc) st += sDesc;
            else        st += "<blank description>";
            free((void*) sDesc);
          } else st += "<0x0 description>";
        } else st += "<0x0 pointer>";
        st += "\n" ;
      }
    }
    return MM_STRDUP(st.c_str());
  }

  template<class T, class C> void StringMultiMap<T,C>::elements_free_delete() {
    typename StringMultiMap::const_iterator iItem;
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      MMO_FREE(iItem->first);
      delete iItem->second;
    }
    erase(StringMultiMap::begin(), StringMultiMap::end());
  }

  template<class T, class C> void StringMultiMap<T,C>::elements_free() {
    typename StringMultiMap::const_iterator iItem;
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      MMO_FREE(iItem->first);
    }
    erase(StringMultiMap::begin(), StringMultiMap::end());
  }

  /*
  template<class T, class C> const char *StringMultiMap<T,C>::toStringFromObjects(FTOSTRING fToString, const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMultiMap::const_iterator iItem;
    string st;
    const char *sDesc;
    for (iItem = StringMultiMap::begin(); iItem != StringMultiMap::end(); iItem++) {
      if (!bIgnoreBlanks || iItem->first) {
        //first is always a const char*
        if      (!iItem->first)  st += "<0x0>";
        else if (!*iItem->first) st += "<blank string>";
        else                     st += iItem->first;

        st += " = ";

        //explicit object conversion
        if (iItem->second) {
          if (sDesc = iItem->second->fToString()) {
            if (*sDesc) st += sDesc;
            else        st += "<blank description>";
            MM_FREE(sDesc);
          } else st += "<0x0 description>";
        } else st += "<0x0 pointer>";
        st += "\n" ;
      }
    }
    return MM_STRDUP(st.c_str());
  }
  */
}

#endif
