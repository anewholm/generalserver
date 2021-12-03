#ifndef _GDB_C
#define _GDB_C

#include "Utilities/container.c"
using namespace std;

namespace general_server {
  IFDEBUG(
    const char *pv(StringVector *pvVector) {
      return container_contents_single(pvVector);
    }
    const char *pm(map<const char*, const char*> *pmMap) {
      return container_contents_double(pmMap);
    }
  )
}

#endif
