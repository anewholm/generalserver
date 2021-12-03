//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXMLBASENODE_H
#define _IXMLBASENODE_H

#include "define.h" //includes platform define also
#include "types.h"  //percentage

//cloning when copying and appending nodes
#define DEEP_CLONE true
#define SHALLOW_CLONE false
#define COPY_ONLY false
#define EVALUATE_VALUES true
#define HARDLINK_AWARE true
#define EXACT_MATCH false
#define MORPH_SOFTLINKS true

#define WARN_IF_TRANSIENT true
#define INCLUDE_TRANSIENT false
#define FORCE_BASE_NODE true
#define NO_FORCE_BASE_NODE false
#define CREATE_TRANSIENT true
#define NO_BASE_NODE NULL
#define ENABLE_IDS true
#define DISABLE_IDS false
#define NO_CONTENT 0
#define REJECT_ABSOLUTE true
#define ALLOW_ABSOLUTE false
#define NO_PARENTROUTE NULL

#define NO_EMO 0
#define NO_PARAMS_INT 0
#define NO_PARAMS_CHAR 0
#define NO_PARAMS_NODE 0
#define NO_MODE 0
#define NOT_TRANSFORM_CONTEXT 0

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "MemoryLifetimeOwner.h"       //direct inheritance
#include "Utilities/StringMap.h"

#define IGNORE_IDS false
#define NO_REGISTRATION false
#define REGISTER true

namespace general_server {
  //http://xmlsoft.org/html/libxml-xpath.html#xmlXPathObject
  //same as the LibXML ones :)
  enum iXPathType {
    TYPE_UNDEFINED = 0,
    TYPE_NODESET = 1,
    TYPE_BOOLEAN = 2,
    TYPE_NUMBER = 3,
    TYPE_STRING = 4,
    TYPE_POINT = 5,
    TYPE_RANGE = 6,
    TYPE_LOCATIONSET = 7,
    TYPE_USERS = 8,
    TYPE_XSLT_TREE = 9
  };

  //Specification: http://www.w3.org/TR/DOM-Level-2-Core/core.html#ID-1950641247
  enum iDOMNodeType {
    NODE_INVALID = 0,
    NODE_ELEMENT = 1,
    NODE_ATTRIBUTE = 2,
    NODE_TEXT = 3,
    NODE_CDATA_SECTION = 4,
    NODE_ENTITY_REFERENCE = 5,
    NODE_ENTITY = 6,
    NODE_PROCESSING_INSTRUCTION = 7,
    NODE_COMMENT = 8,
    NODE_DOCUMENT = 9,
    NODE_DOCUMENT_TYPE = 10,
    NODE_DOCUMENT_FRAGMENT = 11,
    NODE_NOTATION = 12,
    //LibXML extended types
    NODE_HTML_DOCUMENT_NODE = 13,
    NODE_DTD_NODE = 14,
    NODE_ELEMENT_DECL = 15,
    NODE_ATTRIBUTE_DECL = 16,
    NODE_ENTITY_DECL = 17,
    NODE_NAMESPACE_DECL = 18,
    NODE_XINCLUDE_START = 19,
    NODE_XINCLUDE_END = 20,
    NODE_DOCB_DOCUMENT_NODE = 21
  };

  enum iDOMNodeTypeRequest {
    node_type_element_or_text = 0, //default
    node_type_element_only,        //common
    node_type_processing_instruction, //strip PIs (can be multiple, maybe we introduce throughout)
    //node_type_dtd,               //NO: because there can be only one under xmlgetIntSubset()
    node_type_any                  //TODO: this includes processing instructions and DTD etc. remove?
  };

  //copy, move, replace operations between and within documents
  enum iAreaXMLIDPolicy {
    xmlid_area_normal = 0,
    xmlid_area_ignore
  };
  enum iXMLIDPolicy {
    xmlid_default = 0,    //depends on operation and same-document
    copy_xmlid,           //one doc to another, e.g. @xml -> main tree
    ignore_xmlid,         //main tree RegularX commands -> <request>
    morph_to_xmlidcopy,   //saving   transactions
    morph_from_xmlidcopy  //applying transactions
  };

