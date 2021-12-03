//platform agnostic file
#ifndef _COMPILATIONCONTEXT_H
#define _COMPILATIONCONTEXT_H

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
  class CompilationContext: public IXmlTriggerContext {
    const IXmlQueryEnvironment *m_pOwnerQE;
    IXslDoc *m_pMainXSLDoc;

    //only the Database is allowed to *create* a TriggerContext for now
    friend class CompilationEnvironment;
    CompilationContext(IXslDoc *pMainXSLDoc);
    CompilationContext(const CompilationContext& trig);
    CompilationContext *clone_with_resources() const;
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}
    IXmlBaseDoc *singleWriteableDocumentContext() const;

  public:
    const char   *toString() const;
    const char   *type() const;
    IXmlTriggerContext *inherit() const {return 0;}

    //trigger control
    bool enabled() const {return true;}
    bool enable()        {return true;}
    bool disable()       {assert(false); return true;}
    
    bool cyclicTrigger(IXmlBaseNode *pCur ATTRIBUTE_UNUSED) const  {return false;}
    void addRunningTrigger(IXmlBaseNode *pCur ATTRIBUTE_UNUSED)    {}
    bool removeRunningTrigger(IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {return false;}

    IXmlQueryEnvironment::accessResult beforeReadTriggerCallback( const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    IXmlQueryEnvironment::accessResult afterReadTriggerCallback(  const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur);
    
    IXmlQueryEnvironment::accessResult beforeWriteTriggerCallback(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {return IXmlQueryEnvironment::RESULT_INCLUDE;}
    IXmlQueryEnvironment::accessResult afterWriteTriggerCallback( const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {return IXmlQueryEnvironment::RESULT_INCLUDE;}
    IXmlQueryEnvironment::accessResult beforeAddTriggerCallback(  const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {return IXmlQueryEnvironment::RESULT_INCLUDE;}
    IXmlQueryEnvironment::accessResult afterAddTriggerCallback(   const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {return IXmlQueryEnvironment::RESULT_INCLUDE;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
