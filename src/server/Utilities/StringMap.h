//platform agnostic file
#ifndef _STRINGMAP_H
#define _STRINGMAP_H

#include "define.h"
#include "MemoryLifetimeOwner.h"

#include <map>
#include <iostream>
#include <string.h>
#include <sstream>
using namespace std;

namespace general_server {
  //################################################ classes ###################################
  //with clone_with_resources
  class LessFctr {
    //the default strcmp class for ordering and finding keys
    public:
    bool operator () (const char* s1, const char* s2) const {return strcmp(s1, s2) < 0;}
  };

  template<class T = const char*, class C = LessFctr> class StringMap: public map<const char*, T, C> {
    //typedef const char *(T::*FTOSTRING)(); //can't do this because T is a pointer, not a class

  public:
    StringMap<T,C>() {}
    //direct copy constructor ONLY for T->clone_with_resources() copy constructor
    StringMap<T,C>(const StringMap<T,C> &sm); 
    StringMap<T,C> *clone_with_resources() const;
    //StringMap(begin, end, <T>) for non T->clone_with_resources()
    StringMap<T,C>(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const bool  bCloneWithResources) {insert(iBegin, iEnd, bCloneWithResources);}
    StringMap<T,C>(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const int   bCloneWithResources) {insert(iBegin, iEnd, bCloneWithResources);}
    StringMap<T,C>(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const char *bCloneWithResources) {insert(iBegin, iEnd, bCloneWithResources);}
    
    //using function overloading INSTEAD of class and member template specialisation here
    //call insert(map, "true") to invoke the const char* version
    typename StringMap<T,C>::iterator insert(const char *sKey, T obj);
    void insert(const StringMap<T,C> *pmMap);
    void insert(const StringMap<T,C> *pmMap, const bool bCloneWithResources);
    void insert(const StringMap<T,C> *pmMap, const int  bCloneWithResources);
    void insert(const StringMap<T,C> *pmMap, const char *bCloneWithResources);
    void insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd);
    void insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const bool  bCloneWithResources);
    void insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const int   bCloneWithResources);
    void insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const char *bCloneWithResources);
    
    T& at(const char *sKey);
    const T& at(const char *sKey) const;
    
    const char *toString(           const bool bIgnoreBlanks = false) const;
    const char *toStringKeys(       const char *sDelimeter = ", ", const bool bIgnoreBlanks = false) const;
    const char *toStringFromObjects(const bool bIgnoreBlanks = false) const;
    //const char *toStringFromObjects(FTOSTRING fToString, const bool bIgnoreBlanks = false) const;
    StringMap<T,C> *elements_free_delete();
    StringMap<T,C> *key_free();
    StringMap<T,C> *elements_free();
  };
  
  //################################################ same compilation unit functions ###################################
  template<class T, class C> StringMap<T,C>::StringMap(const StringMap<T,C> &sm):
    std::map<const char*, T, C>(sm)
  {
    typename StringMap<T,C>::const_iterator iItem;
    
    for (iItem = sm.begin(); iItem != sm.end(); iItem++)
      insert(
        MM_STRDUP(iItem->first), 
        (iItem->second ? iItem->second->clone_with_resources() : 0)
      );
  }
  
  template<class T, class C> T& StringMap<T,C>::at(const char *sKey) {
      
  }
  
  template<class T, class C> const T& StringMap<T,C>::at(const char *sKey) const {

  }

  template<class T, class C> StringMap<T,C> *StringMap<T,C>::clone_with_resources() const {
    typename StringMap<T,C>::const_iterator iItem;
    StringMap<T,C> *pClone = new StringMap<T,C>();
    
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) pClone->insert(
      MM_STRDUP(iItem->first), 
      (iItem->second ? iItem->second->clone_with_resources() : 0)
    );
    
    return pClone;
  }
  
  template<class T, class C> typename StringMap<T,C>::iterator StringMap<T,C>::insert(const char *sKey, T obj) {
    pair<typename StringMap<T,C>::iterator, bool> pInserted = map<const char*, T, C>::insert(pair<const char*, T>(sKey, obj));
    return pInserted.first;
  }

  template<class T, class C> void StringMap<T,C>::insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd) {
    typename StringMap<T,C>::const_iterator iItem;
    for (iItem = iBegin; iItem != iEnd; iItem++) insert(iItem->first, iItem->second);
  }

  template<class T, class C> void StringMap<T,C>::insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const bool bCloneWithResources) {
    typename StringMap<T,C>::const_iterator iItem;
    for (iItem = iBegin; iItem != iEnd; iItem++) insert(MM_STRDUP(iItem->first), iItem->second->clone_with_resources());
  }

  template<class T, class C> void StringMap<T,C>::insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const int  bCloneWithResources FAKE_OVERLOAD) {
    typename StringMap<T,C>::const_iterator iItem;
    for (iItem = iBegin; iItem != iEnd; iItem++) insert(MM_STRDUP(iItem->first), iItem->second);
  }

  template<class T, class C> void StringMap<T,C>::insert(typename StringMap<T,C>::const_iterator iBegin, typename StringMap<T,C>::const_iterator iEnd, const char *sCloneWithResources FAKE_OVERLOAD) {
    typename StringMap<T,C>::const_iterator iItem;
    for (iItem = iBegin; iItem != iEnd; iItem++) insert(MM_STRDUP(iItem->first), MM_STRDUP(iItem->second));
  }

  template<class T, class C> void StringMap<T,C>::insert(const StringMap<T,C> *pmMap) {
    if (pmMap) insert(pmMap->begin(), pmMap->end());
  }

  template<class T, class C> void StringMap<T,C>::insert(const StringMap<T,C> *pmMap, const bool bCloneWithResources) {
    if (pmMap) insert(pmMap->begin(), pmMap->end(), bCloneWithResources);
  }

  template<class T, class C> void StringMap<T,C>::insert(const StringMap<T,C> *pmMap, const char *sCloneWithResources) {
    if (pmMap) insert(pmMap->begin(), pmMap->end(), sCloneWithResources);
  }

  template<class T, class C> const char *StringMap<T,C>::toStringKeys(const char *sDelimeter, const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMap::const_iterator iItem;
    string st;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
      if (!bIgnoreBlanks || iItem->first) {
        if (iItem != StringMap::begin()) st += sDelimeter;

        //first is always a const char*
        if      (!iItem->first)  st += "<0x0>";
        else if (!*iItem->first) st += "<blank string>";
        else                     st += iItem->first;
      }
    }
    return MM_STRDUP(st.c_str());
  }

  template<class T, class C> const char *StringMap<T,C>::toString(const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMap::const_iterator iItem;
    stringstream st;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
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

  template<class T, class C> const char *StringMap<T,C>::toStringFromObjects(const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMap::const_iterator iItem;
    string st;
    const char *sDesc;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
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
            free((void*)sDesc);
          } else st += "<0x0 description>";
        } else st += "<0x0 pointer>";
        st += "\n" ;
      }
    }
    return MM_STRDUP(st.c_str());
  }

  template<class T, class C> StringMap<T,C> *StringMap<T,C>::elements_free_delete() {
    typename StringMap::const_iterator iItem;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
      if (iItem->first)  free((void*)iItem->first);
      if (iItem->second) delete iItem->second;
    }
    this->erase(StringMap::begin(), StringMap::end());
    return this;
  }

  template<class T, class C> StringMap<T,C> *StringMap<T,C>::key_free() {
    typename StringMap::const_iterator iItem;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
      if (iItem->first) free((void*)iItem->first);
    }
    this->erase(StringMap::begin(), StringMap::end());
    return this;
  }

  template<class T, class C> StringMap<T,C> *StringMap<T,C>::elements_free() {
    typename StringMap::const_iterator iItem;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
      if (iItem->first)  free((void*)iItem->first);
      if (iItem->second) free((void*)iItem->second);
    }
    this->erase(StringMap::begin(), StringMap::end());
    return this;
  }

  /*
  template<class T, class C> const char *StringMap<T>::toStringFromObjects(FTOSTRING fToString, const bool bIgnoreBlanks) const {
    //caller frees return
    //NOTE: always returns a malloc string, even if only ""
    typename StringMap::const_iterator iItem;
    string st;
    const char *sDesc;
    for (iItem = StringMap::begin(); iItem != StringMap::end(); iItem++) {
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
            free((void*)sDesc);
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
