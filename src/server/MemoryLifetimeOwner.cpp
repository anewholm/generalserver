#include "MemoryLifetimeOwner.h"

#include "Exceptions.h"
#include "Debug.h"

#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

namespace general_server {
  pthread_mutex_t MemoryLifetimeOwner::m_mListManager    = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  MemoryLifetimeOwner *MemoryLifetimeOwner::m_pSingleton = 0;

  MemoryLifetimeOwner::MemoryLifetimeOwner(): m_pMMParent(0) {
    if (m_pSingleton) Debug::report("MemoryLifetimeOwner:double singleton attempt", rtWarning);
    else m_pSingleton = this;
  }

#ifdef WITH_MM //------------------------------------- WITH_MM
  MemoryLifetimeOwner::MemoryLifetimeOwner(const IMemoryLifetimeOwner *pMMParent,
    const IMemoryLifetimeOwner *pChangeParent1, 
    const IMemoryLifetimeOwner *pChangeParent2,
    const IMemoryLifetimeOwner *pChangeParent3
  ): 
    m_pMMParent(pMMParent) 
  {
    //protected
    //top level will use the above Server based constructor instead
    //TODO: NodeExceptionBase uses NO_OWNER and will crash here
    assert(m_pMMParent);
    m_pMMParent->addChild(this);
    
    //automatic resource grabbing during instantiation
    //e.g. MemoryLifetimeOwner(pService, pNode)
    //  to automatically take over other existing passed resources
    if (pChangeParent1) pChangeParent1->changeParent(this);
    if (pChangeParent2) pChangeParent2->changeParent(this);
    if (pChangeParent3) pChangeParent3->changeParent(this);
  }

  MemoryLifetimeOwner::MemoryLifetimeOwner(const MemoryLifetimeOwner &mlo):
    m_pMMParent(mlo.m_pMMParent)
  {
    if (m_pMMParent) m_pMMParent->addChild(this);
  }

  MemoryLifetimeOwner::~MemoryLifetimeOwner() {
    //m_pMMParent only zero for class friend (Main)
    if (m_pMMParent) m_pMMParent->removeChild(this);

    //do not leave resources
    if (m_lChildren.size() || m_vAllocations.size()) {
      Debug::report("%s", toStringMemory(0,true), rtError);
      assert(false);
    }
  }

  void MemoryLifetimeOwner::changeParent(const IMemoryLifetimeOwner *pNewMMParent) {
    assert(m_pMMParent);
    assert(pNewMMParent);
    
    m_pMMParent->removeChild(this);
    m_pMMParent = pNewMMParent;
    m_pMMParent->addChild(this);
  }

  void MemoryLifetimeOwner::addChild(const IMemoryLifetimeOwner *pChild) {
    assert(pChild);
    
    pthread_mutex_lock(&m_mListManager); {
      IFDEBUG(
        //debug check that the item is not added twice
        list<const IMemoryLifetimeOwner*>::iterator iThis;
        iThis = std::find(m_lChildren.begin(), m_lChildren.end(), pChild);
        assert(iThis == m_lChildren.end());
      );
      m_lChildren.push_back(pChild);
    } pthread_mutex_unlock(&m_mListManager);
  }

