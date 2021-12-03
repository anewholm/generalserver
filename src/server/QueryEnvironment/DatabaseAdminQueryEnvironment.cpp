//platform agnostic file
#include "DatabaseAdminQueryEnvironment.h"
#include "IXml/IXslTransformContext.h"
#include "Utilities/container.c"
#include "GrammarContext.h"
#include "XPathProcessingContext.h"

namespace general_server {
  DatabaseAdminQueryEnvironment::DatabaseAdminQueryEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseDoc *pSingularDocumentContext, IXslModuleManager *pEMO, IXslTransformContext *pTransformContext):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pSingularDocumentContext(pSingularDocumentContext),
    m_pEMO(pEMO),
    m_pGrammar(new GrammarContext()),
    m_pXPathProcessingContext(new XPathProcessingContext()),
    m_pSec(new AdminSecurityContext(pSingularDocumentContext)),
    m_pTransformContext(pTransformContext)
  {
    assert(m_pSingularDocumentContext);
    assert(m_pSec);

    m_pGrammar->setQueryEnvironment(this);
    if (m_pXPathProcessingContext) m_pXPathProcessingContext->setQueryEnvironment(this);
    m_pSec->setQueryEnvironment(this);
    m_pNodes = new XmlNodeList<const IXmlBaseNode>(this);
  }

  DatabaseAdminQueryEnvironment::~DatabaseAdminQueryEnvironment() {
    if (m_pSec)           delete m_pSec;
    if (m_pGrammar)       delete m_pGrammar;
    if (m_pXPathProcessingContext) delete m_pXPathProcessingContext;
    vector_element_destroy(m_pNodes);
  }

  IXmlQueryEnvironment *DatabaseAdminQueryEnvironment::inherit(IXslDoc *pNewStylesheet) const {
    //new QE running inherit on all applicable stateful components
    //pStylesheet parameter indicates that a new / inherited transfomContext is required for this pStylesheet
    IXslTransformContext *pNewTransformContext = 0;
    IXmlQueryEnvironment *pInheritedQE         = 0;

    //auto-add a transform context based on the stylesheet if present
    if (pNewStylesheet) {
      //transform contexts need to point to the correct QE and the new SS
      if (m_pTransformContext) pNewTransformContext = m_pTransformContext->inherit(pNewStylesheet);
      else                     pNewTransformContext = xmlLibrary()->factory_transformContext(this, this, m_pSingularDocumentContext, pNewStylesheet);
    }
    pInheritedQE = new DatabaseAdminQueryEnvironment(mmParent(), m_pSingularDocumentContext, m_pEMO, pNewTransformContext);

    return pInheritedQE;
  }

  IXmlQueryEnvironment *DatabaseAdminQueryEnvironment::inherit(IXslModuleManager *pNewEMO)  const {
    return new DatabaseAdminQueryEnvironment(mmParent(), m_pSingularDocumentContext, pNewEMO);
  }

  bool DatabaseAdminQueryEnvironment::inherited() const {return false;} //is this an inherited environment

  const char *DatabaseAdminQueryEnvironment::toString() const {return "DatabaseAdminQueryEnvironment";}

  void DatabaseAdminQueryEnvironment::reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes) const {
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    for (iNode = pNodes->begin(); iNode != pNodes->end(); iNode++) {
      m_pNodes->push_back((*iNode)->clone_with_resources());
    }
  }

  IXmlSecurityContext  *DatabaseAdminQueryEnvironment::securityContext()  const {return m_pSec;} //required security context
  IXmlTriggerContext   *DatabaseAdminQueryEnvironment::triggerContext()   const {return 0;}
  IXmlMaskContext      *DatabaseAdminQueryEnvironment::maskContext()      const {return 0;}
  IXmlGrammarContext   *DatabaseAdminQueryEnvironment::grammarContext()   const {return m_pGrammar;}
  IXmlXPathProcessingContext *DatabaseAdminQueryEnvironment::xpathProcessingContext() const {return m_pXPathProcessingContext;}
  IXslModuleManager    *DatabaseAdminQueryEnvironment::emoContext()       const {return m_pEMO;}
  IXmlDebugContext     *DatabaseAdminQueryEnvironment::debugContext()     const {return 0;}
  IXmlDebugContext     *DatabaseAdminQueryEnvironment::debugContext(IXmlDebugContext *pDebug ATTRIBUTE_UNUSED) {return 0;}
  IXslTransformContext *DatabaseAdminQueryEnvironment::transformContext() const {return m_pTransformContext;}
  IXslXPathFunctionContext *DatabaseAdminQueryEnvironment::newXPathContext(const IXmlBaseNode *pCurrentNode, const char *sXPath) const {
    return xmlLibrary()->factory_xpathContext(this, this, pCurrentNode, sXPath);
  }
  const IXmlLibrary *DatabaseAdminQueryEnvironment::xmlLibrary()      const {return m_pSingularDocumentContext->xmlLibrary();}
  const IXmlBaseDoc *DatabaseAdminQueryEnvironment::singularDocumentContext() const {return m_pSingularDocumentContext;}
  IXmlProfiler  *DatabaseAdminQueryEnvironment::profiler()    const {return 0;}
  void DatabaseAdminQueryEnvironment::setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags ATTRIBUTE_UNUSED) const {}
  void DatabaseAdminQueryEnvironment::clearXSLTTraceFlags() const {}
  void DatabaseAdminQueryEnvironment::setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags ATTRIBUTE_UNUSED) const {}
  void DatabaseAdminQueryEnvironment::clearXMLTraceFlags() const {}
  const char *DatabaseAdminQueryEnvironment::currentParentRoute() const {
    return (transformContext() ? transformContext()->currentParentRoute() : 0);
  }
  const char *DatabaseAdminQueryEnvironment::currentXSLCommandXPath() const {
    return (transformContext() ? transformContext()->currentXSLCommandXPath() : 0);
  }
  const char *DatabaseAdminQueryEnvironment::currentSourceNodeXPath() const {
    return (transformContext() ? transformContext()->currentSourceNodeXPath() : 0);
  }
}
