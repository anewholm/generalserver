//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXMLBASEDOC_H
#define _IXMLBASEDOC_H

#include "define.h" //includes platform define also

//misc (see functions for explanation)
#define LOAD_FILE true
#define NO_RESET_CACHE false
#define RESET_CACHE true

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>
#include "Utilities/StringMap.h"
using namespace std;

#include "MemoryLifetimeOwner.h" //direct inheritance
#include "IReportable.h"         //direct inheritance
#include "IXml/IXmlBaseNode.h"        //enums

namespace general_server {
  //normal ahead declerations
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlDoc;
  interface_class IXslDoc;
  interface_class IXslStylesheetNode;
  interface_class IXmlNamespaceDefinition;
  interface_class ITXmlDirectFunctions;
  class Repository;
  class MemoryLifetimeOwner;


  //-------------------------------------------------------------------------------------------
  //-------------------------------------- IXmlBaseDoc ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  interface_class IXmlBaseDoc: implements_interface IMemoryLifetimeOwner, implements_interface IReportable {
  public:
    //private debug xml()
    IFDEBUG(virtual const char *x() const = 0;)

  protected:
    //----------------------------------------------------- contructors!
    //when constructing a new IXml* document from an existing node source
    //security context must be sent through to read the source document

    //----------------------------------------------------- internal doc use
  public: //TODO: make these operations protected because they are internal
    friend class XmlBaseDoc; //(*iDoc)->rootNode() doc assembly from multiple other docs
    virtual const IXmlBaseNode *documentNode(const char *sLockingReason = "<document node>") const = 0; //returns a pointer to the actual document
    virtual IXmlBaseNode       *documentNode(const char *sLockingReason = "<document node>") = 0;             //returns a pointer to the actual document
    virtual const IXmlBaseNode *dtdNode() const = 0;
    virtual IXmlBaseNode       *rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest bNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = "<root node>") = 0; //calls node firstChild()
    virtual const IXmlBaseNode *rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest bNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = "<root node>") const = 0; //calls node firstChild()
    
  public:
    virtual void importXmlNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pSourceNode, const bool bDeepClone = DEEP_CLONE) = 0;    
    virtual void importXmlDocs(const IXmlQueryEnvironment *pQE, const vector<const IXmlBaseDoc *> *pDocs, const IXmlBaseNode *pRootNode) = 0;
    virtual void loadXml(const char *sXML) = 0;
    virtual void streamLoadXML(Repository *pRepository) = 0;
    virtual ~IXmlBaseDoc() {} //virtual public destructors allow proper destruction of derived object

    //----------------------------------------------------- read-only section
    //info (read-only)
    virtual const char *alias() const = 0;
    virtual const char *encoding() const = 0;
    virtual bool validate() = 0;             //DTD and XSchema checks
    virtual bool loadedAndParsed()  const = 0;
    virtual bool resolveExternals() const = 0;
    virtual const char *stylesheetPILink(const IXmlQueryEnvironment *pQE, const bool bAppendServerSide = false, const bool bAppendDebug = false) const = 0; //<?xml-stylesheet ... ?>
    virtual bool specified()        const = 0;
    virtual bool validateOnParse()  const = 0;
    virtual const StringMap<const char*> *allPrefixedNamespaceDefinitions() const = 0; //proactively cache please
    virtual int  validityCheck(const char *sReason = 0) const = 0; //recurse LibXML structures looking for consistency errors
    virtual bool ours() const = 0; //indicates if this document wrapper should free the underlying structure on destruction
    virtual IXmlBaseDoc *clone_wrapper_only() const = 0;
    virtual IXmlBaseDoc *clone_with_resources() const = 0;

    virtual void stylesheetCacheLock(  const char *sReason = 0) = 0;
    virtual void stylesheetCacheUnlock(const char *sReason = 0) = 0;

    //Memento pattern: serialise (read-only)
    virtual void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const = 0;
    virtual const char *xml( const IXmlQueryEnvironment *pQE, const bool bNoEscape = false) const = 0; //re-root to node
    virtual bool is(const IXmlBaseDoc *pDoc2) const = 0; //node creation should check the incoming doc parameter with is(...)
    virtual bool isRootNode(const IXmlBaseNode *pNode) const = 0;
    //accessors (read-only)
    virtual const IXmlLibrary *xmlLibrary() const = 0;

    //traversal (read-only)
    //UNIX uses xmlDocPtr as the document xmlNodePtr but there are differences
    //WINDOWS inherits Doc from Node so also returns a pointer to the document
    //rootNode = documentNode->firstChild()
    //however, careful that different implementations may consider different node types to be firstChild()s
    //these mostly re-rooted to the IXmlBaseNode rootNode:
    //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
    virtual IXmlBaseNode       *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0) = 0;
    virtual const IXmlBaseNode *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode>       *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) = 0;
    virtual XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const = 0;

    //indexes (read-only)
    //TODO: ID + IDREF and xml:id in-built indexing and other Xml linking standards
    //some native implementations, e.g. MSXML
    virtual XmlNodeList<IXmlBaseNode> *nodesWithXmlIDs(const IXmlQueryEnvironment *pQE) const       = 0;
    virtual XmlNodeList<IXmlBaseNode> *nodesAreIDs(    const IXmlQueryEnvironment *pQE) const       = 0;
    virtual XmlNodeList<IXmlBaseNode> *nodesAreIDRefs( const IXmlQueryEnvironment *pQE) const       = 0;
    virtual IXmlBaseNode              *nodeFromID(     const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason = 0) = 0;
    virtual const IXmlBaseNode        *nodeFromID(     const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason = 0) const = 0;
    virtual XmlNodeList<IXmlBaseNode> *nodeFromRef(    const IXmlQueryEnvironment *pQE, const char *sRef) = 0;

    virtual bool hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const = 0;
    virtual bool isEmpty(const IXmlQueryEnvironment *pQE) const = 0; //no root elements
    virtual size_t childElementCount(const IXmlQueryEnvironment *pQE) const = 0;

    virtual IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner,
      const IXslDoc *pXslStylesheet,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) const = 0;
    virtual IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner,
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) const = 0;
    virtual void transform(
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutput,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) = 0;

    //----------------------------------------------------- write section
    //instanciation (create)
    //suggested: StringMap<XmlNamespaceDefinition> m_mNamespaceDefinitions; //proactively cached, specific event updated
  public:
    virtual int xmlIDNextAvailable(const char *sPrefix, const unsigned int iFrom = 0, const char *sSuffix = 0) = 0;

    //namespace cache (write)
    virtual void cacheAllPrefixedNamespaceDefinitionsForXPath(IXmlBaseNode *pFromNode = 0, const bool bReset = NO_RESET_CACHE) = 0; //proactively cache please
    virtual void addPrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF) = 0;
    virtual void ensurePrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF) = 0;
    virtual bool hasNamespacePrefix(const char *sNamespacePrefix) const = 0;
    virtual bool namespaceDefinitionsCached() const = 0;
    virtual void copyNamespaceCacheFrom(const IXmlBaseDoc *pSourceDoc) = 0;
    //see also the XslModuleManager::extensionElementPrefixes() which comes from the application layer
    //this info comes from the XML Library
    virtual const char *extensionElementPrefixes() const = 0;
    virtual const char *standardPrefix(const char *sHREF) const = 0;
    virtual void addAllStandardNamespaceDefinitionsToAppropriateRoots( IXmlBaseNode *pFromNode = NULL, const bool bIgnoreResponseNamespace = true) = 0;
    virtual void moveAllPrefixedNamespaceDefinitionsToAppropriateRoots(IXmlBaseNode *pFromNode = NULL, const bool bIgnoreResponseNamespace = true) = 0;
    virtual bool documentRootNamespacing() const = 0;

    //tree changes (write)
    //including the IXmlBaseNode concrete type in the parameters so we can use Visitor
    //and also make direct changes to this in a constructor by reference
    virtual IXmlBaseNode *createRootChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const IXmlBaseDoc *pNamespaceDoc, const char *sContent = 0, const char *sNamespace = 0) = 0;
    virtual IXmlBaseNode *createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const char *sReason = 0) = 0;

    //misc (write)
    virtual bool removeProcessingInstructions() = 0; //some native libraries do not require this (its a setting)
    virtual bool clearCachedStylesheetsFor(IXmlBaseNode *pDecendentNode) = 0;

    //----------------------------------------------------- interface navigation
    //concrete classes MUST implement relevant queries to return the correct vtable pointer
    //leaf Interfaces may REQUIRE this with pure virtual, e.g. IXslStylesheetNode
    //this base one will return a fail on any Derived class that hasnt provided a local context implicit casting
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
  public:
    virtual const IXmlBaseDoc    *queryInterface(const IXmlBaseDoc *p) const   = 0;
    virtual IXmlBaseDoc          *queryInterface(IXmlBaseDoc *p)               = 0;
    virtual IXslDoc              *queryInterface(IXslDoc *p)                   = 0;
    virtual const IXslDoc        *queryInterface(const IXslDoc *p) const       = 0;
  };
}

#endif
