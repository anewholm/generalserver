//platform agnostic file
#include "CompilationEnvironment.h"

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

#include "CompilationContext.h"

#include "Utilities/container.c"

using namespace std;

namespace general_server {
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  CompilationEnvironment::CompilationEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, IXslDoc *pMainXSLDoc):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pSourceQE(pQE),
    m_pMainXSLDoc(pMainXSLDoc),
    
    //components
    m_pSec(pQE->securityContext()),
    m_pTrigger(new CompilationContext(pMainXSLDoc)),
    m_pGrammar(new GrammarContext()),
    m_pTransformContext(pQE->transformContext()),
    m_pDebug(pQE->debugContext())
  {
    //component alignment (point back to this)
    //TODO: use the m_vComponents elsewhere: inheritance, toString, etc.
    //this also makes sure, at compile time, that these components have correctly supported the interfaces
    if (m_pSec)              m_vComponents.push_back(m_pSec);
    if (m_pTrigger)          m_vComponents.push_back(m_pTrigger);
    if (m_pGrammar)          m_vComponents.push_back(m_pGrammar);
    if (m_pTransformContext) m_vComponents.push_back(m_pTransformContext);
    if (m_pDebug)            m_vComponents.push_back(m_pDebug);

    //PERFORMANCE: borrowed components!
    //we are only borrowing these components
    //and will point them back to the original QE on ~destruction
    for (vector<IXmlQueryEnvironmentComponent*>::iterator iComponent = m_vComponents.begin(); iComponent != m_vComponents.end(); iComponent++)
      (*iComponent)->setQueryEnvironment(this);

    //REQUIRED components
    assert(m_pSec);
    assert(m_pMainXSLDoc);
  }

  CompilationEnvironment::~CompilationEnvironment() {
    //not much is ours!
    
    //we are only borrowing these components
    //point them back to the original QE
    for (vector<IXmlQueryEnvironmentComponent*>::iterator iComponent = m_vComponents.begin(); iComponent != m_vComponents.end(); iComponent++)
      (*iComponent)->setQueryEnvironment(m_pSourceQE);

    if (m_pGrammar) delete m_pGrammar;
    if (m_pTrigger) delete m_pTrigger;
  }

  CompilationEnvironment::CompilationEnvironment(const CompilationEnvironment &qe2):
    //public pointer copying
    //exceptions situations only
    //use inherit() for inheritance logic
    MemoryLifetimeOwner(qe2.mmParent()), //addChild()
    m_pSec(qe2.m_pSec),
    m_pDebug(qe2.m_pDebug),
    m_pTrigger(qe2.m_pTrigger),
    m_pGrammar(qe2.m_pGrammar),
    m_pTransformContext(qe2.m_pTransformContext),

    m_vComponents(qe2.m_vComponents)
  {
    NOT_CURRENTLY_USED(""); //or tested!
  }

  const char *CompilationEnvironment::toString() const {
    stringstream sOut;

    sOut << "CompilationEnvironment (" << ((void*) this) << ")\n";
    sOut << "  Security: " << (m_pSec ? m_pSec->toString() : "(NO SECURITY!)") << "\n";
    sOut << "  Debug: " << (m_pDebug ? m_pDebug->toString() : "(DEBUG off)") << "\n";
    sOut << "  Trigger: " << (m_pTrigger ? m_pTrigger->toString() : "(none)") << "\n";
    sOut << "  Grammar: " << (m_pGrammar ? m_pGrammar->toString() : "(none)") << "\n";
    sOut << "  " << (emoContext() ? emoContext()->toStringModules() : "ModulesManager: (none)\n");
    sOut << "  " << (transformContext() ? transformContext()->toString() : "Transform context: (none)\n");

    return MM_STRDUP(sOut.str().c_str());
  }

  IXslXPathFunctionContext *CompilationEnvironment::newXPathContext(const IXmlBaseNode *pCurrentNode, const char *sXPath) const {
    //create a blank xpath context related to this transfom context
    //also use the XML Library to create a blank one
    return xmlLibrary()->factory_xpathContext(this, this, pCurrentNode, sXPath);
  }

  const IXmlBaseDoc *CompilationEnvironment::singularDocumentContext() const {return m_pMainXSLDoc->queryInterface((IXmlBaseDoc*) 0);}

  //--------------------------------------- debug
  void CompilationEnvironment::setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const {
    IXslTransformContext *pTC;
    xmlLibrary()->setDefaultXSLTTraceFlags(iFlags);
    if (pTC = transformContext()) pTC->setXSLTTraceFlags(iFlags);
  }

  void CompilationEnvironment::clearXSLTTraceFlags() const {
    IXslTransformContext *pTC;
    xmlLibrary()->clearDefaultXSLTTraceFlags();
    if (pTC = transformContext()) pTC->clearXSLTTraceFlags();
  }

  void CompilationEnvironment::setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const {
    //actually sets it at library level
    xmlLibrary()->setDefaultXMLTraceFlags(iFlags);
  }

  void CompilationEnvironment::clearXMLTraceFlags() const {
    //actually sets it at library level
    xmlLibrary()->clearDefaultXMLTraceFlags();
  }
}
