//platform agnostic file
#include "Thread.h"

#include "Exceptions.h"
#include "Debug.h"

using namespace std;

namespace general_server {
  map<pthread_t, Thread*> Thread::m_mThreads; //map of current threads for self()

  ThreadCaller::ThreadCaller(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner)
  {}

  Thread::Thread(ThreadCaller *tc, const char *sName, const bool bAutoDelete):
    MemoryLifetimeOwner(tc),
    ThreadCaller(tc),
    m_threadCaller(tc),
    m_bContinue(true),
    m_bAutoDeleteOnExit(bAutoDelete),
    m_tThread(0),
    m_sName(_STRDDUP(sName, "thread")),
    m_iCount(0)
  {}

  Thread::~Thread() {
    if (m_tThread) Debug::report("Thread class [%s] destroyed when threadProcess() still running", (m_sName ? m_sName : "thread"));
    if (m_sName) MMO_FREE(m_sName);
  }

  void ThreadCaller::childThreadFinished(Thread *t) {}

  Thread *Thread::self() {
    Thread *t = 0;
    map<pthread_t, Thread*>::iterator iThread;
    iThread = m_mThreads.find(pthread_self());
    if (iThread != m_mThreads.end()) t = iThread->second;
    return t;
  }

  void Thread::threadInit() {}    //e.g. LibXML2 thread initialisation calls, esxltRegistration, etc.
  void Thread::threadCleanup() {}

  Thread *Thread::runThreaded() {
#ifdef HAVE_LIBPTHREAD
    //branch
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //allow resource reclaim at exit
    pthread_create(&m_tThread, &attr, (void* (*)(void*)) Thread::static_runThreaded, this);
    pthread_attr_destroy(&attr);
#else
    //run in current thread
    runInCurrentThread();
#endif

    return this;
  }

  void Thread::runInCurrentThread() {
    //force single threaded running of this object
    threadProcess();
  }

  bool Thread::isContinue()   const {return m_bContinue;}
  bool Thread::isRunning()    const {return m_tThread;}
  bool Thread::isAutoDelete() const {return m_bAutoDeleteOnExit;}

  void Thread::static_runThreaded(Thread *thread) {
    //now in a separate thread
    thread->m_tThread = pthread_self();
    //cout << thread->toString() << "\n";
    m_mThreads.insert(_PAIR_THREAD(thread->m_tThread, thread)); //add to global thread list

    //inform the parent thread also that the child has finished
    pthread_cleanup_push((void (*)(void*)) Thread::static_cleanup, thread);
    //sister of threadCleanup() in the Thread::static_cleanup()
    //does e.g. LibXML2 thread initialisation calls, esxltRegistration, etc.
    thread->threadInit();
    //main running of the object in threaded mode
    thread->threadProcess();
    //explicitly pop and execute static_cleanup()
    //this cleanup may delete thread (isAutoDelete)
    pthread_cleanup_pop(PTHREAD_EXECUTE_CLEANUP);
  }

  void Thread::static_cleanup(Thread *thread) {
    const bool bAutoDelete = thread->isAutoDelete();
    thread->threadCleanup(); //thread specific cleanup work
    //TODO: remove me from the m_mThreads (not so important as they will be re-used
    thread->m_tThread = 0;
    //inform the parent thread that all child threads are complete and destroyed
    if (thread->threadCaller()) thread->threadCaller()->childThreadFinished(thread);
    if (bAutoDelete) delete thread;
  }

  void Thread::blockingCancel() {
    m_bContinue = false;
  }

  void Thread::immediateKillObjectThread()  {
    assert(m_tThread);
    //copy member variable on to the stack in case static_cleanup() deletes this
    pthread_t tThread = m_tThread;
    m_tThread = 0;
    pthread_cancel(tThread); //pthread_cleanup_pop() ARE processed
    //NOTE: this may have been deleted here (isAutoDelete): so SIGSEV danger
  }

  void Thread::immediateKillThreadSelf() {
    Thread *pThread = Thread::self();
    assert(pThread);
    pThread->immediateKillObjectThread();
    //NOTE: this may have been deleted here (isAutoDelete): so SIGSEV danger
  }

  void Thread::allowAsynchronousKill() {
    int iOldValue;
    //ensure that the main ProfilerThread can kill this one instantly
    //no resources should be aquired
    //http://man7.org/linux/man-pages/man3/pthread_setcanceltype.3.html
    //cancellation points for deferred state: http://man7.org/linux/man-pages/man7/pthreads.7.html
    //includes sleep()
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,       &iOldValue);
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, &iOldValue);
  }

  bool Thread::incrementUntil(unsigned int iMax) {
    //NOTE: this only works on threaded objects
    //Main does not have a thread so will return false always
    Thread *t = self();
    return (!t || t->m_iCount++ >= iMax);
  }

  ostream& Thread::operator<<(ostream& out) {
    const char *s = toString();
    out << s;
    MM_FREE(s);
    return out;
  }

  const char *Thread::threadingModel() const {
    //result is a constant
    #if defined(HAVE_LIBPTHREAD) && !defined(WITHOUT_THREADING)
      return "multi-THREADED";
    #else
      return "single-THREADED";
    #endif
  }

  const char *Thread::toString() const {
    //caller free
    size_t iLen = 12 + strlen(name()) + 1 + 20 + 1;
    char *s = MM_MALLOC_FOR_RETURN(iLen + 1);
    _SNPRINTF5(s, iLen+1, "%s%s%s%u%s", "New Thread [", name(), ":", id(), "]");
    return s;
  }

  Service      *Thread::queryInterface(Service *p) {throw InterfaceNotSupported(this, MM_STRDUP("Service *")); return 0;}
  Conversation *Thread::queryInterface(Conversation *p) {throw InterfaceNotSupported(this, MM_STRDUP("Conversation *")); return 0;}
  /*
  void *Thread::threadLocalStorageCreate(const int i, void *p) {
    Thread *thread = Thread::self();
    if (thread) thread->localStorageCreate(i, p);
    return p;
  }
  void *Thread::threadLocalStorageRetrieve(const int i) {
    Thread *thread = Thread::self();
    return thread ? thread->localStorageRetrieve(i) : 0;
  }
  void *Thread::localStorageCreate(const int i, void *p) {
    this->m_threadLocalStorage[i] = p;
    return p;
  }
  void *Thread::localStorageRetrieve(const int i) const {
    return this ? this->m_threadLocalStorage.at(i) : 0;
  }
  */
}

