//platform independent
#include "LibXmlBaseNode.h"

#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslNode.h"
#include "IXml/IXmlSecurityContext.h"
#include "IXml/IXmlDebugContext.h"
#include "IXml/IXmlTriggerContext.h"
#include "IXml/IXmlMaskContext.h"
#include "Thread.h"
#include "TXml.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"
#include "LibXmlLibrary.h"
#include "LibXmlDoc.h"
#include "LibXmlNode.h"
#include "LibXslDoc.h"
#include "LibXslNode.h"
#include "Exceptions.h"

#include "Utilities/container.c"

#include <libxslt/xsltInternals.h>
#include <libxslt/variables.h>      //xmlCopyGlobalVar(...)
#include <libxml/parserInternals.h> //this will be all internalised in to LibXML2 one day...?
#include <libxml/xmlfilter.h>
#include <libxml/xmltrigger.h>
#include <libxml/debugXML.h>
#include <libxml/xmlversion.h>

#include "Utilities/regexpr2.h"
using namespace regex;

namespace general_server {
  //------------------------------------------------------------------------------------------- debugging
  IFDEBUG(const char           *LibXmlBaseNode::x() const {const XmlAdminQueryEnvironment ibqe_debug(this, document());return xml(&ibqe_debug);})
  IFDEBUG(const LibXmlBaseNode *LibXmlBaseNode::d() const {return dynamic_cast<const LibXmlBaseNode*>(this);})
  IFDEBUG(const xmlNodePtr      LibXmlBaseNode::o() const {return dynamic_cast<const LibXmlBaseNode*>(this)->m_oNode;})

  //------------------------------------------------------------------------------------------- construction
  NODELINK_MULTIMAP LibXmlBaseNode::m_mNodeLinks;
  pthread_mutex_t LibXmlBaseNode::m_mNodeLinksAccess = PTHREAD_MUTEX_INITIALIZER;

