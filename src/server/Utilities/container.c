#ifndef _CONTAINER_C
#define _CONTAINER_C

#include "Utilities/StringVector.h"

//----------------------------------------------------------------------------------------------------
//-------------------------------------- output ------------------------------------------------------
//----------------------------------------------------------------------------------------------------
template<class T> const char *container_contents_single(T *pVec, const char *sSpacer = 0, const char *sSplitter = 0) {
  typename T::const_iterator iVec;
  stringstream sOut;
  for (iVec = pVec->begin(); iVec != pVec->end(); iVec++) {
    if (iVec != pVec->begin()) sOut << (sSplitter ? sSplitter : "\n");
    sOut << (sSpacer ? sSpacer : "") << *iVec;
  }
  return sOut.str().c_str();
}

template<class T> const char *container_contents_double(T *pVec, const char *sSpacer = 0, const char *sSplitter = 0) {
  typename T::const_iterator iVec;
  stringstream sOut;
  for (iVec = pVec->begin(); iVec != pVec->end(); iVec++) {
    if (iVec != pVec->begin()) sOut << (sSplitter ? sSplitter : "\n");
    sOut << (sSpacer ? sSpacer : "") << iVec->first << "=" << iVec->second;
  }
  return sOut.str().c_str();
}


//----------------------------------------------------------------------------------------------------
//-------------------------------------- destruction -------------------------------------------------
//----------------------------------------------------------------------------------------------------
template< typename T >
struct delete_pointer_element
{
    void operator()( T element ) const
    {
        delete element;
    }
};

//############################################ vector ############################################
template<class T> void vector_const_element_destroy(T *pVec) {
  typename T::const_iterator iVec;
  for (iVec = pVec->begin(); iVec != pVec->end(); iVec++) delete *iVec;
  delete pVec;
}
template<class T> void vector_element_destroy(T *pVec) {
  typename T::iterator iVec;
  for (iVec = pVec->begin(); iVec != pVec->end(); iVec++) delete *iVec;
  delete pVec;
}

template<class T> void vector_const_element_destroy(T &pVec) {
  typename T::const_iterator iVec;
  for (iVec = pVec.begin(); iVec != pVec.end(); iVec++) delete *iVec;
}
template<class T> void vector_element_destroy(T &pVec) {
  typename T::iterator iVec;
  for (iVec = pVec.begin(); iVec != pVec.end(); iVec++) delete *iVec;
}
template<class T> void vector_element_free(T &pVec) {
  typename T::iterator iVec;
  for (iVec = pVec.begin(); iVec != pVec.end(); iVec++) free((void*) *iVec);
}

//############################################ maps ############################################
template<class T> void map_element_free(T *pMap) {
  typename T::iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) free((void*) iMap->second);
  delete pMap;
}
template<class T> void map_element_free(T& mMap) {
  typename T::iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) free((void*) iMap->second);
}

template<class T> void map_key_free(T *pMap) {
  typename T::iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) free((void*) iMap->first);
  delete pMap;
}
template<class T> void map_key_free(T& mMap) {
  typename T::iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) free((void*) iMap->first);
}

template<class T> void map_element_delete(T *pMap) {
  typename T::iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) delete iMap->second;
  delete pMap;
}
template<class T> void map_element_delete(T& mMap) {
  typename T::iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) delete iMap->second;
}

template<class T> void map_elements_free(T *pMap) {
  typename T::iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) {
    free((void*) iMap->first);
    free((void*) iMap->second);
  }
  delete pMap;
}
template<class T> void map_elements_free(T& mMap) {
  typename T::iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) {
    free((void*) iMap->first);
    free((void*) iMap->second);
  }
}

template<class T> void map_elements_delete(T *pMap) {
  typename T::iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) {
    free((void*) iMap->first);
    delete iMap->second;
  }
  delete pMap;
}
template<class T> void map_elements_delete(T& mMap) {
  typename T::iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) {
    free((void*) iMap->first);
    delete iMap->second;
  }
}

template<class T> void map_const_elements_free(T *pMap) {
  typename T::const_iterator iMap;
  for (iMap = pMap->begin(); iMap != pMap->end(); iMap++) {
    free((void*) iMap->first);
    free((void*) iMap->second);
  }
  delete pMap;
}
template<class T> void map_const_elements_free(T &mMap) {
  typename T::const_iterator iMap;
  for (iMap = mMap.begin(); iMap != mMap.end(); iMap++) {
    free((void*) iMap->first);
    free((void*) iMap->second);
  }
}

#endif
