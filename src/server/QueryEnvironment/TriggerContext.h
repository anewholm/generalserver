//platform agnostic file
#ifndef _TRIGGERCONTEXT_H
#define _TRIGGERCONTEXT_H

#include "define.h"
#include "IXml/IXmlTriggerContext.h" //direct inheritance
#include "Utilities/StringMap.h"
#include "Xml/XmlBaseNode.h"
#include "Xml/XmlNodeMap.h"

#include <vector>
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;
  interface_class IXmlLibrary;
  interface_class IXslModuleManager;
  interface_class IXmlQueryEnvironment;
  class Database;

  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  class TriggerContextDatabaseAttribute: public IXmlTriggerContext {
    Database *m_pDatabase;
    IXmlBaseDoc *m_pSingleWriteableDocumentContext;
    bool m_bEnabled;
    const IXmlQueryEnvironment *m_pOwnerQE;
    XmlNodeList<const IXmlBaseNode> m_vCyclicRunningTriggers;
    static XmlNodeMap<pthread_mutex_t> m_mGlobalRunningTriggers; //globally running triggers for waiting
    static pthread_mutex_t m_mGlobalRunningTriggersManagementMutex;

    //only the Database is allowed to *create* a TriggerContext for now
    friend class Database;
    TriggerContextDatabaseAttribute(Database *pDatabase, IXmlBaseDoc *pSingleWriteableDocumentContext);
    TriggerContextDatabaseAttribute(const TriggerContextDatabaseAttribute& trig);
    TriggerContextDatabaseAttribute *clone_with_resources() const;
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}
    IXmlBaseDoc *singleWriteableDocumentContext() const {return m_pSingleWriteableDocumentContext;}

    IXmlQueryEnvironment::accessResult runAttributeTrigger(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur, const char *sAttributeLocalName);
    IXmlQueryEnvironment::accessResult runAttributeTriggerTree(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur, const char *sAttributeLocalName);

  public:
    const char   *toString() const;
    const char   *type() const;
    IXmlTriggerContext *inherit() const;

    //trigger control
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}
    
    bool cyclicTrigger(IXmlBaseNode *pCur) const;
    void addRunningTrigger(IXmlBaseNode *pCur);
    bool removeRunningTrigger(IXmlBaseNode *pCur);

    IXmlQueryEnvironment::accessResult beforeReadTriggerCallback( const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult afterReadTriggerCallback(  const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult beforeWriteTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult afterWriteTriggerCallback( const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult beforeAddTriggerCallback(  const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult afterAddTriggerCallback(   const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
