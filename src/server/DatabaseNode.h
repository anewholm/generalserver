//platform agnostic file
//XML DatabaseNode class wraps the IXmlBaseNode
#ifndef _DATABASENODE_H
#define _DATABASENODE_H

#include "Database.h"
#include "types.h"
using namespace std;

namespace general_server {
  class DatabaseNode: implements_interface IReportable, virtual public MemoryLifetimeOwner {
    //wrapper class to ensure control of write access to IXmlBaseDoc
    //used also by DatabaseNodeServerObjects as they save their changes and control their XML config
    //compile time controlled processed access to the Database write functions, direct access to read functions
    //  access to base IXmlBaseNode only through the new "transient" function!
    //has-a, rather than is-a, because we do not know the final polymorphic type of the XML_LIBRARY
    IXmlBaseNode *m_pNode; //required non-const: no point in DatabaseNode if it cannot make changes!
    const bool m_bOurs;
    Database *m_pDatabase;

  protected:
    friend class Database; //DatabaseNodes are created only with factory_node(...)
    DatabaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const Database *pDatabase, const IXmlBaseNode *pNode, const bool bOurs = OURS);
    DatabaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, Database *pDatabase, IXmlBaseNode *pNode, const bool bOurs = OURS);

  public:
    ~DatabaseNode();
    DatabaseNode *clone_with_resources() const;

    //const access to the underlying base node is allowed
    //  for read-only purposes
    template<class NodeType> const NodeType *node_const(NodeType *p) const {return m_pNode->queryInterface((const NodeType*) 0);}
    const IXmlBaseNode *node_const() const {return m_pNode;}
    const IXmlBaseDoc *document_const() const;
    //things like RegularX work on these directly
    //  nothing would be saved by the Database
    //need to make sure that all transient behaviour is not required to be stored:
    //  grep -rs node_transient\( for transient behaviour list
    IXmlBaseNode *node_transient() const;
    IXmlBaseNode *node_admin(XmlAdminQueryEnvironment *pQE);
    void *persistentUniqueMemoryLocationID() const; //unique
    Database *db();
    const Database *db() const;
    DatabaseNode       *factory_node(      IXmlBaseNode *pNode); //db()->factory_node(pNode);
    const DatabaseNode *factory_node(const IXmlBaseNode *pNode) const; //db()->factory_node(pNode);
    const IXmlLibrary *xmlLibrary() const;
    unsigned long documentOrder() const;
    const char *currentParentRoute() const;
    const char *uniqueXPathToNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = true, const bool bForceBaseNode = NO_FORCE_BASE_NODE, const bool bEnableIDs = false, const bool bEnableNameAxis = true, const bool bThrowIfNotBasePath = true) const;
    const char *fullyQualifiedName() const;
    bool isNamespace(const char *sHREF) const;
    bool registerNode();
    bool deregisterNode();

    //private debug xml()
    IFDEBUG(const char *x() const;)

    //----------------------------------------------------- read-only wrapper section
    //DatabaseNode manages read / write semantics. it always holds a non-const node
    //node traversal (read-only)
    DatabaseNode *firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0);
    DatabaseNode *parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    XmlNodeList<      DatabaseNode> *children(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    XmlNodeList<const DatabaseNode> *children(const  IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const;
    XmlNodeList<      DatabaseNode> *ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    XmlNodeList<const DatabaseNode> *ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const;
    DatabaseNode       *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0);
    const DatabaseNode *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0) const;
    XmlNodeList<DatabaseNode>       *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0);
    XmlNodeList<const DatabaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const;

    void transform(
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutputNode,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0, const IXmlBaseNode *pIncludeDirectory = 0
    ) const;
    IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0, const IXmlBaseNode *pIncludeDirectory = 0
    ) const;
    void inheritanceTransform(const char *sStylesheetName,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutputNode,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0, const IXmlBaseNode *pIncludeDirectory = 0
    ) const;
    const IXmlBaseNode *generateDynamicStylesheet(
      const IXmlQueryEnvironment *pQE
    ) const;

    //attributes, values, names (read-only)
    //  process curly brackets with pTransformingStylesheet
    const char *value(const IXmlQueryEnvironment *pQE) const;
    DatabaseNode *parseXMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sXML); //for incoming XML parsing before use in a transaction
    const char *attributeValue(           const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const;
    const char *attributeValueDirect(     const XmlAdminQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const;
    bool  attributeValueBoolInterpret(    const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const bool bDefault = false) const;
    int   attributeValueInt(              const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const int iDefault = 0) const;
    bool  attributeValueBoolDynamicString(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const bool bDefault = false) const;
    DatabaseNode     *attributeValueNode( const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const;
    const char *attributeValueUniqueXPath(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pBaseNode = 0, const bool bForceBaseNode = NO_FORCE_BASE_NODE) const;
    bool  attributeExists(          const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const;
    const char *xmlID(const IXmlQueryEnvironment *pQE) const;
    const char *group(const IXmlQueryEnvironment *pQE, const bool bDuplicate = true) const;
    bool  is(const DatabaseNode *p2, const bool bHardlinkAware) const;
    bool  is(const IXmlBaseNode *p2, const bool bHardlinkAware) const;
    bool  isRootNode()  const;
    
    //xml NodeType
    const char *typeName()  const;
    bool isNodeElement()    const;
    bool isNodeDocument()   const;
    bool isNodeAttribute()  const;
    bool isNodeText()       const;
    bool isNodeNamespaceDecleration() const;
    bool isProcessingInstruction() const;
    bool isCDATASection()   const;
    
    bool  isTransient() const;
    bool  isHardLink()  const;
    bool  isSoftLink()  const;
    size_t childElementCount(const IXmlQueryEnvironment *pQE) const;
    bool  in(const XmlNodeList<const IXmlBaseNode> *pVec, const bool bHardlinkAware) const;
    const char *localName(const bool bDuplicate = true) const;
    size_t position() const;
    percentage     similarity( const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const;
    vector<TXml*> *differences(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const;

    const char *xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth = 0) const;
    const char *toString() const; //IReportable

    //----------------------------------------------------- write section
    //TXml input writing is done through the inherited Database::TXmlProcessor system
    //OR:
    //direct functions like setAttribute(...) that use TXml internally
    //  by creating a TXml object and then sending it internally to parent Database
    //OR:
    //through direct access through node_transient() which should minimised
    void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue,  const char *sNamespace = NULL);
    void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue,    const char *sNamespace = NULL);
    void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace = NULL);
    void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue,   const char *sNamespace = NULL);
    IXmlBaseNode       *setTransient(    const IXmlQueryEnvironment *pQE);
    const IXmlBaseNode *setTransientArea(const IXmlQueryEnvironment *pQE);
    void removeAttribute(const IXmlQueryEnvironment *pQE, const char *sName);
    DatabaseNode *createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0);
    IXmlBaseNode *createTransientChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const char *sReason = 0);
    DatabaseNode *copyChild(const IXmlQueryEnvironment *pQE, const DatabaseNode *pNodeToMove, const bool bDeepClone = DEEP_CLONE, const DatabaseNode *pBeforeNode = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default);
    DatabaseNode *mergeNode(const IXmlQueryEnvironment *pQE, const DatabaseNode *pNodeToMove, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default); //must be implemented, should throw CapabilityNotSupported(this)
    DatabaseNode *deviateNode(const IXmlQueryEnvironment *pQE, DatabaseNode *pHardlink, const DatabaseNode *pNewNode, const iDeviationType iType = deviation_replacement);
    DatabaseNode *hardlinkChild(const IXmlQueryEnvironment *pQE, DatabaseNode *pCommonChild, const DatabaseNode *pBeforeNode = NULL);
    DatabaseNode *softlinkChild(const IXmlQueryEnvironment *pQE, DatabaseNode *pCommonChild, const DatabaseNode *pBeforeNode = NULL);
    DatabaseNode *getOrCreateChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0);
    IXmlBaseNode *getOrCreateTransientChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const char *sReason = 0);
    void touch(const IXmlQueryEnvironment *pQE, const char *sReason);
    bool removeAndDestroyNode(const IXmlQueryEnvironment *pQE, const char *sReason = 0);

    DatabaseNode *copyNode(const IXmlQueryEnvironment *pQE, const bool bDeepClone = DEEP_CLONE, const bool bEvaluateValues = COPY_ONLY, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) const;

    //----------------------------------------------------- transient write section
    //setTransientAttribute()
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue,  const char *sNamespace = NULL);
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue,    const char *sNamespace = NULL);
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace = NULL);
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue,   const char *sNamespace = NULL);

    //----------------------------------------------------- interface navigation
    //these are only here so that XmlNodeList<DatabaseNode> works
    //because it uses IXmlBaseNode for a lot of things
    //and then needs to navigate back to the original IXmlNodeType
    DatabaseNode       *queryInterface(DatabaseNode *p)             {return this;}
    const DatabaseNode *queryInterface(const DatabaseNode *p) const {return this;}
    const DatabaseNode *queryInterfaceIXmlBaseNode() const          {return this;}
  };
}

#endif
