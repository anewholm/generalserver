//platform agnostic file
#ifndef _DATABASEADMINISTRATIONQUERYENVIRONMENT_H
#define _DATABASEADMINISTRATIONQUERYENVIRONMENT_H

#include "IXml/IXmlQueryEnvironment.h" //direct inheritance
#include "IXml/IXmlBaseDoc.h"          //usage in header file in-line functions
#include "IXml/IXmlLibrary.h"          //usage in header file in-line functions
#include "AdminSecurityContext.h" //has-a
#include "Exceptions.h"

namespace general_server {
  interface_class IXslModuleManager;
  interface_class IXslTransformContext;
  interface_class IXslXPathFunctionContext;
  interface_class IXmlBaseNode;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //Database AdminQueryEnvironment which has full capability
  //including:
  //  EMO (Database)
  //  root security
  //  volatile IXmlBaseDoc pointer
  //only the Database may instanciate it, only the Database can access it's non-const doc
  //see Database::newQueryEnvironment() for general use User Environments
  //and XmlAdminQueryEnvironment class for low level query setup
  class DatabaseAdminQueryEnvironment: virtual public MemoryLifetimeOwner, implements_interface IXmlQueryEnvironment {
    friend class Database;
    const IXmlBaseDoc    *m_pSingularDocumentContext;
    IXslModuleManager    *m_pEMO;
    IXmlSecurityContext  *m_pSec;
    IXslTransformContext *m_pTransformContext;
    IXmlGrammarContext   *m_pGrammar;       //LibXML2: ctxt->gp       ->param = pQueryEnvironment
    IXmlXPathProcessingContext *m_pXPathProcessingContext;
    XmlNodeList<const IXmlBaseNode> *m_pNodes; //external pointer because it is non-const

    DatabaseAdminQueryEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseDoc *pSingularDocumentContext, IXslModuleManager *pEMO, IXslTransformContext *pTransformContext = 0);
    ~DatabaseAdminQueryEnvironment();

  public:
    //--------------------------------------- inheritance
    IXmlQueryEnvironment *inherit(IXslDoc *pNewStylesheet = 0) const;
    IXmlQueryEnvironment *inherit(IXslModuleManager *pNewEMO)  const;
    bool inherited() const; //is this an inherited environment
    const char *toString() const;
    void addNode(const IXmlBaseNode *pNode) const {m_pNodes->push_back(pNode);}
    XmlNodeList<const IXmlBaseNode> *nodes() const {return m_pNodes;}
    void reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes) const;

    //--------------------------------------- component access
    IXmlSecurityContext  *securityContext()  const; //required security context
    IXmlTriggerContext   *triggerContext()   const;
    IXmlMaskContext      *maskContext()      const;
    IXmlGrammarContext *grammarContext()   const;
    IXmlXPathProcessingContext *xpathProcessingContext() const;
    IXslModuleManager    *emoContext()       const;
    IXmlDebugContext     *debugContext()     const;
    IXmlDebugContext     *debugContext(IXmlDebugContext *pDebug);
    IXslTransformContext *transformContext() const;
    IXslXPathFunctionContext *newXPathContext(const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const;

    //--------------------------------------- node-masks
    void push_back(const IXmlArea *pArea);
    void pop_back();
    int applyCurrentAreaMask(const IXmlBaseNode *pNode) const;
    void disable();
    void enable();

    //--------------------------------------- generic environment uses (conveinience)
    const IXmlLibrary *xmlLibrary()      const;

    //--------------------------------------- pre-execution requirements
    const IXmlBaseDoc *singularDocumentContext() const;
    bool               isMultiDocumentEnvironment() const {return false;}
    IXmlProfiler  *profiler()    const;

    //--------------------------------------- debug
    void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const;
    void clearXSLTTraceFlags() const;
    void setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const;
    void clearXMLTraceFlags() const;
    const char *currentParentRoute() const;
    const char *currentXSLCommandXPath() const;
    const char *currentSourceNodeXPath() const;
  };
}

#endif