  enum iDeviationType {
    deviation_replacement = 0,
    deviation_extra_child,
    deviation_removal
  };

  enum iErrorPolicy {
    throw_error,
    continue_onerror
  };

  //normal ahead declerations
  interface_class IXmlLibrary;
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlNodeMask;
  interface_class IXmlDoc;
  interface_class IXslDoc;
  interface_class IXslCommandNode;
  interface_class IXslNode;
  interface_class IXslStylesheetNode;
  interface_class IXslTemplateNode;
  interface_class IXslTransformContext;
  interface_class IXslModuleManager;
  interface_class IXmlQueryEnvironment;

  interface_class IXmlNamespaced;
  interface_class IXmlHasNamespaceDefinitions;

  template<class IXmlNodeType> class XmlNodeList;
  
  class XmlAdminQueryEnvironment;
  class TXml;

  IFDEBUG(class LibXmlBaseNode;)

  interface_class IXmlBaseNode: implements_interface IMemoryLifetimeOwner, implements_interface IReportable {
    //only classes in this file should have access to the platform specific members
    //no concept of child XmlNodes exists here (yet)
    //transforms and XPath should be used to query

  public:
    //private debug xml()
    IFDEBUG(virtual const char           *x() const = 0;)
    IFDEBUG(virtual const LibXmlBaseNode *d() const = 0;)
    //virtual const xmlNodePtr      o() const = 0; //can't get the circular references to behave with this

  public:
    virtual iDOMNodeType nodeType() const = 0; //use is...() instead
    virtual IXmlBaseNode *clone_wrapper_only()   const = 0;
    virtual IXmlBaseNode *clone_with_resources() const = 0;
    //resources registration and notifications
    virtual bool registerNode() = 0;
    virtual bool deregisterNode() = 0;
    virtual bool isRegistered() const = 0;
    virtual bool deregistrationNotify() = 0;
    virtual void stopListeningForDeregistionNotify() = 0;

    //accessors
    virtual IXmlBaseDoc       *document() = 0;
    virtual const IXmlBaseDoc *document() const = 0;
    virtual const IXmlLibrary *xmlLibrary() const = 0;

    //----------------------------------------------------- read-only section
  public:
    virtual ~IXmlBaseNode() {} //virtual public destructors allow proper destruction of derived object

    //node info: attributes, types, values, names (read-only)
    //uniqueXPathToNode() NOT_CACHED intelligence because it will only be used once a time and then the XmlNode destroyed
    virtual const char *currentParentRoute() const = 0;
    virtual XmlNodeList<const IXmlBaseNode> *fileSystemPathToNodeList(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const bool bRejectAbsolute = ALLOW_ABSOLUTE, const char *sTargetType = "*", const char *sDirectoryNamespacePrefix = NULL, const bool bUseIndexes = true) const = 0;
    virtual const char *uniqueXPathToNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = WARN_IF_TRANSIENT, const bool bForceBaseNode = NO_FORCE_BASE_NODE, const bool bEnableIDs = false, const bool bEnableNameAxis = true, const bool bThrowIfNotBasePath = true) const = 0;
    virtual void *persistentUniqueMemoryLocationID() const = 0; //unique
    virtual const IXslStylesheetNode *relativeXSLIncludeFileSystemPathToXSLStylesheetNode(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath) const = 0;
    virtual const char *fullyQualifiedName() const   = 0;
    virtual const char *standardPrefix() const = 0;
    virtual const char *localName(const bool bDuplicate = true) const = 0; //local-name()
    virtual const char *group(const IXmlQueryEnvironment *pQE, const bool bDuplicate = true) const = 0; //name(..)
    virtual unsigned long documentOrder() const = 0;
    virtual const char *nameAxisName(const IXmlQueryEnvironment *pQE) const = 0; //name::?
    virtual size_t position() const            = 0; //TODO: not implemented yet
    virtual const char *xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth = 0, const bool bNoEscape = false) const = 0;     //is
    virtual const char *toString() const = 0;

    virtual bool in(const XmlNodeList<const IXmlBaseNode> *pVec, const bool bHardlinkAware) const = 0;
    virtual bool in(const XmlNodeList<IXmlBaseNode> *pVec, const bool bHardlinkAware) const = 0;
    virtual bool is(const IXmlBaseNode *p2, const bool bHardlinkAware, const bool bParentRouteAware = true) const = 0; //hardlink aware: returns true if nodes are hardlinks of each other
    virtual bool canHaveNamespace() const = 0; //isNodeElement() || isNodeAttribute()
    virtual bool hasNamespace() const = 0;
    virtual bool isNamespace(const char *sHREF) const = 0;
    virtual bool isName(const char *sHREF, const char *sLocalName) const = 0;
    virtual bool isRootNode() const = 0;
    virtual bool isAncestorOf(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pDesendentNode) const = 0;
    //xml NodeType
    virtual const char *typeName()        const = 0;
    virtual bool isNodeElement()    const = 0;
    virtual bool isNodeDocument()   const = 0;
    virtual bool isNodeAttribute()  const = 0;
    virtual bool isNodeText()       const = 0;
    virtual bool isNodeNamespaceDecleration() const = 0;
    virtual bool isProcessingInstruction() const = 0;
    virtual bool isCDATASection()   const = 0;
    //transiency
    virtual bool isTransient()      const = 0;
    virtual bool isTransientAreaParent() const = 0;
    virtual iAreaXMLIDPolicy xmlIDPolicy() const = 0;
    virtual bool isHardLinked()        const = 0; //part of a hardlink chain, including the original
    virtual bool isHardLink()          const = 0; //is hardlinked but NOT the original
    virtual bool isSoftLink()          const = 0; //is softlink
    virtual bool isOriginalHardLink()  const = 0; //first entry in the list is the original
    virtual bool isDeviant() const = 0;
    virtual bool hasDescendantDeviants(const IXmlBaseNode *pOriginal = 0) const = 0; //a hardlink that has sub-tree deviants, optionally checking for a specific original
    virtual bool hasDeviants(const IXmlBaseNode *pHardlink = 0)   const = 0; //a node that has deviants, optionally in relation to a specific hardlink
    virtual bool hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const = 0; //used to determine document validity
    virtual size_t childElementCount(const IXmlQueryEnvironment *pQE) const = 0;
    virtual bool isEmpty(const IXmlQueryEnvironment *pQE) const = 0;
    virtual iErrorPolicy errorPolicy() const = 0;

    //process curly brackets with pTransformingStylesheet
    virtual const char   *value(             const IXmlQueryEnvironment *pQE) const = 0;
    virtual bool          isValue(const char *sValue) const = 0; //fast
    virtual const char   *attributeValue(    const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const = 0;
    virtual const char   *attributeValueDirect(const XmlAdminQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const = 0;
    virtual int     attributeValueInt( const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const int iDefault = 0) const = 0;
    virtual bool    attributeValueBoolDynamicString(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const bool bDefault = false) const = 0;
    virtual bool    attributeValueBoolInterpret(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const bool bDefault = false) const = 0;
    virtual IXmlBaseNode *attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pFromNode = 0) = 0;
    virtual const IXmlBaseNode *attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pFromNode = 0) const = 0;
    virtual XmlNodeList<const IXmlBaseNode> *attributeValueNodes(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pBaseNode = 0) const = 0;
    virtual IXmlNodeMask *attributeValueNodeMask(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const = 0;
    virtual const char   *attributeValueUniqueXPath(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pBaseNode = NULL, const bool bWarnIfTransient = INCLUDE_TRANSIENT, const bool bForceBaseNode = NO_FORCE_BASE_NODE) const = 0;
    virtual bool          attributeExists(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const = 0;
    virtual const char   *attributeValueDynamic(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL, const unsigned int iRecursions = 1) const = 0;
    virtual const char   *valueDynamic(  const IXmlQueryEnvironment *pQE, const char *sValue, const unsigned int iRecursions = 1) const = 0;
    virtual const char   *xmlID(         const IXmlQueryEnvironment *pQE) const = 0;

    //merging
    virtual percentage     similarity( const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const = 0;
    virtual vector<TXml*> *differences(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const = 0;
    
    //debug
    virtual int  validityCheck(const char *sReason = 0) const = 0; //examine LibXML structures looking for consistency errors

    //direct (non-xpath) node traversal (read-only)
    //this only happens outside an xpath environment and thus refuses an XSLT environment
    virtual const IXmlBaseNode *firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0) const = 0;
    virtual IXmlBaseNode       *firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode *lastChild( const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sLockingReason = 0) const = 0;
    virtual IXmlBaseNode       *lastChild( const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode *child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sNamespace = 0, const char *sLockingReason = 0) const = 0;
    virtual IXmlBaseNode       *child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sNamespace = 0, const char *sLockingReason = 0) = 0;
    virtual IXmlBaseNode       *parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode *parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const = 0;
    virtual IXmlBaseNode       *followingSibling( const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode *followingSibling( const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const = 0;
    virtual IXmlBaseNode *hardlinkRouteToAncestor(const IXmlBaseNode *pAncestor, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode>       *children(  const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) = 0;
    virtual XmlNodeList<const IXmlBaseNode> *children(  const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode>       *ancestors( const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) = 0;
    virtual XmlNodeList<const IXmlBaseNode> *ancestors( const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode>       *attributes(const IXmlQueryEnvironment *pQE) = 0;
    virtual XmlNodeList<const IXmlBaseNode> *attributes(const IXmlQueryEnvironment *pQE) const = 0;

    //in-direct (interpreted xpath) traversal (read-only)
    //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
    virtual IXmlBaseNode       *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode>       *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) = 0;
    virtual XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const bool bRegister, const char *sLockingReason = 0) const = 0;
    virtual void getObject(const IXmlQueryEnvironment *pQE, const char *sXPath, XmlNodeList<IXmlBaseNode> **ppNodeList, char **psResult, int *piResult, bool *pbResult, iXPathType *piType) const = 0;

    //for incoming XML parsing before use in a transaction or by a trigger
    //does not add child, but instead leaves the new node un-attached
    virtual IXmlBaseNode *parseXMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sXML,  const bool bRemoveRedundantNamespaceDeclarations = false, const bool bTranslateEntities = false) const = 0;
    virtual IXmlBaseNode *parseHTMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sHTML, const bool bRemoveRedundantNamespaceDeclarations = false, const bool bTranslateEntities = false) const = 0;

    //all nodes can be transformed (read-only)
    virtual IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0, const IXmlBaseNode *pIncludeDirectory = 0
    ) const = 0;
    virtual void transform(
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutputNode,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0, const IXmlBaseNode *pIncludeDirectory = 0
    ) const = 0;

    virtual bool clearMyCachedStylesheets(const IXmlQueryEnvironment *pQE) = 0;

    //----------------------------------------------------- write section
    //NOTE: changes made to IXmlNodes without TXml will not be permanently stored in associated Repositories...
    //attributes, values, names (write)
  public:
    virtual const char *value(const IXmlQueryEnvironment *pQE, const char *sValue) = 0;
    virtual void localName(const IXmlQueryEnvironment *pQE, const char *sName) = 0;
    virtual void removeAttribute(const IXmlQueryEnvironment *pQE, const char *sName) const = 0;
    virtual void appendAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sSeperator = ",")  const = 0;
    virtual void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue,  const char *sNamespace = NULL)  const = 0;
    virtual void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue,    const char *sNamespace = NULL)    const = 0;
    virtual void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace = NULL) const = 0;
    virtual void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue,   const char *sNamespace = NULL)   const = 0;
    virtual void setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const unsigned int uValue, const char *sSuffix = 0)    const = 0;
    virtual void setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const char *sName = 0) const = 0;
    virtual IXmlBaseNode *setTransient(const IXmlQueryEnvironment *pQE) const = 0;
    virtual IXmlBaseNode *setTransientArea(const IXmlQueryEnvironment *pQE) = 0;
    virtual void createTextNode(const IXmlQueryEnvironment *pQE, const char *sContent = 0) = 0;

    //tree changes (write)
    //ALWAYS changes the tree by directly creating a new node in it somewhere
    //[this] parent with new take child [pNodeToMove], optionally placing it before [pBeforeNode]
    //after the last node is the default, so send zero for that case
    virtual IXmlBaseNode *createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const bool bTransient = false, const char *sReason = 0) = 0;
    virtual IXmlBaseNode *copyChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, const bool bDeepClone = DEEP_CLONE, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *copyChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const bool bDeepClone = DEEP_CLONE, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *moveChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNodeToMove, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *moveChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *deviateNode(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pHardlink, const IXmlBaseNode *pNewNode, const char *sAttributeName = 0, const iDeviationType iType = deviation_replacement) = 0;
    virtual IXmlBaseNode *hardlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCommonChild, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0) = 0;
    virtual IXmlBaseNode *hardlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0) = 0;
    virtual IXmlBaseNode *softlinkChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, const IXmlBaseNode *pBeforeNode = NULL) = 0;
    virtual IXmlBaseNode *softlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc) = 0;
    virtual IXmlBaseNode *mergeNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToCopy, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *mergeNode(const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *replaceNodeCopy(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToCopy, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual IXmlBaseNode *replaceNodeCopy(const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;
    virtual bool removeAndDestroyNode(const IXmlQueryEnvironment *pQE)     = 0;
    virtual bool removeAndDestroyChildren(const IXmlQueryEnvironment *pQE) = 0;
    virtual IXmlBaseNode *getOrCreateChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0) = 0;
    virtual void touch(const IXmlQueryEnvironment *pQE) = 0;

    virtual void createHardlinkPlaceholderIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const = 0;
    virtual void createDeviationPlaceholdersIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const = 0;

    //used to remove gs:server-nodes primarily
    //moves ALL node types to a new parent
    //no TXml for this baby
    //TODO: implement this moveChildren(...) at LibXml2 level
    virtual void moveChildren(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNewParent) = 0;
    //copies THIS node and does not attach it anywhere
    virtual IXmlBaseNode *copyNode(const IXmlQueryEnvironment *pQE, const bool bDeepClone = DEEP_CLONE, const bool bEvaluateValues = COPY_ONLY, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) const = 0;
    virtual const IXmlBaseNode *softlinkTarget(const IXmlQueryEnvironment *pQE) const = 0;

    //----------------------------------------------------- interface navigation
    //concrete classes MUST implement relevant queries to return the correct vtable pointer
    //leaf Interfaces may REQUIRE this with pure virtual, e.g. IXslStylesheetNode
    //this base one will return a fail on any Derived class that hasnt provided a local context implicit cast
  public:
    virtual IXslCommandNode          *queryInterface(IXslCommandNode *p)                = 0;
    virtual const IXslCommandNode    *queryInterface(const IXslCommandNode *p) const    = 0;
    virtual const IXmlBaseNode       *queryInterface(const IXmlBaseNode *p) const       = 0;
    virtual IXmlBaseNode             *queryInterface(IXmlBaseNode *p)                   = 0;
    virtual const IXslStylesheetNode *queryInterface(const IXslStylesheetNode *p) const = 0;
    virtual const IXslTemplateNode   *queryInterface(const IXslTemplateNode *p) const   = 0;
    virtual const IXslNode           *queryInterface(const IXslNode *p) const           = 0;
    virtual IXmlNamespaced           *queryInterface(IXmlNamespaced *p)                 = 0;
    virtual const IXmlNamespaced     *queryInterface(const IXmlNamespaced *p) const     = 0;
    virtual IXmlHasNamespaceDefinitions *queryInterface(IXmlHasNamespaceDefinitions*) = 0;
    virtual const IXmlHasNamespaceDefinitions *queryInterface(const IXmlHasNamespaceDefinitions*) const = 0;
    //useful also for GDB:
    virtual const IXmlBaseNode       *queryInterfaceIXmlBaseNode() const                = 0;
  };
}

#endif
