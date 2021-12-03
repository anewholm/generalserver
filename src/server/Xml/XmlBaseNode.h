//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XMLBASENODE_H
#define _XMLBASENODE_H

#include "define.h" //includes platform define also

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Utilities/StringMap.h"

//lots of these needed because of all-in-one template declerations
#include "IXml/IXmlBaseNode.h"        //direct inheritance
#include "Xml/XmlNamespace.h"        //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance

using namespace std;

namespace general_server {
  //normal ahead declerations
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlNodeMask;
  interface_class IXmlDoc;
  interface_class IXmlNode;
  interface_class IXslDoc;
  interface_class IXslNode;
  interface_class IXslStylesheetNode;
  interface_class IXSchemaDoc;
  interface_class IXSchemaNode;

  interface_class IXslModule;
  interface_class IXslTransformContext;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //NOTE: inheritance order and virtuality are important here
  class XmlBaseNode: implements_interface IXmlBaseNode, virtual public MemoryLifetimeOwner, virtual public XmlNamespaced, virtual public XmlHasNamespaceDefinitions {
    //only classes in this file should have access to the platform specific members
    //no concept of child XmlNodes exists here (yet)
    //transforms and XPath should be used to query
    static StringMap<const char*> m_mStandardNamespaceDefinitions;
    const bool m_bDocOurs;
    
    const char *constructName(const char *sPrefix, const char *sLocalName) const;

  protected: //only derived classes can create these helper Xml*
    //constructors, this is a utility class, never to be directly instantiated
    XmlBaseNode(); //only used in cloning. we need so we can indicate the type to clone into
    XmlBaseNode(const XmlBaseNode &node);
    XmlBaseNode(IXmlBaseDoc *pDoc, const bool bDocOurs = false);
    ~XmlBaseNode();

    IXmlBaseDoc *m_pDoc;
    
    bool uniqueXPathToNodeInternal(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const IXmlBaseNode *pBaseNode, stringstream &sXPath, const bool bWarnIfTransient, const bool bForceBaseNode, const bool bEnableIDs, const bool bEnableNameAxis, const bool bHasDefaultNameAxis, const bool bDebug) const; //internal recursive
    IXmlBaseNode *attributeValueIDNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const char *sIDAttributeName, const char *sNamespace = NAMESPACE_NONE);

  public:
    //accessors
    const IXmlBaseDoc *document() const;
    IXmlBaseDoc *document();
    const IXmlLibrary *xmlLibrary() const;

