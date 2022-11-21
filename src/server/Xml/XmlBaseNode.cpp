//platform independent
#include "Xml/XmlBaseNode.h"

#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlNodeMask.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlMaskContext.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "Xml/XmlNamespace.h"
#include "Xml/XmlBaseDoc.h"
#include "Iterators.h"
#include "Server.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"
#include "Utilities/strtools.h"
#include "Utilities/container.c"

#include "Xml/XmlNodeList.h"

#include <stdarg.h>
#include <sstream>
using namespace std;

namespace general_server {
  XmlBaseNode::XmlBaseNode():
    MemoryLifetimeOwner(0),
    m_pDoc(0), 
    m_bDocOurs(false)
  {
    //only used in cloning. we need so we can indicate the type to clone into
  } 
  
  XmlBaseNode::XmlBaseNode(const XmlBaseNode &node):
    MemoryLifetimeOwner(node.mmParent()), //addChild()
    m_pDoc(node.m_pDoc),
    m_bDocOurs(false)
  {
    assert(!node.m_bDocOurs); //we do not want to copy construct and have to copy the doc also
  }

  XmlBaseNode::XmlBaseNode(IXmlBaseDoc *pDoc, const bool bDocOurs): 
    MemoryLifetimeOwner(pDoc),
    m_pDoc(pDoc),
    m_bDocOurs(bDocOurs)
  {
    assert(pDoc);
  }

  XmlBaseNode::~XmlBaseNode() {
    if (m_pDoc && m_bDocOurs) MMO_DELETE(m_pDoc);
  }

  const char *XmlBaseNode::toString() const {
    //ADMINISTRATION_FUNCTION_ONLY;
    const XmlAdminQueryEnvironment ibqe_tostring(mmParent(), m_pDoc);
    stringstream sOut;
    const char *sU = uniqueXPathToNode(&ibqe_tostring, NO_BASE_NODE, INCLUDE_TRANSIENT);
    sOut << "[" << typeName() << "] Node: [" << sU << "]";
    MM_FREE(sU);
    return MM_STRDUP(sOut.str().c_str());
  }

  const IXmlLibrary *XmlBaseNode::xmlLibrary() const {return m_pDoc->xmlLibrary();}

  const IXslStylesheetNode *XmlBaseNode::relativeXSLIncludeFileSystemPathToXSLStylesheetNode(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath) const {
    //used by all xsl:include @href resolvers
    //will throw an error if this xsl:include @href is not relative
    //all xsl:include @href must be relative because they need to work on server side also
    const XmlNodeList<const IXmlBaseNode> *pTargetNodes  = 0;
    const IXmlBaseNode *pRelativeXSLStylesheetNode = 0;

    pTargetNodes = fileSystemPathToNodeList(pQE, sFileSystemPath, REJECT_ABSOLUTE, "xsl:stylesheet");
    if (pTargetNodes && pTargetNodes->size() == 1) pRelativeXSLStylesheetNode = *pTargetNodes->begin();
    else pTargetNodes->element_destroy();

    delete pTargetNodes;
    return pRelativeXSLStylesheetNode ? pRelativeXSLStylesheetNode->queryInterface((const IXslStylesheetNode*) 0) : NULL;
  }

  bool XmlBaseNode::deregistrationNotify() {
    //we come here when our m_oNode pointer will be deleted from under us
    //this can happen for example when something deletes the node in the LibXML2 document 
    //  through another route than the owner of this node
    Debug::report("xmlDeregisterNodeFunc(%s)", localName(NO_DUPLICATE));
    return false;
  }

  IXmlBaseDoc *XmlBaseNode::document() {return m_pDoc;}
  const IXmlBaseDoc *XmlBaseNode::document() const {return m_pDoc;}

  iErrorPolicy XmlBaseNode::errorPolicy() const {
    const XmlAdminQueryEnvironment ibqe(mmParent(), document()); //errorPolicy() attributeValue()
    const char *sErrorPolicy = attributeValue(&ibqe, "error-policy", NAMESPACE_XSL);
    iErrorPolicy er = throw_error;

    if (sErrorPolicy && *sErrorPolicy) {
      if (!strcmp(sErrorPolicy, "continue")) er = continue_onerror;
    }

    return er;
  }

