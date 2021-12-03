#ifndef _MEMORYMANAGER_H
#define _MEMORYMANAGER_H

#include "define.h"

#include "IReportable.h"

#include <stdlib.h>
#include <vector>
#include <list>
using namespace std;

#define MM_S MemoryLifetimeOwner
#define NO_OWNER MemoryLifetimeOwner::m_pSingleton

#ifdef WITH_MM //------------------------------------- WITH_MM
//static general memory management
//works in const methods
#define MM_FREE(p)      MemoryLifetimeOwner::mm_free(p)
#define MM_MALLOC(n)    MemoryLifetimeOwner::mm_malloc(n)
#define MM_REALLOC(p,n) MemoryLifetimeOwner::mm_realloc(p,n)
#define MM_NEW(C, p)    MemoryLifetimeOwner::mm_new<C>(new p)
#define MM_DELETE(p)    MemoryLifetimeOwner::mm_delete(p)
#define MM_OWN(p)       NO_OWNER->mmo_own(p)
#define MM_STRDUP(s)    MM_OWN(s ? _STRDUP(s) : NULL)
#define MM_STRNDUP(s,i) MM_OWN(s ? _STRNDUP(s,i) : NULL)
#define MM_CLONE(p)     MM_OWN(p ? p->clone_with_resources() : NULL)
//object memory management
//class must inherit MemoryLifetimeOwner
//may not work in const methods
#define MMO_FREE(p)      mmo_free(p)
#define MMO_MALLOC(n)    mmo_malloc(n)
#define MMO_REALLOC(p,n) mmo_realloc(p,n)
#define MMO_NEW(C, p)    mmo_new<C>(new p)
#define MMO_DELETE(p)    mmo_delete(p)
#define MMO_OWN(s)       mmo_own(s)
#define MMO_RETURN(p)    return mmo_transfer(NO_OWNER, p)
#define MMO_STRDUP(s)    MMO_OWN(s ? _STRDUP(s) : NULL)
#define MMO_STRNDUP(s,i) MMO_OWN(s ? _STRNDUP(s,i) : NULL)
#define MMO_CLONE(p)     MMO_OWN(p ? p->clone_with_resources() : NULL)

#else       //------------------------------------- MM_OFF
//static
#define MM_FREE(p)      free((void*) p)
#define MM_MALLOC(n)    (char*) malloc(n)
#define MM_REALLOC(p,n) (char*) realloc(p,n)
#define MM_NEW(C, p)    new p
#define MM_DELETE(p)    delete p
#define MM_OWN(p) 
#define MM_STRDUP(s)    (s ? _STRDUP(s) : NULL)
#define MM_STRNDUP(s,i) (s ? _STRNDUP(s,i) : NULL)
#define MM_CLONE(p)     (p ? p->clone_with_resources() : NULL)
//object
#define MMO_FREE(p)      free((void*) p)
#define MMO_MALLOC(n)    (char*) malloc(n)
#define MMO_REALLOC(p,n) (char*) realloc(p,n)
#define MMO_NEW(C, p)    new p
#define MMO_DELETE(p)    delete p
#define MMO_OWN(p) 
#define MMO_RETURN(p)    return p
#define MMO_STRDUP(s)    (s ? _STRDUP(s) : NULL)
#define MMO_STRNDUP(s,i) (s ? _STRNDUP(s,i) : NULL)
#define MMO_CLONE(p)     (p ? p->clone_with_resources() : NULL)
#endif

#define MM_MALLOC_FOR_CALLER(i) (char*) malloc(i)
#define MM_MALLOC_FOR_RETURN(i)   (char*) malloc(i)
#define MM_STRDUP_FOR_RETURN(s)   _STRDUP(s)

namespace general_server {
  class MemoryLifetimeOwner;
  interface_class IXmlQueryEnvironment;
  interface_class IXmlBaseNode;

  interface_class IFakeConstClass {
    protected:
      virtual void* volatile_this() const = 0;
  };
  
  interface_class IMemoryLifetimeOwner {
    virtual void *mmo_release(void *pBlock) = 0;
  protected: 
    friend class MemoryLifetimeOwner;
    virtual void changeParent(const IMemoryLifetimeOwner *pNewParent) const = 0;
    virtual void addChild(    const IMemoryLifetimeOwner *pChild) const = 0;
    virtual void removeChild( const IMemoryLifetimeOwner *pChild) const = 0;
    
    virtual void *mmo_malloc(size_t iAmount, const char *sReason) const = 0;
    virtual void  mmo_free(const void *pBlock) const = 0;
    virtual void *mmo_own(void *pBlock, const size_t iAmount = 0, const char *sReason = 0) const = 0;
    
    virtual void *mmo_malloc(size_t iAmount, const char *sReason) = 0;
    virtual void  mmo_free(const void *pBlock) = 0;
    virtual void *mmo_own(void *pBlock, const size_t iAmount = 0, const char *sReason = 0) = 0;
    virtual void *mmo_transfer(const IMemoryLifetimeOwner *pNewMM, void *pBlock) = 0;
  
    virtual unsigned int  memAllocated() const = 0;
    virtual const char   *toStringMemory(size_t iLevel = 0, const bool bInDestructor = false) const = 0;

  public:
    virtual const IMemoryLifetimeOwner *queryInterface(const IMemoryLifetimeOwner *p) const = 0;
    virtual const IMemoryLifetimeOwner *mmParent() const = 0;
  };
  