  void MemoryLifetimeOwner::removeChild(const IMemoryLifetimeOwner *pChild) {
    assert(pChild);
    
    list<const IMemoryLifetimeOwner*>::iterator iThis;
    pthread_mutex_lock(&m_mListManager); {
      iThis = std::find(m_lChildren.begin(), m_lChildren.end(), pChild);
      assert(iThis != m_lChildren.end());
      m_lChildren.erase(iThis);
    } pthread_mutex_unlock(&m_mListManager);
  }
#else
  MemoryLifetimeOwner::MemoryLifetimeOwner(const IMemoryLifetimeOwner *pMMParent,
    const IMemoryLifetimeOwner *pChangeParent1, 
    const IMemoryLifetimeOwner *pChangeParent2,
    const IMemoryLifetimeOwner *pChangeParent3
  ): m_pMMParent(pMMParent) {}
  MemoryLifetimeOwner::MemoryLifetimeOwner(const MemoryLifetimeOwner &mlo): m_pMMParent(mlo.m_pMMParent) {}
  MemoryLifetimeOwner::~MemoryLifetimeOwner() {}
  void MemoryLifetimeOwner::changeParent(const IMemoryLifetimeOwner *pNewMMParent) {}
  void MemoryLifetimeOwner::addChild(const IMemoryLifetimeOwner *pChild) {}
  void MemoryLifetimeOwner::removeChild(const IMemoryLifetimeOwner *pChild) {}
#endif
    
  void MemoryLifetimeOwner::changeParent(const IMemoryLifetimeOwner *pChild) const {
    ((MemoryLifetimeOwner*) volatile_this())->changeParent(pChild);
  }
  
  void MemoryLifetimeOwner::addChild(const IMemoryLifetimeOwner *pChild) const {
    ((MemoryLifetimeOwner*) volatile_this())->addChild(pChild);
  }
  
  void MemoryLifetimeOwner::removeChild(const IMemoryLifetimeOwner *pChild) const {
    ((MemoryLifetimeOwner*) volatile_this())->removeChild(pChild);
  }
  
  const IMemoryLifetimeOwner *MemoryLifetimeOwner::queryInterface(const IMemoryLifetimeOwner *p) const {return this;}

  unsigned int MemoryLifetimeOwner::memAllocated() const {
    vector<Allocation>::const_iterator iAllocation;
    list<const IMemoryLifetimeOwner*>::const_iterator iChild;
    unsigned int iAllocated = 0;

    for (iAllocation = m_vAllocations.begin(); iAllocation != m_vAllocations.end(); iAllocation++) {
      iAllocated += iAllocation->iAmount;
    }

    for (iChild = m_lChildren.begin(); iChild != m_lChildren.end(); iChild++) {
      iAllocated += (*iChild)->memAllocated();
    }

    return iAllocated;
  }

  double MemoryLifetimeOwner::memUsageAllocations() {
    return (m_pSingleton ? m_pSingleton->memAllocated() : 0);
  }

  double MemoryLifetimeOwner::memUsageOperatingSystem() {
    //http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
    //process_mem_usage(double &, double &) - takes two doubles by reference,
    //attempts to read the system-dependent data for a process' virtual memory
    //size and resident set size, and return the results in KB.
    //On failure, returns 0.0, 0.0
    double vm_usage, resident_set;
    using std::ios_base;
    using std::ifstream;
    using std::string;

    vm_usage     = 0.0;
    resident_set = 0.0;

    //'file' stat seems to give the most reliable results
    ifstream stat_stream("/proc/self/stat",ios_base::in);

    //dummy vars for leading entries in stat that we don't care about
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;

    //the two fields we want
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                >> utime >> stime >> cutime >> cstime >> priority >> nice
                >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage     = vsize / 1024.0;
    resident_set = rss * page_size_kb;

    return vm_usage + resident_set;
  }

  const IMemoryLifetimeOwner *MemoryLifetimeOwner::mmParent() const {return m_pMMParent;}
  
  void *MemoryLifetimeOwner::mmo_malloc(size_t iAmount, const char *sReason) const {
    MemoryLifetimeOwner *pNonConstThis = (MemoryLifetimeOwner*) volatile_this();
    return pNonConstThis->mmo_malloc(iAmount, sReason);
  }
  
  void *MemoryLifetimeOwner::mmo_malloc(size_t iAmount, const char *sReason) {
    void *pBlock = malloc(iAmount);
    return mmo_own(pBlock, iAmount, sReason);
  }

