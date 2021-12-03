//platform independent
#include "Xml/XmlBaseDoc.h"

#include <sstream>
using namespace std;

#include "IXml/IXmlNamespace.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "Utilities/StringMap.h"
#include "Exceptions.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include "Xml/XmlNodeList.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"

namespace general_server {
  XmlBaseDoc::XmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pLib(pLib), 
    m_sAlias(MM_STRDUP(sAlias)), 
    m_bNamespaceDefinitionsCached(false), 
    m_bDocumentRootNamespacing(false) 
  {
    assert(m_sAlias);
  }

  XmlBaseDoc::XmlBaseDoc(const XmlBaseDoc &oDoc):
    MemoryLifetimeOwner(oDoc.mmParent()) //addChild()
  {
    m_sAlias                   = MM_STRDUP(oDoc.m_sAlias);
    m_pLib                     = oDoc.m_pLib;
    m_mNamespaceDefinitions.insert(&oDoc.m_mNamespaceDefinitions, "true"); //MM_STRDUP(first and second strings)
    m_bNamespaceDefinitionsCached = oDoc.m_bNamespaceDefinitionsCached;
    m_bDocumentRootNamespacing    = oDoc.m_bDocumentRootNamespacing;

    assert(m_sAlias);
  }

  void XmlBaseDoc::importXmlDocs(const IXmlQueryEnvironment *pQE, const vector<const IXmlBaseDoc *> *pDocs, const IXmlBaseNode *pRootNode) {
    //caller frees pRootNode, we shallow copy it
    NOT_CURRENTLY_USED("");

    assert(pRootNode);

    XmlAdminQueryEnvironment ibqe_newDoc(this, this); //QE control of new, created document
    const IXmlBaseNode *pDocRootNode = 0;
    IXmlBaseNode *pThisRootNode;
    IXmlBaseNode *pThisDocNode = documentNode();

    if (pThisDocNode) {
      if (pThisRootNode = pThisDocNode->copyChild(&ibqe_newDoc, pRootNode, SHALLOW_CLONE)) { //(NOT_CURRENTLY_USED)
        if (pDocs) { //actually optional
          vector<const IXmlBaseDoc *>::const_iterator iDoc;
          for (iDoc = pDocs->begin(); iDoc != pDocs->end(); iDoc++) {
            pDocRootNode = (*iDoc)->rootNode(&ibqe_newDoc);
            DELETE_IF(pThisRootNode->copyChild(&ibqe_newDoc, pDocRootNode));
            delete pDocRootNode;
          }
          cacheAllPrefixedNamespaceDefinitionsForXPath();
        }
        delete pThisRootNode;
      }
      delete pThisDocNode;
    } else throw NoDocumentNodeFound(this, m_sAlias);
  }

