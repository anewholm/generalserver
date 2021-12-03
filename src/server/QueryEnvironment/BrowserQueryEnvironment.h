//platform agnostic file
#ifndef _BROWSERQUERYENVIRONMENT_H
#define _BROWSERQUERYENVIRONMENT_H

#include "QueryEnvironment.h" //direct inheritance
#include "IXml/IXmlBaseDoc.h"      //usage in header file in-line functions
#include "AdminSecurityContext.h"

namespace general_server {
  interface_class IXslModuleManager;
  interface_class IXslTransformContext;
  interface_class IXslXPathFunctionContext;
  interface_class IXmlBaseNode;

  //-------------------------------------------------------------------------------------------
  //------------------------ BrowserQueryEnvironment -----------------------------------
  //-------------------------------------------------------------------------------------------
  //internal admin use by:
  //  application level (TXml, Repository, ...)
  //for browser transforms emulation
  class BrowserQueryEnvironment: public QueryEnvironment {
  public:
    BrowserQueryEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlBaseDoc *pDoc): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      QueryEnvironment(
        pMemoryLifetimeOwner, 0, pDoc, 0, 
        new AdminSecurityContext(pDoc), 
        0, 0, 0, 0)
    {}
    
    BrowserQueryEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseDoc *pDoc): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      QueryEnvironment(
        pMemoryLifetimeOwner, 0, pDoc, 0, 
        new AdminSecurityContext(pDoc),   //SECURITY: no security restrictions in browser transforms!
        0, 0, 0, 0)                       //no GrammarContext, EMO, etc.
    {}
    ~BrowserQueryEnvironment() {}

    const char *toString() const {return "BrowserQueryEnvironment";}
  };
}

#endif