    //queries
    XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const = 0;
    XmlNodeList<IXmlBaseNode>       *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0);
    XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const bool bRegister, const char *sLockingReason = 0) const;
    XmlNodeList<const IXmlBaseNode> *fileSystemPathToNodeList(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const bool bRejectAbsolute = ALLOW_ABSOLUTE, const char *sTargetType = "*", const char *sDirectoryNamespacePrefix = NULL, const bool bUseIndexes = true) const;
    const IXslStylesheetNode *relativeXSLIncludeFileSystemPathToXSLStylesheetNode(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath) const;
    const char *uniqueXPathToNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = WARN_IF_TRANSIENT, const bool bForceBaseNode = NO_FORCE_BASE_NODE, const bool bEnableIDs = false, const bool bEnableNameAxis = true, const bool bThrowIfNotBasePath = true) const;
    IXmlBaseNode       *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const char *sLockingReason = 0); //uses this getMultipleNodes
    const IXmlBaseNode *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const char *sLockingReason = 0) const; //uses this getMultipleNodes
    const IXmlBaseNode *firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0) const;
    IXmlBaseNode       *firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0);
    const IXmlBaseNode *lastChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sLockingReason = 0) const;
    IXmlBaseNode *lastChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sLockingReason = 0);
    const IXmlBaseNode *child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sNamespace = 0, const char *sLockingReason = 0) const;
    IXmlBaseNode *child(const IXmlQueryEnvironment *pQE, const int iPosition, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sNamespace = 0, const char *sLockingReason = 0);
    XmlNodeList<IXmlBaseNode> *children(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    XmlNodeList<const IXmlBaseNode> *children(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const;
    XmlNodeList<IXmlBaseNode> *attributes(const IXmlQueryEnvironment *pQE);
    XmlNodeList<const IXmlBaseNode> *attributes(const IXmlQueryEnvironment *pQE) const;
    XmlNodeList<      IXmlBaseNode> *ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    XmlNodeList<const IXmlBaseNode> *ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const; //uses this getMultipleNodes
    const IXmlBaseNode *parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const;
    IXmlBaseNode *parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    IXmlBaseNode *followingSibling(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0);
    const IXmlBaseNode *followingSibling(const IXmlQueryEnvironment *pQE, const char *sLockingReason = 0) const;
    virtual IXmlBaseNode *clone_wrapper_only() const = 0;
    bool deregistrationNotify();

    //attributes, values names
    //primary attributeValue(...) uses xpath so is implemented here
    const char   *attributeValue(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const;
    bool    attributeExists(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const;
    int     attributeValueInt(       const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const int iDefault = 0) const;
    bool    attributeValueBoolInterpret(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const bool bDefault) const;
    bool    attributeValueBoolDynamicString(const IXmlQueryEnvironment *pQE,        const char *sName, const char *sNamespace = NAMESPACE_NONE, const bool bDefault = false) const;
    IXmlBaseNode *attributeValueNode(      const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pFromNode = 0);
    const IXmlBaseNode *attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pFromNode = 0) const;
    XmlNodeList<const IXmlBaseNode> *attributeValueNodes(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pBaseNode = 0) const;
    IXmlNodeMask *attributeValueNodeMask(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE) const;
    const char   *attributeValueUniqueXPath(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = INCLUDE_TRANSIENT, const bool bForceBaseNode = NO_FORCE_BASE_NODE) const;
    const char   *xmlID(const IXmlQueryEnvironment *pQE) const;
    const char   *group(const IXmlQueryEnvironment *pQE, const bool bDuplicate = true) const; //name(..)
    
    void          appendAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sSeperator = ",")  const;
    virtual void  setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue,  const char *sNamespace = NULL) const = 0; //called here by overriden funcs
    void          setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue,    const char *sNamespace = NULL) const;
    void          setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace = NULL) const;
    void          setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue,   const char *sNamespace = NULL) const;
    void          setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const unsigned int uValue, const char *sSuffix = 0) const;
    void          setXmlID_serverFree(const IXmlQueryEnvironment *pQE, const char *sPrefix, const unsigned int uValue, const char *sSuffix = 0) const;
    void          setXmlID(const IXmlQueryEnvironment *pQE, const char *sPrefix, const char *sName = 0) const;
    const char   *attributeValueDynamic(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL, const unsigned int iRecursions = 1) const;
    IXmlBaseNode *setTransient(const IXmlQueryEnvironment *pQE) const;
    IXmlBaseNode *setTransientArea(const IXmlQueryEnvironment *pQE);
    const char   *fullyQualifiedName() const;
    const char   *standardPrefix() const;
    const char   *toString() const;
    bool hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const;
    bool isEmpty(const IXmlQueryEnvironment *pQE) const; //no root elements
    size_t childElementCount(const IXmlQueryEnvironment *pQE) const;
    virtual iErrorPolicy errorPolicy() const;

    //is
    bool in(const XmlNodeList<const IXmlBaseNode> *pVec, const bool bHardlinkAware) const;
    bool in(const XmlNodeList<IXmlBaseNode> *pVec, const bool bHardlinkAware) const;
    bool isAncestorOf(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pDesendentNode) const;
    const char *typeName()       const;
    bool isNodeElement()   const;
    bool isNodeDocument()  const;
    bool canHaveNamespace() const;
    bool isNodeAttribute() const;
    bool isNodeText()      const;
    bool isProcessingInstruction() const;
    bool isNodeNamespaceDecleration() const;
    bool isCDATASection()  const;
    //const bool isTransient()     const; //PERFORMANCE: needs to be low down
    bool isTransientAreaParent() const;
    bool isRootNode()      const;

    //namespaces and definitions
    //W3C namespace definitions: http://www.w3.org/TR/REC-xml-names/#defaulting
    bool hasNamespace() const;
    bool isNamespace(const char *sHREF) const;
    bool isName(const char *sHREF, const char *sLocalName) const;
    virtual void addDefaultNamespaceDefinition(const char *sHREF);

    //update
    virtual IXmlBaseNode *copyChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const bool bDeepClone = DEEP_CLONE, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0; //must be implemented
    IXmlBaseNode *copyChild(        const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const bool bDeepClone = DEEP_CLONE, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default);
    virtual IXmlBaseNode *moveChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0;      //must be implemented
    IXmlBaseNode *moveChild(        const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default);
    virtual IXmlBaseNode *hardlinkChild(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0) = 0;       //must be implemented, should throw CapabilityNotSupported(this)
    IXmlBaseNode *hardlinkChild(    const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0);
    virtual IXmlBaseNode *softlinkChild(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = NULL) = 0; //must be implemented, should throw CapabilityNotSupported(this)
    IXmlBaseNode *softlinkChild(    const IXmlQueryEnvironment *pQE, IXmlBaseDoc *pDoc);
    virtual IXmlBaseNode *mergeNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0; //must be implemented, should throw CapabilityNotSupported(this)
    IXmlBaseNode *mergeNode(  const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default);
    virtual IXmlBaseNode *replaceNodeCopy(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) = 0; //must be implemented, should throw CapabilityNotSupported(this)
    IXmlBaseNode *replaceNodeCopy(  const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pDoc, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default);
    virtual void moveChildren(                 const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNewParent);
    IXmlBaseNode *getOrCreateChildElement(     const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0);

  public:
    //interface navigation
    virtual IXslCommandNode          *queryInterface(IXslCommandNode *p);
    virtual const IXslCommandNode    *queryInterface(const IXslCommandNode *p) const;
    virtual const IXmlBaseNode       *queryInterface(const IXmlBaseNode *p) const;
    virtual IXmlBaseNode             *queryInterface(IXmlBaseNode *p);
    virtual const IXslStylesheetNode *queryInterface(const IXslStylesheetNode *p) const;
    virtual const IXslTemplateNode   *queryInterface(const IXslTemplateNode *p) const;
    virtual const IXslNode           *queryInterface(const IXslNode *p) const;
    virtual IXmlNamespaced           *queryInterface(IXmlNamespaced *p);
    virtual const IXmlNamespaced     *queryInterface(const IXmlNamespaced *p) const;
    virtual IXmlHasNamespaceDefinitions *queryInterface(IXmlHasNamespaceDefinitions*);
    virtual const IXmlHasNamespaceDefinitions *queryInterface(const IXmlHasNamespaceDefinitions*) const;
    const IXmlBaseNode               *queryInterfaceIXmlBaseNode() const;
  };
}

#endif
