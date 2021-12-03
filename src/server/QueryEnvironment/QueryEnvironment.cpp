//platform agnostic file
#include "QueryEnvironment.h"

//full definitions of the ahead declerations in Database.h
#include "IXml/IXmlSecurityContext.h"
#include "IXml/IXmlProfiler.h"
#include "IXml/IXmlDebugContext.h"
#include "IXml/IXmlTriggerContext.h"
#include "IXml/IXmlMaskContext.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlNode.h"
#include "IXml/IXmlArea.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "IXml/IXmlLibrary.h"
#include "IXml/IXslModule.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"

#include "Utilities/container.c"

using namespace std;

namespace general_server {
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  QueryEnvironment::QueryEnvironment(
    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
    IXmlProfiler         *pProfiler, 
    const IXmlBaseDoc    *pSingularDocumentContext, 
    
    IXslModuleManager    *pEMO, 
    IXmlSecurityContext  *pSec, 
    IXmlTriggerContext   *pTrigger, 
    IXmlMaskContext      *pMask, 
    IXmlDebugContext     *pDebug, 
    IXmlGrammarContext   *pGrammar,
    
    IXslTransformContext *pTransformContext, 
    
    bool bInherited
  ):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pSingularDocumentContext(pSingularDocumentContext),

    //components
    m_pProfiler(pProfiler),
    m_pEMO(pEMO),
    m_pSec(pSec),
    m_pTrigger(pTrigger),
    m_pMask(pMask),
    m_pDebug(pDebug),
    m_pTransformContext(pTransformContext),
    
    m_pGrammar(pGrammar),
    m_pXPathProcessingContext(new XPathProcessingContext()),

    m_bSecOurs(false),
    m_bInherited(bInherited)
  {
    //REQUIRED components
    //assert(m_pProfiler);
    assert(m_pSingularDocumentContext);
    assert(m_pSec);

    //component alignment (point back to this)
    //TODO: use the m_vComponents elsewhere: inheritance, toString, etc.
    //this also makes sure, at compile time, that these components have correctly supported the interfaces
    if (m_pSec)              m_vComponents.push_back(m_pSec);
    if (m_pTrigger)          m_vComponents.push_back(m_pTrigger);
    if (m_pMask)             m_vComponents.push_back(m_pMask);
    if (m_pGrammar)          m_vComponents.push_back(m_pGrammar);
    if (m_pXPathProcessingContext) m_vComponents.push_back(m_pXPathProcessingContext);
    if (m_pTransformContext) m_vComponents.push_back(m_pTransformContext);
    if (m_pDebug)            m_vComponents.push_back(m_pDebug);

    for (vector<IXmlQueryEnvironmentComponent*>::iterator iComponent = m_vComponents.begin(); iComponent != m_vComponents.end(); iComponent++)
      (*iComponent)->setQueryEnvironment(this);

    m_pNodes = new XmlNodeList<const IXmlBaseNode>(this);
  }

  QueryEnvironment::~QueryEnvironment() {
    if (m_pSec && m_bSecOurs)      delete m_pSec;
    if (m_pTrigger)                delete m_pTrigger;
    if (m_pMask)                   delete m_pMask;
    if (m_pXPathProcessingContext) delete m_pXPathProcessingContext;
    //if (m_pDebug)                delete m_pDebug;   //NEVER_OURS
    //if (m_pEMO)                  delete m_pEMO;     //NEVER_OURS
    if (m_pTransformContext)       delete m_pTransformContext;
    vector_element_destroy(m_pNodes);
  }

  QueryEnvironment::QueryEnvironment(const QueryEnvironment &qe2):
    //public pointer copying
    //exceptions situations only
    //use inherit() for inheritance logic
    MemoryLifetimeOwner(qe2.mmParent()), //addChild()
    m_pProfiler(qe2.m_pProfiler),
    m_pSingularDocumentContext(qe2.m_pSingularDocumentContext),
    m_pEMO(qe2.m_pEMO),

