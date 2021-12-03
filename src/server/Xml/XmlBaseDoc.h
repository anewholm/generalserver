//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XMLBASEDOC_H
#define _XMLBASEDOC_H

#include "define.h" //includes platform define also

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>

#include "IXml/IXmlBaseDoc.h"         //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance

using namespace std;

namespace general_server {
  //aheads
  interface_class IXmlNamespaceDefinition;
  interface_class IXmlQueryEnvironment;

  class XmlBaseDoc: implements_interface IXmlBaseDoc, virtual public MemoryLifetimeOwner {
    //private debug purposes only (see interface definition)
    IFDEBUG(const char *x() const;)

  protected: //only derived classes can create these helper Xml*
    //const char *x(); //private debug func needed because the debugger cannot resolve the virtual calls
    StringMap<const char*> m_mNamespaceDefinitions;
    bool m_bNamespaceDefinitionsCached;
    bool m_bDocumentRootNamespacing;

    XmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias);
    XmlBaseDoc(const XmlBaseDoc &oDoc);
    void importXmlNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pSourceNode, const bool bDeepClone = DEEP_CLONE);
    void importXmlDocs(const IXmlQueryEnvironment *pQE, const vector<const IXmlBaseDoc *> *pDocs, const IXmlBaseNode *pRootNode);
    
    virtual void cacheAllPrefixedNamespaceDefinitionsForXPath(IXmlBaseNode *pFromNode = 0, const bool bReset = NO_RESET_CACHE);
    void addPrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF);
    void ensurePrefixedNamespaceDefinition(const char *sPrefix, const char *sHREF);

  protected:
    const char  *m_sAlias;
    const IXmlLibrary *m_pLib;

    bool findAppropriateNamespaceRoots(XmlNodeList<IXmlHasNamespaceDefinitions>& vRoots, IXmlBaseNode *pRootNode, IXmlBaseNode *pFromNode, const bool bIgnoreResponseNamespace = true);
    IXmlBaseNode       *rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest bNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = "<root node>"); //calls node docNode->firstChild()
    const IXmlBaseNode *rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest bNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = "<root node>") const; //calls node docNode->firstChild()
    
  public:
    ~XmlBaseDoc();
    virtual void loadXml(const char *sXML ATTRIBUTE_UNUSED) {}
    int xmlIDNextAvailable(const char *sPrefix, const unsigned int iFrom = 0, const char *sSuffix = 0);
    const StringMap<const char*> *allPrefixedNamespaceDefinitions() const {return &m_mNamespaceDefinitions;}
    const char *standardPrefix(const char *sHREF) const;
    bool namespaceDefinitionsCached() const {return m_bNamespaceDefinitionsCached;}
    bool hasNamespacePrefix(const char *sNamespacePrefix) const;
    void copyNamespaceCacheFrom(const IXmlBaseDoc *pSourceDoc);
    const char *alias() const {return m_sAlias;}
    bool hasMultipleChildElements(const IXmlQueryEnvironment *pQE) const;
    size_t childElementCount(const IXmlQueryEnvironment *pQE) const;
    bool isEmpty(const IXmlQueryEnvironment *pQE) const; //no root elements
    bool isRootNode(const IXmlBaseNode *pNode) const;
    const char *stylesheetPILink(const IXmlQueryEnvironment *pQE, const bool bAppendServerSide = false, const bool bAppendDebug = false) const; //<?xml-stylesheet ... ?>

    //information
    virtual bool loadedAndParsed()  const {return true;}
    virtual bool resolveExternals() const {return false;}
    virtual bool specified()        const {return false;}
    virtual bool validateOnParse()  const {return false;}
    virtual const char *toString() const;

    //in-built indexing and other Xml linking standards
    //some native implementations, e.g. MSXML
    virtual XmlNodeList<IXmlBaseNode> *nodesWithXmlIDs(const IXmlQueryEnvironment *pQE) const; //returning table info is not required
    virtual XmlNodeList<IXmlBaseNode> *nodesAreIDs(    const IXmlQueryEnvironment *pQE) const;
    virtual XmlNodeList<IXmlBaseNode> *nodesAreIDRefs( const IXmlQueryEnvironment *pQE) const;

    //query functions
    //all primary move, link, copy functions are built off IXmlBaseNode
    IXmlBaseNode       *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0); //re-root to node
    const IXmlBaseNode *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0) const; //re-root to node
    XmlNodeList<IXmlBaseNode>       *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0); //re-root to node
    XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const; //re-root to node

    IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner,
      const IXslDoc *pXslStylesheet,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) const;
    IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) const;

    void transform(
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutputNode,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    );

    const char *xml(const IXmlQueryEnvironment *pQE, const bool bNoEscape = false) const; //re-root to node
    const IXmlLibrary *xmlLibrary() const;

    //handled by the IXmlBaseNode class because namespaces need a search context
    IXmlBaseNode *createRootChildElement(  const IXmlQueryEnvironment *pQE, const char *sName, const IXmlBaseDoc *pNamespaceDoc, const char *sContent = 0, const char *sNamespace = 0);
    IXmlBaseNode *createChildElement(      const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const char *sReason = 0);
    IXmlBaseNode *rootNode(                const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode);           //calls node docNode->moveChild(pNode)
    IXmlBaseNode *nodeFromID(              const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason = 0);
    virtual const IXmlBaseNode *nodeFromID(const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason = 0) const = 0;

    virtual bool removeProcessingInstructions();
    bool clearCachedStylesheetsFor(IXmlBaseNode *pDecendentNode);
    bool encodeCDATASections(const char *sReplacement = "gs:CDATA-section-escaped");
    void addAllStandardNamespaceDefinitionsToAppropriateRoots(IXmlBaseNode *pFromNode = NULL, const bool bIgnoreResponseNamespace = true);
    void moveAllPrefixedNamespaceDefinitionsToAppropriateRoots(IXmlBaseNode *pFromNode = NULL, const bool bIgnoreResponseNamespace = true);
    bool documentRootNamespacing() const;

    //interface navigation
    virtual const IXmlBaseDoc    *queryInterface(const IXmlBaseDoc *p) const;
    virtual IXmlBaseDoc          *queryInterface(IXmlBaseDoc *p);
    virtual IXslDoc              *queryInterface(IXslDoc *p);
    virtual const IXslDoc        *queryInterface(const IXslDoc *p) const;
  };
}

#endif