  void XmlBaseDoc::importXmlNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pSourceNode, const bool bDeepClone) {
    //caller is responsible for deleting pSourceNode, we didnt use it, just copied it
    //beacause it is const always COPIES the data in to the new doc

    //Uses:
    //  1) factory_stylesheet() also produces a new IXmlDoc directly from the xsl:stylesheet root node
    //NOT_CURRENTLY_USED by:
    //  1) RSXL copies direct in to the Requests parent node now
    //       and requires a SHALLOW_CLONE of the input node and will thus have bDeepClone = false
    //  2) TXML creates a root node with required namespaces on first, and then copy the node in manually
    //NOTE: direct creation from pSourceNode will require that it contains all the relevant namespace declarations it needs
    //just like a <xsl:stylesheet xmlns:xsl="..."> should be
    //USE: to add a namespaced root node to the document before copyChild() the source node in:
    //  pNewDoc = xmlLibrary()->factory_document("replace-xml backup doc");
    //  pNewDocRootNode = pNewDoc->createRootChildElement("root_namespace_holder", 0, pDocumentToCopyNamespacesFrom);
    //  delete pNewDocRootNode->copyChild(pSourceNode);
    assert(pSourceNode);

    IXmlBaseNode *pNewNode  = 0,
                 *pDocNode  = 0;

    XmlAdminQueryEnvironment ibqe_newDoc(this, this); //QE control of new, created document

    if (pDocNode = documentNode()) {
      //between-document copy
      pNewNode = pDocNode->copyChild(&ibqe_newDoc, pSourceNode, bDeepClone); //this constructor ALWAYS clones, but RegularX requires SHALLOW_CLONE
    } else throw NoDocumentNodeFound(this, m_sAlias);

    copyNamespaceCacheFrom(pSourceNode->document());

    //clear up
    if (pNewNode)  delete pNewNode;
    if (pDocNode)  delete pDocNode;
  }

  XmlBaseDoc::~XmlBaseDoc() {
    //this ~destructor is CALLED a lot by the security filter
    //it's just a wrapper
    IFDEBUG(if (_STREQUAL(m_sAlias, "config")) Debug::report("freeing doc [config]", rtWarning);)

    //free pointers to served elements
    //note that some of these may be internal
    if (m_sAlias) MMO_FREE(m_sAlias);
    map_elements_free(m_mNamespaceDefinitions); //frees first AND second
  }

  //private debug function
  IFDEBUG(
    const char *XmlBaseDoc::x() const {
      const XmlAdminQueryEnvironment ibqe(mmParent(), this); //x() debug xml function
      return xml(&ibqe);
    }
  )

  const char *XmlBaseDoc::stylesheetPILink(const IXmlQueryEnvironment *pQE, const bool bAppendServerSide, const bool bAppendDebug) const {
    //<?xml-stylesheet ... ?>
    const char *sStylesheetPILink = 0,
               *sStylesheetPIAttr = 0;
    char *sStylesheetPILinkExt    = 0;
    size_t iStylesheetPILinkLen;
    const IXmlBaseNode *pPI;
    
    if (pPI = rootNode(pQE, node_type_processing_instruction, "xml-stylesheet", "stylesheetPILink()")) {
      sStylesheetPIAttr = pPI->attributeValue(pQE, "href");
    
      if (bAppendServerSide) {
        iStylesheetPILinkLen = strlen(sStylesheetPIAttr);
        sStylesheetPILinkExt = MMO_MALLOC(iStylesheetPILinkLen + 1 + 21 + 1);
        _SNPRINTF3(sStylesheetPILinkExt, iStylesheetPILinkLen + 1 + 21 + 1, 
                    "%s%c%s",
                    sStylesheetPIAttr,
                    (strchr(sStylesheetPIAttr, '?') ? '&' : '?'),
                    "server-side-xslt=true"
        );
        sStylesheetPILink = sStylesheetPILinkExt;
      } else sStylesheetPILink = sStylesheetPIAttr;

      if (bAppendDebug) {
        NOT_COMPLETE("bAppendDebug");
/*      
        iStylesheetPILinkLen = strlen(sStylesheetPIAttr);
        sStylesheetPILinkExt = MMO_MALLOC(iStylesheetPILinkLen + 1 + 21 + 1);
        _SNPRINTF3(sStylesheetPILinkExt, iStylesheetPILinkLen + 1 + 21 + 1, 
                    "%s%c%s",
                    sStylesheetPIAttr,
                    (strchr(sStylesheetPIAttr, '?') ? '&' : '?'),
                    "server-side-xslt=true"
        );
        sStylesheetPILink = sStylesheetPILinkExt;
      */
      }
    }
    
    //free up
    if (sStylesheetPIAttr && sStylesheetPIAttr != sStylesheetPILink) MMO_FREE(sStylesheetPIAttr);
    if (pPI) MMO_DELETE(pPI);
    
    return sStylesheetPILink;
  }
  
  bool XmlBaseDoc::isRootNode(const IXmlBaseNode *pNode) const {
    const XmlAdminQueryEnvironment ibqe(this, this); //isRootNode() rootNode()
    const IXmlBaseNode *pRootNode = rootNode(&ibqe);
    bool bIsRoot = false;
    if (pRootNode) bIsRoot = pNode->is(pRootNode, EXACT_MATCH);
    delete pRootNode;
    return bIsRoot;
  }
  
  XmlNodeList<IXmlBaseNode> *XmlBaseDoc::nodesWithXmlIDs(const IXmlQueryEnvironment *pQE) const {NOT_COMPLETE(""); return 0;} //returning table info is not required
  XmlNodeList<IXmlBaseNode> *XmlBaseDoc::nodesAreIDs(    const IXmlQueryEnvironment *pQE) const {NOT_COMPLETE(""); return 0;}
  XmlNodeList<IXmlBaseNode> *XmlBaseDoc::nodesAreIDRefs( const IXmlQueryEnvironment *pQE) const {NOT_COMPLETE(""); return 0;}
  
  void XmlBaseDoc::addAllStandardNamespaceDefinitionsToAppropriateRoots(IXmlBaseNode *pFromNode, const bool bIgnoreResponseNamespace) {
    IXmlBaseNode *pDocumentNode        = 0;
    XmlNodeList<IXmlHasNamespaceDefinitions> vRoots(mmParent());
    XmlNodeList<IXmlHasNamespaceDefinitions>::iterator iRoot;
    
    UNWIND_EXCEPTION_BEGIN {
      pDocumentNode = documentNode();
      findAppropriateNamespaceRoots(vRoots, pDocumentNode, pFromNode, bIgnoreResponseNamespace);
      for (iRoot = vRoots.begin(); iRoot != vRoots.end(); iRoot++) {
        (*iRoot)->addAllStandardNamespaceDefinitions();
      }
      if (!vRoots.size()) Debug::warn("no appropriate roots found to hold namespaces");
    } UNWIND_EXCEPTION_END;
    
    //free up
    //if (pDocumentNode) delete pDocumentNode; //deleted by findAppropriateNamespaceRoots()
    
    UNWIND_EXCEPTION_THROW;
  }
  
  void XmlBaseDoc::moveAllPrefixedNamespaceDefinitionsToAppropriateRoots(IXmlBaseNode *pFromNode, const bool bIgnoreResponseNamespace) {
    //transform() will have already copied the primary NS cache to the output doc, but not touched the root(s) directly
    //sanitise the nsdef on all nodes because all roots, AND gs:transform_example have ALL xmlns definitions on them
    //for example (initial XML get):
    //  <response:remove xmlns:...>           (singular removeable XML doc root)
    //    HTTP/1.0 200 OK\n...                (text nodes)
    //    <response:remove-cdata>             (top level response root: needs nsdefs)
    //      <![CDATA[<?xml-stylesheet ... />]]>
    //    </response:remove-cdata>
    //
    //    <object:Response>                   (top level response root: needs nsdefs)
    //      ...
    //      <gs:transform_result xmlns:... /> (database:query -> Database:transform() -> pOutputNode)
    //    </object:Response>
    //  </response:remove>

    //also, the xhtml:head tag is fine but all the tags inserted in to the xhtml:head:
    //  <xsl:include  xmlns:...
    //  <xhtml:meta   xmlns:...
    //  <xhtml:script xmlns:...
    //by the singular MessageInterpretation XSL stylesheet transform
    //have ALL the remaining nsdefs on them

    //removeable root nodes may be:
    //  response:remove
    //  response:remove-cdata
    //  response:wrap-cdata-sections-for-client-encoding
    //  response:remove-tag
    //children may be a selection of text(), comment() and PI()
    //also: there may not be any appropriate nodes, e.g. css
    IXmlBaseNode *pDocumentNode        = 0;
    XmlNodeList<IXmlHasNamespaceDefinitions> vRoots(mmParent());
    XmlNodeList<IXmlHasNamespaceDefinitions>::iterator iRoot;
    
    UNWIND_EXCEPTION_BEGIN {
      pDocumentNode = documentNode();
      findAppropriateNamespaceRoots(vRoots, pDocumentNode, pFromNode, bIgnoreResponseNamespace);
      for (iRoot = vRoots.begin(); iRoot != vRoots.end(); iRoot++) {
        //this is a single LibXML2 call to repoint node->ns to root->nsDefs, freeing all child node->nsDefs afterwards
        (*iRoot)->setNamespaceRoot(pFromNode);
      }
      if (!vRoots.size()) Debug::warn("no appropriate roots found to hold namespaces");
    } UNWIND_EXCEPTION_END;
    
    m_bDocumentRootNamespacing = true;
    
    //free up
    //if (pDocumentNode) delete pDocumentNode; //deleted by findAppropriateNamespaceRoots()
    
    UNWIND_EXCEPTION_THROW;
  }

  bool XmlBaseDoc::findAppropriateNamespaceRoots(XmlNodeList<IXmlHasNamespaceDefinitions>& vRoots, IXmlBaseNode *pNamespaceRootNode, IXmlBaseNode *pFromNode, const bool bIgnoreResponseNamespace) {
    //recursive
    XmlNodeList<IXmlBaseNode> *pvChildren = NULL;
    XmlNodeList<IXmlBaseNode>::iterator iChild;
    IXmlHasNamespaceDefinitions *pNamespaceRootNodeType;
    XmlAdminQueryEnvironment ibqe(this, this);
    
    UNWIND_EXCEPTION_BEGIN {
      //response: and response: cannot be namespace roots because they will be removed
      //and text() etc. cannot hold namespaces (children() should not return text() but just in case)
      if (   (pNamespaceRootNode->isNodeElement())
          && (!bIgnoreResponseNamespace || !pNamespaceRootNode->isNamespace(NAMESPACE_RESPONSE))
          && (pNamespaceRootNodeType = pNamespaceRootNode->queryInterface((IXmlHasNamespaceDefinitions*) 0))
      ) {
        vRoots.push_back(pNamespaceRootNodeType);
      } else {
        //recurse looking for a suitable namespace root
        if (pvChildren = pNamespaceRootNode->children(&ibqe)) {
          for (iChild = pvChildren->begin(); iChild != pvChildren->end(); iChild++)
            findAppropriateNamespaceRoots(vRoots, *iChild, pFromNode, bIgnoreResponseNamespace);
        }
        MMO_DELETE(pNamespaceRootNode);
      }
    } UNWIND_EXCEPTION_END;
    
    //free up
    //if (pvChildren) vector_element_delete(pvChildren); //some of these are returned
    if (pvChildren) MMO_DELETE(pvChildren);
    
    UNWIND_EXCEPTION_THROW;
    
    return vRoots.size();
  }

  bool XmlBaseDoc::documentRootNamespacing() const {
    return m_bDocumentRootNamespacing;
  }
  
  bool XmlBaseDoc::hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const {
    return childElementCount(pQE) > 1;
  }

  bool XmlBaseDoc::isEmpty(const IXmlQueryEnvironment *pQE) const {
    return childElementCount(pQE) == 0;
  }

  size_t XmlBaseDoc::childElementCount(const IXmlQueryEnvironment *pQE) const {
    bool bHasMultipleChildElements = false;
    const IXmlBaseNode *pDocumentElement;
    if (pDocumentElement = documentNode()) {
      bHasMultipleChildElements = pDocumentElement->childElementCount(pQE);
      delete pDocumentElement;
    }
    return bHasMultipleChildElements;
  }

  int XmlBaseDoc::xmlIDNextAvailable(const char *sPrefix, const unsigned int uFrom, const char *sSuffix) {
    const XmlAdminQueryEnvironment ibqe(this, this); //xmlIDNextAvailable() xml:id lookups
    char *sID;
    const char *sValue;
    size_t iLen;
    size_t iFixLen = strlen(sPrefix) + (sSuffix ? strlen(sSuffix) : 0) + 1;
    unsigned int uValue = uFrom;
    IXmlBaseNode *pNode;

    do {
      sValue = utoa(uValue);
      iLen   = iFixLen + strlen(sValue);
      sID    = MMO_MALLOC(iLen);
      _SNPRINTF3(sID, iLen, "%s%s%s", sPrefix, sValue, sSuffix);

      //check availability...
      //returns zero if not found
      //NO_TRIGGERS!
      if (pNode = nodeFromID(&ibqe, sID)) 
        delete pNode;

      MMO_FREE(sID);
      MMO_FREE(sValue);
    } while (pNode && ++uValue);

    return uValue;
  }
  
  bool XmlBaseDoc::hasNamespacePrefix(const char *sNamespacePrefix) const {
    const XmlAdminQueryEnvironment ibqe(mmParent(), this); //hasNamespacePrefix() namespace management
    bool bHasNamespace = false;
    const IXmlBaseNode *pRootNode = rootNode(&ibqe);
    if (pRootNode) {
      bHasNamespace = (m_mNamespaceDefinitions.find(sNamespacePrefix) != m_mNamespaceDefinitions.end());
      delete pRootNode;
    }
    return bHasNamespace;
  }

  void XmlBaseDoc::cacheAllPrefixedNamespaceDefinitionsForXPath(IXmlBaseNode *pFromNode, const bool bReset) {
    //TODO: move this in to LibXml level
    //TODO: maybe a xmlTraverseíNatural(fCallback) thing?
    XmlAdminQueryEnvironment ibqe(this, this); //cacheAllPrefixedNamespaceDefinitionsForXPath() namespace management
    IXmlBaseNode *pNodeToUse;
    IXmlHasNamespaceDefinitions *pNodeToUseType;
    XmlNodeList<IXmlBaseNode>::const_iterator iChildNode;
    XmlNodeList<IXmlBaseNode> *pChildren;
    const StringMap<const char*> *pNamespaceDefinitions;
    StringMap<const char*>::const_iterator iNamespaceDefinition, iExistingNamespaceDefinition;
    const char *sPrefix, *sHREF;
    bool bIsTopLevelCacheRequest;

    //optionally reset
    //note that parts of the document can be re-analysed
    if (bReset) m_mNamespaceDefinitions.clear();

    //navigate DOWN entire tree looking for prefixed namespace declerations
    //we use documentNode() here because it does not require an XPath query 
    bIsTopLevelCacheRequest = !pFromNode;
    pNodeToUse     = bIsTopLevelCacheRequest ? documentNode() : pFromNode;
    pNodeToUseType = pNodeToUse->queryInterface((IXmlHasNamespaceDefinitions*) 0);
    m_bNamespaceDefinitionsCached = true; //here because we will make children requests

    if (!pNodeToUse->isHardLink()) {
      //include standard namespaces directly
      addPrefixedNamespaceDefinition("xml",          NAMESPACE_XML);
      addPrefixedNamespaceDefinition("xmlsecurity",  NAMESPACE_XMLSECURITY);
      addPrefixedNamespaceDefinition("gs",           NAMESPACE_GS);
      
      //add these namespace definitions
      //they are held as strings because maintaining node links would be problematic
      //and because they need to be unique in this map, multiple nodes may define the same nsDef
      //using a zero potential return value for speed if nsDef is zero
      if (pNodeToUseType) {
        if (pNamespaceDefinitions = pNodeToUseType->namespaceDefinitions()) { //contents are MM_STRDUP() already
          for (iNamespaceDefinition = pNamespaceDefinitions->begin(); iNamespaceDefinition != pNamespaceDefinitions->end(); iNamespaceDefinition++) {
            //everything is already MM_STRDUP() and we free everything in our ~destructor
            //unless we dont add it here of course
            sPrefix = iNamespaceDefinition->first;
            sHREF   = iNamespaceDefinition->second;

            if (strlen(sPrefix)) { //dont need xmlns defaults, and anyway they will always be multiple
              iExistingNamespaceDefinition = m_mNamespaceDefinitions.find(sPrefix);
              if (iExistingNamespaceDefinition == m_mNamespaceDefinitions.end()) {
                //new document namespace association
                addPrefixedNamespaceDefinition(sPrefix, sHREF);
              } else {
                //we dont allow one prefix to point to two different namespaces within the same document
                //  even though this is technically valid in XML
                //because it fucks up the XPATH namespace registration for queries
                //  if it cant use distinct meaningful prefixes in the xpath query
                if (strcmp(iExistingNamespaceDefinition->second, sHREF)) {
                  //same prefix, different namespace
                  const char *sPath = pNodeToUse->uniqueXPathToNode(&ibqe);
                  throw MismatchedNamespacePrefixAssociation(this, sPrefix, sHREF, sPath);
                }
              }
            }
          }

          map_const_elements_free(pNamespaceDefinitions);
        }
      }

      //traverse children (XML_NODE_ELEMENT)
      if (!pNodeToUse->isSoftLink()) {
        pChildren = pNodeToUse->children(&ibqe, "cacheAllPrefixedNamespaceDefinitionsForXPath()");
        for (iChildNode = pChildren->begin(); iChildNode != pChildren->end(); iChildNode++) {
          cacheAllPrefixedNamespaceDefinitionsForXPath(*iChildNode, NO_RESET_CACHE);
          delete *iChildNode;
        }
        delete pChildren;
      }
    }

    //clean up
    if (bIsTopLevelCacheRequest) {
      //indicates !pFromNode
      //only in the case of using an auto root-node
      delete pNodeToUse;
    }
  }

  void XmlBaseDoc::copyNamespaceCacheFrom(const IXmlBaseDoc *pSourceDoc) {
    const StringMap<const char*> *pNamespaceDefinitions;
    StringMap<const char*>::const_iterator iNamespaceDefinition;

    if (pNamespaceDefinitions = pSourceDoc->allPrefixedNamespaceDefinitions()) {
      for (iNamespaceDefinition = pNamespaceDefinitions->begin(); iNamespaceDefinition != pNamespaceDefinitions->end(); iNamespaceDefinition++) {
        addPrefixedNamespaceDefinition(iNamespaceDefinition->first, iNamespaceDefinition->second);
      }
      m_bNamespaceDefinitionsCached = true;
    }
  }

  void XmlBaseDoc::ensurePrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF) {
    //caller frees inputs
    //they are STRDUP by addPrefixedNamespaceDefinition()
    if (!hasNamespacePrefix(sPrefix)) addPrefixedNamespaceDefinition(sPrefix, sHREF);
    m_bNamespaceDefinitionsCached = true;
  }

  void XmlBaseDoc::addPrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF) {
    //caller frees inputs
    m_mNamespaceDefinitions.insert(MM_STRDUP(sPrefix), MM_STRDUP(sHREF));
  }

  const char *XmlBaseDoc::standardPrefix(const char *sHREF) const {
    //DO NOT free return!
    //caller frees inputs
    StringMap<const char*>::const_iterator i;
    const char *sPrefix = 0;
    
    if (_STREQUAL(NAMESPACE_XML, sHREF)) return "xml";

    for (i = m_mNamespaceDefinitions.begin(); !sPrefix && i != m_mNamespaceDefinitions.end(); i++) {
      if (_STREQUAL(i->second, sHREF)) sPrefix = i->first;
    }

    return sPrefix;
  }

  bool XmlBaseDoc::clearCachedStylesheetsFor(IXmlBaseNode *pDecendentNode ATTRIBUTE_UNUSED) {
    //TODO: return pDecendentNode->clearMyCachedStylesheets();
    NOT_COMPLETE("clearCachedStylesheetsFor");
    return false;
  }

  IXmlBaseDoc *XmlBaseDoc::transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner,
      const IXslDoc *pXslStylesheet,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
      const char *sWithMode
  ) const {
    IXmlBaseDoc *pResultDoc = 0;
    const IXmlBaseNode *pXslStylesheetNode = 0;
    if (pXslStylesheetNode = pXslStylesheet->rootNode(pQE)) 
      pResultDoc = transform(pMemoryLifetimeOwner, pXslStylesheetNode->queryInterface((const IXslStylesheetNode*) 0), pQE,
        pParamsInt, pParamsChar, pParamsNodeSet,
        sWithMode
      );
    
    //free up
    if (pXslStylesheetNode) delete pXslStylesheetNode;
    
    return pResultDoc;
  }
   
  IXmlBaseDoc *XmlBaseDoc::transform(
    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode
  ) const {
    IXmlBaseDoc *pOutputDoc = 0;
    const IXmlBaseNode *pDocumentNode = documentNode();

    if (pDocumentNode) {
      pOutputDoc = pDocumentNode->transform(
        pMemoryLifetimeOwner, pXslStylesheetNode, pQE,
        pParamsInt, pParamsChar, pParamsNodeSet, sWithMode
      );
      delete pDocumentNode;
    }

    return pOutputDoc;
  }

  void XmlBaseDoc::transform(
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    IXmlBaseNode *pOutputNode,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode
  ) {
    IXmlBaseNode *pDocumentNode = documentNode();

    if (pDocumentNode) {
      pDocumentNode->transform(
        pXslStylesheetNode, pQE, pOutputNode,
        pParamsInt, pParamsChar, pParamsNodeSet, sWithMode
      );
      delete pDocumentNode;
    }
  }

  const char *XmlBaseDoc::toString() const {
    const XmlAdminQueryEnvironment ibqe(mmParent(), this); //toString() debug server output
    stringstream sOut;
    const StringMap<const char*> *pNamespaceDefinitions;
    StringMap<const char*>::const_iterator iNamespaceDefinition;
    const char *sDescription;
    
    sOut << "XMLdocument [" << m_sAlias << "]\n";
    /*
    const char *sXML = xml(&ibqe);
    size_t iMainDocSize;
    iMainDocSize = sXML ? strlen(sXML) : 0;
    sOut << " [" << (int) (iMainDocSize / (1024*1024)) << "] Mb (string XML size)\n";
    if (sXML) MMO_FREE(sXML);
    */

    if (pNamespaceDefinitions = allPrefixedNamespaceDefinitions()) {
      for (iNamespaceDefinition = pNamespaceDefinitions->begin(); iNamespaceDefinition != pNamespaceDefinitions->end(); iNamespaceDefinition++) {
        sOut << "  xmlns:" << iNamespaceDefinition->first << " = " << iNamespaceDefinition->second;
        //TODO: output C++ Class handler, description, etc.
        sOut << "\n";
      }
    }

    return MM_STRDUP(sOut.str().c_str());
  }

  IXmlBaseNode *XmlBaseDoc::nodeFromID(const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const XmlBaseDoc*) this)->nodeFromID(pQE, sID, sLockingReason);
  }
  
  IXmlBaseNode *XmlBaseDoc::rootNode(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode) {
    NOT_CURRENTLY_USED("");
    //called only from XmlBaseDoc_construct(const IXmlQueryEnvironment *pQE, const char *sAlias, IXmlBaseNode *pSourceNode)
    //which is also NOT_CURRENTLY_USED

    //caller manages pNode
    //caller DOES NOT free result because pNode = pNewNode
    IXmlBaseNode *pNewNode = 0;
    IXmlBaseNode *pDocNode = documentNode();
    if (pDocNode) {
      pNewNode = pDocNode->moveChild(pQE, pNode);
      delete pDocNode;
    }
    return pNewNode;
  }

  IXmlBaseNode *XmlBaseDoc::createRootChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const IXmlBaseDoc *pNamespaceDoc, const char *sContent, const char *sNamespace) {
    //used by TXml internal doc
    //need to use the root node cause thats where the namespaces are
    IXmlBaseNode *pDocNode, *pNewNode;
    IXmlHasNamespaceDefinitions *pNewNodeType;
    IXmlNamespaced *pNewNodeType2;
    
    assert(pNamespaceDoc);

    if (pDocNode = documentNode()) {
      //this will actually trigger LibXml2::setRootElement(...)
      //because pDocNode, the parent, is the XML_DOC_ELEMENT
      //prevent this from being used on an already populated document
      //  because we are doing it in admin mode
      
      //TODO: move this to the factory_document call?
      copyNamespaceCacheFrom(pNamespaceDoc); //needed for isEmpty(pQE)
      
      if (!isEmpty(pQE)) throw MultipleRootNodes(this, m_sAlias);
      pNewNode  = pDocNode->createChildElement(pQE, sName, sContent, sNamespace);
      delete pDocNode;

      //namespaces on the root node
      if (pNewNode) {
        pNewNodeType = pNewNode->queryInterface((IXmlHasNamespaceDefinitions*) 0);

        //copy namespaces from the document sent through
        pNewNodeType->addNamespaceDefinitions(pNamespaceDoc->allPrefixedNamespaceDefinitions());

        //place the standard ones on the root node
        //which are not already included in the pNamespaceDoc
        pNewNodeType->addAllStandardNamespaceDefinitions();

        //set default namespace and namespace
        pNewNodeType->addDefaultNamespaceDefinition(NAMESPACE_GS);
        if (pNewNodeType2 = pNewNode->queryInterface((IXmlNamespaced*) 0)) {
          //will search for a valid namespace definition up the tree
          //or set the default namespace if not found
          pNewNodeType2->setNamespace(NAMESPACE_GS);
        } else throw InterfaceNotSupported(this, MM_STRDUP("IXmlNamespaced*"));
        
        m_bNamespaceDefinitionsCached = true;
      }
    } else throw NoDocumentNodeFound(this, m_sAlias);

    return pNewNode;
  }

  IXmlBaseNode *XmlBaseDoc::createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace, const char *sReason) {
    IXmlBaseNode *pRootNode, *pNewNode = 0;

    if (pRootNode = rootNode(pQE)) {
      pNewNode = pRootNode->createChildElement(pQE, sName, sContent, sNamespace, sReason);
    } else throw NoDocumentNodeFound(this, m_sAlias);

    return pNewNode;
  }

  IXmlBaseNode *XmlBaseDoc::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint) {
    return (IXmlBaseNode*) ((const IXmlBaseDoc*) this)->getSingleNode(pQE, sXPath, argchar, argint);
  }

  const IXmlBaseNode *XmlBaseDoc::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint) const {
    //caller deletes result
    const IXmlBaseNode *pNode = 0;
    const IXmlBaseNode *pDocumentNode = documentNode();
    if (pDocumentNode) {
      pNode = pDocumentNode->getSingleNode(pQE, sXPath, argchar, argint);
      delete pDocumentNode;
    }

    return pNode;
  }

  XmlNodeList<IXmlBaseNode> *XmlBaseDoc::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) {
    return (XmlNodeList<IXmlBaseNode>*) ((const IXmlBaseDoc*) this)->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason);
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseDoc::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) const {
    //caller deletes result
    XmlNodeList<const IXmlBaseNode> *pNodes = 0;
    const IXmlBaseNode *pDocumentNode = documentNode();
    if (pDocumentNode) {
      pNodes = pDocumentNode->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason);
      delete pDocumentNode;
    }

    return pNodes;
  }

  IXmlBaseNode *XmlBaseDoc::rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const IXmlBaseDoc*) this)->rootNode(pQE, iNodeTypes, sName, sLockingReason);
  }

  const IXmlBaseNode *XmlBaseDoc::rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) const {
    //caller deletes result
    //see commenting in the header
    const IXmlBaseNode *pNode = 0;
    const IXmlBaseNode *pDocumentNode = documentNode("retrieve root node"); //= XmlDoc node
    if (pDocumentNode) {
      pNode = pDocumentNode->firstChild(pQE, iNodeTypes, sName, sLockingReason); //platform specific implementation
      delete pDocumentNode;
    }

    return pNode;
  }

  const IXmlLibrary *XmlBaseDoc::xmlLibrary()    const {return m_pLib;}

  const char *XmlBaseDoc::xml(const IXmlQueryEnvironment *pQE, const bool bNoEscape) const {
    //re-root to node
    const char *sXML = 0;
    const IXmlBaseNode *pDocNode = documentNode();
    if (pDocNode) {
      sXML = pDocNode->xml(pQE, 0, bNoEscape);
      delete pDocNode;
    }

    return sXML;
  }

  bool XmlBaseDoc::removeProcessingInstructions() {
    XmlAdminQueryEnvironment ibqe(this, this); //removeProcessingInstructions() rootNode()
    bool bRemoved = false;
    IXmlBaseNode *pNode;

    //note that, for this to work: rootNode() must return processing instructions
    //rootNode() calls XmlDoc->firstChild()
    if (pNode = rootNode(&ibqe, node_type_processing_instruction)) {
      bRemoved = true;
      pNode->removeAndDestroyNode(&ibqe);
      delete pNode;
    }

    return bRemoved;
  }
  
  const IXmlBaseDoc    *XmlBaseDoc::queryInterface(const IXmlBaseDoc *p ATTRIBUTE_UNUSED) const {throw InterfaceNotSupported(this, MM_STRDUP("const IXmlBaseDoc *")); return 0;}
  IXmlBaseDoc          *XmlBaseDoc::queryInterface(IXmlBaseDoc *p ATTRIBUTE_UNUSED)             {throw InterfaceNotSupported(this, MM_STRDUP("IXmlBaseDoc *")); return 0;}
  IXslDoc              *XmlBaseDoc::queryInterface(IXslDoc *p ATTRIBUTE_UNUSED)                 {throw InterfaceNotSupported(this, MM_STRDUP("IXslDoc *")); return 0;}
  const IXslDoc        *XmlBaseDoc::queryInterface(const IXslDoc *p ATTRIBUTE_UNUSED) const     {throw InterfaceNotSupported(this, MM_STRDUP("IXslDoc *")); return 0;}
}
