//platform agnostic file
#include "Xml/XslNode.h"

#include "IXml/IXslDoc.h"
#include "Debug.h"
#include "Xml/XmlBaseNode.h"
#include "Xml/XmlBaseDoc.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "QueryEnvironment/CompilationEnvironment.h"

using namespace std;

namespace general_server {
  //---------------------------------------------------------------------------------------
  //------------------------------- XslCommandNode ----------------------------------------
  //---------------------------------------------------------------------------------------
  bool XslCommandNode::isCommandNamespace(const char *sHREF) {
    //TODO: isCommandNamespace() automate?
    return (sHREF && *sHREF && (
            _STREQUAL(sHREF, NAMESPACE_DEBUG)
         || _STREQUAL(sHREF, NAMESPACE_REPOSITORY)
         || _STREQUAL(sHREF, NAMESPACE_DATABASE)
         || _STREQUAL(sHREF, NAMESPACE_SERVER)
         || _STREQUAL(sHREF, NAMESPACE_SERVICE)
         || _STREQUAL(sHREF, NAMESPACE_CONVERSATION)
         || _STREQUAL(sHREF, NAMESPACE_USER)
         || _STREQUAL(sHREF, NAMESPACE_REQUEST)
         || _STREQUAL(sHREF, NAMESPACE_SESSION)
         || _STREQUAL(sHREF, NAMESPACE_RESPONSE)
         || _STREQUAL(sHREF, NAMESPACE_EXSLT_REGEXP)
         || _STREQUAL(sHREF, NAMESPACE_EXSLT_FLOW)
         || _STREQUAL(sHREF, NAMESPACE_EXSLT_STRINGS)
    ));
  }

  const IXslDoc *XslCommandNode::stylesheet() const {
    return document()->queryInterface((const IXslDoc*) 0);
  }


  //---------------------------------------------------------------------------------------
  //---------------------------- XslStylesheetNode ----------------------------------------
  //---------------------------------------------------------------------------------------
  IXslDoc *XslStylesheetNode::preprocessToStylesheet(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pIncludeDirectory) const {
    //create a new Doc for the LibXML parser to compile
    //changes an XSL Node in to an XSL Document copy with translated bits ready for transformation
    //we need to work out the associated concrete IXmlDoc type that goes with this IXmlBaseNode type
    //we are cloning so we can loose the const
    //document linkage to the original place is necessary for the xpath includes to work
    //as we are not using working directories anymore
    //xsl:includes @href translation re-based, checks include existence
    //we use this DOC for the IBQE because of id() lookups etc. that need to reference here, not the new DOC
    const IXmlBaseNode *pDirectParentDirectory = 0,
                       *pIncludeDirectoryToUse = 0;
    XmlNodeList<IXmlBaseNode> *pvIncludes      = 0;
    IXslDoc *pXslDoc = xmlLibrary()->factory_stylesheet(pQE, "read-only XSL copy"); //pQE as MM
    XmlAdminQueryEnvironment ibqe(pXslDoc, document()); //translateIncludes() master document IBQE
    CompilationEnvironment ce(pXslDoc, pQE, pXslDoc);
    
    pXslDoc->importXmlNode(pQE, this, DEEP_CLONE); //IXmlBaseNode function
    pXslDoc->sourceNode(this); //remember were we came from (will also lock the node)
    pXslDoc->copyNamespaceCacheFrom(document()); //enable xpath namespaces usage
    
    //parent node for stylesheet linkage 
    //during processing of xsl:include @xpath
    if (pIncludeDirectory) {
      //used in server side XSLT
      pIncludeDirectoryToUse = pIncludeDirectory;
    } else {
      //stylesheet node and its parent are in the original document here
      //need to ensure no masks
      //SECURITY: should we be allowing root doc access for compilation like this?
      pDirectParentDirectory = parentNode(&ibqe, "preprocessToStylesheet() parent");
      pIncludeDirectoryToUse = pDirectParentDirectory;
    }
    if (pIncludeDirectoryToUse) pXslDoc->setParent(pIncludeDirectoryToUse);
    else Debug::report("Stylesheet has no parent anchor for xsl:include @xpath. Was it excluded by a maskContext()?", rtWarning);

    //-------------------------------- xsl:include processing
    pvIncludes = pXslDoc->includes();
    if (pvIncludes->size()) {
      //sanity checks
      IFDEBUG(
        const char   *sXmlId                       = 0;
        const IXmlBaseNode *pParentParentDirectory = 0;

        //get the @xml:id of the parent "include directory" node
        if (pIncludeDirectoryToUse) {
          pParentParentDirectory = pIncludeDirectoryToUse->parentNode(pQE);
          if (pParentParentDirectory) sXmlId = pParentParentDirectory->xmlID(pQE);
          if (pIncludeDirectoryToUse->isTransient()) Debug::report("XSLStylesheet xsl:includes pre-process is in a @gs:transient-area: xsl:includes will not find their target", rtWarning);
          if (_STREQUAL(sXmlId, "idx_tmp"))          Debug::report("XSLStylesheet xsl:includes pre-process is in the <gs:tmp> area: xsl:includes will not find their target",    rtWarning);
        } else Debug::report("falied to find parent directory node for XSLStylesheet: xsl:includes will not find their target",            rtWarning);

        //free up debug stuff
        if (pParentParentDirectory) delete pParentParentDirectory;
        if (sXmlId)                 MM_FREE(sXmlId);
      );

      //GDB: p *dynamic_cast<LibXmlBaseDoc*>(pXslDoc)->m_oDoc
      //SECURITY: should we be allowing root doc access for compilation like this?
      pXslDoc->translateIncludes(pvIncludes, &ibqe, pIncludeDirectoryToUse);
    }

    //-------------------------------- parse to compiled object
    //CompilationContext XPath context contains:
    //  EMO:       pQE dynamic @match={database:derived-template-match()} functions during compilation
    //  XFilter:   pQE normal user security that can restrict access to XSL
    //  node-mask: DISABLED because the node-mask is only for restricting access to data, not instructions
    //  XTrigger:  special trigger context that re-populates things like dynamic @match statements
    //oDoc is already a clone and linked to its source parent for xpath multiple includes and class:* lookup
    pXslDoc->parse(&ce); //uses the document linkage parent during parsing
    if (pIncludeDirectoryToUse) pXslDoc->clearParent();

    //free up
    //if (pvIncludes) delete pvIncludes; //this is done by translateIncludes()
    if (pDirectParentDirectory) delete pDirectParentDirectory;
    //if (cc) delete cc; //local variable

    return pXslDoc;
  }
}