  const char *XmlBaseNode::uniqueXPathToNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode, const bool bWarnIfTransient, const bool bForceBaseNode, const bool bEnableIDs, const bool bEnableNameAxis, const bool bThrowIfNotBasePath) const {
    //caller frees result
    //TODO: LibXML library strategy instead?
    //TXml uses this to store serialised relative paths to nodes
    //xmlGetNodePath() does a similar thing: http://xmlsoft.org/html/libxml-tree.html#xmlGetNodePath
    //  produces: /*/object:Server/repository:databases/object:db/*[2]/*[4]/*[3]
    //but we want something different:
    //  /object:Server/repository:databases/object:db/gs:test[2]/gs:this[4]/gs:that[3]"
    //because xmlGetNodePath() does not resolve the best prefix fit in the case of default, no prefix namespace nodes
    //  it simply uses *[x] instead
    stringstream ssXPath;
    const char *sXPath = 0;
    bool bOldEnabled   = false;
    
    if (bWarnIfTransient && isTransient())
      throw TransientNode(this, MM_STRDUP("uniqueXPathToNode()"), this);
    
    //disable the node-mask so we can access the ancestors
    if (pQE->maskContext()) bOldEnabled =  pQE->maskContext()->disable(); 
    {
      //------------------------------------- initial attempt
      try {
        uniqueXPathToNodeInternal(pQE, this, pBaseNode, ssXPath, bWarnIfTransient, bForceBaseNode, bEnableIDs, bEnableNameAxis && xmlLibrary()->hasNameAxis(), xmlLibrary()->hasDefaultNameAxis(), false);
      }
      catch (IntendedAncestorBaseNodeNotTraversed &eb1) {
        if (bForceBaseNode) throw; //no route was found to traverse up through the base node
        else {
          
          //------------------------------------- force base find attempt
          //try the traverse again with FORCE_BASE_NODE
          //let it throw if we cannot find it
          //no resources allocated above so we throw without UNWIND_EXCEPTION
          ssXPath.str(std::string());
          try {
            uniqueXPathToNodeInternal(pQE, this, pBaseNode, ssXPath, bWarnIfTransient, FORCE_BASE_NODE, bEnableIDs, bEnableNameAxis && xmlLibrary()->hasNameAxis(), xmlLibrary()->hasDefaultNameAxis(), false);
          } catch (IntendedAncestorBaseNodeNotTraversed &eb2) {
            //finally give up finding the base-node
            if (bThrowIfNotBasePath) throw;
          }
        }
      }
      sXPath = MM_STRDUP(ssXPath.str().c_str());
      
      //-------------------------------------- debug
      IFDEBUG(
        //check uniqueness
        //TODO: why are the object:Request paths not unique?
        const XmlNodeList<const IXmlBaseNode> *plDuplicates = 0;
        if (!strstr(sXPath, "object:Request")) {
          if (pBaseNode) plDuplicates = pBaseNode->getMultipleNodes( pQE, sXPath);
          else           plDuplicates = document()->getMultipleNodes(pQE, sXPath);
          if (plDuplicates->size() != 1) { 
            if (!_STREQUAL(document()->alias(), "config"))
              Debug::warn("uniqueXPathToNode() failed but not the config doc! is [%s]", document()->alias());
            else {
              if (plDuplicates->size() == 0) 
                //throw XPathStringNot1Nodes(MM_STRDUP("uniqueXPathToNode() returned not 1 nodes"), pBaseNode, sXPath);
                Debug::warn("uniqueXPathToNode(%s %s) returned 0 nodes", (pBaseNode ? pBaseNode->localName(NO_DUPLICATE) : ""), sXPath);
              else if (plDuplicates->size() > 1)
                Debug::warn("uniqueXPathToNode(%s %s) returned more than 1 nodes", (pBaseNode ? pBaseNode->localName(NO_DUPLICATE) : ""), sXPath);
            }
          }
          delete plDuplicates->element_destroy();
        }
        if (pBaseNode && strstr(sXPath, "config/")) 
          Debug::warn("base-node but final path contains config [%s]:", sXPath);
      )
    } 
    if (bOldEnabled && pQE->maskContext()) pQE->maskContext()->enable();
    
    return sXPath;
  }

  bool XmlBaseNode::uniqueXPathToNodeInternal(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const IXmlBaseNode *pBaseNode, stringstream &ssXPath, const bool bWarnIfTransient, const bool bForceBaseNode, const bool bEnableIDs, const bool bEnableNameAxis, const bool bHasDefaultNameAxis, const bool bDebug) const {
    //relatively expensive function...
    //uses attributes that uniquely identify nodes, e.g. xsl:template @match @mode
    //starts at node and climbs up to doc
    //GDB: p *((LibXmlBaseNode*)pCurrentNode)->m_oNode->parent
    const IXmlBaseNode *pParentNode = 0;
    const char *sQualifiedName  = 0,
               *sXmlID          = 0,
               *sAxisName       = 0;
    const char *sIDValue1       = 0,
               *sIDName1        = 0,
               *sIDValue2       = 0,
               *sIDName2        = 0,
               *sIDValue1Escaped = 0,
               *sIDValue2Escaped = 0;
    const XmlNodeList<const IXmlBaseNode> *plDuplicates = 0;
    bool bNeedsSeperator = true;
    stringstream ssStep;

    //-------------------------------- finish at this node
    //all these situations can finish at this node, e.g. @xml:id
    if (pCurrentNode->is(pBaseNode, EXACT_MATCH)) {
      //we have reached the base node, exit
      //xpath realtive to the base does not start with a / root indicator
      //xpath from the root, starts with the root indicator /
      //comes here also if this is hardlinked
      if (bDebug) Debug::report("  base-node");
      bNeedsSeperator = false;
    } 
    else {
      if (pCurrentNode->isNodeDocument()) {
        //this is the root node
        //return and output the root marker as the separator
        if (bDebug)    Debug::report("  #DOCUMENT");
        ssStep << "/";
        bNeedsSeperator = false;
        //no resources allocated here yet, we can throw without UNWIND_EXCEPTION
        if (pBaseNode && !pBaseNode->isNodeDocument()) throw IntendedAncestorBaseNodeNotTraversed(mmParent(), "uniqueXPathToNode()", this, pBaseNode);
      }
      
      else if (bEnableIDs 
        && pCurrentNode->isNodeElement() 
        && (sXmlID = pCurrentNode->xmlID(pQE)) 
        && *sXmlID
      ) {
        if (pCurrentNode->isNamespace(NAMESPACE_CLASS) && strlen(sXmlID) > 7) {
          //class:*/@xml:id MUST be in the format Class__<classname>
          if (bDebug) Debug::report("  Class [%s]", sXmlID);
          ssStep << "~" << (sXmlID + 7);
        } else {
          //xml:id is unique and we can start from the id(xml_id)/... function
          //we do not recurse parents in this case
          if (bDebug) Debug::report("  @xml:id [%s]", sXmlID);
          ssStep << "^" << sXmlID;
        }
      } 
      
      //----------------------------------- continue up ancestors
      //up to the parent first to get the string in the right order
      else {
        if (pParentNode = pCurrentNode->parentNode(pQE, "uniqueXPathToNodeInternal()")) {
          //When parent_route is not available but a specific xpath base-node is requested:
          //e.g. XSL literals like <interfae:AJAXHTMLLoader 
          //  @meta:xpath-to-node-client=database:xpath-to-node($gs_website_root)
          //because the XSL literal does not have an understanding of the current instruction parent_route
          //normally: the obeyed parent_route should pass the desired base-node anyway
          if (bForceBaseNode && pParentNode->isHardLinked() && pBaseNode) {
            //PERFORMANCE: hardlinkRouteToAncestor()
            //eek: we need to make sure we take the correct parent path to the stated base node
            const IXmlBaseNode *pBranchNode = pParentNode->hardlinkRouteToAncestor(pBaseNode, "uniqueXPathToNodeInternal()");
            if (pBranchNode) {
              if (!pBranchNode->is(pParentNode, false, false)) { //no hardlink or parent_route check
                //Debug::report("uniqueXPathToNode(FORCE_BASE_NODE) parent strafe forced because of hardlinkRouteToAncestor()");
                delete pParentNode;
                pParentNode = pBranchNode;
              } else 
                delete pBranchNode;
            }
          }

          //returns immediately if parent is zero
          if (uniqueXPathToNodeInternal(pQE, pParentNode, pBaseNode, ssXPath, bWarnIfTransient, bForceBaseNode, bEnableIDs, bEnableNameAxis, bHasDefaultNameAxis, bDebug)) {
            //the parent node (base node, document node or otherwise) has indicated that a separator is required
            ssXPath << "/";
          }
        } else {
          //if the parent is blank and this is NOT the document node then it is a disconnected node
          //or Security was denied
          //or there is a structural problem
          //GDB: p *((LibXmlBaseNode*)pCurrentNode)->m_oNode->parent
          throw OrphanNode(this, MM_STRDUP("xpath-to-node unlinked without @xml:id"), pCurrentNode);
        }

        //----------------------------------- output this main node
        //name axis
        //xmlLibrary()->hasNameAxis() taken in to account by caller
        switch (pCurrentNode->nodeType()) {
          //-------------------------------------------------------------------
          case NODE_TEXT: {
            //PERFORMANCE: bad, because of multiple text() possibility
            //TODO: better faster text() handling LibXML2 functions
            if (bDebug) Debug::report("  NODE_TEXT");
            plDuplicates = pCurrentNode->getMultipleNodes(pQE, "preceding-sibling::text()");
            ssStep << "text()" << "[" << (plDuplicates->size() + 1) << "]";
            break;
          }
          //-------------------------------------------------------------------
          case NODE_ATTRIBUTE: {
            //node names are unique unlike element names
            sQualifiedName = pCurrentNode->fullyQualifiedName();
            if (bDebug) Debug::report("  NODE_ATTRIBUTE [%s]", sQualifiedName);
            if (pCurrentNode->isNodeAttribute()) ssStep << "@" << sQualifiedName;
            break;
          }
          //-------------------------------------------------------------------
          case NODE_ELEMENT: {   
            if ( bEnableNameAxis 
              && (sAxisName = pCurrentNode->nameAxisName(pQE)) 
              && *sAxisName
            ) {
              if (bDebug) Debug::report("  NODE_ELEMENT [%s] name axis", sQualifiedName);
              if (!bHasDefaultNameAxis) ssStep << "name::";
              ssStep << sAxisName;
            } else {
              sQualifiedName = pCurrentNode->fullyQualifiedName();
              if (bDebug) Debug::report("  NODE_ELEMENT [%s]", sQualifiedName);
              ssStep << sQualifiedName;

              //look for extra identity in case extra same name nodes are added
              //XSL aware
              if (pCurrentNode->isNamespace(NAMESPACE_XSL)) {
                if (_STREQUAL(sQualifiedName, "xsl:template")) {
                  sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "match");
                  sIDValue2 = pCurrentNode->attributeValue(pQE, sIDName2 = "mode");
                }
                else if (_STREQUAL(sQualifiedName, "xsl:apply-templates")) {
                  sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "select");
                  sIDValue2 = pCurrentNode->attributeValue(pQE, sIDName2 = "mode");
                }
                else if (_STREQUAL(sQualifiedName, "xsl:variable")) sIDValue1 = attributeValue(pQE, sIDName1 = "name");
                else if (_STREQUAL(sQualifiedName, "xsl:include")) {
                  sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "xpath");
                  sIDValue2 = pCurrentNode->attributeValue(pQE, sIDName2 = "href");
                }
              }

              //generic
              if ( !sIDValue1 //(from specials section above)
                && !(sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "xml:id"))
                && !(sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "guid"))
                && !(sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "unique-id"))
                && !(sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "unique-name"))
                //&& !(sIDValue1 = pCurrentNode->attributeValue(pQE, sIDName1 = "name")) //name::axis should already catch this
              ) {
                //we have no explicit identity attributes
                //so we need to check if there are other siblings with the same name
                //this is a problem if more are insterted in the mean time at the destination
                //but we can only add a position predicate (zero based array)
                //PERFORMANCE: bad
                plDuplicates = pCurrentNode->getMultipleNodes(pQE, "preceding-sibling::%s", sQualifiedName);
                ssStep << "[" << (plDuplicates->size() + 1) << "]";
              } else {
                //we have at least one sibling level unique identity value, so rely on it
                //namespace:name[@x=y]
                ssStep << "[";
                if (sIDValue1) {
                  sIDValue1Escaped = xmlLibrary()->escape(sIDValue1);
                  ssStep << "@" << sIDName1 << "='" << sIDValue1Escaped << "'";
                } else           ssStep << "not(@" << sIDName1 << ")";
                if (sIDName2) {
                  ssStep << " and ";
                  if (sIDValue2) {
                    sIDValue2Escaped = xmlLibrary()->escape(sIDValue2);
                    ssStep << "@" << sIDName2 << "='" << sIDValue2Escaped << "'";
                  } else           ssStep << "not(@" << sIDName2 << ")";
                }
                ssStep << "]";
              }
            }
            break;
          }
          default: throw XPathTypeUnhandled(this, pCurrentNode->nodeType());
        }
      }
      
      //check required step content
      IFDEBUG(  
        if (!ssStep.str().size()) 
          Debug::report("uniqueXPathToNode() step is '\\0' for [%s] on step [%s]", ssXPath.str().c_str(), pCurrentNode->fullyQualifiedName());
      );
      ssXPath << ssStep.str();
    } //pCurrentNode->is(pBaseNode, EXACT_MATCH) no step content

    if (bDebug) Debug::report("  [%s]", ssXPath.str().c_str());

    //free up
    if (sXmlID)          MM_FREE(sXmlID);
    if (sAxisName)       MM_FREE(sAxisName);
    if (sQualifiedName)  MM_FREE(sQualifiedName);
    if (sIDValue1)       MM_FREE(sIDValue1);
    if (sIDValue2)       MM_FREE(sIDValue2);
    if (sIDValue1Escaped && sIDValue1Escaped != sIDValue1) MM_FREE(sIDValue1Escaped);
    if (sIDValue2Escaped && sIDValue2Escaped != sIDValue2) MM_FREE(sIDValue2Escaped);
    if (plDuplicates)    delete plDuplicates->element_destroy();
    if (pParentNode)     delete pParentNode;

    return bNeedsSeperator;
  }

  void XmlBaseNode::addDefaultNamespaceDefinition(const char *sHREF) {
    addNamespaceDefinition(0, sHREF);
  }

  const char *XmlBaseNode::group(const IXmlQueryEnvironment *pQE, const bool bDuplicate) const {
    const IXmlBaseNode *pParentNode = parentNode(pQE);
    const char *sGroupName = (pParentNode ? pParentNode->localName(bDuplicate) : NULL);
    if (pParentNode) delete pParentNode;
    return sGroupName;
  }
  
  const char *XmlBaseNode::typeName() const {
    const char *sRet = 0;
    switch (nodeType()) {
      case NODE_INVALID: sRet = "NODE_INVALID"; break;
      case NODE_ELEMENT: sRet = "NODE_ELEMENT"; break;
      case NODE_ATTRIBUTE: sRet = "NODE_ATTRIBUTE"; break;
      case NODE_TEXT: sRet = "NODE_TEXT"; break;
      case NODE_CDATA_SECTION: sRet = "NODE_CDATA_SECTION"; break;
      case NODE_ENTITY_REFERENCE: sRet = "NODE_ENTITY_REFERENCE"; break;
      case NODE_ENTITY: sRet = "NODE_ENTITY"; break;
      case NODE_PROCESSING_INSTRUCTION: sRet = "NODE_PROCESSING_INSTRUCTION"; break;

      case NODE_COMMENT: sRet = "NODE_COMMENT"; break;
      case NODE_DOCUMENT: sRet = "NODE_DOCUMENT"; break;
      case NODE_DOCUMENT_TYPE: sRet = "NODE_DOCUMENT_TYPE"; break;
      case NODE_DOCUMENT_FRAGMENT: sRet = "NODE_DOCUMENT_FRAGMENT"; break;
      case NODE_NOTATION: sRet = "NODE_NOTATION"; break;
      case NODE_HTML_DOCUMENT_NODE: sRet = "NODE_HTML_DOCUMENT_NODE"; break;
      case NODE_DTD_NODE: sRet = "NODE_DTD_NODE"; break;
      case NODE_ELEMENT_DECL: sRet = "NODE_ELEMENT_DECL"; break;
      case NODE_ATTRIBUTE_DECL: sRet = "NODE_ATTRIBUTE_DECL"; break;
      case NODE_ENTITY_DECL: sRet = "NODE_ENTITY_DECL"; break;
      case NODE_NAMESPACE_DECL: sRet = "NODE_NAMESPACE_DECL"; break;
      case NODE_XINCLUDE_START: sRet = "NODE_XINCLUDE_START"; break;
      case NODE_XINCLUDE_END: sRet = "NODE_XINCLUDE_END"; break;
      case NODE_DOCB_DOCUMENT_NODE: sRet = "NODE_DOCB_DOCUMENT_NODE"; break;
      default: sRet = "UNKNOWN";
    }

    return sRet;
  }

  void XmlBaseNode::appendAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sSeperator)  const {
    size_t iExistingValueLen, iValueLen, iSeperatorLen, iTotalLen;
    char *sNewValue;
    const char *sExistingValue;
    
    if (sName && *sName && sValue && *sValue) {
      sExistingValue = attributeValue(pQE, sName);
      if (sExistingValue && *sExistingValue) {
        iExistingValueLen = strlen(sExistingValue);
        iValueLen         = strlen(sValue);
        iSeperatorLen     = (sSeperator ? strlen(sSeperator) : 0);
        iTotalLen         = iExistingValueLen + iSeperatorLen + iValueLen + 1;
        sNewValue         = MMO_MALLOC(iTotalLen);
        
        strcpy(sNewValue, sExistingValue);
        if (sSeperator) strcpy(sNewValue + iExistingValueLen, sSeperator);
        strcpy(sNewValue + iExistingValueLen + iSeperatorLen, sValue);
        sNewValue[iTotalLen-1] = '\0';
        
        setAttribute(pQE, sName, sNewValue);
        MMO_FREE(sNewValue);
      } else {
        setAttribute(pQE, sName, sValue);
      }
    }
  }

  void XmlBaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue, const char *sNamespace) const {
    char *sValue = itoa(iValue);
    setAttribute(pQE, sName, sValue, sNamespace);
    MM_FREE(sValue);
  }

  void XmlBaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace) const {
    char *sValue = itoa(uValue);
    setAttribute(pQE, sName, sValue, sNamespace);
    MM_FREE(sValue);
  }

  bool XmlBaseNode::hasNamespace() const {
    return namespaceHREF(NO_DUPLICATE);
  }

  bool XmlBaseNode::isName(const char *sHREF, const char *sLocalName) const {
    return _STREQUAL(namespaceHREF(NO_DUPLICATE), sHREF) && _STREQUAL(localName(NO_DUPLICATE), sLocalName);;
  }

  bool XmlBaseNode::isNamespace(const char *sHREFs) const {
    //sHREFs CAN be a space delimited multiple HREF string
    //e.g. "http://general_server.org/xmlnamespaces/dummyxsl/2006 http://www.w3.org/1999/xhtml"
    bool bIsNamespace = false;
    const char *sThisNamespaceHREF = namespaceHREF(NO_DUPLICATE),
               *sSpace = 0,
               *sMatchPoint = sHREFs;

    if (sMatchPoint && sThisNamespaceHREF) {
      do {
        sSpace = strchr(sMatchPoint, ' ');
        if (sSpace) {
          bIsNamespace = !strncmp(sMatchPoint, sThisNamespaceHREF, (sSpace - sMatchPoint - 1));
          sMatchPoint = sSpace + 1;
        } else bIsNamespace = _STREQUAL(sMatchPoint, sThisNamespaceHREF);
      } while (!bIsNamespace && sSpace);
    }

    return bIsNamespace;
  }

  const char *XmlBaseNode::standardPrefix() const {
    //DO NOT free the result!
    return m_pDoc->standardPrefix(namespaceHREF(NO_DUPLICATE));
  }

  const char *XmlBaseNode::fullyQualifiedName() const {
    //caller frees result
    const char *sName   = 0, 
               *sPrefix = 0;
    
    switch (nodeType()) {
      case NODE_ELEMENT:
      case NODE_ATTRIBUTE: {
        sPrefix = namespacePrefix(RECONCILE_DEFAULT, NO_DUPLICATE);
        sName   = constructName(sPrefix, localName(NO_DUPLICATE));
        IFDEBUG(if (!sPrefix) 
          throw Up(this, "prefix not found for [%s]", sName)
        );
        break;
      }
      case NODE_TEXT: {
        sName = MM_STRDUP("#TEXT");
        break;
      }
      case NODE_DOCUMENT: {
        sName = MM_STRDUP("#DOCUMENT");
        break;
      }
      default: throw XPathTypeUnhandled(this, nodeType());
    }
    
    return sName;
  }

  const char *XmlBaseNode::constructName(const char *sPrefix, const char *sLocalName) const {
    //caller frees result
    char *sFullyQualifiedName = 0;
    size_t iLen;
    
    assert(sLocalName && sLocalName[0]);

    if (sPrefix && sPrefix[0]) {
      iLen = strlen(sPrefix) + 1 + strlen(sLocalName);
      sFullyQualifiedName = MMO_MALLOC(iLen + 1); //private
      _SNPRINTF2(sFullyQualifiedName, iLen + 1, "%s:%s", sPrefix, sLocalName);
    } else {
      sFullyQualifiedName = MM_STRDUP(sLocalName);
    }

    return sFullyQualifiedName;
  }

  bool XmlBaseNode::isAncestorOf(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pDesendentNode) const {
    //if this is an ancestor of pDesendentNode
    XmlNodeList<const IXmlBaseNode>::const_iterator iAncestorNode;
    const XmlNodeList<const IXmlBaseNode> *pvAncestors;
    bool ret = false;

    pvAncestors = pDesendentNode->ancestors(pQE, "ancestor check");
    for (iAncestorNode = pvAncestors->begin(); !ret && iAncestorNode != pvAncestors->end(); iAncestorNode++) {
      if (is(*iAncestorNode, HARDLINK_AWARE)) ret = true;
    }
    vector_const_element_destroy(pvAncestors);

    return ret;
  }

  bool XmlBaseNode::in(const XmlNodeList<IXmlBaseNode> *pVec, const bool bHardlinkAware) const {
    return in((const XmlNodeList<const IXmlBaseNode>*) pVec, bHardlinkAware);
  }

  bool XmlBaseNode::in(const XmlNodeList<const IXmlBaseNode> *pVec, const bool bHardlinkAware) const {
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    bool bIsIn = false;

    for (iNode = pVec->begin(); !bIsIn && iNode != pVec->end(); iNode++) {
      if (is(*iNode, bHardlinkAware)) bIsIn = true;
    }

    return bIsIn;
  }

  bool XmlBaseNode::isRootNode() const {
    return m_pDoc->isRootNode(this);
  }
  
  bool XmlBaseNode::isNodeDocument() const {
    return (nodeType() == NODE_DOCUMENT);
  }
  bool XmlBaseNode::isNodeElement() const {
    return (nodeType() == NODE_ELEMENT);
  }
  bool XmlBaseNode::isNodeAttribute() const {
    return (nodeType() == NODE_ATTRIBUTE);
  }
  bool XmlBaseNode::canHaveNamespace() const {
    return isNodeElement() || isNodeAttribute();
  }
  bool XmlBaseNode::isCDATASection()  const {
    return (nodeType() == NODE_CDATA_SECTION);
  }
  bool XmlBaseNode::isNodeText() const {
    //include CDATA as we cant have sub-nodes in CDATA
    return (nodeType() == NODE_TEXT || nodeType() == NODE_CDATA_SECTION);
  }

  bool XmlBaseNode::isNodeNamespaceDecleration() const {
    return (nodeType() == NODE_NAMESPACE_DECL);
  }

  bool XmlBaseNode::isProcessingInstruction() const {
    return (nodeType() == NODE_PROCESSING_INSTRUCTION);
  }
  bool XmlBaseNode::isTransientAreaParent() const {
    //transient definition: only there temporarily, certainly not there on server restart
    //transient nodes are not saved to the repository and thus only exist temporarily during server lifetime
    //is the node new temporary or in a temporary area?
    const XmlAdminQueryEnvironment ibqe(mmParent(), m_pDoc); //isTransientAreaParent() attributeValueBoolDynamicString()
    return attributeValueBoolDynamicString(&ibqe, "gs:transient-area");
  }
  bool XmlBaseNode::hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const {
    return childElementCount(pQE) > 1;
  }
  bool XmlBaseNode::isEmpty(const IXmlQueryEnvironment *pQE) const {
    return childElementCount(pQE) == 0;
  }

  size_t XmlBaseNode::childElementCount(const IXmlQueryEnvironment *pQE) const {
    const XmlNodeList<const IXmlBaseNode> *pvChildren = getMultipleNodes(pQE, "*");
    size_t iCount = pvChildren->size();
    
    //free up
    vector_const_element_destroy(pvChildren);
    
    return iCount;
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const bool bRegister, const char *sLockingReason) const {
    XmlNodeList<IXmlBaseNode>::iterator iNode;
    XmlNodeList<IXmlBaseNode> *pvNodes = ((IXmlBaseNode*) this)->getMultipleNodes(pQE, sXPath, NULL, 0, false, sLockingReason);
    if (bRegister) for (iNode = pvNodes->begin(); iNode != pvNodes->end(); iNode++) (*iNode)->registerNode();
    return (XmlNodeList<const IXmlBaseNode>*) pvNodes;
  }

  XmlNodeList<IXmlBaseNode> *XmlBaseNode::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) {
    return (XmlNodeList<IXmlBaseNode>*) ((const IXmlBaseNode*) this)->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason);
  }

  const char *XmlBaseNode::xmlID(const IXmlQueryEnvironment *pQE) const {
    //caller frees result
    return attributeValue(pQE, "id", NAMESPACE_XML);
  }

  void XmlBaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue, const char *sNamespace) const {
    setAttribute(pQE, sName, (bValue ? "yes" : "no"), sNamespace);
  }

  IXmlBaseNode *XmlBaseNode::setTransient(const IXmlQueryEnvironment *pQE) const {
    //NOTE: (IXmlBaseNode*) conversion to transform this in to a non-const (ok!)
    ((IXmlBaseNode*) this)->setAttribute(pQE, "transient", true, NAMESPACE_GS);
    return ((IXmlBaseNode*) this);
  }

  IXmlBaseNode *XmlBaseNode::setTransientArea(const IXmlQueryEnvironment *pQE) {
    setAttribute(pQE, "transient-area", true, NAMESPACE_GS);
    return this;
  }

  int XmlBaseNode::attributeValueInt(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const int iDefault) const {
    int iAttVal = 0;
    const char *sValue = attributeValue(pQE, sName, sNamespace);
    if (sValue) {
      iAttVal = atoi(sValue);
      MM_FREE(sValue);
    } else iAttVal = iDefault;

    return iAttVal;
  }

  const char *XmlBaseNode::attributeValueDynamic(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const char *sDefault, const unsigned int iRecursions) const {
    //returns NULL if the attribute is not found
    const char *sValue, *sNewValue = 0;

    if (sValue = attributeValue(pQE, sName, sNamespace, sDefault)) {
      sNewValue = valueDynamic(pQE, sValue, iRecursions);
      if (sNewValue != sValue) MM_FREE(sValue);
    }

    return sNewValue;
  }

  bool  XmlBaseNode::attributeValueBoolInterpret(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const bool bDefault) const {
    //ONLY converts xpath result to a bool
    //see: attributeValueBoolDynamicString() for string conversion
    //  @if="$gs_some_variable"
    //  @if="true()"
    //  @if="'true'"
    //  @if="true" will NOT work because it will be treated as xpath
    bool ret = bDefault;
    size_t iLen;
    char *sDynamicTemplate = 0;
    const char *sXPath  = 0,
               *sValue  = 0;

    UNWIND_EXCEPTION_BEGIN {
      if (sXPath = attributeValue(pQE, sName, sNamespace)) {
        iLen = 9 + strlen(sXPath) + 2 + 1;
        sDynamicTemplate = MMO_MALLOC(iLen);
        _SNPRINTF1(sDynamicTemplate, iLen, "{boolean(%s)}", sXPath);
        sValue = valueDynamic(pQE, sDynamicTemplate);
        ret    = xmlLibrary()->textEqualsTrue(sValue);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sValue)           MMO_FREE(sValue);
    if (sDynamicTemplate) MMO_FREE(sDynamicTemplate);

    UNWIND_EXCEPTION_THROW;
    
    return ret;
  }
  
  bool  XmlBaseNode::attributeValueBoolDynamicString(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const bool bDefault) const {
    //ONLY converts a direct string to bool
    //see: attributeValueBoolInterpret() for xpath result conversion
    //  @if="true"
    //  @if="{{'true'}}"
    //  @if="$gs_some_variable" will NOT work because treated as a string
    //uses the non-standard textEqualsTrue() from GS to evluate the text => bool
    bool ret = bDefault;
    const char *sValue = 0;

    if (sValue = attributeValueDynamic(pQE, sName, sNamespace)) {
      ret = xmlLibrary()->textEqualsTrue(sValue);
    }

    //free up
    if (sValue) MM_FREE(sValue);

    return ret;
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::attributeValueNodes(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const IXmlBaseNode *pBaseNode) const {
    //caller frees non-zero result
    XmlNodeList<const IXmlBaseNode> *pNodes = 0;
    const char *sValue;

    if (sValue = attributeValue(pQE, sName, sNamespace)) {
      //now get the node, using the transforming stylesheet for variables
      //  in relation to the current() node from the transformation stylesheet if present
      //  OR this m_oNode if not
      pNodes = (pBaseNode ? pBaseNode : this)->getMultipleNodes(pQE, sValue);
      MM_FREE(sValue);
    }

    return pNodes;
  }

  IXmlBaseNode *XmlBaseNode::attributeValueIDNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const char *sIDAttributeName, const char *sNamespace) {
    //caller frees non-zero result
    //SHOULD ONLY be used by administration system
    //USE hardlinking instead of references
    IXmlBaseNode *pIDNode = 0;
    const char *sValue;

    if (sValue = pNode->attributeValue(pQE, sIDAttributeName, sNamespace)) {
      //now get the node, using the transforming stylesheet for variables
      //  in relation to the current() node from the transformation stylesheet if present
      //  OR this m_oNode if not
      pIDNode = m_pDoc->nodeFromID(pQE, sValue);
      MM_FREE(sValue);
    }

    return pIDNode;
  }

  IXmlBaseNode *XmlBaseNode::attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const IXmlBaseNode *pFromNode) {
    return (IXmlBaseNode*) ((const IXmlBaseNode*) this)->attributeValueNode(pQE, sName, sNamespace, pFromNode);
  }

  const IXmlBaseNode *XmlBaseNode::attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const IXmlBaseNode *pFromNode) const {
    //caller frees non-zero result
    const IXmlBaseNode *pNode = 0;
    const char *sXPath;

    if (sXPath = attributeValue(pQE, sName, sNamespace)) {
      //now get the node, using the transforming stylesheet for variables
      //  in relation to the current() node from the transformation stylesheet if present
      //  OR this m_oNode if not
      if (!pFromNode) {
        pFromNode = this;
        IFDEBUG(if (*sXPath != '/' && *sXPath != '$') Debug::warn("relative @%s=%s to node from same node", MM_STRDUP(sName), MM_STRDUP(sXPath));)
      }
      pNode = pFromNode->getSingleNode(pQE, sXPath);
      MM_FREE(sXPath);
    }

    return pNode;
  }

  IXmlNodeMask *XmlBaseNode::attributeValueNodeMask(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace) const {
    IXmlNodeMask *pNodeMask       = 0;
    char *sNodeMaskTypeName       = 0;
    const char *sValue            = 0,
               *sNodeMaskType     = 0;
    size_t iNameLen;
    
    assert(pQE);

    if (sName) {
      if (sValue = attributeValueDynamic(pQE, sName, sNamespace)) {
        //mask type
        iNameLen = strlen(sName);
        sNodeMaskTypeName = MMO_MALLOC(iNameLen + 5 + 1);
        strcpy(sNodeMaskTypeName, sName);
        strcpy(sNodeMaskTypeName + iNameLen, "-type"); //includes termination zero
        sNodeMaskType = attributeValueDynamic(pQE, sNodeMaskTypeName);

        //nodes
        if (*sValue) pNodeMask = xmlLibrary()->factory_nodeMask(this, sValue, sNodeMaskType);
      }
    }
    
    //free up
    if (sNodeMaskTypeName) MMO_FREE(sNodeMaskTypeName);
    if (sNodeMaskType)     MMO_FREE(sNodeMaskType);

    return pNodeMask;
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::fileSystemPathToNodeList(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const bool bRejectAbsolute, const char *sTargetType, const char *sDirectoryNamespacePrefix, const bool bUseIndexes) const {
    XmlNodeList<const IXmlBaseNode> *pTargetNodes = 0;
    const char *sXPath                  = 0,
               *sBaseNodeXPath          = 0,
               *sCurrentXSLCommandXPath = 0,
               *sError                  = 0;
    bool bWriteable = false;

    //set readOnly because we do not want to trigger node discovery based on write actions
    //e.g. database:set-attribute(...) calls
    //this function is specifically for node discovery only
    //will throw NodeWriteableAccessDenied() if it attempts to write
    bWriteable = pQE->securityContext()->setReadOnly(); 
    UNWIND_EXCEPTION_BEGIN {
      if (sFileSystemPath && *sFileSystemPath) {
        if (bRejectAbsolute && Repository::issplitter(*sFileSystemPath)) throw CannotConvertAbsoluteHREFToXPathWithoutRoot(this, sFileSystemPath);
        sXPath = xmlLibrary()->fileSystemPathToXPath(pQE, sFileSystemPath, sTargetType, sDirectoryNamespacePrefix, bUseIndexes);
        if (sXPath && *sXPath) {
          pTargetNodes = getMultipleNodes(pQE, sXPath);

          if (!pTargetNodes || !pTargetNodes->size()) {
            if (bUseIndexes) sError = "not found, @use-indexes is true";
            else             sError = "not found"; //) not found
          }
        } else sError = "resultant path empty";
      } else sError = "path empty";
    } UNWIND_EXCEPTION_END;
    if (bWriteable) pQE->securityContext()->setWriteable();

    if (sError) {
      sBaseNodeXPath          = uniqueXPathToNode(pQE);
      sCurrentXSLCommandXPath = pQE->currentXSLCommandXPath();
      Debug::report("[%s]: repository:filesystempath-to-nodes(%s %s) %s", 
        (strstr(sCurrentXSLCommandXPath, "/gs_request_target") ? "$gs_request_target" : sCurrentXSLCommandXPath), 
        sBaseNodeXPath,
        sXPath, //may include /index if @use-indexes is true
        sError,
        rtWarning
      );
    }
    
    //free up
    if (sXPath)                   MMO_FREE(sXPath);
    if (sCurrentXSLCommandXPath)  MMO_FREE(sCurrentXSLCommandXPath);
    if (sBaseNodeXPath)           MMO_FREE(sBaseNodeXPath);
    //if (sError)                   MMO_FREE(sError); //constant

    UNWIND_EXCEPTION_THROW;
    
    return pTargetNodes;
  }

  const char *XmlBaseNode::attributeValueUniqueXPath(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const IXmlBaseNode *pBaseNode, const bool bWarnIfTransient, const bool bForceBaseNode) const {
    //caller frees non-zero result
    const IXmlBaseNode *pNode;
    const char *sValue = 0;

    if (pNode = attributeValueNode(pQE, sName, sNamespace)) {
      sValue = pNode->uniqueXPathToNode(pQE, pBaseNode, bWarnIfTransient, bForceBaseNode);
      delete pNode;
    }

    return sValue;
  }

  void XmlBaseNode::setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const char *sName) const {
    char *sID;
    size_t iLen;

    iLen   = strlen(sPrefix) + (sName ? strlen(sName) : 0) + 1;
    sID    = MMO_MALLOC(iLen);
    _SNPRINTF2(sID, iLen, "%s%s", sPrefix, sName);

    setAttribute(pQE, "id", sID, NAMESPACE_XML);

    //clear up
    MMO_FREE(sID);
  }

  void XmlBaseNode::setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const unsigned int uValue, const char *sSuffix) const {
    char *sID;
    size_t iLen;
    const char *sValue;

    sValue = utoa(uValue);
    iLen   = strlen(sPrefix) + strlen(sValue) + (sSuffix ? strlen(sSuffix) : 0) + 1;
    sID    = MMO_MALLOC(iLen);
    _SNPRINTF3(sID, iLen, "%s%s%s", sPrefix, sValue, sSuffix);

    setAttribute(pQE, "id", sID, NAMESPACE_XML);

    //clear up
    MMO_FREE(sID);
    MMO_FREE(sValue);
  }

  IXmlBaseNode *XmlBaseNode::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const IXmlBaseNode*) this)->getSingleNode(pQE, sXPath, argchar, argint, sLockingReason);
  }

  const IXmlBaseNode *XmlBaseNode::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const char *sLockingReason) const {
    const IXmlBaseNode *pNode       = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    const XmlNodeList<const IXmlBaseNode> *pNodes;

    if (sXPath) {
      pNodes = getMultipleNodes(pQE, sXPath, argchar, argint, sLockingReason);
      if (pNodes) {
        for (iNode = pNodes->begin(); iNode != pNodes->end(); iNode++) {
          if (!pNode) pNode = *iNode; //assign the first node
          else        delete *iNode;  //delete the rest
        }
        delete pNodes;
      }
    }

    return pNode;
  }

  bool XmlBaseNode::attributeExists(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace) const {
    //can use a prefix qualified singular name, e.g. "repository:place" (EXPENSIVE)
    //or 2 parameters "place" and NAMESPACE_REPOSITORY
    return (attributeValue(pQE, sName, sNamespace) != 0);
  }

  const char *XmlBaseNode::attributeValue(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const char *sDefault) const {
    //caller frees result
    //return value NULL indicates that the attribute does not exist
    //NAMESPACE AWARE: name may include namespace prefix, e.g. rx:regex or not e.g. regex
    //xmlGetProp does not resolve prefix namespace lookups like with xmlSetProp()
    const IXmlBaseNode *pAttributeNode = 0;
    size_t iLen         = 0;
    const char *sValue  = 0,
               *sPrefix = 0;
    char       *sXPath  = 0;

    assert(pQE);
    assert(sName);
    
    if (sNamespace) {
      if (sPrefix = document()->standardPrefix(sNamespace)) { //NAMESPACE_ALL
        iLen   = 1 + strlen(sPrefix) + 1 + strlen(sName) + 1; //@prefix:name0
        sXPath = MMO_MALLOC(iLen);
        _SNPRINTF2(sXPath, iLen, "@%s:%s", sPrefix, sName);
      } else {
        //ok, we can't trap this at this layer, leave xpath to deal with it or throw up
        iLen   = 2 + 15 + strlen(sName) + 23 + strlen(sNamespace) + 2 + 1;
        sXPath = MMO_MALLOC(iLen);
        _SNPRINTF2(sXPath, iLen, "@*[local-name()='%s' and namespace-uri()='%s']", sName, sNamespace);
      }
    } else if (*sName) {
      iLen   = 1 + strlen(sName) + 1; //@name0
      sXPath = MMO_MALLOC(iLen);
      _SNPRINTF1(sXPath, iLen, "@%s", sName);
    }
    
    if (pAttributeNode = getSingleNode(pQE, sXPath)) sValue = pAttributeNode->value(pQE);
    else if (sDefault)                               sValue = MM_STRDUP(sDefault);
      
    //XML specification requires that newlines are only \n and newlines will be normalised by this function
    //so we incorporate a special attribute "normalise" to allow c-string style escaping \r\n
    //note that HTTP newlines and Windows newlines are \r\n or &#13;&#10;
    //defaults to off
    if (sValue && *sValue && strstr(sName, "-normalise")) {
      //swap them
      const char *sValueNormalised = xmlLibrary()->normalise(sValue);
      MM_FREE(sValue);
      sValue = sValueNormalised;
    }

    //free up
    //if (sLocalName)     MM_FREE(sLocalName); //pointer in to the sName, e.g. object:Server => Server
    //if (sPrefix)        MM_FREE(sPrefix);    //pointer in to constant
    if (iLen)           MMO_FREE(sXPath);
    if (pAttributeNode) delete pAttributeNode;

    return sValue;
  }

  XmlNodeList<IXmlBaseNode> *XmlBaseNode::ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return (XmlNodeList<IXmlBaseNode>*) ((const IXmlBaseNode*) this)->ancestors(pQE, sLockingReason);
  }

  XmlNodeList<IXmlBaseNode> *XmlBaseNode::children(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return (XmlNodeList<IXmlBaseNode>*) ((const IXmlBaseNode*) this)->children(pQE, sLockingReason);
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::children(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    //caller always frees result
    return getMultipleNodes(pQE, "*", sLockingReason);
  }

  XmlNodeList<IXmlBaseNode> *XmlBaseNode::attributes(const IXmlQueryEnvironment *pQE) {
    return (XmlNodeList<IXmlBaseNode>*) ((const IXmlBaseNode*) this)->attributes(pQE);
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::attributes(const IXmlQueryEnvironment *pQE) const {
    //caller always frees result
    return getMultipleNodes(pQE, "@*");;
  }

  IXmlBaseNode *XmlBaseNode::firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const XmlBaseNode*) this)->firstChild(pQE, iNodeTypes, sName, sLockingReason);
  }
  
  IXmlBaseNode *XmlBaseNode::lastChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const XmlBaseNode*) this)->lastChild(pQE, iNodeTypes, sLockingReason);
  }
  
  IXmlBaseNode *XmlBaseNode::child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sNamespace, const char *sLockingReason) {
    return (IXmlBaseNode*) ((const XmlBaseNode*) this)->child(pQE, iPosition, iNodeTypes, sName, sNamespace, sLockingReason);
  }

  const IXmlBaseNode *XmlBaseNode::lastChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sLockingReason) const {
    //caller fees result
    const char *sXPath = 0;

    switch (iNodeTypes) {
      //NOTE: this will not currently return CDATA nodes
      case node_type_element_or_text: {sXPath = "(*|text())[last()]"; break;} //default
      case node_type_element_only:    {sXPath = "*[last()]";          break;}
      case node_type_processing_instruction: {sXPath = "processing-instruction()[last()]"; break;}
      case node_type_any:             {sXPath = "node()[last()]";      NOT_CURRENTLY_USED(""); break;}
    }

    return getSingleNode(pQE, sXPath, sLockingReason);
  }

  const IXmlBaseNode *XmlBaseNode::firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) const {
    //caller fees result
    const char *sXPath      = 0;
    
    switch (iNodeTypes) {
      //NOTE: this will not currently return CDATA nodes
      case node_type_element_or_text: {sXPath = "(*|text())[1]"; break;} //default
      case node_type_element_only:    {sXPath = "*[1]";          break;}
      case node_type_any:             {sXPath = "node()[1]";      NOT_CURRENTLY_USED(""); break;}
      case node_type_processing_instruction: {
        sXPath = (sName ? "processing-instruction('%s')[1]" : "processing-instruction()[1]"); 
        break;
      }
    }
    
    return getSingleNode(pQE, sXPath, sName, 0, sLockingReason);
  }
  
  const IXmlBaseNode *XmlBaseNode::child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sNamespace, const char *sLockingReason) const {
    //destination position = 0:  not specified, position() is 1 based: will THROW PositionOutOfRange
    //destination position > 0:  position() is 1 based
    //destination position = -1: NULL indicating after the last position. that there is no BeforeNode
    //destination position = -2: last() node
    //destination position = -3: penultimate node
    char *sPositionalPredicate     = 0,
         *sNamespacePredicate      = 0,
         *sXPath                   = 0;
    const IXmlBaseNode *pChildNode = 0;
    size_t iLen;

    //node type
    //  node_type_element_or_text
    //  node_type_element_only
    //  node_type_processing_instruction
    //  node_type_dtd
    //  node_type_any
    if (iNodeTypes != node_type_element_only) NOT_COMPLETE("child() supports only node_type_element_only");
    
    //namespace predicate
    //used by getOrCreate*() functions to check for exitence
    if (sNamespace) {
      iLen = strlen(sNamespace) + 22 + 1;
      sNamespacePredicate = MMO_MALLOC(iLen);
      _SNPRINTF1(sNamespacePredicate, iLen, "[namespace-uri() = '%s']", sNamespace);
    }

    //position (non-zero)
    if      (iPosition > 0)   sPositionalPredicate = itoa(iPosition, "[", "]");
    else if (iPosition == -2) sPositionalPredicate = MMO_STRDUP("[position() = last()]");
    else if (iPosition < -2)  sPositionalPredicate = itoa(-(iPosition + 2), "[position() = last() - ", "]");
    
    //construct
    iLen = 
        (sName                ? strlen(sName)                : 1)
      + (sNamespacePredicate  ? strlen(sNamespacePredicate)  : 0)
      + (sPositionalPredicate ? strlen(sPositionalPredicate) : 3)
      + 1;
    sXPath = MMO_MALLOC(iLen);
    _SNPRINTF3(sXPath, iLen, "%s%s%s", 
      (sName                ? sName                : "*"),
      (sNamespacePredicate  ? sNamespacePredicate  : ""),
      (sPositionalPredicate ? sPositionalPredicate : "[1]")
    );
    
    //run
    //getSingleNode() will always return only the first match
    if (sXPath) pChildNode = getSingleNode(pQE, sXPath, sLockingReason);
    
    //free up
    if (sXPath)               MMO_FREE(sXPath);
    if (sNamespacePredicate)  MMO_FREE(sNamespacePredicate);
    if (sPositionalPredicate) MMO_FREE(sPositionalPredicate);
    
    return pChildNode;
  }
  
  IXmlBaseNode *XmlBaseNode::parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    //caller fees result
    //GS disobeys W3C at this level in that it will return the document node as the parent of the root node
    IXmlBaseNode *parentNode;
    
    if (isRootNode()) parentNode = document()->documentNode();
    else              parentNode = getSingleNode(pQE, "..", sLockingReason);

    return parentNode;
  }
  
  const IXmlBaseNode *XmlBaseNode::parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    return ((IXmlBaseNode*) this)->parentNode(pQE, sLockingReason);
  }

  IXmlBaseNode *XmlBaseNode::followingSibling(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return getSingleNode(pQE, "following-sibling::*[1]", sLockingReason);
  }
  
  const IXmlBaseNode *XmlBaseNode::followingSibling(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    return ((IXmlBaseNode*) this)->followingSibling(pQE, sLockingReason);
  }

  XmlNodeList<const IXmlBaseNode> *XmlBaseNode::ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    return getMultipleNodes(pQE, "ancestor::*", sLockingReason);
  }

  IXmlBaseNode *XmlBaseNode::copyChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const bool bDeepClone, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition, iXMLIDPolicy iXMLIDMovePolicy) {
    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    IXmlBaseNode *pNewNode = 0;
    const IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = copyChild(pQE, pRootNode, bDeepClone, pBeforeNode, iXMLIDMovePolicy); //virtual
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }

  IXmlBaseNode *XmlBaseNode::moveChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition, iXMLIDPolicy iXMLIDMovePolicy) {
    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    IXmlBaseNode *pNewNode = 0;
    IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = moveChild(pQE, pRootNode, pBeforeNode, iXMLIDMovePolicy); //virtual
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }

  IXmlBaseNode *XmlBaseNode::hardlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition) {
    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    IXmlBaseNode *pNewNode = 0;
    IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = hardlinkChild(pQE, pRootNode, pBeforeNode); //virtual
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }
  IXmlBaseNode *XmlBaseNode::softlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc) {
    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    IXmlBaseNode *pNewNode = 0;
    IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = softlinkChild(pQE, pRootNode); //virtual
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }
  
  IXmlBaseNode *XmlBaseNode::mergeNode(const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy) {
    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    //If oMovingNode was already inserted in a document it is first unlinked from its existing context
    IXmlBaseNode *pNewNode = 0;
    const IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = mergeNode(pQE, pRootNode, iXMLIDMovePolicy);
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }

  IXmlBaseNode *XmlBaseNode::replaceNodeCopy(const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy) {
    NOT_CURRENTLY_USED("");

    //caller frees the result if not 0
    //only use serverFree if the item is not persistent and will delete its mess quickly
    //If oMovingNode was already inserted in a document it is first unlinked from its existing context
    IXmlBaseNode *pNewNode = 0;
    const IXmlBaseNode *pRootNode = pDoc->rootNode(pQE);
    if (pRootNode) {
      pNewNode = replaceNodeCopy(pQE, pRootNode, iXMLIDMovePolicy); //virtual
      if (pRootNode != pNewNode) delete pRootNode;
    }
    return pNewNode;
  }

  void XmlBaseNode::moveChildren(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNewParent) {
    XmlNodeList<IXmlBaseNode> *pvChildren = 0;
    XmlNodeList<IXmlBaseNode>::iterator iChildNode;
    IXmlBaseNode *pChildNode  = 0;
    const IXmlBaseNode *pParentNode = 0;

    if (pParentNode = parentNode(pQE)) {
      pvChildren = children(pQE);
      for (iChildNode = pvChildren->begin(); iChildNode != pvChildren->end(); iChildNode++) {
        pChildNode = *iChildNode;
        delete moveChild(pQE, pChildNode, this);
      }
    }

    //free up
    if (pParentNode) delete pParentNode;
    if (pvChildren)  vector_element_destroy(pvChildren);
  }

  IXmlBaseNode *XmlBaseNode::getOrCreateChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace) {
    //use the pFilter here
    //we need to make sure that we create it if it is not there or we can't see it
    IXmlBaseNode *pNode = child(pQE, 0, iDOMNodeTypeRequest::node_type_element_only, sName, sNamespace);
    if (!pNode) pNode = createChildElement(pQE, sName, sContent, sNamespace);
    return pNode;
  }

  IXslCommandNode          *XmlBaseNode::queryInterface(IXslCommandNode *p)                {throw InterfaceNotSupported(this, MM_STRDUP("IXslCommandNode *")); return 0;}
  const IXslCommandNode    *XmlBaseNode::queryInterface(const IXslCommandNode *p) const    {throw InterfaceNotSupported(this, MM_STRDUP("const IXslCommandNode *")); return 0;}
  const IXmlBaseNode       *XmlBaseNode::queryInterface(const IXmlBaseNode *p) const       {throw InterfaceNotSupported(this, MM_STRDUP("const IXmlBaseNode *")); return 0;}
  IXmlBaseNode             *XmlBaseNode::queryInterface(IXmlBaseNode *p)                   {throw InterfaceNotSupported(this, MM_STRDUP("IXmlBaseNode *")); return 0;}
  const IXslStylesheetNode *XmlBaseNode::queryInterface(const IXslStylesheetNode *p) const {throw InterfaceNotSupported(this, MM_STRDUP("const IXslStylesheetNode *")); return 0;}
  const IXslTemplateNode   *XmlBaseNode::queryInterface(const IXslTemplateNode *p) const   {throw InterfaceNotSupported(this, MM_STRDUP("const IXslTemplateNode *")); return 0;}
  const IXslNode           *XmlBaseNode::queryInterface(const IXslNode *p) const           {throw InterfaceNotSupported(this, MM_STRDUP("const IXslNode *")); return 0;}
  IXmlNamespaced           *XmlBaseNode::queryInterface(IXmlNamespaced *p)                 {throw InterfaceNotSupported(this, MM_STRDUP("IXmlNamespaced *")); return 0;}
  const IXmlNamespaced     *XmlBaseNode::queryInterface(const IXmlNamespaced *p) const     {throw InterfaceNotSupported(this, MM_STRDUP("const IXmlNamespaced *")); return 0;}
  IXmlHasNamespaceDefinitions *XmlBaseNode::queryInterface(IXmlHasNamespaceDefinitions*)   {throw InterfaceNotSupported(this, MM_STRDUP("IXmlHasNamespaceDefinitions *")); return 0;}
  const IXmlHasNamespaceDefinitions *XmlBaseNode::queryInterface(const IXmlHasNamespaceDefinitions*) const {throw InterfaceNotSupported(this, MM_STRDUP("const IXmlHasNamespaceDefinitions *")); return 0;}
  const IXmlBaseNode       *XmlBaseNode::queryInterfaceIXmlBaseNode() const                {return this;}
}
