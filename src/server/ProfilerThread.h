#ifndef _PROFILER_H
#define _PROFILER_H

#include "define.h"

#include "IXml/IXmlProfiler.h"        //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance
#include "Thread.h"              //direct inheritance

using namespace std;

namespace general_server {
  class ProfilerThread;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class ProfileMonitor: public Thread {
    ProfilerThread *m_pProfilerThread;

    friend class ProfilerThread;
    ProfileMonitor(ProfilerThread *pProfiler, const char *sName = 0);

    void threadProcess();
    void profilerThreadDestructor();
  };


  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class ProfilerThread: public Thread, implements_interface IXmlProfiler {
    //the ProfilerThread, on a timed basis, actively checks this Thread
    //  for exceeding the limits set below
    //  and KILLS it if it exceeded()
    //it is also a MemoryLifetimeOwner and additions can be made there for monitoring
    friend class ProfileMonitor;
    unsigned int m_iTimeLimitSeconds;
    unsigned int m_iMemoryLimitMb;
    ProfileMonitor *m_pMonitor; //NOT a local variable because we want to control its destruction time carefully

    void monitorActive(const bool bOn);

  protected:
    ProfilerThread(ThreadCaller *tc, const char *sName = 0, const unsigned int iTimeLimitSeconds = 0);
    virtual ~ProfilerThread();

    void setTimeLimit(  unsigned int iTimeLimitSeconds);
    void clearTimeLimit();
    void setMemoryLimit(unsigned int iMemoryLimitMb);
    bool exceeded(); //make this virtual if overriding is ever needed
  };
}

#endif
