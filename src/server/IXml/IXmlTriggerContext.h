//platform agnostic file
#ifndef _IXMLTRIGGERCONTEXT_H
#define _IXMLTRIGGERCONTEXT_H

#include "define.h"
#include "Utilities/StringMap.h"
#include "IXml/IXmlQueryEnvironment.h"     //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //inheritance
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  interface_class IXslTransformContext;
  interface_class IXmlQueryEnvironment;

  interface_class IXmlTriggerContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    virtual ~IXmlTriggerContext() {}

    virtual IXmlQueryEnvironment::accessResult beforeReadTriggerCallback( const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;
    virtual IXmlQueryEnvironment::accessResult afterReadTriggerCallback(  const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;
    virtual IXmlQueryEnvironment::accessResult beforeWriteTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;
    virtual IXmlQueryEnvironment::accessResult afterWriteTriggerCallback( const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;
    virtual IXmlQueryEnvironment::accessResult beforeAddTriggerCallback(  const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;
    virtual IXmlQueryEnvironment::accessResult afterAddTriggerCallback(   const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) = 0;

    virtual IXmlTriggerContext *inherit() const = 0;
    virtual IXmlBaseDoc *singleWriteableDocumentContext() const = 0;

    //cyclic behaviour and concurrency
    virtual bool cyclicTrigger(IXmlBaseNode *pCur) const = 0;
    virtual void addRunningTrigger(IXmlBaseNode *pCur) = 0;
    virtual bool removeRunningTrigger(IXmlBaseNode *pCur) = 0;
  };
}

#endif
