//platform agnostic file
#ifndef _THREAD_H
#define _THREAD_H

#include "define.h"
#include "stdafx.h"

#include <iostream>
#include "string.h"
#include <map>
#include <vector>

using namespace std;

#include "MemoryLifetimeOwner.h" //direct inheritance

//http://sourceware.org/pthreads-win32/
#include "pthread.h"
#define _PAIR_THREAD pair<pthread_t, Thread*>
#define PTHREAD_EXECUTE_CLEANUP 1
#define THREAD_NOPARENT 0

#define BLOCK true
#define NO_BLOCK false

#define AUTO_DELETE true

namespace general_server {
  class Thread;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- ThreadCaller ---------------------------------------
  //-------------------------------------------------------------------------------------------
  class ThreadCaller: virtual public MemoryLifetimeOwner {
  protected:
    ThreadCaller(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
  public:
    virtual void childThreadFinished(Thread *t);
  };

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class Service;
  class Request;
  class Conversation;
  interface_class IXmlQueryEnvironment;
  interface_class IXmlBaseNode;

  class Thread: public ThreadCaller {
    static map<pthread_t, Thread*> m_mThreads; //map of current threads for self()
    const char   *m_sName;
    ThreadCaller *m_threadCaller;
    pthread_t     m_tThread;
    //vector<void*> m_threadLocalStorage;

    //continue should be used be Derived classes for finish and thread exit behaviour
    //blockingCancel() for nice exit setting m_bContinue = false (BLOCKING)
    //immediateKillObjectThread()  for hard exit
    bool          m_bContinue;
    unsigned int  m_iCount;    //generic counter for this thread: use incrementUntil(1000)
    bool          m_bAutoDeleteOnExit;

    //access to hierarchy
    static Thread *self();
    unsigned int id() const {return m_tThread;}

    //init and cleanup
    static void static_cleanup(Thread *thread);
    static void static_runThreaded(Thread *thread);
    virtual void threadInit();
    virtual void threadCleanup();

  protected:
    Thread(ThreadCaller *tc, const char *sName = 0, const bool bAutoDelete = false);
    virtual ~Thread();

    ThreadCaller *threadCaller() const {return m_threadCaller;}
    virtual void threadProcess() = 0;
    void allowAsynchronousKill();

  public:
    const char *name() const {return m_sName;}

    //public run and stop interface
    Thread *runThreaded();          //run threadProcess() in new thread
    void runInCurrentThread();      //run in thread
    virtual void blockingCancel();  //ask thread to nicely exit please, waiting for sub-threads
    void immediateKillObjectThread(); //kill m_tThread without cleanup
    void immediateKillThreadSelf(); //kill Thread::self() without cleanup
    bool isContinue() const;        //is thread stilling accepting new work
    bool isRunning() const;         //is threadProcess() running?
    bool isAutoDelete() const;
    static bool incrementUntil(unsigned int iMax); //keep counting until iMax

    //information
    ostream& operator<<(ostream& out);
    const char *toString() const;
    const char *threadingModel() const;

    //----------------------------------------------------- thread storage
    //superceeded by threadInit()
    /*
    void *localStorageCreate(const int i, void *p);
    void *localStorageRetrieve(const int i) const;
    static void *threadLocalStorageCreate(const int i, void *p);
    static void *threadLocalStorageRetrieve(const int i);
    */

    //----------------------------------------------------- interface navigation
    //concrete classes MUST implement relevant queries to return the correct vtable pointer
    //leaf Interfaces may REQUIRE this with pure virtual, e.g. IXslStylesheetNode
    //this base one will return a fail on any Derived class that hasnt provided a local context implicit casting
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual Service      *queryInterface(Service *p);
    virtual Conversation *queryInterface(Conversation *p);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