    m_pSec(qe2.m_pSec),
    m_pDebug(qe2.m_pDebug),
    m_pTrigger(qe2.m_pTrigger),
    m_pMask(qe2.m_pMask),
    m_pGrammar(qe2.m_pGrammar),
    m_pXPathProcessingContext(qe2.m_pXPathProcessingContext),
    m_pTransformContext(qe2.m_pTransformContext),

    m_vComponents(qe2.m_vComponents),
    m_bInherited(qe2.m_bInherited),
    m_bSecOurs(qe2.m_bSecOurs),
    m_pNodes(0)
  {
    NOT_CURRENTLY_USED("needs to clone components"); //or tested!

    assert(m_pSec);

    //component alignment (point back to this)
    for (vector<IXmlQueryEnvironmentComponent*>::iterator iComponent = m_vComponents.begin(); iComponent != m_vComponents.end(); iComponent++)
      (*iComponent)->setQueryEnvironment(this);
  }

  const IXmlLibrary *QueryEnvironment::xmlLibrary() const {
    return m_pSingularDocumentContext->xmlLibrary();
  }

  IXmlQueryEnvironment *QueryEnvironment::inherit(IXslDoc *pNewStylesheet) const {
    //new QE running inherit on all applicable stateful components
    //pStylesheet parameter indicates that a new / inherited transfomContext is required for this pStylesheet
    IXslTransformContext *pNewTransformContext = 0;
    QueryEnvironment     *pInheritedQE         = 0;

    //auto-add a transform context based on the stylesheet if present
    if (pNewStylesheet) {
      //transform contexts need to point to the correct QE and the new StyleSheet
      if (m_pTransformContext) pNewTransformContext = m_pTransformContext->inherit(pNewStylesheet);
      else                     pNewTransformContext = xmlLibrary()->factory_transformContext(this, this, m_pSingularDocumentContext, pNewStylesheet);
    }

    pInheritedQE = new QueryEnvironment(
      //------------------ stateless components:
      //these components do not need to point back to pInheritedQE
      //so they can be passed in directly
      //point to the same document, NOT_OURS
      mmParent(),
      m_pProfiler,
      m_pSingularDocumentContext,
      //currently stateless and big e.g. the Database or Request!, so just return a pointer
      m_pEMO,

      //------------------ components with point back
      //setQueryEnvironment(pInheritedQE) will be run on all these components during construction
      //new security context that doesn't affect parent security
      //this is currently a direct clone: clone_with_resources()
      m_pSec->inherit(),
      //optional
      //contains inherited m_vCyclicRunningTriggers, m_bTriggerEnabled, etc.
      (m_pTrigger ? m_pTrigger->inherit() : NULL),
      (m_pMask    ? m_pMask->inherit()    : NULL),
      m_pDebug, //NEVER_OURS. usually a User and const
      (m_pGrammar ? m_pGrammar->inherit() : NULL),

      //------------------- optional situation components
      //new or inherited transform context
      pNewTransformContext, //inherited above
      INHERITED
    );

    return pInheritedQE;
  }

  IXmlQueryEnvironment *QueryEnvironment::inherit(IXslModuleManager *pNewEMO) const {
    return new QueryEnvironment(
      mmParent(),
      m_pProfiler,
      m_pSingularDocumentContext,
      //currently stateless and big e.g. the Database or Request!, so just return a pointer
      pNewEMO,
      m_pSec->inherit(),
      (m_pTrigger ? m_pTrigger->inherit() : NULL),
      (m_pMask    ? m_pMask->inherit()    : NULL),
      m_pDebug,
      (m_pGrammar ? m_pGrammar->inherit() : NULL),
      m_pTransformContext,
      INHERITED
    );
  }