  LibXmlBaseNode::LibXmlBaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlLibrary *pLib, const bool bRegister, const char *sLockingReason):
    XmlBaseNode(new LibXmlDoc(pMemoryLifetimeOwner, pLib, "auto OURS doc", oNode->doc), OURS),
    m_bRegistered(false), 
    m_bFreeInDestructor(false),
    m_oNode(oNode), 
    m_oParentRoute(xmlListDup(oParentRoute)), //xpath.c OWNS this resource
    m_oReconciledNs(0) 
  {
    assert(m_oNode);
    
    NOT_CURRENTLY_USED("stack decleration of the LibXmlDoc is more usual");
    
    if (!xmlLockNode(m_oNode, this, (const xmlChar*) sLockingReason)) throw NodeLockFailed(this, sLockingReason);
    if (bRegister) registerNode();
  }
  
  LibXmlBaseNode::LibXmlBaseNode(xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister, const char *sLockingReason, const bool pDocOurs): 
    XmlBaseNode((IXmlBaseDoc *) pDoc, pDocOurs), //the variaed document() [const] will return the correct const
    m_bRegistered(false), 
    m_bFreeInDestructor(false),
    m_oNode(oNode), 
    m_oParentRoute(xmlListDup(oParentRoute)), //xpath.c OWNS this resource
    m_oReconciledNs(0) 
  {
    assert(m_oNode);
    //TODO: cannot sync docs yet because too many cases where it fails
    //e.g. compiled stylesheet nodes with children with source doc pointers
    //assert(dynamic_cast<const LibXmlBaseDoc*>(pDoc)->libXmlDocPtr() == m_oNode->doc);

    if (!xmlLockNode(m_oNode, this, (const xmlChar*) sLockingReason)) throw NodeLockFailed(this, sLockingReason);
    if (bRegister) registerNode();
  }
  
  LibXmlBaseNode::LibXmlBaseNode(xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister, const char *sLockingReason, const bool pDocOurs): 
    XmlBaseNode(pDoc, pDocOurs), 
    m_bRegistered(false), 
    m_bFreeInDestructor(false),
    m_oNode(oNode), 
    m_oParentRoute(xmlListDup(oParentRoute)), //xpath.c OWNS this resource
    m_oReconciledNs(0) 
  {
    assert(m_oNode);
    //TODO: cannot sync docs yet because too many cases where it fails
    //e.g. compiled stylesheet nodes with children with source doc pointers
    //assert(dynamic_cast<const LibXmlBaseDoc*>(pDoc)->libXmlDocPtr() == m_oNode->doc);
    
    if (!xmlLockNode(m_oNode, this, (const xmlChar*) sLockingReason)) throw NodeLockFailed(this, sLockingReason);
    if (bRegister) registerNode();
  }

  LibXmlBaseNode::LibXmlBaseNode(const LibXmlBaseNode &node):
    MemoryLifetimeOwner(node.mmParent()),             //addChild()
    XmlBaseNode(node),
    m_oNode(node.m_oNode),
    m_oParentRoute(xmlListDup(node.m_oParentRoute)),  //xpath.c OWNS this resource
    m_bRegistered(false),                             //allow node to deregisterNode()
    m_bFreeInDestructor(false),                       //allow node to ~destruction of m_oNode
    m_oReconciledNs(node.m_oReconciledNs)
  {
    assert(m_oNode);
    
    //WE do not re-register this node of copyig it...
    //if (node.m_bRegistered) registerNode();
    
    IFDEBUG(
      if (node.m_bFreeInDestructor)       
        Debug::report("copy constructing a m_bFreeInDestructor node [%s]\n  made a new lock, original node left to handle destruction", fullyQualifiedName());
      if (MARKED_FOR_DESTRUCTION(m_oNode)) 
        Debug::report("copy constructing a marked_for_destruction node [%s]\n  lock will FAIL", fullyQualifiedName());
    );
    
    //LOCK the node first in this copy constructor
    //the subsequent original ~destructor on pNode will xmlUnLockNode() its reference
    if (!xmlLockNode(m_oNode, this, (const xmlChar*) "copy constructor")) 
      throw NodeLockFailed(this, MM_STRDUP("copy constructor: probably marked_for_destruction"));
  }

  LibXmlBaseNode::LibXmlBaseNode(): 
    m_oNode(0),
    m_oParentRoute(0),
    m_bRegistered(false),
    m_bFreeInDestructor(false)
  {
    NOT_CURRENTLY_USED("");
  }

  LibXmlBaseNode::~LibXmlBaseNode() {
    deregisterNode();
    xmlUnLockNode(m_oNode, this); //throws Up if no locks
    if (m_oParentRoute) xmlListDelete(m_oParentRoute); //xpath.c owned this xmlListDup(resource)
    
    //if (m_oReconciledNs) xmlFreeNs(m_oReconciledNs); //freed in XmlBaseNode, the owner
    
    //removeAndDestroyNode() m_bFreeInDestructor = true;
    //xmlFreeNode(m_oNode): if (cur->locks != 0) raiseError(...)
    //must happen after xmlUnLockNode() above
    // and before any copy constructors of this node
    if (m_bFreeInDestructor) 
      xmlFreeNode(m_oNode); 
  }

  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- resource registration --------------------------------------
  //-----------------------------------------------------------------------------------------------------
  bool LibXmlBaseNode::deregistrationNotify() {
    bool bOk = XmlBaseNode::deregistrationNotify();
    //m_oNode = 0;
    return bOk;
  }

  void LibXmlBaseNode::stopListeningForDeregistionNotify() {
    deregisterNode();
  }

  void LibXmlBaseNode::xmlDeregisterNodeFunc(xmlNodePtr oNode) {
    //called by the xmlFreeNode(xmlNodePtr) AND others (xmlDeregisterNodeDefaultValue)
    //  can XML_ELEMENT_NODE, XML_ATTRIBUTE_NODE, XML_DOCUMENT_NODE, XML_DTD_NODE, ...
    //IXmlBaseNode::deregistrationNotify() should NOT xmlFreeNode(m_oNode) as this will cause:
    //  a mutex race condition
    //  a double-free
    //IXmlBaseNode::deregistrationNotify() deletes itself:
    //  ~LibXmlBaseNode()
    //  deregisterNode(IXmlBaseNode)
    //  causing also a race condition AND / OR the deletion of the iNode target here
    NODELINK_MULTIMAP::iterator iNode;
    pair<NODELINK_MULTIMAP::iterator, NODELINK_MULTIMAP::iterator> rRange;
    IXmlBaseNode *pNode;

    //traverse the range
    //inform all IXmlBaseNode pointers that they are about to loose their m_oNode
    if (oNode->type == XML_ELEMENT_NODE) {
      pthread_mutex_lock(&m_mNodeLinksAccess); {
        rRange = m_mNodeLinks.equal_range(oNode);
        for (iNode = rRange.first; iNode != rRange.second; iNode++) {
          pNode = iNode->second;
          IFDEBUG(
            LibXmlBaseNode *pNodeType = dynamic_cast<LibXmlBaseNode*>(pNode);
            assert(pNodeType->m_iNodeLink == iNode);
            assert(pNodeType->m_bRegistered);
          )
          pNode->deregisterNode();
          pNode->deregistrationNotify();
        }
      } pthread_mutex_unlock(&m_mNodeLinksAccess);
    }
  }

  bool LibXmlBaseNode::isRegistered() const {
    //returns if something is watching this LibXML2 node, not the current wrapper!
    IFNDEBUG(return false); //preformance reasons
    return m_mNodeLinks.find(m_oNode) != m_mNodeLinks.end();
  }

  bool LibXmlBaseNode::registerNode() {
    //called by LibXmlBaseNode::constructor()
    IFDEBUG(
      if (m_bRegistered) {
        assert(m_iNodeLink->second == this);
        assert(m_iNodeLink->first == m_oNode);
      }
    );
    static size_t iSize, iMaxSize = 0;

    const bool bWasRegistered = m_bRegistered;
    if (!m_bRegistered) {
      pthread_mutex_lock(&m_mNodeLinksAccess); {
        IFDEBUG(
          iSize = m_mNodeLinks.size();
          if (iSize > iMaxSize) {
            iMaxSize = iSize;
            if (iMaxSize % 300 == 0)
              Debug::report("LibXML2 xmlFreeNode notifications map has exceeded [%s] entries...", iMaxSize, rtWarning);
          }
        );
        m_iNodeLink   = m_mNodeLinks.insert(pair<const xmlNodePtr, IXmlBaseNode*>(m_oNode, this));
        m_bRegistered = true;
      } pthread_mutex_unlock(&m_mNodeLinksAccess);
    }
    return bWasRegistered;
  }

  bool LibXmlBaseNode::deregisterNode() {
    //called by ~LibXmlBaseNode::destructor()
    const bool bWasRegistered = m_bRegistered;
    if (m_bRegistered) {
      pthread_mutex_lock(&m_mNodeLinksAccess); {
        m_mNodeLinks.erase(m_iNodeLink);
        m_bRegistered = false;
      } pthread_mutex_unlock(&m_mNodeLinksAccess);
    }
    return bWasRegistered;
  }

  //------------------------------------------------------------------------------------------- functions
  void LibXmlBaseNode::createHardlinkPlaceholderIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    LibXmlBaseNode  *pOutputNodeType    = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    pOutputNodeType = dynamic_cast<LibXmlBaseNode*>(pOutputNode);

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    //xmlCreateHardlinkPlaceholderFor(output, hardlink, security...);
    //<xml:hardlink...
    //destDoc = NULL
    xmlCreateHardlinkPlaceholder(m_oNode, NULL, pOutputNodeType->m_oNode, xtrigger, xfilter);
  }

  void LibXmlBaseNode::createDeviationPlaceholdersIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    //<xml:deviation... with @hardlink-id
    LibXmlBaseNode  *pOutputNodeType = dynamic_cast<LibXmlBaseNode*>(pOutputNode);
    if (pOutputNodeType) xmlCreateDeviationPlaceholders(m_oNode, pOutputNodeType->m_oNode, 1);
  }

  IXmlBaseNode *LibXmlBaseNode::createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace, const bool bCreateTransient, const char *sReason) {
    //caller frees result and inputs
    //NAMESPACE rules:
    //  if the user sends through an explicit namespace, e.g. object:Session, then it takes precendence
    //  if no namespace is used then the node INHERITS the parent nodes namespace explicitly with the same explicit prefix
    //  if no namespace is required then call the document()->createChildElement() function instead
    //namespace prefix resolution is done from this node, up through the document tree
    LibXmlBaseNode  *pNewNode  = 0;
    IXmlBaseNode *pNewNodeType = 0;
    xmlNodePtr oNode           = 0;
    xmlNsPtr oNS               = 0;
    const char *sLocalName     = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    //namespace resolution
    //if the prefix is blank then we try to resolve the namespace against the top level nsDefs
    //getNsFromString throws CannotReconcileNamespacePrefix()
    if (sNamespace) {
      sLocalName = sName;
      oNS = xmlSearchNsWithPrefixByHref(m_oNode->doc, m_oNode, (const xmlChar*) sNamespace);
      if (!oNS) throw CannotReconcileNamespaces(pQE, sNamespace); //pQE as MM
    } else {
      oNS = getNsFromString(sName, &sLocalName); //will assign sLocalName always
      if (sName != sLocalName && !oNS)  throw CannotReconcileNamespaces(pQE, sName); //pQE as MM
      if (!oNS && m_oNode->type == XML_ELEMENT_NODE) oNS = m_oNode->ns;  //otherwise inherit
    }

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    //this creates a node with no current parent
    //and thus no default namespace resolution possibility
    //BTW: xmlNewDocNodeEatName(...) simply doesnt do a MM_STRDUP() on the incoming name string
    oNode = xmlNewDocNode(libXmlDocPtr(), oNS, (const xmlChar*) sLocalName, (const xmlChar*) sContent);
    oNode = xmlAddChildSecure(m_oNode, m_oParentRoute, oNode, xtrigger, xfilter); //namespace resolution tree context

    pNewNode     = LibXmlLibrary::factory_node(mmParent(), oNode, document(), NULL, sReason); //auto namespace resolution
    pNewNodeType = pNewNode->queryInterface((IXmlBaseNode*) 0);

    if (bCreateTransient) pNewNodeType->setAttribute(pQE, "transient", "yes", NAMESPACE_GS);

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return pNewNodeType;
  }

  void LibXmlBaseNode::createTextNode(const IXmlQueryEnvironment *pQE, const char *sContent) {
    //caller frees result and inputs
    //NAMESPACE rules:
    //  if the user sends through an explicit namespace, e.g. object:Session, then it takes precendence
    //  if no namespace is used then the node INHERITS the parent nodes namespace explicitly with the same explicit prefix
    //  if no namespace is required then call the document()->createChildElement() function instead
    //namespace prefix resolution is done from this node, up through the document tree
    LibXslTransformContext *pCtxt;
    xsltTransformContextPtr ctxt;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    if (pCtxt = (LibXslTransformContext*) pQE->transformContext()) {
      //security and optional triggers
      ctxt                                = pCtxt->libXmlContext();
      xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
      if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

      //we have to use the transform version here because
      //lasttext and things need to be updated
      xsltCopyTextString(ctxt, m_oNode, (const xmlChar*) sContent, 1);
      
      //this would be overwritten
      //xmlNewDocText(libXmlDocPtr(), (const xmlChar*) sContent);
      //oNode = xmlAddChildSecure(m_oNode, m_oParentRoute, oNode, xtrigger, xfilter); //namespace resolution tree context
    }

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
  }
  
  xmlNodePtr LibXmlBaseNode::addChildToMeOrSetRootSecure(const IXmlQueryEnvironment *pQE, xmlNodePtr oNodeToMove, const IXmlBaseNode *pBeforeNode) {
    const LibXmlBaseNode *pBeforeNodeType;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    xmlNodePtr oRet = 0;

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    if (m_oNode->type == XML_DOCUMENT_NODE) {
      //xmlDocSetRootElement(...) ensures:
      //  there is only one root (so it xmlUnlinks the previous and returns it)
      //    using xmlReplaceNode(...) if there is an existing root
      //  that the XML_ELEMENT_NODE root is the LAST in thechildren (after PIs and comments)
      //  using xmlAddSibling(...) on doc-children where necessary
      xmlDocSetRootElement(m_oNode->doc, oNodeToMove);
      oRet = oNodeToMove;
    } else {
      //These calls DO NOT reconcile namespace links
      if (pBeforeNode) {
        pBeforeNodeType = dynamic_cast<const LibXmlBaseNode*>(pBeforeNode);
        oRet            = xmlAddPrevSiblingSecure(pBeforeNodeType->m_oNode, pBeforeNodeType->m_oParentRoute, oNodeToMove, xtrigger, xfilter);
      } else oRet = xmlAddChildSecure(m_oNode, m_oParentRoute, oNodeToMove, xtrigger, xfilter);
    }

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
    
    //for example: an XML_ATTRIBUTE_NODE is added to an XML_ATTRIBUTE_NODE
    if (!oRet) throw StructuralChangeInappropriate(this, this, "for example: an XML_ATTRIBUTE_NODE is added to an XML_ATTRIBUTE_NODE");
    
    //LibXML2 policy of conditionally freeing input nodes!!
    //xmlAddChild() has always freed the input if a node is returned that is not the input
    //it can also return NULL, in which case the input was not freed
    if (oRet != oNodeToMove) throw TextNodeMergedAndFreed(this);
    
    return oRet;
  }

  xmlNodePtr LibXmlBaseNode::docToRootIf(xmlNodePtr oNodeToCopy) {
    if (oNodeToCopy->type == XML_DOCUMENT_NODE) {
      if (!oNodeToCopy->children) throw NoRootNodeFound(this, m_pDoc->alias());
      oNodeToCopy = oNodeToCopy->children;
    }
    return oNodeToCopy;
  }

  void LibXmlBaseNode::traverse(bool (LibXmlBaseNode::*elementCallback)(xmlNodePtr oNode), xmlNodePtr oNode) {
    xmlNodePtr oParent, oNextNode = 0;
    
    NOT_CURRENTLY_USED("");

    //callback (only on this class because it is LibXML2 specific)
    //returns true if we are to continue
    if ((this->*elementCallback)(m_oNode)) {
      //down
      if      (m_oNode->children) oNextNode = m_oNode->children;
      //across
      else if (m_oNode->next)     oNextNode = m_oNode->next;
      //up [, up] and across
      else {
        oParent = m_oNode->parent;
        while (oParent && !oParent->next) oParent = oParent->parent;
        if (oParent) oNextNode = oParent->next;
      }

      if (oNextNode) traverse(elementCallback, oNextNode);
    }
  }

  IXmlBaseNode *LibXmlBaseNode::parseHTMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sHTML, const bool bRemoveRedundantNamespaceDeclarations, const bool bTranslateEntities) const {
    //see parseXMLInContextNoAddChild() below for documentation
    LibXmlBaseNode *pNewNode = 0;
    xmlNodePtr oNodeFirst; //filled out by the subsequent call
    xmlParserErrors errors;
    const char *sParseStart, *sEndOfDOCTYPE, *sStartRootTag;

    //XML_PARSE_NSCLEAN will delete the namespaces on the main node if the document has then on its root
    //thus rendering the main node INVALID by itself
    //SO: don't do this for xsl:stylesheet for example which must be a valid independent document for compilation
    int iOptions = XML_PARSE_RECOVER | XML_PARSE_NONET;
    if (bRemoveRedundantNamespaceDeclarations) iOptions |= XML_PARSE_NSCLEAN;
    if (bTranslateEntities)                    iOptions |= XML_PARSE_NOENT;

    //skip <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
    if ( sHTML[0] == '<' &&
         sHTML[1] == '!' &&
        (sEndOfDOCTYPE = strchr(sHTML, '>')) &&
        (sStartRootTag = strchr(sEndOfDOCTYPE, '<'))
    ) {
      sParseStart = sStartRootTag;
    } else
      sParseStart = sHTML;

    errors = xmlParseInNodeContext(m_oNode, sParseStart, strlen(sParseStart), iOptions, &oNodeFirst);

    if (errors == XML_ERR_OK && oNodeFirst) {
      pNewNode = LibXmlLibrary::factory_node(mmParent(), oNodeFirst, m_pDoc);
    } else {
      //TODO: parseHTMLInContextNoAddChild(...) in depth error reporting framework required here
      throw InvalidXML(this, sHTML);
    }

    IFDEBUG(m_pDoc->validityCheck("parseXMLInContextNoAddChild");)

    return pNewNode ? pNewNode->queryInterface((IXmlBaseNode*) 0) : 0;
  }

  IXmlBaseNode *LibXmlBaseNode::parseXMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sXML,
    const bool bRemoveRedundantNamespaceDeclarations,
    const bool bTranslateEntities
  ) const {
    //caller frees result
    //  which is a new node, the first child of this
    /*
    Enum xmlParserOption {
      XML_PARSE_RECOVER = 1 : recover on errors
      XML_PARSE_NOENT = 2 : substitute entities
      XML_PARSE_DTDLOAD = 4 : load the external subset
      XML_PARSE_DTDATTR = 8 : default DTD attributes
      XML_PARSE_DTDVALID = 16 : validate with the DTD
      XML_PARSE_NOERROR = 32 : suppress error reports
      XML_PARSE_NOWARNING = 64 : suppress warning reports
      XML_PARSE_PEDANTIC = 128 : pedantic error reporting
      XML_PARSE_NOBLANKS = 256 : remove blank nodes
      XML_PARSE_SAX1 = 512 : use the SAX1 interface internally
      XML_PARSE_XINCLUDE = 1024 : Implement XInclude substitition
      XML_PARSE_NONET = 2048 : Forbid network access
      XML_PARSE_NODICT = 4096 : Do not reuse the context dictionnary
      XML_PARSE_NSCLEAN = 8192 : remove redundant namespaces declarations
      XML_PARSE_NOCDATA = 16384 : merge CDATA as text nodes
      XML_PARSE_NOXINCNODE = 32768 : do not generate XINCLUDE START/END nodes
      XML_PARSE_COMPACT = 65536 : compact small text nodes; no modification of the tree allowed afterwards (will possibly crash if you try to modify the tree)
      XML_PARSE_OLD10 = 131072 : parse using XML-1.0 before update 5
      XML_PARSE_NOBASEFIX = 262144 : do not fixup XINCLUDE xml:base uris
      XML_PARSE_HUGE = 524288 : relax any hardcoded limit from the parser
      XML_PARSE_OLDSAX = 1048576 : parse using SAX2 interface before 2.7.0
      XML_PARSE_IGNORE_ENC = 2097152 : ignore internal document encoding hint
      XML_PARSE_BIG_LINES = 4194304 : Store big lines numbers in text PSVI field
    }
    xmlParserErrors: http://xmlsoft.org/html/libxml-xmlerror.html#xmlParserErrors
    */
    LibXmlBaseNode *pNewNode = 0;
    xmlNodePtr oNodeFirst; //filled out by the subsequent call
    xmlParserErrors errors;

    //XML_PARSE_NSCLEAN will delete the namespaces on the main node if the document has then on its root
    //thus rendering the main node INVALID by itself
    //SO: don't do this for xsl:stylesheet for example which must be a valid independent document for compilation
    int iOptions = XML_PARSE_RECOVER | XML_PARSE_NONET;
    if (bRemoveRedundantNamespaceDeclarations) iOptions |= XML_PARSE_NSCLEAN;
    if (bTranslateEntities)                    iOptions |= XML_PARSE_NOENT;

    errors = xmlParseInNodeContext(m_oNode, sXML, strlen(sXML), iOptions, &oNodeFirst);

    if (errors == XML_ERR_OK && oNodeFirst) {
      pNewNode = LibXmlLibrary::factory_node(mmParent(), oNodeFirst, m_pDoc);
    } else {
      //TODO: parseXMLInContextNoAddChild(...) in depth error reporting framework required here
      throw InvalidXML(this, sXML);
    }

    IFDEBUG(m_pDoc->validityCheck("parseXMLInContextNoAddChild");)

    return pNewNode ? pNewNode->queryInterface((IXmlBaseNode*) 0) : 0;
  }

  iDOMNodeType LibXmlBaseNode::nodeType() const {
    //specs are the same under 13, LibXML extends MSXML after value 12 (see IXML.h)
    return m_oNode ? (iDOMNodeType) m_oNode->type : NODE_DOCUMENT;
  }
  
  const char *LibXmlBaseNode::localName(const bool bDuplicate) const {
    //caller frees result
    //this is the local-name() only
    const char *sName = 0;

    if (m_oNode && m_oNode->name) sName = (const char*) m_oNode->name;
    else sName = "#document";

    if (bDuplicate) sName = MM_STRDUP(sName);
    
    return sName;
  }

  unsigned long LibXmlBaseNode::documentOrder() const {
    //not throwing errors on other node types...
    return xmlXPathDocumentOrder(m_oNode);    
  }

  size_t LibXmlBaseNode::position() const {
    size_t iPosition = 1;
    xmlNodePtr oNode = m_oNode;
    while (oNode = oNode->prev) iPosition++;
    return iPosition;
  }

  bool LibXmlBaseNode::isTransient() const {
    //transient definition: only there temporarily, certainly not there on server restart
    //transient nodes are not saved to the repository and thus only exist temporarily during server lifetime
    //is the node new temporary or in a temporary area?
    //this method DOES search up the tree for transient information on ancestors
    //implemented here for efficiency
    //natural parents only, no hardlinkery
    //ancestor::*[@gs:transient-area = 'yes' or @gs:transient = 'yes'] | self::*[@gs:transient = 'yes']
    bool bTransient = false;
    xmlNodePtr oAncestorNode, oOriginalNode;
    xmlAttrPtr oAttr;

    //self::*[@gs:transient = 'yes']
    if (m_oNode->type == XML_ELEMENT_NODE) {
      oAttr = m_oNode->properties;
      while (!bTransient && oAttr != NULL) {
        if (oAttr->ns && _STREQUAL((const char*) oAttr->ns->prefix, "gs")) {
          if (_STREQUAL((const char*) oAttr->name, "transient")) {
            if (xmlLibrary()->textEqualsTrue((const char*) oAttr->children->content)) bTransient = true;
          }
        }
        oAttr = oAttr->next;
      }
      
      //Top level hardlinks are attached to their hardlink place
      //their children are connected to their original parents (unless also hardlinks)
      oOriginalNode = xmlFirstHardLink(m_oNode);

      //ancestor::*[@gs:transient-area = 'yes' or @gs:transient = 'yes']
      oAncestorNode = oOriginalNode->parent;
      while (!bTransient && oAncestorNode != NULL) {
        oAttr = oAncestorNode->properties;
        while (!bTransient && oAttr != NULL) {
          if (oAttr->ns && _STREQUAL((const char*) oAttr->ns->prefix, "gs")) {
            if (_STREQUAL((const char*) oAttr->name, "transient")) {
              if (xmlLibrary()->textEqualsTrue((const char*) oAttr->children->content)) bTransient = true;
            } else if (_STREQUAL((const char*) oAttr->name, "transient-area")) {
              if (xmlLibrary()->textEqualsTrue((const char*) oAttr->children->content)) bTransient = true;
            }
          }
          oAttr = oAttr->next;
        }
        oAncestorNode = oAncestorNode->parent;
      }
    }

    return bTransient;
  }

  bool LibXmlBaseNode::is(const IXmlBaseNode *pNode, const bool bHardlinkAware, const bool bParentRouteAware) const {
    //hardlink aware: returns true if nodes are equal or hardlinks of each other
    //if types are different then return false
    bool bRet = false;
    const LibXmlBaseNode *pNodeType;
    xmlNodePtr oNode;

    if ( (pNode)
      && (pNodeType = dynamic_cast<const LibXmlBaseNode*>(pNode))
      && (oNode     = pNodeType->libXmlNodePtr())
    ) {
      if (bHardlinkAware) {
        bRet = xmlAreEqualOrHardLinked(m_oNode, oNode);
      } else if (oNode == m_oNode) {
        //GDB: *((xmlNodePtr )m_oParentRoute->sentinel->next->data)
        if (bParentRouteAware) {
          if (xmlListPointerCompare(pNodeType->m_oParentRoute, m_oParentRoute) == 1) bRet = true;
        } else bRet = true;
      }
    }

    return bRet;
  }

  const char *LibXmlBaseNode::namespacePrefix(const bool bReconcileDefault, const bool bDuplicate) const {
    //caller frees non-zero result
    //PERFORMANCE: potentially very expensive process
    const char *sPrefix = (const char*) xmlNamespacePrefix(m_oNode, REQUIRE_NAMESPACES, bReconcileDefault); //do not free!
    IFDEBUG(
      if (bReconcileDefault && (!sPrefix || !*sPrefix)) 
        throw CannotReconcileDefaultNamespace(this, "namespacePrefix()", this)
    );
    
    if (bDuplicate && sPrefix) sPrefix = MM_STRDUP(sPrefix);
    
    return sPrefix;
  }

  xmlAttrPtr LibXmlBaseNode::attribute(const char *sName, const char *sNamespace) const {
    //parameters:
    //  "place": no namespace name (FAST)
    //  "place", NAMESPACE_REPOSITORY (HREF): (FAST)
    //  "repository:place": use a prefix qualified singular name (EXPENSIVE)
    xmlAttrPtr oAtt        = 0;
    const char *sLocalName = 0;
    xmlNsPtr oNS           = 0;

    if (sNamespace) {
      sLocalName = sName;
    } else {
      oNS = getNsFromString(sName, &sLocalName); //will assign sLocalName always
      if (oNS) sNamespace = (const char*) oNS->href;
    }

    oAtt = xmlHasNsProp(m_oNode, (const xmlChar*) sLocalName, (const xmlChar*) sNamespace);

    //free up
    //if (sLocalName) MMO_FREE(sLocalName); //pointer in to the sName

    return oAtt;
  }

  const char *LibXmlBaseNode::namespaceHREF(const bool bDuplicate) const {
    //possible, but ILLEGAL in gs, that the ns is 0x0
    const char *sNamespaceHREF = (canHaveNamespace() && m_oNode->ns ? (const char*) m_oNode->ns->href : 0);
    return (bDuplicate ? MM_STRDUP(sNamespaceHREF) : sNamespaceHREF); 
  }

  const StringMap<const char*> *LibXmlBaseNode::namespaceDefinitions() const {
    //returns ZERO in the case of no definitions
    //caller frees result, AND contents
    StringMap<const char*> *pNamespaceDefinitions = 0;
    xmlNsPtr ns;

    if (ns = m_oNode->nsDef) {
      pNamespaceDefinitions = new StringMap<const char*>();
      do {
        if (ns->prefix) pNamespaceDefinitions->insert(MM_STRDUP((const char*) ns->prefix), MM_STRDUP((const char*) ns->href));
      } while (ns = ns->next);
    }
    return pNamespaceDefinitions;
  }

  void LibXmlBaseNode::addNamespaceDefinition(const char *sPrefix, const char *sHREF) {
    //xmlNewNs() will xmlStrDup() the sHREF and sPrefix
    //xmlNsPtr oNS = 
    xmlNewNs(m_oNode, (const xmlChar*) sHREF, (const xmlChar*) sPrefix);
  }

  void LibXmlBaseNode::setNamespace(const char *sHREF) {
    //used by TXml when setting up the internal doc
    xmlNsPtr oExistingNS = NULL;

    //xmlSearchNsByHref() returns a direct pointer to the NSDef found
    //find the namespace defined further up the tree
    //searches the node->doc root node first as m_oNode->parent might be NULL
    switch (m_oNode->type) {
      case XML_ELEMENT_NODE:
      case XML_ATTRIBUTE_NODE: {
        oExistingNS = xmlSearchNsWithPrefixByHref(m_oNode->doc, m_oNode, (const xmlChar*) sHREF);

        //xmlSetNs() DOES NOT create a copy, or re-xmlStrDup() the href or prefix
        //however, node->ns in LibXML2 points to the definition up the tree: it is SHARED
        if (oExistingNS) xmlSetNs(m_oNode, oExistingNS); //no xmlCopyNamespace(oExistingNS)
        else             throw CannotReconcileDefaultNamespace(this, "setNamespace() could not find or declare the [%s] namespace anywhere", this, MM_STRDUP(sHREF));
        break;
      }
      default: throw NodeWithoutNamespace(mmParent(), localName());
    }
  }

  bool LibXmlBaseNode::setNamespaceRoot(IXmlBaseNode *pFromNode) {
    //set this node as the namespace root
    //namespace defs including the default namespace may be declared on an ancestor of this root
    //thus, for document validity, all definitions, including the default, must be re-aligned to this root
    //  especially if the ancestor is removed later on (response:*)
    //
    //LIBXML2 DOES SHARE namespace definitions: a node->ns ALWAYS points to it's corresponding node->nsDef on an ancestor
    //  so we cannot xmlFreeNs(nsDef) without giving descendant node->ns pointers something to point to on an ancestor
    //  we should MOVE nsDef, rather than free them, but we need to check for multiple repeat nsDef on ancestors
    //we are doing this all in LibXML2, not with the IXml* interface
    const XmlAdminQueryEnvironment ibqe_namespace_organisation(this, m_pDoc);
    xmlNsPtr oRootNs, oXSLNs = 0, oGSNs = 0;
    xmlNodePtr oFromNode;
    const bool bIncrementalMode = (pFromNode); //usually new nodes within the document

    //some common NS for performance
    //create if not present
    oRootNs = m_oNode->nsDef;
    while (oRootNs) {
      //look for the XSL namespace also for xxx morphing
      //for our prefix setup only
      if (oRootNs->prefix) {
        if (xmlStrEqual(oRootNs->href, (xmlChar*) NAMESPACE_XSL)) oXSLNs = oRootNs;
        if (xmlStrEqual(oRootNs->href, (xmlChar*) NAMESPACE_GS))  oGSNs  = oRootNs;
      }
      oRootNs = oRootNs->next;
    }
    if (!oXSLNs) oXSLNs = xmlNewNs(m_oNode, (xmlChar*) NAMESPACE_XSL, (xmlChar*) NAMESPACE_XSL_ALIAS);
    if (!oGSNs)  oGSNs  = xmlNewNs(m_oNode, (xmlChar*) NAMESPACE_GS,  (xmlChar*) NAMESPACE_GS_ALIAS);

    //useful for incremental changes in the DOM like moves and copies into
    //to only analyse the new area of namespaces
    oFromNode = (pFromNode ? dynamic_cast<LibXmlBaseNode*>(pFromNode)->m_oNode : m_oNode);
    
    //copy all extra definitions,
    //because we cannot spot those declared in attributes like @match="gs:what"
    if (!bIncrementalMode) setNamespaceRootInternal_copyAncestorNsDefs();
    //re-aligns oNode namespaces (ns) to oRootNode (m_oNode) nsDefs also
    setNamespaceRootInternal_realignSubTree(oFromNode, oGSNs, oXSLNs);
    //start on children only, just remove now defunct nsDefs
    setNamespaceRootInternal_removeNsDefs(oFromNode);
    
    return true;
  }

  bool LibXmlBaseNode::setNamespaceRootInternal_copyAncestorNsDefs() {
    xmlNodePtr oAncestorNode;
    xmlNsPtr oRootNs, oNsDef;
    bool bFound;

    oAncestorNode = m_oNode->parent;
    while (oAncestorNode) {
      //cycle through nsDefs on ancestor
      oNsDef = oAncestorNode->nsDef;
      while (oNsDef) {
        //------------------------------ look for each nsDef on our root
        bFound = false;
        oRootNs = m_oNode->nsDef;
        while (oRootNs && !bFound) {
          //test on prefix, because we do not want to hit the default namespace match on href
          if (oNsDef->prefix && xmlStrEqual(oNsDef->prefix, oRootNs->prefix)) bFound = true;
          else oRootNs = oRootNs->next;
        }
        
        //------------------------------ if not found, create it
        if (!bFound) oRootNs = xmlNewNs(m_oNode, oNsDef->href, oNsDef->prefix);
        IFDEBUG(
          if (bFound && !xmlStrEqual(oNsDef->href, oRootNs->href)) throw MismatchedNamespacePrefixAssociation(this, (const char*) oNsDef->prefix, (const char*) oNsDef->href, (const char*) oAncestorNode->name);
        );

        oNsDef = oNsDef->next;
      }
      
      //always an original when hardlinked
      oAncestorNode = oAncestorNode->parent;
    }
    
    return true;
  }

  bool LibXmlBaseNode::setNamespaceRootInternal_realignSubTree(xmlNodePtr oNode, xmlNsPtr oGSNs, xmlNsPtr oXSLNs) {
    xmlNodePtr oAncestorNode;
    xmlAttrPtr oAttNode;
    xmlNsPtr oRootNs, oNsDef, oCurrentDefaultNsDef;
    bool bFoundOnRoot, bOutOfScope;

    //assert(oNode); //allow relaxed traversal to children
    assert(oGSNs);
    assert(oXSLNs);
    
    if ((oNode->type == XML_ELEMENT_NODE) || (oNode->type == XML_ATTRIBUTE_NODE)) {
      //hardlinks are handled below at the point that the original is found
      if (!xmlIsHardLink(oNode) && !xmlIsSoftLink(oNode)) { //not original
        //we also re-align the root node namespace to point to itself
        //although we will not delete it's nsDefs later on
        //don't repoint default namespace declerations
        //xxx => xsl xsl:namespace-alias:
        //  during transform XSL will change the xxx namespace to point to the XSL Transform namespace
        //  this means that xxx will point to 2 different namespaces if the output becomes part of the main database
        //  currently we hardcode ignoring this
        //  TODO: examine the servers knowledge of xsl:namespace-alias and make the namespace change ignore intelligent
        //primary namespace oNode->ns repoint
        //in GS all nodes always have an NS, which may be without prefix
        //  however, this function can work without NS as well
        if (oNode->ns) {
          if (oNode->ns->prefix) {
            //---------------------------------------- prefixed namespaces
            if (xmlStrEqual(oNode->ns->prefix, (xmlChar*) NAMESPACE_XML_ALIAS)) {
              //predefined xml namespace
              if (!xmlStrEqual(oNode->ns->href, (xmlChar*) NAMESPACE_XML))
                throw MismatchedNamespacePrefixAssociation(this, "XML namespace");
            } else {
              //descendant has a prefixed namespace which will be moved to the root
              bFoundOnRoot = false;
              oRootNs      = m_oNode->nsDef;
              //look for the prefix on our root, e.g. xmlns:gs="..."
              if (xmlStrEqual(oNode->ns->prefix, oGSNs->prefix)) {
                oRootNs      = oGSNs;
                bFoundOnRoot = true;
              }
              while (oRootNs && !bFoundOnRoot) {
                //test on prefix
                //if the HREF is different in general server then its an invalid document
                //we also do not want to hit the default namespace match on href
                if (xmlStrEqual(oRootNs->prefix, oNode->ns->prefix)) bFoundOnRoot = true;
                else oRootNs = oRootNs->next;
              }
              if (bFoundOnRoot) {
                //check that the HREF is the same
                //this is required by GS. the document is invalid if not
                if (!xmlStrEqual(oRootNs->href, oNode->ns->href)) {
                  if (xmlStrEqual(oNode->ns->prefix, (xmlChar*) NAMESPACE_XXX_ALIAS)) {
                    //all xxx prefix here have been given a direct NAMESPACE_XSL href by the
                    //  <xsl:namespace-alias stylesheet-prefix=xxx result-prefix=xsl />
                    //xsl:namespace-alias is acceptable in GS ONLY on xxx: prefix to xsl: prefix
                    //we still need to move the NS pointer to the root as all local NSDefs will be removed afterwards
                    if (xmlStrEqual(oNode->ns->href, (xmlChar*) NAMESPACE_XSL)) {
                      //ok, it is correct pointing to the local XSL namespace now after the aliasing
                      //re-point it to the root XSL namespace
                      oRootNs = oXSLNs;
                    } else {
                      //xxx: has been aliased but is now pointing to not XSL!
                      Debug::report("[%s] prefix mismatch only acceptable if pointing to XSL namespace\n  root: [%s]\n  node: [%s]", (const char*) oNode->ns->prefix, (const char*) oRootNs->href, (const char*) oNode->ns->href);
                    }
                  } else {
                    Debug::report("[%s] prefix mismatch\n  root: [%s]\n  node: [%s]", (const char*) oNode->ns->prefix, (const char*) oRootNs->href, (const char*) oNode->ns->href);
                  }
                }
              } else {
                //prefix not found on the root node, create it same as this parent
                oRootNs = xmlNewNs(m_oNode, oNode->ns->href, oNode->ns->prefix);
              }
              
              //assign it to our root nsDef
              //possible that it was already pointing to it
              oNode->ns = oRootNs;
            }
          } 
          else {
            //---------------------------------------- default namespaces
            //node->ns default namespace pointer to somewhere
            //lazy find the node with the nearest default namespace decleration
            //could use xmlSearchNs() but we want the node also
            
            //XML_ATTRIBUTE_NODE cannot have default namespaces
            assert(oNode->type != XML_ATTRIBUTE_NODE);
            
            bOutOfScope          = false;
            oCurrentDefaultNsDef = 0;
            oAncestorNode = oNode;
            while (oAncestorNode && !oCurrentDefaultNsDef) {
              oNsDef = oAncestorNode->nsDef;
              while (oNsDef && !oCurrentDefaultNsDef) {
                if (!oNsDef->prefix) oCurrentDefaultNsDef = oNsDef;
                else                 oNsDef               = oNsDef->next;
              }
              if (!oCurrentDefaultNsDef) {
                if (oAncestorNode == m_oNode) bOutOfScope = true;
                oAncestorNode = oAncestorNode->parent;
              }
            }
            //checks
            if (!oCurrentDefaultNsDef)
              throw MissingDefaultNamespaceDecleration(this, (const char*) oNode->ns->href, (const char*) oNode->name);
            if (!xmlStrEqual(oCurrentDefaultNsDef->href, oNode->ns->href))
              throw MismatchedNamespacePrefixAssociation(this, (const char*) oCurrentDefaultNsDef->prefix, (const char*) oNode->ns->href, (const char*) oNode->name);

            //we only need to do something if the default namespace definition is out of scope
            if (bOutOfScope) {
              //cannot already be an existing default namespace on oRootNode, 
              //because it would have found it
              oCurrentDefaultNsDef = xmlNewNs(m_oNode, oNode->ns->href, 0);
            }
            //we need to re-point in-scope ones also
            //we may have already created the new, closer root one
            oNode->ns = oCurrentDefaultNsDef;
          }
        }

        //----------------------------- copy result to other hardlinks
        if (xmlIsOriginalHardLink(oNode)) {
          NOT_COMPLETE("copy ns move result to other hardlinks");
        }

        //---------------------------------------- attributes, then children
        oAttNode = oNode->properties;
        while (oAttNode) {
          setNamespaceRootInternal_realignSubTree((xmlNodePtr) oAttNode, oGSNs, oXSLNs);
          oAttNode = oAttNode->next;
        }
        oNode = oNode->children;
        while (oNode) {
          setNamespaceRootInternal_realignSubTree(oNode, oGSNs, oXSLNs);
          oNode = oNode->next;
        }
      }
    }
    
    return true;
  }

  bool LibXmlBaseNode::setNamespaceRootInternal_removeNsDefs(xmlNodePtr oNode) {
    xmlNsPtr oNsDef, oNsNextDef, oDefaultNoPrefixNs;

    //not hardlinks
    //original hardlinks are handled below at the point that the original is found
    if ( (oNode->type == XML_ELEMENT_NODE)
      && (!xmlIsHardLink(oNode)) 
      && (!xmlIsSoftLink(oNode)) 
    ) { 
      //do not strip the root namespace node!
      if (oNode != m_oNode) {    
        //----------------------------- remove ns definitions
        //xmlns:*=...
        //default ns definitions xmlns=... must remain on this node
        //  there can be only one maximum
        oDefaultNoPrefixNs = 0;
        oNsDef             = oNode->nsDef;
        while (oNsDef) {
          oNsNextDef = oNsDef->next;
          if (oNsDef->prefix) xmlFreeNs(oNsDef);
          else                oDefaultNoPrefixNs = oNsDef;
          oNsDef = oNsNextDef;
        }
        
        //----------------------------- assign new ns definition if any
        if (oDefaultNoPrefixNs) {
          //node has an xmlns="..." decleration
          oNode->nsDef = oDefaultNoPrefixNs;
          oDefaultNoPrefixNs->next = NULL;
        } else {
          //node does not have an xmlns="..." decleration
          oNode->nsDef = 0;
        }
        
        //----------------------------- copy result to other hardlinks
        if (xmlIsOriginalHardLink(oNode)) {
          NOT_COMPLETE("copy ns-def removal result to other hardlinks");
        }
      }

      //----------------------------- traverse children
      oNode = oNode->children;
      while (oNode) {
        setNamespaceRootInternal_removeNsDefs(oNode);
        oNode = oNode->next;
      }
    }
    
    return true;
  }

  void LibXmlBaseNode::removeAttribute(const IXmlQueryEnvironment *pQE, const char *sName) const {
    const char *sLocalName = 0;
    xmlNsPtr oNS;

    if (oNS = getNsFromString(sName, &sLocalName)) {
      //namespace prefix included: look it up
      xmlUnsetNsProp(m_oNode, oNS, (const xmlChar*) sLocalName);
    } else {
      //no namespace
      xmlUnsetProp(m_oNode, (const xmlChar*) sName);
    }
  }

  const IXmlBaseNode *LibXmlBaseNode::softlinkTarget(const IXmlQueryEnvironment *pQE) const {
    /* caller frees result IF deifferent */
    const IXmlBaseNode *pNode = this;
    xmlListPtr vertical_parent_route_adopt;
    
    assert(!xmlIsSoftLink(m_oNode) || xmlListSize(m_oNode->hardlink_info->vertical_parent_route_adopt));
    
    if (xmlIsSoftLink(m_oNode)) {
      vertical_parent_route_adopt = xmlListDup(m_oNode->hardlink_info->vertical_parent_route_adopt);
      pNode = LibXmlLibrary::factory_node(
        mmParent(), 
        (xmlNodePtr) xmlListPopBack(vertical_parent_route_adopt),
        m_pDoc, 
        vertical_parent_route_adopt
      );
    }
    
    return pNode;
  }

  XmlNodeList<const IXmlBaseNode> *LibXmlBaseNode::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) const {
    //these new nodes are added to the NodeList vector, not the document vector
    //they are not freed by the document, but ny the NodeList object in its destructor
    //NAMESPACE AWARE: xpath requires the namespace prefixes registered and correct xpath namespace prefixes
    //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
    assert(pQE);
    assert(sXPath);

    const IXslXPathFunctionContext   *pXCtxt     = 0;
    const LibXslXPathFunctionContext *pXCtxtType = 0;
    XmlNodeList<const IXmlBaseNode> *pNodeList   = 0;

    //process any extended argchar
    char *sFinalXPath = (char*) sXPath; //freed if not equal
    size_t iLen;

    //LibXML stuff
    xmlXPathContextPtr oContext = 0;
    xmlXPathObjectPtr  oResult  = 0;

    if (*sXPath) {
      //argint OR argchar
      if (argchar) {
        iLen        = strlen(sXPath) + strlen(argchar) + 1;
        sFinalXPath = (char*) MMO_MALLOC(iLen);
        _SNPRINTF1(sFinalXPath, iLen, sXPath, argchar);
      } else if (argint) {
        iLen        = strlen(sXPath) + 22 + 1; //22 is enough for a 64 bit integer
        sFinalXPath = (char*) MMO_MALLOC(iLen);
        _SNPRINTF1(sFinalXPath, iLen, sXPath, *argint);
      }

      //request an appropriate context from the QueryEnvironment
      //this will include the current parent_route
      pXCtxt     = pQE->newXPathContext(this, sFinalXPath);
      pXCtxtType = dynamic_cast<const LibXslXPathFunctionContext*>(pXCtxt);
      oContext   = pXCtxtType->libXmlXPathContext();

      //check that we have a valid namespace cache in the doc
      //TODO: CannotRunXPathWithoutNamespaceCache: this looks more like a structural assert() problem, rather than a recoverable throw situation
      if (xmlLibrary()->maybeNamespacedXPath(sFinalXPath)) {
        if (!oContext->nsHash->nbElems) throw CannotRunXPathWithoutNamespaceCache(this, MM_STRDUP("is the wrong doc being used, e.g. the stylesheet?"));
      }

      //TODO: process xpath schemas properly
      //NOTE: thread safety is implemented at LibXml2 level
      assert(oContext->xfilter);
      oResult = xmlXPathEvalExpression((xmlChar*) sFinalXPath, oContext);

      //TODO: sXPath may be freed later on!
      //factory_const_nodeset will copy each xmlNodePtr in to separate LibXmlBaseNode
      //thus we do not need to prevent xmlFree(oResult) or xmlFree(oResult->nodesetval)
      if (oResult) pNodeList = LibXmlLibrary::factory_const_nodeset(mmParent(), oResult->nodesetval, m_pDoc, CREATE_IFNULL, sXPath, MORPH_SOFTLINKS); //sXPath = Locking Reason
    }
    
    //return something...
    if (!pNodeList) pNodeList = new XmlNodeList<const IXmlBaseNode>(this);

    //IFDEBUG(
      if (_STREQUAL(sXPath, "..") && pNodeList->size() == 0 && m_oNode && m_oNode->parent) {
        Debug::report("parent node access failure", rtWarning);
        //re-run the expression to debug...:
        xmlXPathEvalExpression((xmlChar*) sFinalXPath, oContext);
      }
    //)

    //free
    if (sFinalXPath != sXPath)   MMO_FREE(sFinalXPath);
    if (oResult)                 xmlXPathFreeObject(oResult);
    if (pXCtxt)                  MM_DELETE(pXCtxt);
    //if (oContext)                xmlFreeContext(oContext);                 //done in IXslXPathFunctionContext
    //if (oContext)                xmlXPathRegisteredNsCleanup(oContext);    //done in xmlXPathFreeContext()
    //if (oContext)                xmlXPathRegisteredFuncsCleanup(oContext); //done in xmlXPathFreeContext()

    return pNodeList;
  }

  const xmlNsPtr LibXmlBaseNode::getNsFromString(const char *sString, const char **sLocalName) const {
    //server frees result, caller frees nothing, NS points in to document
    //caller may additionally provide sLocalName variable for the resultant string parse output
    //node->getNsFromString(...) finds THE closest correctly *prefixed* oNS pointer on an ancestor node
    //GS prefixes are fixed and the same for any given namespace, e.g. gs:
    xmlNsPtr oNS                = 0;
    char *sPrefix               = 0;
    int iLen                    = 0;
    const char *sLocalNameStart = 0;

    UNWIND_EXCEPTION_BEGIN {
      if (sLocalNameStart = (const char*) xmlSplitQName3((const xmlChar*) sString, &iLen)) {
        sPrefix = (char*) MMO_MALLOC(iLen + 1);
        strncpy(sPrefix, sString, iLen);
        sPrefix[iLen] = 0;
        oNS = xmlSearchNs(m_oNode->doc, m_oNode, (const xmlChar*) sPrefix);
        if (!oNS) 
          throw CannotReconcileNamespacePrefix(this, MM_STRDUP(sPrefix));
      } else sLocalNameStart = sString;
    } UNWIND_EXCEPTION_END;

    //output
    if (sLocalName) *sLocalName = sLocalNameStart;
    
    //free up
    if (sPrefix) MMO_FREE(sPrefix);
    //if (sLocalNameStart) MMO_FREE(sPrefix); //pointer in to sLocalName
  
    UNWIND_EXCEPTION_THROW;
    
    return oNS;
  }

  void LibXmlBaseNode::localName(const IXmlQueryEnvironment *pQE, const char *sName) {
    //TODO: xmlNodeSetNameSecure(...)
    xmlNodeSetName(m_oNode, (const xmlChar*) sName);
  }

  percentage LibXmlBaseNode::similarity(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const {
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    const LibXmlBaseNode *pWithNodeType = dynamic_cast<const LibXmlBaseNode*>(pWithNode);
    percentage iSimilarity = 0;

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
    
    iSimilarity = xmlSimilarity(m_oNode, pWithNodeType->libXmlNodePtr(), NULL, xtrigger, xfilter);
    
    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return iSimilarity;
  }
  
  void* LibXmlBaseNode::convertTreeChangesToTXml(const void *tree_change_void, const void *pQE_void, const void *pvTXMLs_void) {
    const xmlTreeChangePtr tree_change = (const xmlTreeChangePtr)      tree_change_void;
    const IXmlQueryEnvironment *pQE    = (const IXmlQueryEnvironment*) pQE_void;
    vector<TXml*> *pvTXMLs             = (vector<TXml*>*)              pvTXMLs_void;
    
    TXml *pTXml;
    static const char           *sDescription   = "LibXml tree_changes";
    static const LibXmlBaseNode *pBaseNode      = 0;
    
    const char *sValue = 0;
    const IXmlBaseDoc *pDoc           = new LibXmlDoc( pQE, pQE->xmlLibrary(), "tree changes", tree_change->node->doc); //pQE as MM
    const LibXmlNode  *pNode          = new LibXmlNode(pQE, tree_change->node, NULL, pDoc, OURS);                       //pQE as MM
    LibXmlNode        *pRelatedNode   = 0;
    const LibXmlNode  *pValueNode     = 0; //temporary
    
    switch (tree_change->type) {
      case LibXml_changeName: {
        //name is copied now from the source new node
        //no TXml child doc or link to the original node necessary
        if (!tree_change->related_node) throw TXmlInvalid(pQE); //pQE as MM
        pValueNode = new LibXmlNode(pQE, tree_change->related_node, NULL, pDoc); //pQE as MM
        sValue     = pValueNode->localName();
        pTXml      = new TXml_ChangeName(pQE, pQE, sDescription, pBaseNode, pNode, sValue); //pQE as MM
        break;
      }
      case LibXml_setValue:   {
        //value is copied now from the source new node
        //no TXml child doc or link to the original node necessary
        if (!tree_change->related_node) throw TXmlInvalid(pQE); //pQE as MM
        pValueNode = new LibXmlNode(pQE, tree_change->related_node, NULL, pDoc); //pQE as MM
        sValue     = pValueNode->value(pQE);
        pTXml      = new TXml_SetValue(pQE, pQE, sDescription, pBaseNode, pNode, sValue); //pQE as MM
        break;
      }
      case LibXml_removeNode: {
        //removing an existing node in the target doc
        //no TXml child doc or link to the original node necessary
        pTXml = new TXml_RemoveNode(pQE, pQE, sDescription, pBaseNode, pNode);  //pQE as MM
        break;
      }
      case LibXml_copyChild:  {
        //copy the source tree in to this new sub-TXml
        //pNode points in to the TXml_MergeNode::childDoc
        //pNode may be NODE_TEXT, NODE_ATTRIBUTE, NODE_ELEMENT
        if (!tree_change->related_node) throw TXmlInvalid(pQE); //pQE as MM
        pRelatedNode = new LibXmlNode(pQE, tree_change->related_node, NULL, pDoc); //pQE as MM
        pTXml        = new TXml_CopyChild(pQE, pQE, sDescription, pBaseNode, pNode, pRelatedNode, NULL, 
                                          tree_change->position, DEEP_CLONE, SELECT_COPY);  //pQE as MM
        break;
      }
      case LibXml_moveChild: { 
        //moving an existing child's position in the target doc
        //no TXml child doc or link to the original node necessary
        if (!tree_change->related_node) throw TXmlInvalid(pQE); //pQE as MM
        pRelatedNode = new LibXmlNode(pQE, tree_change->related_node, NULL, pDoc); //pQE as MM
        pTXml        = new TXml_MoveChild(pQE, pQE, sDescription, pBaseNode, pNode, pRelatedNode, NULL, tree_change->position);  //pQE as MM
        break;
      }
      /* not relevant TXml cases
      case LibXml_hardlinkChild:   pTXml = new TXml_HardlinkChild(pQE, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode); break;} //pQE as MM
      case LibXml_softlinkChild:   pTXml = new TXml_SoftlinkChild(pQE, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode); break;} //pQE as MM
      case LibXml_mergeNode:       pTXml = new TXml_MergeNode(    pQE, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode); break;} //pQE as MM
      case LibXml_replaceNodeCopy: pTXml = new TXml_ReplaceNode(  pQE, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode); break;} //pQE as MM
      case LibXml_setAttribute:    pTXml = new TXml_SetAttribute( pQE, pQE, sDescription, pBaseNode, pSelectNode, sName, sValue); break;} //pQE as MM
      case LibXml_touchNode:       pTXml = new TXml_TouchNode(    pQE, pQE, sDescription, pBaseNode, pSelectNode, as); break;} //pQE as MM
      */
      default: throw TXmlUnknownTransactionType(pQE); //pQE as MM
    }

    pvTXMLs->push_back(pTXml);
    
    //free up
    if (pValueNode)   delete pValueNode;
    if (sValue)       MM_FREE(sValue);  //MM_STRDUP by TXml
    //if (pNode)        delete pNode;        //managed by TXml
    //if (pRelatedNode) delete pRelatedNode; //managed by TXml
    
    return NULL; //keep walking
  }

  vector<TXml*> *LibXmlBaseNode::differences(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const {
    vector<TXml*> *pvTXMLs = 0;
    xmlListPtr tree_changes = 0;
    const LibXmlBaseNode *pWithNodeType = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    pWithNodeType = dynamic_cast<const LibXmlBaseNode*>(pWithNode);
    tree_changes = xmlCreateTreeChangesList();
    pvTXMLs = new vector<TXml*>();
    xmlSimilarity(m_oNode, pWithNodeType->libXmlNodePtr(), tree_changes, xtrigger, xfilter);
    
    //Translate LibXML tree_changes in to TXml
    pvTXMLs->reserve(xmlListSize(tree_changes));
    xmlListWalk(tree_changes, LibXmlBaseNode::convertTreeChangesToTXml, pQE, pvTXMLs);
    
    //free up
    if (xfilter)      xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger)     xmlFreeNodeTriggerCallbackContext(xtrigger);
    if (tree_changes) xmlListDelete(tree_changes);

    return pvTXMLs;
  }
  
  const char *LibXmlBaseNode::nameAxisName(const IXmlQueryEnvironment *pQE) const {
    //caller frees non-zero result
    //name::?
    const char *sName = 0;
    const IXslXPathFunctionContext   *pXCtxt     = 0;
    const LibXslXPathFunctionContext *pXCtxtType = 0;
    xmlXPathContextPtr oContext = 0;
    
    pXCtxt     = pQE->newXPathContext(this);
    pXCtxtType = dynamic_cast<const LibXslXPathFunctionContext*>(pXCtxt);
    oContext   = pXCtxtType->libXmlXPathContext();
    
    sName = (const char*) xmlXPathNameAxisName(m_oNode, NULL, oContext);
    
    //free up
    //TODO: if (oContext) xmlXPathFreeContext(oContext); //frees oContext->nsHash
    
    return sName;
  }

  const char *LibXmlBaseNode::attributeValueDirect(const XmlAdminQueryEnvironment *pQE, const char *sName, const char *sNamespace, const char *sDefault) const {
    //DO NOT FREE result!
    //only available during compilation to direct locally declared XmlAdminQueryEnvironment contexts
    //usually declared: const XmlAdminQueryEnvironment ibqe_*;
    //  NO security is used
    //  NO deviations are checked
    //generally only used when checking things like @xmlsecurity:* settings
    const char *sValue    = 0;
    xmlAttrPtr oAtt = 0;

    if (oAtt = attribute(sName, sNamespace)) sValue = (const char*) oAtt->children->content;
    else                                     sValue = sDefault;
    
    return sValue;
  }

  const char *LibXmlBaseNode::valueDynamic(const IXmlQueryEnvironment *pQE, const char *sValue, const unsigned int iRecursions) const {
    //caller ONLY frees result IF different from sValue!!!
    //recursively processes the {} dynamic values
    //context processing: only should be called from within a Transform situation
    assert(pQE);

    LibXslTransformContext     *pCTxtType     = 0;
    LibXslXPathFunctionContext *pXPathContext = 0;
    const char *sOldValue,
               *sNewValue = sValue;
    unsigned int iLoop = iRecursions;

    //lets assume a LibXslTransformContext rather than visiting it
    if (pCTxtType = static_cast<LibXslTransformContext*>(pQE->transformContext())) {
      //http://xmlsoft.org/XSLT/html/libxslt-templates.html#xsltAttrTemplateValueProcessNode
      //const char *sNewValue = (const char*) xsltAttrTemplateValueProcess(pCTxtType->libXmlContext(), (const xmlChar*) sValue);
      //const char *sNewValue = (const char*) xsltEvalAttrValueTemplate(pCTxtType->libXmlContext(), m_oNode, (const xmlChar*) sName, (const xmlChar*) "");
      while (sNewValue && strchr(sNewValue, '{') && iLoop--) {
        sOldValue = sNewValue;
        sNewValue = (const char*) xsltAttrTemplateValueProcessNode(pCTxtType->libXmlContext(), (const xmlChar*) sNewValue, m_oNode, m_oParentRoute);
        if (sOldValue != sValue) MMO_FREE(sOldValue);
      }
    } 
    
    else {
      if (strchr(sNewValue, '{')) { //avoid creating the xpath context if possible
        pXPathContext = static_cast<LibXslXPathFunctionContext*>(pQE->newXPathContext(this));
        while (sNewValue && strchr(sNewValue, '{') && iLoop--) {
          sOldValue = sNewValue;
          sNewValue = (const char*) xmlTemplateValueProcess(pXPathContext->libXmlXPathContext(), (const xmlChar*) sNewValue);
          if (sOldValue != sValue) MMO_FREE(sOldValue);
        }
      }
    }
    
    //free up
    if (pXPathContext) delete pXPathContext;
    //if (pCTxtType)     delete pCTxtType; //pointer in to pQE

    return sNewValue;
  }

  void LibXmlBaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace) const {
    //set the attribute even if it is not there
    //NAMESPACE AWARE: name may include namespace prefix, e.g. rx:regex or not e.g. regex
    const char *sLocalName   = 0;
    xmlNsPtr oNS             = 0;
    const xmlChar *xValue    = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    LibXmlBaseDoc *pDoc      = dynamic_cast<LibXmlBaseDoc*>(m_pDoc);
    
    assert(pDoc);
    
    //get the namespace if any
    if (sNamespace) {
      sLocalName = sName;
      oNS = xmlSearchNsWithPrefixByHref(m_oNode->doc, m_oNode, (const xmlChar*) sNamespace);
      if (!oNS) throw CannotReconcileNamespaces(pQE, sNamespace); //pQE as MM
    } else {
      oNS = getNsFromString(sName, &sLocalName); //will assign sLocalName always
      if (sName != sLocalName && !oNS)  throw CannotReconcileNamespaces(pQE, sName); //pQE as MM
    }
    
    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    if (xValue = xmlEncodeEntitiesReentrant(pDoc->libXmlDocPtr(), (const xmlChar*) sValue)) {
      IFDEBUG(
        const char *sCurrentValue = (const char *) xmlGetNsProp(m_oNode, (const xmlChar*) sLocalName, oNS ? oNS->href : NULL);
        if (sCurrentValue) {
          if (_STREQUAL(sCurrentValue, xValue) && !_STREQUAL(xValue, "yes"))
            Debug::report("set-attribute change will not trigger because content is the same [%s]", (const char*) xValue);
          MM_FREE(sCurrentValue);
        }
      )

      //http://www.xmlsoft.org/html/libxml-entities.html
      xmlSetNsPropSecure(m_oNode, m_oParentRoute, oNS, (const xmlChar*) sLocalName, xValue, xtrigger, xfilter);
    }

    //free up
    if (xValue)   xmlFree((void*) xValue);
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
  }

  const char *LibXmlBaseNode::nodeValueInternal() const {
    xmlNode* child;
    const xmlChar *sValue = 0;

    switch (nodeType()) {
      case XML_TEXT_NODE:      sValue = m_oNode->content;break;
      case XML_ATTRIBUTE_NODE: sValue = m_oNode->children->content;break;
      case XML_NAMESPACE_DECL: sValue = m_oNode->name;break;
      case XML_ELEMENT_NODE:
        for (child = m_oNode->children; child && !sValue; child = child->next) {
          if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE)
            sValue = child->content;
        }
        break;
      default: NOT_COMPLETE("nodeValueInternal()");
    }

    return (const char*) sValue;
  }

  bool LibXmlBaseNode::isValue(const char *sValue) const {
    return _STREQUAL(nodeValueInternal(), sValue);
  }

  const char *LibXmlBaseNode::value(const IXmlQueryEnvironment *pQE) const {
    //caller frees result if non-zero
    //get value (singular)
    const XmlAdminQueryEnvironment ibqe_normalise_attribute(this, m_pDoc);
    const char *sNodeValueInternal, *sValue = 0;

    sNodeValueInternal = nodeValueInternal();
    sValue = (sNodeValueInternal ? MM_STRDUP(sNodeValueInternal) : 0);

    //XML specification requires that newlines are only \n and newlines will be normalised by this function
    //so we incorporate a special attribute "normalise" to allow c-string style escaping \r\n
    //note that HTTP newlines and Windows newlines are \r\n or &#13;&#10;
    //defaults to off
    if (sValue && *sValue && isNodeElement()) {
      const char *sNormalise = attributeValue(&ibqe_normalise_attribute, "normalise");
      if (sNormalise) {
        if (xmlLibrary()->textEqualsTrue(sNormalise)) {
          const char *sValueNormalised = xmlLibrary()->normalise(sValue);
          //swap them
          MMO_FREE(sValue);
          sValue = sValueNormalised;
        }
        MMO_FREE(sNormalise);
      }
    }

    return sValue;
  }

  const char *LibXmlBaseNode::value(const IXmlQueryEnvironment *pQE, const char *sValue) {
    LibXmlBaseDoc *pDoc      = 0;
    const xmlChar *xValue    = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    //security and optional triggers
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    //http://www.xmlsoft.org/html/libxml-entities.html
    if (pDoc = dynamic_cast<LibXmlBaseDoc*>(m_pDoc)) {
      if (xValue = xmlEncodeEntitiesReentrant(pDoc->libXmlDocPtr(), (const xmlChar*) sValue)) {
        xmlNodeSetContentSecure(m_oNode, m_oParentRoute, xValue, xtrigger, xfilter);
      }
    }
    /*
    switch (m_oNode->type) {
      case XML_ATTRIBUTE_NODE:
      case XML_TEXT_NODE:
      case XML_CDATA_SECTION_NODE: {
        xmlNodeSetContentSecure(m_oNode, (xmlChar*) sValue, xtrigger, xfilter);
        break;
      }
      default: {
        //XML_ATTRIBUTE_NODE has one child that is a XML_TEXT_NODE but it seems to ignore it
        xmlNode* child;
        for (child = m_oNode->children; child; child = child->next) {
          if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE) {
            xmlNodeSetContentSecure(child, (xmlChar*) sValue, xtrigger, xfilter);
            break;
          }
        }
      }
    }
    */

    //free up
    if (xValue)   xmlFree((void*) xValue);
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return sValue;
  }

  const char *LibXmlBaseNode::toString() const {
    stringstream sOut;
    xmlNsPtr ns;

    sOut << XmlBaseNode::toString();

    if (m_oNode) {
      sOut <<   "\n";
      sOut <<   "  ->type [" << typeName() << "]\n";
      sOut << "  ->parent_route [" 
           << (m_oParentRoute && xmlListSize(m_oParentRoute) ? currentParentRoute() : "no parent route")
           << "]\n";
      if (canHaveNamespace()) {
        ns = m_oNode->ns;
        sOut << "  ->ns [" << (ns ? (const char*) ns->href    : "0x0") << "]\n";
        sOut << "  ->ns->prefix [" << (ns && ns->prefix ? (const char*) ns->prefix  : "0x0") << "]\n";
      }
      if (isNodeElement()) 
        sOut << "  ->nsDef [" << (m_oNode->nsDef ? (const char*) m_oNode->nsDef->href : "0x0") << "]";
    } else sOut << "\n  blank xmlNodePtr!";

    return MM_STRDUP(sOut.str().c_str());
  }

  iAreaXMLIDPolicy LibXmlBaseNode::xmlIDPolicy() const {
    return (iAreaXMLIDPolicy) xmlXMLIDPolicy(m_oNode);
  }

  int LibXmlBaseNode::validityCheck(const char *sContext) const {
    int iErrors = 0;

    NOT_CURRENTLY_USED("validityCheck()");
    
    //[usually] does not throw LibXML2 errors, just pumps out to a FILE*
    //xmlLibrary()->clearErrorFunctions(); //prevent the overlayed throw architecture
    UNWIND_EXCEPTION_BEGIN {
#ifdef LIBXML_DEBUG_ENABLED //xmlversion.h
      try {
        iErrors = xmlDebugCheckNode(stderr, m_oNode, 1);
      } catch (exception &ex) {
        throw ValidityCheckFailure(StandardException(this, ex), sContext); //with note about stderr output
      }
#endif
      //in case LibXML2 is not using setGenericErrorFunc() today
      //and thus not throwing XMLGeneric exceptions
      if (iErrors) throw ValidityCheckFailure(this, sContext);
    } UNWIND_EXCEPTION_END;

    //xmlLibrary()->setErrorFunctions();

    UNWIND_EXCEPTION_THROW;

    return iErrors;
  }

  const char *LibXmlBaseNode::xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth, const bool bNoEscape) const {
    //caller frees result
    //need to omit the XML_DECL with the XML_SAVE_NO_DECL xmlSaveOption

    char *sML                           = 0;
    LibXmlBaseDoc *pDoc                 = dynamic_cast<LibXmlBaseDoc*>(m_pDoc);
    xmlDocPtr oDoc                      = pDoc->libXmlDocPtr();
    xmlBufferPtr nodeBuffer             = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    int iOptions = 0;

    //normal node: use xmlNodeDump
    //http://xmlsoft.org/html/libxml-tree.html#xmlBufNodeDump
    if (nodeBuffer = xmlBufferCreate()) {
      //filtering for security
      xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
      if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
      //go!
      //TODO: We should be using the XSLT output system xsltSaveResultTo
      //  because it pays attention to style->method
      //  xsltSaveResultTo(nodeBuffer, oDoc, xsltStylesheetPtr style);
      //  "text" XSLT_FUNC_TEXT
      if (bNoEscape) iOptions |= XML_SAVE_NOESCAPE; //XML_SAVE_NOESCAPE is an additional LibXML2 option
      iOptions |= XML_SAVE_NO_DECL;                 //never want that
      if (oDoc->type == XML_HTML_DOCUMENT_NODE) iOptions |= XML_SAVE_AS_HTML;
      //xmlNodeDumpSecure() => xmlNodeDumpOutputInternal()
      if (xmlNodeDumpSecure(nodeBuffer, oDoc, m_oNode, ZERO_INITIAL_INDENT, FORMAT_INDENT, xtrigger, xfilter, iMaxDepth, iOptions)) {
        //cheeky swap :)
        sML = (char*) nodeBuffer->content;
        nodeBuffer->content = 0;
      }
      if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
      if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
    }

    //free up
    if (nodeBuffer) xmlBufferFree(nodeBuffer);

    //remove any initial xml decleration in text
    /*
    const char *sEndDECL                = 0;
    int lenDECL                         = 0;
    int lenBuff                         = 0;
    if (sML && !strncmp(sML, "<?xml ", 6)) {
      if (sEndDECL = strstr(sML, "?>")) {
        sEndDECL += 2;
        if (*sEndDECL == '\n') sEndDECL++; //trailing new line
        lenBuff = strlen(sML);
        lenDECL = sEndDECL - sML;
        memmove((void*) sML, (const void*) sEndDECL, lenBuff - lenDECL + 1);
      }
    }
    */
    
    //remove trailing newlines
    /*
    size_t iLen;
    if (sML && *sML)
      for (iLen = strlen(sML) - 1; iLen && sML[iLen] == '\n'; iLen--) 
        sML[iLen] = '\0';
    */

    return sML;
  }

  void LibXmlBaseNode::getObject(const IXmlQueryEnvironment *pQE, const char *sXPath, XmlNodeList<IXmlBaseNode> **ppNodeList, char **psResult, int *piResult, bool *pbResult, iXPathType *piType) const {
    //this function evaluates xpath to an unknown type (the others get*Node() only get nodes)
    //these new nodes are added to the NodeList vector, not the document vector
    //they are not freed by the document, but ny the NodeList object in its destructor
    //NAMESPACE AWARE: xpath requires the namespace prefixes registered and correct xpath namespace prefixes
    //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
    assert(pQE);
    assert(sXPath);

    const IXslXPathFunctionContext   *pXCtxt     = 0;
    const LibXslXPathFunctionContext *pXCtxtType = 0;

    //LibXML stuff
    xmlXPathContextPtr oContext = 0;
    xmlXPathObjectPtr  oResult  = 0;

    *piType = TYPE_UNDEFINED;

    if (*sXPath) {
      //request an appropriate context from the QueryEnvironment
      pXCtxt     = pQE->newXPathContext(this, sXPath);
      pXCtxtType = dynamic_cast<const LibXslXPathFunctionContext*>(pXCtxt);
      oContext   = pXCtxtType->libXmlXPathContext();

      //check that we have a valid namespace cache in the doc
      //TODO: CannotRunXPathWithoutNamespaceCache: this looks more like a structural assert() problem, rather than a recoverable throw situation
      if (xmlLibrary()->maybeNamespacedXPath(sXPath)) {
        if (!oContext->nsHash->nbElems) throw CannotRunXPathWithoutNamespaceCache(this);
      }

      //TODO: process xpath schemas properly
      //NOTE: thread safety is implemented at LibXml2 level
      assert(oContext->xfilter);
      oResult = xmlXPathEvalExpression((xmlChar*) sXPath, oContext);

      if (oResult) {
        *piType = (iXPathType) oResult->type;
        switch (oResult->type) {
          case XPATH_UNDEFINED: break;

          case XPATH_XSLT_TREE:
          case XPATH_NODESET: if (ppNodeList) *ppNodeList = LibXmlLibrary::factory_nodeset(mmParent(), oResult->nodesetval, m_pDoc, CREATE_IFNULL); break;
          case XPATH_BOOLEAN: if (pbResult)   *pbResult   = (bool) oResult->boolval;             break;
          case XPATH_NUMBER:  if (piResult)   *piResult   = (int) oResult->floatval;             break;
          case XPATH_STRING:  if (psResult)   *psResult   = MM_STRDUP((char*) oResult->stringval); break; //caller free

          //TODO: more types
          case XPATH_POINT:
          case XPATH_RANGE:
          case XPATH_LOCATIONSET:
          case XPATH_USERS:
          default: throw XPathTypeUnhandled(this, oResult->type);
        }
      }
    }

    //free
    if (oResult)                 xmlXPathFreeObject(oResult);
    //if (oContext)                xmlXPathRegisteredNsCleanup(oContext);    //done in xmlXPathFreeContext()
    //if (oContext)                xmlXPathRegisteredFuncsCleanup(oContext); //done in xmlXPathFreeContext()
    //TODO: if (oContext)                xmlXPathFreeContext(oContext); //frees oContext->nsHash
  }

  IXmlBaseNode *LibXmlBaseNode::hardlinkRouteToAncestor(const IXmlBaseNode *pAncestor, const char *sLockingReason) const {
    //caller fees result
    assert(pAncestor);
    assert(m_oNode);

    const LibXmlBaseNode *pAncestorType = dynamic_cast<const LibXmlBaseNode*>(pAncestor);
    xmlNodePtr oAncestorNode      = pAncestorType->libXmlNodePtr();
    xmlNodePtr oBranch            = xmlHardLinkRouteToAncestor(m_oNode, oAncestorNode);

    return (oBranch ? LibXmlLibrary::factory_node(mmParent(), oBranch, m_pDoc, NULL, sLockingReason) : NULL);
  }

  bool LibXmlBaseNode::isHardLink() const {
    return xmlIsHardLink(m_oNode);
  }

  bool LibXmlBaseNode::isSoftLink() const {
    return xmlIsHardLink(m_oNode);
  }

  bool LibXmlBaseNode::isHardLinked() const {
    return xmlIsHardLinked(m_oNode);
  }

  bool LibXmlBaseNode::isOriginalHardLink() const {
    return xmlIsOriginalHardLink(m_oNode);
  }

  bool LibXmlBaseNode::isDeviant() const {
    //a node that has an original
    return xmlIsDeviant(m_oNode);
  }

  bool LibXmlBaseNode::hasDescendantDeviants(const IXmlBaseNode *pOriginal) const {
    //a hardlink that has sub-tree deviants, optionally checking for a specific original
    const LibXmlBaseNode *pOriginalType = dynamic_cast<const LibXmlBaseNode*>(pOriginal);
    return xmlHasDescendantDeviants(m_oNode, pOriginalType ? pOriginalType->m_oNode : NULL);
  }

  bool LibXmlBaseNode::hasDeviants(const IXmlBaseNode *pHardlink) const {
    //a node that has deviants, optionally in relation to a specific hardlink
    const LibXmlBaseNode *pHardlinkType = dynamic_cast<const LibXmlBaseNode*>(pHardlink);
    return xmlHasDeviants(m_oNode, pHardlinkType ? pHardlinkType->m_oNode : NULL);
  }

  bool LibXmlBaseNode::removeAndDestroyNode(const IXmlQueryEnvironment *pQE) {
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    if (m_oNode) {
      //filtering for security
      xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
      if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
      xmlUnlinkNodeSecure(m_oNode, m_oParentRoute, xtrigger, xfilter);
      //xmlFreeNode() => xmlFreePropList() => xmlFreeProp() => xmlRemoveID() => @xml:id x
      //we are explicitly freeing our own node:
      //  stop listening to the deregistrationNotify()s
      stopListeningForDeregistionNotify();
      //cannot free here because we still have a lock on it!!!!
      m_bFreeInDestructor = true;
    }

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return true;
  }
  
  bool LibXmlBaseNode::removeAndDestroyChildren(const IXmlQueryEnvironment *pQE) {
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    xmlNodePtr oChild = 0;

    //filtering for security
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
    while (oChild = m_oNode->children) {
      xmlUnlinkNodeSecure(oChild, m_oParentRoute, xtrigger, xfilter);
      xmlFreeNode(oChild); //these will vomit if they have locks
    }

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return true;
  }
  
  void LibXmlBaseNode::swapProperties(xmlNodePtr oNode1, xmlNodePtr oNode2) {
    NOT_CURRENTLY_USED("");
    //swap everything,
    //apart from parent, next, prev
    //DOES NOT:
    //  reconcile namespaces
    //  work between documents

    /*
    struct _xmlNode {
      void *  _private  : application data
      xmlElementType  type  : type number, must be second !
      unsigned int     locks : number of external objects currently using this node
      unsigned short   marked_for_destruction : if now not-lockable: xmlFree will set this to 1 to prevent further locks and reads
      const xmlChar * name  : the name of the node, or the entity
      struct _xmlNode * children  : parent->childs link
      struct _xmlNode * last  : last child link
      struct _xmlNode * parent  : child->parent link
      struct _xmlNode * next  : next sibling link
      struct _xmlNode * prev  : previous sibling link
      struct _xmlDoc *  doc : the containing document End of common p
      xmlNs * ns  : pointer to the associated namespace
      xmlChar * content : the content
      struct _xmlAttr * properties  : properties list
      xmlNs * nsDef : namespace definitions on this node
      void *  psvi  : for type/PSVI informations
      unsigned short  line  : line number
      unsigned short  extra : extra data for XPath/XSLT
    }
    */
    IFDEBUG(assert(oNode1->doc == oNode2->doc);)

    xmlNodePtr oNode1Copy = base_object_clone(oNode1);
    oNode1->_private   = oNode2->_private;
    oNode1->type       = oNode2->type;
    oNode1->name       = oNode2->name;
    oNode1->children   = oNode2->children;
    oNode1->last       = oNode2->last;
    //parent, next, prev are all position, we are swapping only properties
    //doc MUST already be the same
    oNode1->ns         = oNode2->ns;
    oNode1->content    = oNode2->content;
    oNode1->properties = oNode2->properties;
    oNode1->nsDef      = oNode2->nsDef;
    oNode1->psvi       = oNode2->psvi;
    oNode1->line       = oNode2->line;
    oNode1->extra      = oNode2->extra; //maybe...?

    oNode2->_private   = oNode1Copy->_private;
    oNode2->type       = oNode1Copy->type;
    oNode2->name       = oNode1Copy->name;
    oNode2->children   = oNode1Copy->children;
    oNode2->last       = oNode1Copy->last;
    //parent, next, prev are all position, we are swapping only properties
    //doc MUST already be the same
    oNode2->ns         = oNode1Copy->ns;
    oNode2->content    = oNode1Copy->content;
    oNode2->properties = oNode1Copy->properties;
    oNode2->nsDef      = oNode1Copy->nsDef;
    oNode2->psvi       = oNode1Copy->psvi;
    oNode2->line       = oNode1Copy->line;
    oNode2->extra      = oNode1Copy->extra; //maybe...?

    MMO_FREE(oNode1Copy); //free because was a malloc
  }

  bool LibXmlBaseNode::clearMyCachedStylesheets(const IXmlQueryEnvironment *pQE) {
    return LibXslNode::clearMyCachedStylesheets(pQE, this);
  }

  IXmlBaseDoc *LibXmlBaseNode::transform(
    const IMemoryLifetimeOwner *pMemoryLifetimeOwner,
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode, const IXmlBaseNode *pIncludeDirectory
  ) const {
    //caller frees ALL parameters including pXslStylesheetNode (cloned)
    //QE inheritance is carried out where needed, i.e. in the xslCommand_transform(...)
    //this transform may well use custom namespaces and commands built in to this thread by our LibXslModules
    //  calls will arrive at the LibXslModule and be routed to the registering object (Request, Repository, Database, etc)
    if (!pXslStylesheetNode) throw NoStylesheet(this); //recoverable: if XSL node was found through user xpath...

    IXmlBaseDoc *pNewDoc                       = 0;
    IXslDoc *pXslDoc                           = 0;
    const IXmlBaseNode *pXslStylesheetNodeType = 0;
    IXmlQueryEnvironment *pTransformQE         = 0;
    IXslTransformContext *pTCtxt               = 0;

    //pXslDoc will be locked. unlock after finished below
    //cacheing, parsing and locking needs to take place all together
    //this is because cacheing should use the original uniqueXPathToNode(xsl node)
    //GDB: p pXslStylesheetNode->queryInterfaceIXmlBaseNode()->x()
    pXslStylesheetNode->clearRedundantStylesheetNodeCacheItems(pQE);

    m_pDoc->stylesheetCacheLock(); {
      pXslDoc = pXslStylesheetNode->checkStylesheetNodeCache(pQE);
      if (!pXslDoc) {
        //preprocessToStylesheet:
        //  make an LibXslDoc
        //  set parent
        //  translate includes
        //  parse -> compiled member variable
        //  clear parent
        pXslDoc = pXslStylesheetNode->preprocessToStylesheet(pQE, pIncludeDirectory);

        //cache the doc only if the node is not transient
        //stylesheet nodes can come from transient tmp areas
        //especially in the case of server-side-transforms with transformed stylesheets
        if ((pXslStylesheetNodeType = pXslStylesheetNode->queryInterface((const IXmlBaseNode*) 0))
            && !pXslStylesheetNodeType->isTransient()
        ) {
          pXslStylesheetNode->cacheStylesheetNode(pQE, pXslDoc);
        }
      }
      pXslDoc->incrementLockCount();
    } m_pDoc->stylesheetCacheUnlock();

    UNWIND_EXCEPTION_BEGIN {
      //-------------------------------------------- QE / context inheritance
      //the original QE is left unaffected by this sub-transform
      //this inherit function will:
      //  make a copy of the security state:   IXmlSecurityContext->inherit()
      //  make a copy of the trigger strategy: IXmlTriggerContext->inherit()
      //  inherit emo, all settings and the document
      //  inherit some of / or create a, transform context:   IXslTransformContext->inherit(pXSLDoc2)
      pTransformQE = pQE->inherit(pXslDoc);

      //-------------------------------------------- setup context
      //ask pQE for the transformContext
      //if one has not been created, a new one will be
      //note that the pQE will already have inherited the parent one if relevant
      //in the calling xslCommand_transform() for example
      if (pTCtxt = pTransformQE->transformContext()) {
        pTCtxt->setStartNode(this);
        pTCtxt->addParams(pParamsChar);
        pTCtxt->addParams(pParamsInt);
        pTCtxt->addParams(pParamsNodeSet);
        pTCtxt->setMode(sWithMode);

        //-------------------------------------------- run
        pNewDoc = pTCtxt->transform();
        if (pTransformQE->nodes()->size()) {
          //Debug::report("transform selected [%s] nodes", pTransformQE->nodes()->size());
          pQE->reportNodes(pTransformQE->nodes());
        }
      } else throw TransformContextRequired(this);
    } UNWIND_EXCEPTION_END;

    //-------------------------------------------- free up
    pXslDoc->decrementLockCount(); //xsltUnlockStylesheet(oXSLDoc2); //UNWIND_EXCEPTION_THROW is later on
    //if (pXslDoc) delete pXslDoc;   //cache released
    //if (pTCtxt)  delete pTCtxt;    //owned by the pTransformQE
    if (pTransformQE) delete pTransformQE;

    UNWIND_EXCEPTION_THROW;

    return pNewDoc;
  }

  void LibXmlBaseNode::transform(
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    IXmlBaseNode *pOutputNode,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode, const IXmlBaseNode *pIncludeDirectory
  ) const {
    //caller frees ALL parameters including pXslStylesheetNode (cloned)
    //QE inheritance is carried out where needed, i.e. in the xslCommand_transform(...)
    //this transform may well use custom namespaces and commands built in to this thread by our LibXslModules
    //  calls will arrive at the LibXslModule and be routed to the registering object (Request, Repository, Database, etc)
    if (!pXslStylesheetNode) throw NoStylesheet(this); //recoverable: if XSL node was found through user xpath...
    if (!pOutputNode) throw NoOutputDocument(this);     //recoverable: if output node was found through user xpath...

    IXslDoc *pXslDoc                           = 0;
    const IXmlBaseNode *pXslStylesheetNodeType = 0;
    IXmlQueryEnvironment *pTransformQE         = 0;
    IXslTransformContext *pTCtxt               = 0;

    //pXslDoc will be locked. unlock after finished below
    //cacheing, parsing and locking needs to take place all together
    //this is because cacheing should use the original uniqueXPathToNode(xsl node)
    //GDB: p pXslStylesheetNode->queryInterfaceIXmlBaseNode()->x()
    pXslStylesheetNode->clearRedundantStylesheetNodeCacheItems(pQE);

    m_pDoc->stylesheetCacheLock(); {
      pXslDoc = pXslStylesheetNode->checkStylesheetNodeCache(pQE);
      if (!pXslDoc) {
        //preprocessToStylesheet:
        //  make an LibXslDoc
        //  set parent
        //  translate includes
        //  parse -> compiled member variable
        //  clear parent
        pXslDoc = pXslStylesheetNode->preprocessToStylesheet(pQE, pIncludeDirectory);

        //cache the doc only if the node is not transient
        //stylesheet nodes can come from transient tmp areas
        //especially in the case of server-side-transforms with transformed stylesheets
        if ((pXslStylesheetNodeType = pXslStylesheetNode->queryInterface((const IXmlBaseNode*) 0))
            && !pXslStylesheetNodeType->isTransient()
        ) {
          pXslStylesheetNode->cacheStylesheetNode(pQE, pXslDoc);
        }
      }
      pXslDoc->incrementLockCount();
    } m_pDoc->stylesheetCacheUnlock();

    UNWIND_EXCEPTION_BEGIN {
      //-------------------------------------------- QE / context inheritance
      //the original QE is left unaffected by this sub-transform
      //this inherit function will:
      //  make a copy of the security state:   IXmlSecurityContext->inherit()
      //  make a copy of the trigger strategy: IXmlTriggerContext->inherit()
      //  inherit emo, all settings and the document
      //  inherit some of / or create a, transform context:   IXslTransformContext->inherit(pXSLDoc2)
      pTransformQE = pQE->inherit(pXslDoc);

      //-------------------------------------------- setup context
      //ask pQE for the transformContext
      //if one has not been created, a new one will be
      //note that the pQE will already have inherited the parent one if relevant
      //in the calling xslCommand_transform() for example
      pTCtxt = pTransformQE->transformContext();
      pTCtxt->setStartNode(this);
      pTCtxt->addParams(pParamsChar);
      pTCtxt->addParams(pParamsInt);
      pTCtxt->addParams(pParamsNodeSet);
      pTCtxt->setMode(sWithMode);

      //-------------------------------------------- run
      pTCtxt->transform(pOutputNode);
      //datatbase:select-input-nodes allows xpath style return of existing nodes from an XSLT
      //report them to the parent QE (accumulative)
      if (pTransformQE->nodes()->size()) {
        //Debug::report("transform selected [%s] nodes", pTransformQE->nodes()->size());
        pQE->reportNodes(pTransformQE->nodes());
      }
    } UNWIND_EXCEPTION_END;

    //-------------------------------------------- free up
    pXslDoc->decrementLockCount(); //xsltUnlockStylesheet(oXSLDoc2); //UNWIND_EXCEPTION_THROW is later on
    //if (pXslDoc) delete pXslDoc;   //cache released
    //if (pTCtxt)  delete pTCtxt;    //owned by the pTransformQE
    if (pTransformQE) delete pTransformQE;

    UNWIND_EXCEPTION_THROW;
  }

  IXmlBaseNode *LibXmlBaseNode::copyChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToCopy, const bool bDeepClone, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition, const iXMLIDPolicy iCopyXMLIDPolicy) {
    assert(m_oNode);
    assert(pNodeToCopy);

    //caller manages pNode
    //caller additonally frees result (new node)
    //
    //bDeep:
    //  false: element, namespace and namespace definitions
    //  true:  element, namespace and namespace definitions, properties and children
    //
    //(bSameDoc):
    //  namespaces should be reconciled against the target document
    //
    //xml:id: ovbviously cannot be copied in the same document
    //  xmlid_default = 0,    //depends on operation and same-document
    //  copy_xmlid,           //one doc to another, e.g. @xml -> main tree
    //  ignore_xmlid,         //main tree RegularX commands -> <request>
    //  morph_to_xmlidcopy,   //saving   transactions
    //  morph_from_xmlidcopy
    //
    //LibXml2: http://xmlsoft.org/html/libxml-tree.html#xmlCopyNode
    //        (0) = element  NOTE: DOES NOT copy the namespace
    //  true  (1) = element, namespace, namespace declerations, properties, children (recursive)
    //        (2) = element, namespace, namespace declerations, properties
    //        NOTE: added by Annesley to use the namespace resolution without copying the properties
    //  false (3) = element, namespace, namespace declerations
    //namespace copying will create a default: namespace prefix and default: nsDef if the node is in the default namespace
    IXmlHasNamespaceDefinitions *pRootNodeType   = 0;
    LibXmlBaseNode              *pNewNode        = 0;
    const LibXmlBaseNode        *pNodeToCopyType = 0;
    xmlDocPtr oSourceDoc   = 0;
    xmlDocPtr oDestDoc     = 0;
    bool bSameDoc          = false;
    xmlNodePtr oNodeToCopy = 0,
               oNewNode    = 0,
               oDestParent = 0;

    //situation
    pNodeToCopyType = dynamic_cast<const LibXmlBaseNode*>(pNodeToCopy);
    if (!pNodeToCopyType) throw NativeLibraryClassRequired(this, MM_STRDUP("LibXmlBaseNode"));
    oSourceDoc      = pNodeToCopyType->m_oNode->doc;
    oDestDoc        = m_oNode->doc;
    bSameDoc        = (oSourceDoc == oDestDoc);
    oNodeToCopy     = pNodeToCopyType->m_oNode;

    //destination position can be specified by pBeforeNode OR iDestinationPosition
    //NOTE: child() will return NULL if iDestinationPosition == -1
    if (iDestinationPosition) {
      if (pBeforeNode) delete pBeforeNode;
      if (!isNodeElement()) throw SubPositioningOnNonElement(this, "copy to destination", this);
      pBeforeNode = child(pQE, iDestinationPosition);
    }
      
    //copy root elements, not doc elements
    oNodeToCopy = docToRootIf(oNodeToCopy);

    //IFDEBUG(m_pDoc->validityCheck("pre copyChild() check on destination (this)");)
    //IFDEBUG(pNodeToCopy->document()->validityCheck("pre copyChild() check on source (pNodeToCopy)");)

    UNWIND_EXCEPTION_BEGIN {
      if (bSameDoc) {
        //------------------------------------------------------- same document
        //no costly namespace reconciliation?
        //same document
        //no xml:id direct copies, possible morphing if requested. new xml:ids assigned by the caller if they want to
        if (iCopyXMLIDPolicy == copy_xmlid) throw XMLID_DuplicationIssue(this, MM_STRDUP("cannot do a direct in-document copy of xml:id, creates conflicts"));
        //TODO: explicit iCopyXMLIDPolicy == xmlid_default ? ignore_xmlid : iCopyXMLIDPolicy); //same doc initially
        //TODO: what if the node is copied to a place where it's ns is out of scope?
        oNewNode = xmlDocCopyNode(oNodeToCopy, oSourceDoc, (bDeepClone ? 1 : 3));
        addChildToMeOrSetRootSecure(pQE, oNewNode, pBeforeNode);
        pNewNode = LibXmlLibrary::factory_node(mmParent(), oNewNode, m_pDoc, NULL, "copyNode()");
      } else {
        //------------------------------------------------------- between documents
        if (isNodeDocument()) {
          if (m_oNode->children) throw MultipleRootNodes(this, m_pDoc->alias());
          oDestParent = 0;
        } else {
          oDestParent = m_oNode;
        }

        //------------------------------ xmlDOMWrapCloneNode(...) LibXML2 doc:
        //References of out-of scope ns-decls are remapped to point to @destDoc:
        //1) If @destParent is given, then nsDef entries on element-nodes are used
        //     ensures that the tree is namespace wellformed by creating additional ns-decls where needed.
        //2) If *no* @destParent is given, then @destDoc->oldNs entries are used.
        //     This is the case when you don't know already where the cloned branch will be added to.
        //
        //parameters:
        //  ctxt: the optional context for custom processing
        //  sourceDoc:  the optional sourceDoc
        //  node: the node to start with
        //  *resNode:  the clone of the given @node *****!!! this is the output clone, not an input
        //  destDoc:  the destination doc
        //  destParent: the optional new parent of @node in @destDoc
        //  deep: descend into child if set
        //  options:  option flags
        //Returns:  0 if the operation succeeded, 1 if a node of unsupported (or not yet supported) type was given, -1 on API/internal errors.
        //
        //Note that, since prefixes of already existent ns-decls can be shadowed by this process, it could break QNames in attribute values or element content.
        //What to do with XInclude? Currently this returns an error for XInclude.

        //------------------------------ Annesley:
        //xmlDOMWrapCloneNode(...) ensures:
        //  xml:ids are updated using xmlAddID(...)
        //TODO: manage SHALLOW_CLONE between documents (NOT_USED_CURRENTLY)
        //TODO: explicit iCopyXMLIDPolicy == xmlid_default ? ignore_xmlid : iCopyXMLIDPolicy);
        //TODO: hardlinks?

        if (xmlDOMWrapCloneNode(UNUSED_NULL,
          oSourceDoc,
          oNodeToCopy,
          &oNewNode,    //CREATED by xmlDOMWrapCloneNode()
          oDestDoc,
          oDestParent,  //zero if this is an XML_DOCUMENT_NODE, so that doc->oldNS is used
          bDeepClone,
          NO_OPTIONS
        ) != 0) {
          throw XmlGenericException(this, MM_STRDUP("LibXml2 API internal errors during xmlDOMWrapCloneNode"));
        }

        //IFDEBUG(m_pDoc->validityCheck("post copyChild() check on destination (this)");)
        //IFDEBUG(pNodeToCopy->document()->validityCheck("post copyChild() check on source (pNodeToCopy)");)

        //add it to the destination, because xmlDOMWrapCloneNode() doesn't!
        oNewNode = addChildToMeOrSetRootSecure(pQE, oNewNode, pBeforeNode);
        pNewNode = LibXmlLibrary::factory_node(mmParent(), oNewNode, m_pDoc, NULL, "copyNode()");

        if (isNodeDocument()) {
          //bring namespaces in to scope for document situations
          if (pRootNodeType = pNewNode->queryInterface((IXmlHasNamespaceDefinitions*) 0)) {
            pRootNodeType->setNamespaceRoot();
          } else throw InterfaceNotSupported(this, MM_STRDUP("IXmlHasNamespaceDefinitions*"));
        }

        //update with any new namespaces
        if (pNewNode->isNodeElement()) {
          m_pDoc->cacheAllPrefixedNamespaceDefinitionsForXPath(pNewNode);
          if (document()->documentRootNamespacing()) document()->moveAllPrefixedNamespaceDefinitionsToAppropriateRoots(pNewNode);
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    UNWIND_DELETE_IF_EXCEPTION(pNewNode); //return value

    UNWIND_EXCEPTION_THROW;

    return pNewNode;
  }

  IXmlBaseNode *LibXmlBaseNode::moveChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNodeToMove, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition, const iXMLIDPolicy iCopyXMLIDPolicy) {
    assert(pNodeToMove);

    //no clone so return = pNodeToMove
    //caller handles result
    //this = parent, pNodeToMove = node to unlink and move
    //TODO: work out security issues around successfully removing the node and failing to add it to the destination
    //
    //xml:id: cannot to a copy and then remove source, because the xmlid conflicts at the copy point
    //need to move directly, if same document
    //if it is a different document then we can do a copy and remove
    IXmlBaseNode         *pNewNode        = 0;  //only between docs
    LibXmlBaseNode *pNodeToMoveType       = 0;
    xmlNodePtr oNodeToMove = 0;
    xmlDocPtr oSourceDoc   = 0;
    xmlDocPtr oDestDoc     = 0;
    bool      bSameDoc     = false;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    //situation
    pNodeToMoveType = dynamic_cast<LibXmlBaseNode*>(pNodeToMove);
    if (!pNodeToMoveType) throw NativeLibraryClassRequired(this, MM_STRDUP("LibXmlBaseNode"));
    oNodeToMove     = pNodeToMoveType->m_oNode;
    oSourceDoc      = oNodeToMove->doc;
    oDestDoc        = m_oNode->doc;
    bSameDoc        = (oSourceDoc == oDestDoc);

    //destination position can be specified by pBeforeNode OR iDestinationPosition
    //NOTE: child() will return NULL if iDestinationPosition == -1
    if (iDestinationPosition) {
      if (pBeforeNode) delete pBeforeNode;
      pBeforeNode = child(pQE, iDestinationPosition);
    }
      
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

    if (bSameDoc) {
      //------------------------------------------------------- same document
      //no costly namespace reconciliation
      //same document
      //move root elements, not doc elements
      oNodeToMove = docToRootIf(oNodeToMove);

      xmlUnlinkNodeSecure(oNodeToMove, pNodeToMoveType->m_oParentRoute, xtrigger, xfilter); //not implicit in xmlAddChild()
      addChildToMeOrSetRootSecure(pQE, oNodeToMove, pBeforeNode);
    } else {
      //------------------------------------------------------- between documents
      //revert to the intelligent copy logic if this is an inter-document move
      //re-calculate namespace pointers
      //TODO: visitMoveChild(...) change to a between documents move for speed? but need to take care of the xml:ids...
      //  maybe using xmlReconciliateNs(...) and custom xml:id management or a new xmlDOMWrapAdopt?
      pNewNode = copyChild(pQE, pNodeToMove, DEEP_CLONE, pBeforeNode, iCopyXMLIDPolicy);
      //this is a move: remove source from other source doc
      pNodeToMove->removeAndDestroyNode(pQE);
      delete pNodeToMove;
      //new node
      pNodeToMove = pNewNode;
    }

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return pNodeToMove;
  }
  
  IXmlBaseNode *LibXmlBaseNode::deviateNode(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pHardlink, const IXmlBaseNode *pNewNode, const char *sAttributeName, const iDeviationType iType) {
    //causes a node to BECOME deviated!
    xmlNodePtr oHardlink, oOriginal, oDeviant;
    LibXmlBaseNode       *pHardlinkType = dynamic_cast<LibXmlBaseNode*>(pHardlink);
    const LibXmlBaseNode *pNewNodeType  = dynamic_cast<const LibXmlBaseNode*>(pNewNode);

    oHardlink = pHardlinkType->m_oNode;
    if (sAttributeName) {
      //sAttributeName can be prefix qualified, e.g. meta:name (EXPENSIVE)
      //TODO: pass in the optional attribute-namespace
      oOriginal = (xmlNodePtr) attribute(sAttributeName);
      oDeviant  = (xmlNodePtr) pNewNodeType->attribute(sAttributeName);
    } else {
      oOriginal = m_oNode;
      oDeviant  = pNewNodeType->m_oNode;
    }
    xmlDeviateNode(oOriginal, oHardlink, oDeviant, iType);

    return this;
  }

  IXmlBaseNode *LibXmlBaseNode::hardlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCommonChild, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition) {
    //caller frees result
    //this = the pAdditionalParent
    IXmlBaseNode *pNewNode = 0;
    xmlNodePtr    oNode    = 0;
    LibXmlBaseNode       *pCommonChildType = dynamic_cast<LibXmlBaseNode*>(pCommonChild);
    const LibXmlBaseNode *pBeforeNodeType  = dynamic_cast<const LibXmlBaseNode*>(pBeforeNode);
    if (!pCommonChildType) throw NativeLibraryClassRequired(this, MM_STRDUP("LibXmlBaseNode"));
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    //destination position can be specified by pBeforeNode OR iDestinationPosition
    //NOTE: child() will return NULL if iDestinationPosition == -1
    if (iDestinationPosition) {
      if (pBeforeNode) delete pBeforeNode;
      pBeforeNode = child(pQE, iDestinationPosition);
    }
      
    //hardlinking only allows element to element and will throw errors otherwise
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
    //xmlHardlinkInfoChild(xmlNodePtr pAdditionalParent, xmlNodePtr pCommonChild, xtrigger, xfilter)
    if (oNode = xmlHardlinkChild(m_oNode,
      m_oParentRoute,
      pCommonChildType->m_oNode,
      (pBeforeNodeType ? pBeforeNodeType->m_oNode : NULL), //optional!
      xtrigger,
      xfilter)
    ) {
      pNewNode = LibXmlLibrary::factory_node(mmParent(), oNode, m_pDoc);
    }
    pCommonChildType->m_pDoc = m_pDoc; //cross document hard linking is not usually supported...

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return pNewNode ? pNewNode->queryInterface((IXmlBaseNode*) 0) : 0;
  }
  
  IXmlBaseNode *LibXmlBaseNode::mergeNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNewNode, const iXMLIDPolicy iCopyXMLIDPolicy) {
    //returns this!
    //mergeNode() in-place morphing
    const LibXmlBaseNode *pNewNodeType    = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    NOT_CURRENTLY_USED("Database::mergeNode() being used with TXML instead");
    
    //or (do a removeNode and then a moveNode)
    UNWIND_EXCEPTION_BEGIN {
      xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
      if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
      pNewNodeType = dynamic_cast<const LibXmlBaseNode*>(pNewNode);
      xmlMergeHierarchy(m_oNode, pNewNodeType->libXmlNodePtr(), xtrigger, xfilter, 1);
    } UNWIND_EXCEPTION_END;

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
    
    UNWIND_EXCEPTION_THROW;

    return this;
  }
  
  IXmlBaseNode *LibXmlBaseNode::replaceNodeCopy(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToCopy, const iXMLIDPolicy iCopyXMLIDPolicy) {
    //returns this!
    //replaceNodeCopy() means a copy of the source: pNodeToMove will remain after the action
    //  in all cases, same doc and between docs
    IXmlBaseNode *pParentNode       = 0,
                 *pBeforeNode       = 0,
                 *pNewNode          = 0;
    LibXmlBaseNode *pNewNodeType    = 0;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;
    xmlNodePtr hardlink_first       = 0;
    bool bWasRegistered;

    //or (do a removeNode and then a moveNode)
    UNWIND_EXCEPTION_BEGIN {
      //... we MUST destroy the properties and children of this node before adding the new one
      //BECAUSE we need to remove the xml:id s from the table with an xmlFreeNode() -> free properties -> free xml:id
      //we cant move it instead. for the same xml:id reason. so no trash.
      //but: by specifying it in the XSL, we are creating a pointer to it which will be accessed afterwards:
      //  not freed, but read. valgrind will complain. and it is dangerous
      xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
      if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);

      //a pBeforeNode of zero will put it at the end of the children list
      pParentNode = parentNode(pQE);
      pBeforeNode = followingSibling(pQE);
      //TODO: hardlink replacement
      if (xmlIsHardLinked(m_oNode)) hardlink_first = xmlFirstHardLink(m_oNode)->hardlink_info->next;
      
      //original @destination removal
      //@xml:ids deregister all
      //to the temp area so it get's destroyed at the end of the request, or ~doc
      //because we are not destroying this IXmlBaseNode and creating a new
      //we need to stop watching our m_oNode
      //if it is a hardlink then it will always be the first in the chain
      bWasRegistered = deregisterNode();
      //removeAndDestroyNode(pQE); would only register the node for destruction in the ~destructor()
      xmlUnlinkNodeSecure(m_oNode, NULL, xtrigger, xfilter);
      xmlUnLockNode(m_oNode, this);
      xmlFreeNode(m_oNode);
      m_oNode = 0;

      //using the sameDoc logic of moveChild() which uses copyChild()
      //this never removes the source because we don't want to take it out of the TXml
      //most times the source is transient anyway
      pNewNode     = pParentNode->copyChild(pQE, pNodeToCopy, DEEP_CLONE, pBeforeNode, iCopyXMLIDPolicy);
      pNewNodeType = dynamic_cast<LibXmlBaseNode*>(pNewNode);

      //swap in and attach
      m_oNode = pNewNodeType->libXmlNodePtr();
      xmlLockNode(m_oNode, this, (const xmlChar*) "swapping in for node replace"); //aquire
      if (bWasRegistered) registerNode();
      if (hardlink_first) {
        //properties etc. are shared with other nodes in the hardlinked chain
        //xmlUnlinkNodeSecure() has removed the node from the chain though
        NOT_COMPLETE("can't replace hardlinked nodes at the moment. need to repoint all the hardlinks to the new properties and change the deviations. Needs doing in LibXML2, not here");
        hardlink_first->hardlink_info->prev = m_oNode;
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pParentNode) delete pParentNode;
    if (pBeforeNode) delete pBeforeNode;
    if (pNewNode)    delete pNewNode;
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
    
    UNWIND_EXCEPTION_THROW;

    return this;
  }
  
  IXmlBaseNode *LibXmlBaseNode::softlinkChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCommonChild, const IXmlBaseNode *pBeforeNode) {
    //caller frees result
    //this = the pAdditionalParent
    IXmlBaseNode *pNewNode = NULL;
    xmlNodePtr    oSoftlink;
    const LibXmlBaseNode *pCommonChildType, 
                         *pBeforeNodeType;
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    pCommonChildType = dynamic_cast<const LibXmlBaseNode*>(pCommonChild);
    pBeforeNodeType = dynamic_cast<const LibXmlBaseNode*>(pBeforeNode);
    if (!pCommonChildType) throw NativeLibraryClassRequired(this, MM_STRDUP("LibXmlBaseNode"));
    
    //softlinking only allows element to element and will throw errors otherwise
    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
    oSoftlink = xmlSoftlinkChild(m_oNode, //additional_parent
      m_oParentRoute,                     //only for security, does not affect linkage
      pCommonChildType->m_oNode,          //target (original)
      (pBeforeNodeType ? pBeforeNodeType->m_oNode : NULL), //optional before
      xtrigger,
      xfilter
    );
    if (oSoftlink) pNewNode = LibXmlLibrary::factory_node(mmParent(), oSoftlink, m_pDoc);

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);

    return pNewNode ? pNewNode->queryInterface((IXmlBaseNode*) 0) : 0;
  }

  void LibXmlBaseNode::touch(const IXmlQueryEnvironment *pQE) {
    xmlNodeFilterCallbackContextPtr  xfilter  = 0;
    xmlNodeTriggerCallbackContextPtr xtrigger = 0;

    xfilter                             = xmlCreateNodeFilterCallbackContext( xmlElementFilterCallback,  (void*) pQE);
    if (pQE->triggerContext()) xtrigger = xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallback, (void*) pQE, NOT_TRANSFORM_CONTEXT);
    xmlTouchNodeSecure(m_oNode, m_oParentRoute, xtrigger, xfilter);

    //free up
    if (xfilter)  xmlFreeNodeFilterCallbackContext(xfilter);
    if (xtrigger) xmlFreeNodeTriggerCallbackContext(xtrigger);
  }

  //--------------------------------------------------------------------------------- static LibXml2 callbacks
  int LibXmlBaseNode::xmlElementTriggerCallback(xmlNodePtr cur, xmlListPtr oParentRoute, xmlNodeTriggerCallbackContextPtr xtrigger, int iTriggerOperation, int iTriggerStage, xmlNodeFilterCallbackContextPtr xfilter) {
    //STATIC
    //Triggers CAN be used to implement complex application level SECURITY
    //so asserts are used for input sanitation
    //we require all calls to have a valid xtrigger object, although our LibXml2 doesnt
    assert(xtrigger && xtrigger->param);
    assert(cur && cur->doc);

    //VOLATILE variables because triggers CAN change this doc and nodes
    IXmlQueryEnvironment::accessResult iContinue = IXmlQueryEnvironment::RESULT_INCLUDE; // = 1, by default include the elements
    const IXmlQueryEnvironment *pQE = 0; //required: non-const will cause volatile methods to be accessed
    IXmlBaseDoc *pDoc               = 0; //required: pointer in to QE: do not free
    IXmlTriggerContext *pTrigger    = 0; //optional

    UNWIND_EXCEPTION_BEGIN_STATIC(pQE) { //pQE as MM
      if (cur && cur->type <= XML_ATTRIBUTE_NODE) { //XML_ELEMENT_NODE, XML_ATTRIBUTE_NODE
        pQE = (const IXmlQueryEnvironment*) xtrigger->param;

        if (pTrigger = pQE->triggerContext()) { //optional
          //in write [copy, etc.] operations:
          //  read triggers run on the source tree and document
          //  write triggers run on the destination node(s) (if the document is the same)
          //IGNORE: trigger request if it is for the wrong doucment
          //pTrigger->singleWriteableDocumentContext() includes xpath namespace cache
          //pTrigger->singleWriteableDocumentContext() IS NOT necessarily the correct document for the node being checked
          //triggers MAY update the xpath namespace cache so we MUST reference the correct one and feedback in to it
          //cur->doc IS the correct document for cur
          pDoc = pTrigger->singleWriteableDocumentContext();
          assert(pDoc);
          
          if (pQE->isMultiDocumentEnvironment()) {
            //the trigger acts upon ALL documents
            //probably a compilationContext that acts on ALL included stylesheets also
            //its primary singleWriteableDocumentContext will be the source stylesheet doc
            //thus all xpath contexts will point to that doc
            LibXmlDoc  oDoc(pQE, pQE->xmlLibrary(), "trigger document", cur->doc, NOT_OURS); //pQE as MM
            LibXmlNode oCur(pQE, cur, oParentRoute, &oDoc, NO_REGISTRATION, "trigger check"); //pQE as MM
            oDoc.copyNamespaceCacheFrom(pDoc);

            if (!oCur.isTransient()) {
              //run appropriate trigger
              switch ((IXmlQueryEnvironment::accessOperation) iTriggerOperation) {
                case IXmlQueryEnvironment::OP_READ:  {
                  switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                    case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeReadTriggerCallback( pQE, &oCur); break;
                    case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterReadTriggerCallback(  pQE, &oCur); break;
                  }
                  break;
                }
                case IXmlQueryEnvironment::OP_WRITE: {
                  switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                    case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeWriteTriggerCallback(pQE, &oCur); break;
                    case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterWriteTriggerCallback( pQE, &oCur); break;
                  }
                  break;
                }
                case IXmlQueryEnvironment::OP_ADD: {
                  switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                    case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeAddTriggerCallback(pQE, &oCur); break;
                    case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterAddTriggerCallback( pQE, &oCur); break;
                  }
                  break;
                }
                case IXmlQueryEnvironment::OP_EXECUTE: NOT_COMPLETE("OP_EXECUTE"); break;
              }
            }
          } else {
            //trigger is limited to its one stated document
            if (dynamic_cast<LibXmlBaseDoc*>(pDoc)->libXmlDocPtr() == cur->doc) {
              //PERFORMANCE: trigger area needs to be HIGH performance so local stack!
              LibXmlNode oCur(pQE, cur, oParentRoute, pDoc, NO_REGISTRATION, "trigger check"); //pQE as MM

              if (!oCur.isTransient()) {
                //run appropriate trigger
                switch ((IXmlQueryEnvironment::accessOperation) iTriggerOperation) {
                  case IXmlQueryEnvironment::OP_READ:  {
                    switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                      case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeReadTriggerCallback( pQE, &oCur); break;
                      case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterReadTriggerCallback(  pQE, &oCur); break;
                    }
                    break;
                  }
                  case IXmlQueryEnvironment::OP_WRITE: {
                    switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                      case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeWriteTriggerCallback(pQE, &oCur); break;
                      case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterWriteTriggerCallback( pQE, &oCur); break;
                    }
                    break;
                  }
                  case IXmlQueryEnvironment::OP_ADD: {
                    switch ((IXmlQueryEnvironment::accessStage) iTriggerStage) {
                      case IXmlQueryEnvironment::STAGE_BEFORE: iContinue = pTrigger->beforeAddTriggerCallback(pQE, &oCur); break;
                      case IXmlQueryEnvironment::STAGE_AFTER:  iContinue = pTrigger->afterAddTriggerCallback( pQE, &oCur); break;
                    }
                    break;
                  }
                  case IXmlQueryEnvironment::OP_EXECUTE: NOT_COMPLETE("OP_EXECUTE"); break;
                }
              }
            }
          }
        } //OK: no triggers
      }
    } UNWIND_EXCEPTION_END_STATIC(pQE); //pQE as MM

    //free up
    //node and document are local stack

    UNWIND_EXCEPTION_THROW;

    return iContinue;
  }

  IXmlBaseNode *LibXmlBaseNode::copyNode(const IXmlQueryEnvironment *pQE, const bool bDeepClone, const bool bEvaluateValues, iXMLIDPolicy iXMLIDMovePolicy) const {
    xmlNodePtr oNewNode, oChildren;
    xmlChar *sValue = 0, *sNewValue = 0;
    IXslTransformContext *pTC;
    LibXslTransformContext *pCTxtType;

    oNewNode = xmlDocCopyNode(m_oNode, m_oNode->doc, (bDeepClone ? 1 : 3));

    if (bEvaluateValues) {
      if (oChildren = oNewNode->children) {
        switch (oNewNode->type) {
          case XML_ELEMENT_NODE:
            //TODO: calculate entire text value
            NOT_COMPLETE("XML_ELEMENT_NODE");
            if (oChildren && oChildren->type == XML_TEXT_NODE) sValue = oChildren->content;
            break;
          case XML_ATTRIBUTE_NODE:
            if (oChildren && oChildren->type == XML_TEXT_NODE) sValue = oChildren->content;
            break;
          default: NOT_COMPLETE("copyNode element type not finished");
        }

        if (sValue && *sValue && strchr((const char*) sValue, '{')) {
          if (pTC = pQE->transformContext()) {
            //lets assume a LibXslTransformContext rather than visiting it
            pCTxtType = static_cast<LibXslTransformContext*>(pTC);
            //http://xmlsoft.org/XSLT/html/libxslt-templates.html#xsltAttrTemplateValueProcessNode
            //const char *sNewValue = (const char*) xsltAttrTemplateValueProcess(pCTxtType->libXmlContext(), (const xmlChar*) sValue);
            sNewValue = xsltAttrTemplateValueProcessNode(pCTxtType->libXmlContext(), (const xmlChar*) sValue, oNewNode, m_oParentRoute);
            //const char *sNewValue = (const char*) xsltEvalAttrValueTemplate(pCTxtType->libXmlContext(), oNewNode, (const xmlChar*) sName, (const xmlChar*) "");
            xmlFree(sValue);
            oChildren->content = sNewValue;
          }
        }
      }
    }

    return LibXmlLibrary::factory_node(mmParent(), oNewNode, m_pDoc);
  }

  int LibXmlBaseNode::xmlElementFilterCallback(xmlNodePtr cur, xmlListPtr oParentRoute, xmlNodeFilterCallbackContextPtr xfilter, int iFilterOperation) {
    //STATIC
    //the caller is maintaining lNamespaceAttributeValues and destroys it after this call
    //lNamespaceAttributeValues will be zero for efficiency if there are no attributes to report
    assert(xfilter && xfilter->param); //we require all calls to have a valid context object, although our LibXml2 doesnt
    assert(cur);

    //CONST system as changes are NOT made by the security system. only reporting
    IXmlQueryEnvironment::accessResult iIncludeNode = IXmlQueryEnvironment::RESULT_INCLUDE; //by default include the elements
    const IXmlQueryEnvironment *pQE    = 0; //required. const will cause a const doc, and const node also

    //per thread based infinite recursion checking and reporting
    //TODO: assign to the MemoryLifetimeOwner passed in optionally on the QE? (fix the MemoryLifetimeOwner first!!!)
    IFDEBUG(
      /*
      static unsigned int iMax = 1000;
      if (Thread::incrementUntil(iMax)) {
        Debug::report("Thread count at [%s]", iMax);
        exit(1);
      }
      */
    );

    pQE = (const IXmlQueryEnvironment*) xfilter->param; //asserted above
    
    UNWIND_EXCEPTION_BEGIN_STATIC(pQE) { //pQE as MM
      //pQE->document() would not necessarily be the correct doc. e.g. inter-document copy operations
      //  the security check on the destination node is sometimes a check in another document
      //  whilst the security checks on the source tree are the correct document
      //pTargetDocument = pQE->document();
      //pTargetDocument = pNewDoc = LibXmlLibrary::factory_document(pQE->xmlLibrary(), "security document", cur->doc);
      //pTargetDocument->copyNamespaceCacheFrom(pQE->document());
      //PERFORMANCE: security area needs to be HIGH performance so local stack operations!
      //TODO: PERFORMANCE: we could check to see if pTrigger->document()->is(cur->doc)...
      //LibXmlDoc oDoc(pQE->xmlLibrary(), "security document wrapper", cur->doc);
      //NOTE: we do not copy the namespace cache
      //PERFORMANCE: security area needs to be HIGH performance so no xpath
      //use manual navigation like children(...)
      const LibXmlDoc  oDoc(pQE, pQE->xmlLibrary(), "security check", cur->doc, NOT_OURS);     //pQE as MM
      const LibXmlNode oCur(pQE, cur, oParentRoute, &oDoc, NO_REGISTRATION, "security check"); //pQE as MM

      //required for node creation: take server down if this security structure fails
      //asserts used more in security because unexpected failure is always critical / fatal
      assert(pQE->securityContext());

      //optional: node-mask (on any element, TEXT or @attribute)
      if (iIncludeNode == IXmlQueryEnvironment::RESULT_INCLUDE && pQE->maskContext()) {
        iIncludeNode = pQE->maskContext()->applyCurrentAreaMask(&oCur);
      }

      //required: security (no xpath allowed)
      //only works at XML_ELEMENT_NODE level with @xmlsecurity:*
      if (iIncludeNode == IXmlQueryEnvironment::RESULT_INCLUDE) {
        try {
          iIncludeNode = pQE->securityContext()->checkSecurity(&oCur, (const IXmlQueryEnvironment::accessOperation) iFilterOperation); //virtual
        } catch (CannotRunXPathWithoutNamespaceCache& eb) {
          Debug::report("security is a performance area and should not include xpath processing. use manual navigation functions instead");
          assert(false);
        }
      }

      //optional: User XSL stepping debug with breakpoints n stuff
      //breakpoints only at element level
      if (iIncludeNode == IXmlQueryEnvironment::RESULT_INCLUDE && cur->type == XML_ELEMENT_NODE) {
        if (pQE->debugContext()) {
          //note that the debug system has the right to manually ignore this node
          //but not the right to read it if security has rejected it
          iIncludeNode = pQE->debugContext()->informAndWait(pQE, &oCur, (const IXmlQueryEnvironment::accessOperation) iFilterOperation, iIncludeNode); //virtual
        }
      }
      //local stack: node and document deletion
    } UNWIND_EXCEPTION_END_STATIC(pQE); //pQE as MM

    //free up
    //node and document are local stack

    UNWIND_EXCEPTION_THROW;

    return iIncludeNode;
  }

  const xmlChar *LibXmlBaseNode::xmlParentRouteReporter(const xmlNodePtr oParentRouteNode, const void *user, unsigned int iReportLen) {
    return xmlStrdup(oParentRouteNode->name); //will be freed
  }
  
  const char *LibXmlBaseNode::currentParentRoute() const {
    return (const char*) xmlListReport(m_oParentRoute, (xmlListReporter) LibXmlBaseNode::xmlParentRouteReporter, BAD_CAST " => ", NULL);
  }
}
