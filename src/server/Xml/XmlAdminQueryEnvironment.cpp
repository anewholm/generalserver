//platform agnostic file
#include "Xml/XmlAdminQueryEnvironment.h"
#include "IXml/IXslTransformContext.h"
#include "Utilities/container.c"
#include "QueryEnvironment/MaskContext.h"
#include "QueryEnvironment/GrammarContext.h"
#include "QueryEnvironment/XPathProcessingContext.h"

namespace general_server {
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  XmlAdminQueryEnvironment::XmlAdminQueryEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseDoc *pSingularDocumentContext, IXmlSecurityContext *pSec, IXslTransformContext *pTransformContext):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pSingularDocumentContext(pSingularDocumentContext),
    m_pSec(pSec ? pSec : new AdminSecurityContext(pSingularDocumentContext)),
    m_pTransformContext(pTransformContext),
    m_pGrammar(new GrammarContext()),
    m_pXPathProcessingContext(new XPathProcessingContext()),
    m_pMask(new MaskContext())
  {
    assert(m_pSingularDocumentContext);
    assert(m_pSec);

    if (m_pSec)              m_pSec->setQueryEnvironment(this);
    if (m_pTransformContext) m_pTransformContext->setQueryEnvironment(this);
    if (m_pGrammar)          m_pGrammar->setQueryEnvironment(this);
    if (m_pXPathProcessingContext) m_pXPathProcessingContext->setQueryEnvironment(this);
    if (m_pMask)             m_pMask->setQueryEnvironment(this);
  }
  
  XmlAdminQueryEnvironment::~XmlAdminQueryEnvironment() {
    if (m_pSec)     delete m_pSec;
    if (m_pMask)    delete m_pMask;
    if (m_pGrammar) delete m_pGrammar;
    if (m_pXPathProcessingContext) delete m_pXPathProcessingContext;
  }
  const IXmlBaseDoc *XmlAdminQueryEnvironment::changeSingularDocumentContext(const IXmlBaseDoc *pNewDoc) {
    //only XmlAdminQueryEnvironment is allowed to do this
    assert(pNewDoc);
    m_pSingularDocumentContext = pNewDoc;
    return pNewDoc;
  }

  IXmlQueryEnvironment *XmlAdminQueryEnvironment::inherit(IXslDoc *pNewStylesheet) const {return 0;}
  IXmlQueryEnvironment *XmlAdminQueryEnvironment::inherit(IXslModuleManager *pNewEMO) const {return 0;}
  bool XmlAdminQueryEnvironment::inherited() const {return false;} //is this an inherited environment
  const char *XmlAdminQueryEnvironment::toString() const {return "XmlAdminQueryEnvironment";}

  IXmlSecurityContext  *XmlAdminQueryEnvironment::securityContext()  const {return m_pSec;} //required security context
  IXmlTriggerContext   *XmlAdminQueryEnvironment::triggerContext()   const {return 0;}
  IXmlMaskContext      *XmlAdminQueryEnvironment::maskContext()      const {return m_pMask;}
  IXmlGrammarContext   *XmlAdminQueryEnvironment::grammarContext()   const {return m_pGrammar;}
  IXmlXPathProcessingContext *XmlAdminQueryEnvironment::xpathProcessingContext() const {return m_pXPathProcessingContext;}
  IXslModuleManager    *XmlAdminQueryEnvironment::emoContext()       const {return 0;}
  IXmlDebugContext     *XmlAdminQueryEnvironment::debugContext()     const {return 0;}
  IXmlDebugContext     *XmlAdminQueryEnvironment::debugContext(IXmlDebugContext *pDebug) {return 0;}
  IXslTransformContext *XmlAdminQueryEnvironment::transformContext() const {return m_pTransformContext;}
  IXslXPathFunctionContext *XmlAdminQueryEnvironment::newXPathContext(const IXmlBaseNode *pCurrentNode, const char *sXPath) const {
    return xmlLibrary()->factory_xpathContext(this, this, pCurrentNode, sXPath);
  }
  const IXmlLibrary *XmlAdminQueryEnvironment::xmlLibrary() const {return m_pSingularDocumentContext->xmlLibrary();}
  const IXmlBaseDoc *XmlAdminQueryEnvironment::singularDocumentContext() const {return m_pSingularDocumentContext;}
  IXmlProfiler  *XmlAdminQueryEnvironment::profiler()    const {return 0;}
  void XmlAdminQueryEnvironment::setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const {
    IXslTransformContext *pTC;
    xmlLibrary()->setDefaultXSLTTraceFlags(iFlags);
    if (pTC = transformContext()) pTC->setXSLTTraceFlags(iFlags);
  }
  void XmlAdminQueryEnvironment::clearXSLTTraceFlags() const {}
  void XmlAdminQueryEnvironment::setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const {
    //actually sets it at library level
    xmlLibrary()->setDefaultXMLTraceFlags(iFlags);
  }
  void XmlAdminQueryEnvironment::clearXMLTraceFlags() const {}
  const char *XmlAdminQueryEnvironment::currentParentRoute() const {
    return (transformContext() ? transformContext()->currentParentRoute() : 0);
  }
  const char *XmlAdminQueryEnvironment::currentXSLCommandXPath() const {
    return (transformContext() ? transformContext()->currentXSLCommandXPath() : 0);
  }
  const char *XmlAdminQueryEnvironment::currentSourceNodeXPath() const {
    return (transformContext() ? transformContext()->currentSourceNodeXPath() : 0);
  }
}
