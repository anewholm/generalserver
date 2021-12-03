//platform agnostic file
#ifndef _IXMLPROFILER_H
#define _IXMLPROFILER_H

#include "define.h"

#include "IReportable.h"

using namespace std;

namespace general_server {
  interface_class IXmlProfiler: implements_interface IReportable {
  public:
    virtual void setTimeLimit(  unsigned int iTimeLimitSeconds) = 0;
    virtual void setMemoryLimit(unsigned int iMb) = 0;
  };
}

#endif