  void  MemoryLifetimeOwner::mmo_free(const void *pBlock) const {
    MemoryLifetimeOwner *pNonConstThis = (MemoryLifetimeOwner*) volatile_this();
    pNonConstThis->mmo_free(pBlock);
  }
  
  void  MemoryLifetimeOwner::mmo_free(const void *pBlock) {
    vector<Allocation>::iterator iAllocation;
    bool bFound = false;
    iAllocation = m_vAllocations.begin();
    
    while (!bFound && iAllocation != m_vAllocations.end()) {
      if ((*iAllocation).pBlock == pBlock) {
        m_vAllocations.erase(iAllocation);
        free((void*) pBlock);
        bFound = true;
      } else iAllocation++;
    }
    if (!bFound) throw MemoryBlockNotFound(this, MM_STRDUP(""));
  }

  void *MemoryLifetimeOwner::mmo_own(void *pBlock, const size_t iAmount, const char *sReason) const {
    MemoryLifetimeOwner *pNonConstThis = (MemoryLifetimeOwner*) volatile_this();
    return pNonConstThis->mmo_own(pBlock, iAmount, sReason);
  }
  
  void *MemoryLifetimeOwner::mmo_own(void *pBlock, const size_t iAmount, const char *sReason) {
    if (!pBlock) throw OutOfMemory(this, sReason);
    Allocation a = {pBlock, iAmount, allocationtype_malloc, sReason};
    m_vAllocations.push_back(a);
    return pBlock;
  }

  void *MemoryLifetimeOwner::mmo_release(void *pBlock) {
    //PRIVATE
    vector<Allocation>::iterator iAllocation;
    bool bFound = false;
    iAllocation = m_vAllocations.begin();
    
    while (!bFound && iAllocation != m_vAllocations.end()) {
      if ((*iAllocation).pBlock == pBlock) {
        m_vAllocations.erase(iAllocation);
        bFound = true;
      } else iAllocation++;
    }
    if (!bFound) throw MemoryBlockNotFound(this, MM_STRDUP(""));
    
    return pBlock;
  }
  
  void *MemoryLifetimeOwner::mmo_transfer(const IMemoryLifetimeOwner *pNewMM, void *pBlock) {
    assert(pBlock);
    //may throw MemoryBlockNotFound;
    if (mmo_release(pBlock)) pNewMM->mmo_own(pBlock);
    return pBlock;
  }

  void *MemoryLifetimeOwner::mm_malloc(size_t iAmount, const char *sReason) {
    return (m_pSingleton ? m_pSingleton->mmo_malloc(iAmount, sReason) : malloc(iAmount));
  }
  
  void  MemoryLifetimeOwner::mm_free(const void *pBlock) {
    (m_pSingleton ? m_pSingleton->mmo_free(pBlock) : free((void*) pBlock));
  }

  const char *MemoryLifetimeOwner::toStringMemory(size_t iLevel, const bool bInDestructor) const {
    stringstream sOut;
    vector<Allocation>::const_iterator iAllocation;
    list<const IMemoryLifetimeOwner*>::const_iterator iChild;

    sOut << string(iLevel, '-') << " [" << (bInDestructor ? "~destructor" : toString()) << "]\n";
    if (m_vAllocations.size()) {
      sOut << "Allocations (" << m_vAllocations.size() << "):\n";
      for (iAllocation = m_vAllocations.begin(); iAllocation != m_vAllocations.end(); iAllocation++) {
        sOut << "  " << iAllocation->sReason << "\n";
      }
    } else sOut << "Allocations: (none)\n";

    if (m_lChildren.size()) {
      sOut << "MemoryLifetimeOwner children (" << m_lChildren.size() << "):\n";
      for (iChild = m_lChildren.begin(); iChild != m_lChildren.end(); iChild++) {
        sOut << (*iChild)->toStringMemory(iLevel + 1);
      }
    } else sOut << "MemoryLifetimeOwner children: (none)\n";

    return MM_STRDUP(sOut.str().c_str());
  }
}
