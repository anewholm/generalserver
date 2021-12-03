#include "ProfilerThread.h"

#include "Debug.h"

using namespace std;

namespace general_server {
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  ProfilerThread::ProfilerThread(ThreadCaller *tc, const char *sName, const unsigned int iTimeLimitSeconds):
    MemoryLifetimeOwner(tc),
    Thread(tc, sName), //ProfilerThread is-a Thread! NOT_OUR_BUSINESS
    m_pMonitor(0)      //created on demand
  {
    setTimeLimit(iTimeLimitSeconds);
  }

  ProfilerThread::~ProfilerThread() {
    if (m_pMonitor) m_pMonitor->profilerThreadDestructor();
  }

  void ProfilerThread::setTimeLimit(unsigned int iTimeLimitSeconds) {
    monitorActive(m_iTimeLimitSeconds = iTimeLimitSeconds);
  }

  void ProfilerThread::clearTimeLimit() {
    monitorActive(m_iTimeLimitSeconds = 0);
  }

  void ProfilerThread::setMemoryLimit(unsigned int iMemoryLimitMb) {
    monitorActive(m_iMemoryLimitMb = iMemoryLimitMb);
  }

  void ProfilerThread::monitorActive(const bool bOn) {
    if (bOn) {
      //normally a pthread_cancel() is deferred until a cancellation point e.g. sleep()
      //here, we permit immediate asynchronous death
      allowAsynchronousKill();
      if (!m_pMonitor) m_pMonitor = new ProfileMonitor(this, name()); //AUTO_DELETE
      //if (!m_pMonitor->isRunning()) m_pMonitor->runThreaded();
      else Debug::report("monitor already running");
    } else {
      if (m_pMonitor) {
        m_pMonitor->blockingCancel(); //auto-delete self
      }
    }
  }

  bool ProfilerThread::exceeded() {
    //make this virtual if overriding is ever needed
    Debug::report("profile limit(s) exceeded [%s] killing thread...", name());
    immediateKillObjectThread(); //kills this::m_tThread, not pthread_self()
    return true;     //true = stop the ProfileMonitor
  }

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  ProfileMonitor::ProfileMonitor(ProfilerThread *pProfilerThread, const char *sName):
    MemoryLifetimeOwner(pProfilerThread),
    Thread(pProfilerThread, (sName ? sName : "profile monitor"), AUTO_DELETE), //sName is MM_STRDUP in Thread
    m_pProfilerThread(pProfilerThread)
  {}

  void ProfileMonitor::threadProcess() {
    unsigned int iStartTime = time(0);
    unsigned int iExecutionTime;

    //loop until stop request or pthread_cancel()
    while (m_pProfilerThread && isContinue()) {
      iExecutionTime = time(0) - iStartTime;
      if (m_pProfilerThread->m_iTimeLimitSeconds && iExecutionTime >= m_pProfilerThread->m_iTimeLimitSeconds) {
        if (m_pProfilerThread->exceeded()) blockingCancel();
      } else {
        //Debug::report("[%s]", iExecutionTime);
        sleep(1); //this is a valid cancellation point!
      }
    }

    //will auto-delete self (this) on exit
  }

  void ProfileMonitor::profilerThreadDestructor() {
    m_pProfilerThread = 0;
    blockingCancel();
  }
}
