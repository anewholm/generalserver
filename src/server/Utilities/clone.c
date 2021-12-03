#ifndef _CLONE_C
#define _CLONE_C

#include "MemoryLifetimeOwner.h"

template<class T> T *base_object_clone(const T *pOrig) {
  size_t iSize = sizeof(*pOrig);
  T *pNew = (T*) MM_MALLOC_FOR_RETURN(iSize);
  memcpy(pNew, pOrig, iSize);
  return pNew;
}

#endif