  const char *QueryEnvironment::toString() const {
    stringstream sOut;
    const IXmlArea *pArea = (m_pMask && m_pMask->size() ? m_pMask->back() : 0);

    sOut << "QueryEnvironment (" << ((void*) this) << ")\n";
    sOut << "  Profiler: " << (m_pProfiler ? m_pProfiler->toString() : "(none)") << "\n";
    sOut << "  Security: " << m_pSec->toString() << (m_pSec->writeable() ? "" : " (READ-ONLY)") << "\n";
    sOut << "  Debug: "    << (m_pDebug ? m_pDebug->toString() : "(DEBUG off)") << "\n";
    sOut << "  Trigger: "  << (m_pTrigger ? m_pTrigger->toString() : "(none)") << "\n";
    sOut << "  Grammar: "  << (m_pGrammar ? m_pGrammar->toString() : "(none)") << "\n";
    sOut << "  XPath: "    << (m_pXPathProcessingContext ? m_pXPathProcessingContext->toString() : "(none)") << "\n";
    sOut << "  " << (m_pEMO ? m_pEMO->toStringModules() : "ModulesManager: (none)\n");
    sOut << "  " << (m_pTransformContext ? m_pTransformContext->toString() : "Transform context: (none)\n");
    sOut << "  " << (pArea ? pArea->toStringNodes() : "Node-mask: (none)") << "\n";

    return MM_STRDUP(sOut.str().c_str());
  }

  void QueryEnvironment::reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes) const {
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    for (iNode = pNodes->begin(); iNode != pNodes->end(); iNode++) {
      m_pNodes->push_back((*iNode)->clone_with_resources());
    }
  }

  bool QueryEnvironment::inherited() const {
    //is this an inherited environment
    return m_bInherited;
  }

  //----------------------------------------------- environment accessors
  const IXmlBaseDoc    *QueryEnvironment::singularDocumentContext() const {return m_pSingularDocumentContext;}
  IXmlProfiler         *QueryEnvironment::profiler()         const {return m_pProfiler;}
  IXslModuleManager    *QueryEnvironment::emoContext()       const {return m_pEMO;}
  IXmlDebugContext     *QueryEnvironment::debugContext()     const {return m_pDebug;}
  IXmlDebugContext     *QueryEnvironment::debugContext(IXmlDebugContext *pDebug) {return m_pDebug = pDebug;}

  IXmlSecurityContext  *QueryEnvironment::securityContext()  const {return m_pSec;}     //for logging in/out operations and masquerading
  IXmlTriggerContext   *QueryEnvironment::triggerContext()   const {return m_pTrigger;} //for enable / disable triggers
  IXmlMaskContext      *QueryEnvironment::maskContext()      const {return m_pMask;}    //
  IXmlGrammarContext   *QueryEnvironment::grammarContext()   const {return m_pGrammar;}
  IXmlXPathProcessingContext *QueryEnvironment::xpathProcessingContext() const {return m_pXPathProcessingContext;}
  IXslTransformContext *QueryEnvironment::transformContext() const {return m_pTransformContext;}
  IXslXPathFunctionContext *QueryEnvironment::newXPathContext(const IXmlBaseNode *pCurrentNode, const char *sXPath) const {
    //create a blank xpath context related to this transfom context
    //also use the XML Library to create a blank one
    const IXmlLibrary *pLib = xmlLibrary();
    return pLib->factory_xpathContext(this, this, pCurrentNode, sXPath);
  }

  //--------------------------------------- debug
  void QueryEnvironment::setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const {
    IXslTransformContext *pTC;
    xmlLibrary()->setDefaultXSLTTraceFlags(iFlags);
    if (pTC = transformContext()) pTC->setXSLTTraceFlags(iFlags);
  }

  void QueryEnvironment::clearXSLTTraceFlags() const {
    IXslTransformContext *pTC;
    xmlLibrary()->clearDefaultXSLTTraceFlags();
    if (pTC = transformContext()) pTC->clearXSLTTraceFlags();
  }

  void QueryEnvironment::setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const {
    //actually sets it at library level
    xmlLibrary()->setDefaultXMLTraceFlags(iFlags);
  }

  void QueryEnvironment::clearXMLTraceFlags() const {
    //actually sets it at library level
    xmlLibrary()->clearDefaultXMLTraceFlags();
  }

  const char *QueryEnvironment::currentParentRoute() const {
    return (transformContext() ? transformContext()->currentParentRoute() : 0);
  }

  const char *QueryEnvironment::currentXSLCommandXPath() const {
    return (transformContext() ? transformContext()->currentXSLCommandXPath() : 0);
  }

  const char *QueryEnvironment::currentSourceNodeXPath() const {
    return (transformContext() ? transformContext()->currentSourceNodeXPath() : 0);
  }
}
