//platform agnostic file
#include "Xml/XslDoc.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlLibrary.h"
#include "IXml/IXslNode.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"

#include "Xml/XmlNodeList.h"

#include "Utilities/container.c"
using namespace std;

namespace general_server {
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  const char *XslDoc::name() const {
    const XmlAdminQueryEnvironment ibqe(mmParent(), this); //XslDoc::name()
    const char *sName;
    const IXmlBaseDoc  *pDocType  = queryInterface((const IXmlBaseDoc*) 0);
    const IXmlBaseNode *pRootNode = pDocType->rootNode(&ibqe);
    if (pRootNode) sName = pRootNode->attributeValue(&ibqe, "name", NAMESPACE_REPOSITORY);
    else sName = MM_STRDUP("stylesheet");
    return sName;
  }

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  XslTransformContext::XslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pOwnerQE(pOwnerQE) 
  {
    assert(m_pOwnerQE);
  }
  
  XslTransformContext::~XslTransformContext() {}
  
  void XslTransformContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;assert(m_pOwnerQE);}
  
  const IXmlQueryEnvironment *XslTransformContext::ownerQE() const {return m_pOwnerQE;}
  
  const char *XslTransformContext::currentSourceNodeXPath() const {
    const XmlAdminQueryEnvironment ibqe_error_reporting(this, sourceDoc());
    const char *sXPath        = 0;
    const IXmlBaseNode *pNode = 0;
    
    try {
      if (pNode = sourceNode(this)) sXPath = pNode->uniqueXPathToNode(&ibqe_error_reporting);
    } catch (ExceptionBase &eb) {
      sXPath = eb.toString();
    }

    //free up
    if (pNode) delete pNode;
      
    return (sXPath ? sXPath : "<unknown>");
  }

  const char *XslTransformContext::currentXSLTemplateXPath() const {
    const XmlAdminQueryEnvironment ibqe_error_reporting(this, commandDoc());
    const char *sXPath = 0;
    const IXslCommandNode *pNode  = 0;
    const IXmlBaseNode *pNodeType = 0;

    try {
      if (pNode = templateNode(this)) {
        if (pNodeType = pNode->queryInterface((const IXmlBaseNode*) 0))
          sXPath = pNodeType->uniqueXPathToNode(&ibqe_error_reporting);
      }
    } catch (ExceptionBase &eb) {
      sXPath = eb.toString();
    }

    //free up
    if (pNode) delete pNode;

    return (sXPath ? sXPath : "<unknown>");
  }
  
  const char *XslTransformContext::currentXSLCommandXPath() const {
    const XmlAdminQueryEnvironment ibqe_error_reporting(this, commandDoc());
    const char *sXPath        = 0;
    const IXmlBaseNode *pNode = 0;
    
    try {
      if (pNode = instructionNode(this)) sXPath = pNode->uniqueXPathToNode(&ibqe_error_reporting);
    } catch (ExceptionBase &eb) {
      sXPath = eb.toString();
    }

    //free up
    if (pNode) delete pNode;
      
    return (sXPath ? sXPath : "<unknown>");
  }
  
  void XslTransformContext::handleError(const char *sErrorMessage) {
    //when the native XML library throws an XPATH error
    //  we arrive here in the agnostic layer to deal with it
    //  caught by our native library generic error handler
    //  which will bubble up to the XSLTException catch layer if we throw here
    //XPathException() can also bubble up from our own custom xpath functions
    //  directly to the XSLTException catch layer
    //  without the native library generic error handler getting involved
    IXslTransformContext *pTC        = 0;
    const IXmlBaseNode *pCommandNode = 0;
    iErrorPolicy er = throw_error;

    //error policies
    if (pTC = m_pOwnerQE->transformContext()) {
      if (pCommandNode = pTC->literalCommandNode(m_pOwnerQE)) er = pCommandNode->errorPolicy(); //pQE as MM
    }

    //free up
    if (pCommandNode) delete pCommandNode;
    //if (pTC)          delete pTC; //server freed

    if (er == throw_error) throw XSLTException(this, MM_STRDUP(sErrorMessage));
    else Debug::warn("error suppressed by xsl:error-policy");
  }


  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  XslXPathFunctionContext::XslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), //usually just a writeable pOwnerQE
    m_pOwnerQE(pOwnerQE) 
  {
    assert(m_pOwnerQE);
  }
  
  XslXPathFunctionContext::~XslXPathFunctionContext() {}
  
  void XslXPathFunctionContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;assert(m_pOwnerQE);}
  
  const IXmlQueryEnvironment *XslXPathFunctionContext::ownerQE() const {return m_pOwnerQE;}
  
  void XslXPathFunctionContext::handleError(const char *sErrorMessage) {
    //when the native XML library throws an XPATH error
    //  we arrive here in the agnostic layer to deal with it
    //  caught by our native library generic error handler
    //  which will bubble up to the XSLTException catch layer if we throw here
    //XPathException() can also bubble up from our own custom xpath functions
    //  directly to the XSLTException catch layer
    //  without the native library generic error handler getting involved
    IXslTransformContext *pTC        = 0;
    const IXmlBaseNode *pCommandNode = 0;
    iErrorPolicy er = throw_error;

    //error policies
    if (pTC = m_pOwnerQE->transformContext()) {
      if (pCommandNode = pTC->literalCommandNode(m_pOwnerQE)) er = pCommandNode->errorPolicy(); //pQE as MM
    }

    //free up
    if (pCommandNode) delete pCommandNode;
    //if (pTC)          delete pTC; //server freed

    if (er == throw_error) throw XPathException(this, MM_STRDUP(sErrorMessage));
    else Debug::warn("error suppressed by xsl:error-policy");
  }
  
  const char *XslXPathFunctionContext::currentXSLCommandXPath() const {
    //caller frees result
    const char *ret = 0;
    IXslTransformContext *pTCtxt;

    if (pTCtxt = m_pOwnerQE->transformContext()) ret = pTCtxt->currentXSLCommandXPath();

    //free up
    //if (pTCtxt) delete pTCtxt; //pointer in to pQE

    return ret;
  }

  const char *XslXPathFunctionContext::currentXSLTemplateXPath() const {
    //caller frees result
    const char *ret = 0;
    IXslTransformContext *pTCtxt;

    if (pTCtxt = m_pOwnerQE->transformContext()) ret = pTCtxt->currentXSLTemplateXPath();

    //free up
    //if (pTCtxt) delete pTCtxt; //pointer in to pQE

    return ret;
  }

  const char *XslXPathFunctionContext::toString() const {
    return xpath();
  }

  IXmlBaseNode *XslXPathFunctionContext::popInterpretNodeFromXPathFunctionCallStack(const bool bThrowOnNot1) {
    XmlNodeList<IXmlBaseNode>::iterator iNode;
    IXmlBaseNode *pNode               = 0;
    XmlNodeList<IXmlBaseNode> *pNodes = 0;

    UNWIND_EXCEPTION_BEGIN {
      if (pNodes = popInterpretNodeListFromXPathFunctionCallStack()) {
        if (bThrowOnNot1 && pNodes->size() != 1) throw XPathNot1Nodes(this);
        else {
          for (iNode = pNodes->begin(); iNode != pNodes->end(); iNode++) {
            if (!pNode) pNode = *iNode;
            else delete *iNode;
          }
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pNodes) delete pNodes;
    
    UNWIND_EXCEPTION_THROW;

    return pNode;
  }

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  XslDoc::XslDoc(IXslTransformContext *pCtxt): 
    m_pCtxt(pCtxt), 
    m_pSourceNode(0) 
  {}
  
  XslDoc::XslDoc(): 
    m_pCtxt(0), 
    m_pSourceNode(0) 
  {}
  
  const IXmlBaseNode *XslDoc::sourceNode() const {
    return m_pSourceNode;
  }
  
  const IXmlBaseDoc *XslDoc::sourceDoc() const {
    //m_pSourceNode will also lock the source document
    return (m_pSourceNode ? m_pSourceNode->document() : NULL);
  }

  const IXmlBaseNode *XslDoc::sourceNode(const IXmlBaseNode *pSourceNode) {
    assert(pSourceNode);
    m_pSourceNode = pSourceNode;
    return m_pSourceNode;
  }
  
  XmlNodeList<IXmlBaseNode> *XslDoc::includes() {
    XmlAdminQueryEnvironment ibqe(this, this); //includes()
    return getMultipleNodes(&ibqe, "/xsl:stylesheet/xsl:include[@href and not(@xpath)]");
  }

  size_t XslDoc::translateIncludes(XmlNodeList<IXmlBaseNode> *pvIncludes, IXmlQueryEnvironment *pMasterDocQE, const IXmlBaseNode *pIncludeDirectoryNode) {
    //this is enacted on a COPY of the XSL node in the transform() function
    //this is a live update of xsl:include @hrefs. Not a copy
    //USED mostly on client-side relative @hrefs that need translating to server-side @xpath
    //location of xsl:include same as:
    //  C++ clearMyCachedStylesheetsRecursive()
    //  C++ translateIncludes() @href => @xpath
    //  LibXml2: xsl:include processing (of @xpath) during server side compilation
    //  map_url_to_stylesheet_model.xsl REST based client xsl:include requests
    //  AJAX client_stylesheet_transform.xsl xsl:include include directly in main xsl:stylesheet request
    size_t iUpdates = 0;

    const IXmlBaseNode *pIncludeNode                  = 0,
                       *pIncludedNode                 = 0;
    const IXslStylesheetNode *pIncludedStylesheetNode = 0;
    typename XmlNodeList<IXmlBaseNode>::iterator iInclude;
    const char *sHREF  = 0,
               *sXPath = 0;

    //get and alter @hrefs, if @xpath not there...
    //works only from the top level xsl:stylesheet node
    //xsl:includes have been altered in LibXml2 to accept @xpath instead of @href which is faster
    ensurePrefixedNamespaceDefinition(NAMESPACE_XSL_ALIAS, NAMESPACE_XSL);

    UNWIND_EXCEPTION_BEGIN {
      //NOTE: the disembodied stylesheet should have an xpath pointer to it's original parent in the world
      for (iInclude = pvIncludes->begin(); iInclude != pvIncludes->end(); iInclude++) {
        pIncludeNode = (*iInclude);
        sXPath       = pIncludeNode->attributeValue(pMasterDocQE, "xpath");

        if (sXPath) {
          if (pIncludedNode = pIncludeDirectoryNode->getSingleNode(pMasterDocQE, sXPath))
            pIncludedStylesheetNode = pIncludedNode->queryInterface((const IXslStylesheetNode*) 0);
        } else {
          sHREF = pIncludeNode->attributeValue(pMasterDocQE, "href");
          if (!sHREF) throw FailedToResolveXSLIncludePath(this, MM_STRDUP("no @xpath or @href"));
          if (xmlLibrary()->maybeAbsoluteXPath(pMasterDocQE, sHREF)) Debug::report("xsl:include @href=[%s] not verified because cannot resolve class references against non-ids-populated disembodied new doc yet", sXPath);
          pIncludedStylesheetNode = pIncludeDirectoryNode->relativeXSLIncludeFileSystemPathToXSLStylesheetNode(pMasterDocQE, sHREF);
        }

        //TODO: and recurse sub-includes

        //GDB: p *dynamic_cast<LibXmlBaseDoc*>(this)->m_oDoc
        if (!pIncludedStylesheetNode) throw FailedToResolveXSLIncludePath(this, MM_STRDUP("xpath result empty"));
        else {
          //register xpath parameter for later
          //TODO: should we TXml this register xpath parameter for later? better that it is dynamic?
          pIncludeNode->setAttribute(pMasterDocQE, "xpath", sXPath);
          pIncludeNode->removeAttribute(pMasterDocQE, "href");
          iUpdates++;
        }

        //free up
        if (pIncludedStylesheetNode) {delete pIncludedStylesheetNode; pIncludedStylesheetNode = 0;}
        if (sHREF)                   {MM_FREE(sHREF);  sHREF  = 0;}
        if (sXPath)                  {MM_FREE(sXPath); sXPath = 0;}
      } //for loop
    } UNWIND_EXCEPTION_END;

    //free up
    if (pIncludedStylesheetNode) delete pIncludedStylesheetNode;
    if (sHREF)                   MM_FREE(sHREF);
    if (sXPath)                  MM_FREE(sXPath);

    UNWIND_EXCEPTION_THROW;

    return iUpdates;
  }
}