  class MemoryLifetimeOwner: implements_interface IMemoryLifetimeOwner, implements_interface IFakeConstClass, implements_interface IReportable {
    //transient instances that should remove their memory allocations on destruction
    //root, child, parent relationships for MemoryLifetimeOwners
    static pthread_mutex_t m_mListManager;

    //top level EMM, without parent only allowed once: singleton
    //only Server is allowed to do this
    friend class Main;
    MemoryLifetimeOwner();
    void* volatile_this() const {return (void*) this;}

    void changeParent(const IMemoryLifetimeOwner *pNewParent);
    void addChild(    const IMemoryLifetimeOwner *pChild);
    void removeChild( const IMemoryLifetimeOwner *pChild);

    void *mmo_release(void *pBlock);
    
  public:
    static MemoryLifetimeOwner *m_pSingleton;
    
    enum allocationType {
      allocationtype_object,
      allocationtype_malloc
    };

    enum lifetimeType {
      lifetime_unspecified = 0,
      lifetime_parent
    };

    const char *toStringMemory(size_t iLevel = 0, const bool bInDestructor = false) const;

  protected:
    const IMemoryLifetimeOwner *m_pMMParent;    //zero only for the top level (Main)

    struct Allocation {
      const void *pBlock;
      size_t iAmount;
      allocationType iAllocationType;
      const char *sReason;
    };
    vector<Allocation> m_vAllocations;

  protected:
    MemoryLifetimeOwner(const IMemoryLifetimeOwner *pMMParent, 
                        const IMemoryLifetimeOwner *pChangeParent1 = NULL, 
                        const IMemoryLifetimeOwner *pChangeParent2 = NULL,
                        const IMemoryLifetimeOwner *pChangeParent3 = NULL
                       );
    MemoryLifetimeOwner(const MemoryLifetimeOwner &mlo);
    virtual ~MemoryLifetimeOwner();

    list<const IMemoryLifetimeOwner*> m_lChildren;
    void changeParent(const IMemoryLifetimeOwner *pNewParent) const;
    void addChild(    const IMemoryLifetimeOwner *pChild)     const;
    void removeChild( const IMemoryLifetimeOwner *pChild)     const;

  protected: //object
    //---------------------------------------------- instances
    template<class NewInstance> NewInstance *mmo_new(NewInstance *pNewInstance, const char *sReason = 0, lifetimeType iLifetimeType = lifetime_parent) const;
    template<class NewInstance> void mmo_delete(const IMemoryLifetimeOwner *pOldInstance) const;

    //---------------------------------------------- mallocs
    void *mmo_malloc(size_t iAmount, const char *sReason) const;
    void  mmo_free(const void *pBlock) const;
    void *mmo_own(void *pBlock, const size_t iAmount = 0, const char *sReason = 0) const;
    
    void *mmo_malloc(size_t iAmount, const char *sReason);
    void  mmo_free(const void *pBlock);
    void *mmo_own(void *pBlock, const size_t iAmount = 0, const char *sReason = 0);
    void *mmo_transfer(const IMemoryLifetimeOwner *pNewMM, void *pBlock);
  
  public: //static
    //---------------------------------------------- instances
    template<class NewInstance> static NewInstance *mm_new(NewInstance *pNewInstance, const char *sReason = 0, lifetimeType iLifetimeType = lifetime_parent);
    template<class OldInstance> static void mm_delete(const IMemoryLifetimeOwner *pOldInstance);

    //---------------------------------------------- mallocs
    static void *mm_malloc(size_t iAmount, const char *sReason);
    static void  mm_free(const void *pBlock);
    static char *mm_malloc_char(size_t iAmount, const char *sReason);
    
  public:
    static double memUsageOperatingSystem();      //independent OS memory usage report
    static double memUsageAllocations();          //new and delete allocations
    unsigned int  memAllocated() const;
    const IMemoryLifetimeOwner *mmParent() const; //parent inheritance

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}

    const IMemoryLifetimeOwner *queryInterface(const IMemoryLifetimeOwner *p) const;
  };

  //template units MUST be defined in the header file
  //--------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------
  template<class NewInstance> NewInstance *MemoryLifetimeOwner::mmo_new(NewInstance *pNewInstance, const char *sReason, lifetimeType iLifetimeType) const {
    //usage: LibXmlNode *pN = pMemManager->mmo_new(LibXmlLibrary::factory_node(...), "server node", server(), lifetime_parent);
    //note that sReason should be static const char
    //we do not STRDUP or free
    //allow to work in const methods
    Allocation a = {pNewInstance, sizeof(NewInstance), allocationtype_object, sReason};
    ((MemoryLifetimeOwner*) this)->m_vAllocations.push_back(a);
    return pNewInstance;
  }

  template<class OldInstance> void MemoryLifetimeOwner::mmo_delete(const IMemoryLifetimeOwner *pOldInstance) const {
    //allow to work in const methods
    ((MemoryLifetimeOwner*) this)->mmo_free(pOldInstance);
    delete pOldInstance;
  }

  template<class NewInstance> NewInstance *MemoryLifetimeOwner::mm_new(NewInstance *pNewInstance, const char *sReason, lifetimeType iLifetimeType) {
    if (m_pSingleton) m_pSingleton->mmo_new<NewInstance>(pNewInstance);
    return pNewInstance;
  }

  template<class OldInstance> void MemoryLifetimeOwner::mm_delete(const IMemoryLifetimeOwner *pOldInstance) {
    if (m_pSingleton) m_pSingleton->mmo_delete<OldInstance>(pOldInstance);
    delete pOldInstance;
  }
}

#endif
