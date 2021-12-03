//platform agnostic file
#include "LibXslDoc.h"
#include "LibXmlBaseNode.h"
#include "LibXmlLibrary.h"
#include "LibXmlNode.h"
#include "IXml/IXmlMaskContext.h"
#include "QueryEnvironment/CompilationEnvironment.h"

using namespace std;

namespace general_server {
  LibXslDoc::LibXslDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_oXSLDoc2(0), 
    LibXmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias, oDoc, bOurs) 
  {}

  LibXslDoc::LibXslDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, IXslTransformContext *pCTxt, const bool bOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_oXSLDoc2(0), 
    LibXmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias, oDoc, bOurs), XslDoc(pCTxt) 
  {}

  LibXslDoc::~LibXslDoc() {
    //TODO: free these when we are cache deleted
    //if (m_oXSLDoc2) xsltFreeStylesheet(m_oXSLDoc2);
  }

  void LibXslDoc::setParent(const IXmlBaseNode *pIncludeDirectory) {
    assert(pIncludeDirectory);

    //link the doc back to the world
    //doc:               xsl:stylesheet
    //pXslStlesheetNode: xsl:stylesheet
    //pIncludeDirectory:  xsl:stylesheet/..
    //it's xsl:include parent is the parent of the directory
    const LibXmlBaseNode *pIncludeDirectoryType = dynamic_cast<const LibXmlBaseNode*>(pIncludeDirectory);
    m_oDoc->parent = pIncludeDirectoryType->libXmlNodePtr();
  }

  void LibXslDoc::clearParent() {
    //unlink the doc from the world
    m_oDoc->parent = 0;
  }

  IXmlBaseNode *LibXslDoc::parentNode() {
    //caller free
    //get doc parent
    return m_oDoc->parent ? LibXmlLibrary::factory_node(this, m_oDoc->parent, this) : 0;
  }

  void LibXslDoc::parse(const CompilationEnvironment *pCE) {
    LibXslXPathFunctionContext *pXPathContext = 0;
    
    if (pXPathContext = (LibXslXPathFunctionContext*) pCE->newXPathContext()) {
      //context transform
      //note that the xsltStylesheetPtr has a style->doc link
      //and xsltFreeStylesheet() cleans up this xmlDoc so we shouldn't
      relinquishOwnershipOfLibXmlDocPtr();

      if (m_oXSLDoc2 = xsltParseStylesheetImportedDocContext(libXmlDocPtr(), NULL, pXPathContext->libXmlXPathContext())) { //NULL parent xsl doc
        m_oXSLDoc2->omitXmlDeclaration = true;
      }
    }

    //free up
    if (pXPathContext) delete pXPathContext;
    
    //increment the lock on the stylesheet because we have a pointer to it
    //lock decremented after transform
    if (!m_oXSLDoc2) throw FailedToCompileStylesheet(this); //recoverable: user stylesheet incompetence
  }

  unsigned int LibXslDoc::incrementLockCount() {
    //native / application level lock counting
    assert(m_oXSLDoc2);
    return xsltLockStylesheet(m_oXSLDoc2);
  }

  bool LibXslDoc::hasLocks() {
    assert(m_oXSLDoc2);
    return xsltHasLocks(m_oXSLDoc2);
  }

  unsigned int LibXslDoc::decrementLockCount() {
    //native / application level lock counting
    assert(m_oXSLDoc2);
    return xsltUnlockStylesheet(m_oXSLDoc2);
  }
}
