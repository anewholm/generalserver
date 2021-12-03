//platform specific file (UNIX)
#ifndef _LIBXMLBASENODE_H
#define _LIBXMLBASENODE_H

#include "define.h"

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <stdarg.h>
using namespace std;

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

//http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/extensions.h>
#include <libxslt/extra.h>
#include <libxslt/templates.h>
//#include "XSLTExtensions.h"

#include <libexslt/exslt.h>

#include "IXml/IXmlQueryEnvironment.h"
#include "Xml/XmlBaseNode.h"
#include "IXml/IXmlNamespace.h"
#include "LibXmlDoc.h"
#include "Exceptions.h"

#define NO_INDENTATION 0
#define FORMAT_INDENT 1
#define ZERO_INITIAL_INDENT 0
#define NO_NAMESPACE 0
#define NO_CONTENT 0
#define NO_OPTIONS 0

#define REPORT_CACHE false

#define NODELINK_MULTIMAP multimap<const xmlNodePtr, IXmlBaseNode*>

namespace general_server {
  class LibXmlBaseNode: public XmlBaseNode {
    //Lib* sub-node types like LibXslNode should public LibXmlBaseNode
    friend class LibXmlBaseDoc;
    friend class LibXslXPathFunctionContext;

    bool m_bRegistered;
    bool m_bFreeInDestructor;
    static NODELINK_MULTIMAP    m_mNodeLinks;
    static pthread_mutex_t      m_mNodeLinksAccess;
    NODELINK_MULTIMAP::iterator m_iNodeLink;

  private:
    //utilities
    static void* convertTreeChangesToTXml(const void *tree_change_void, const void *pQE_void, const void *pvTXMLs_void);
    const xmlNsPtr getNsFromString(const char *sString, const char **sLocalName = 0) const;
    void traverse(bool (LibXmlBaseNode::*elementCallback)(xmlNodePtr oNode), xmlNodePtr oNode);
    xmlNodePtr addChildToMeOrSetRootSecure(const IXmlQueryEnvironment *pQE, xmlNodePtr oNode, const IXmlBaseNode *pBeforeNode = 0);
    xmlNodePtr docToRootIf(xmlNodePtr oNode);                  //if XML_DOCUMENT_NODE -> firstChild
    bool setNamespaceRootInternal_copyAncestorNsDefs();
    bool setNamespaceRootInternal_realignSubTree(xmlNodePtr oSiblingNode, xmlNsPtr oGSNs, xmlNsPtr oXSLNs);
    bool setNamespaceRootInternal_removeNsDefs(xmlNodePtr oSiblingNode);
    void swapProperties(xmlNodePtr oNode1, xmlNodePtr oNode2);
    void stripAndUnlinkNode(xmlNodePtr oNode);
    xmlAttrPtr attribute(const char *sName, const char *sNamespace = NAMESPACE_NONE) const;
    const char *nodeValueInternal() const;
    unsigned long documentOrder() const;

    bool registerNode();
    bool deregisterNode();
    void stopListeningForDeregistionNotify();
    bool isRegistered() const;

  public:
    IFDEBUG(const char           *x() const;) //XML
    IFDEBUG(const LibXmlBaseNode *d() const;) //Derived this
    IFDEBUG(const xmlNodePtr      o() const;) //xmlNodePtr

  protected:
    //platform specific
    xmlNodePtr m_oNode;
    xmlListPtr m_oParentRoute;
    xmlNsPtr   m_oReconciledNs; //the reconciled namespace which will exist in the document namespace pool
    //just makes the appropriate assignments
    //called by the 2 other constructors
    LibXmlBaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlLibrary *pLib, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0);
    LibXmlBaseNode(xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool pDocOurs = NOT_OURS);
    LibXmlBaseNode(xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool pDocOurs = NOT_OURS);

    //platform agnostic
    LibXmlBaseNode(); //only used in cloning. we need so we can indicate the type to clone into
    LibXmlBaseNode(const LibXmlBaseNode &node);
    iDOMNodeType nodeType() const;

    //constructors cannot be inherited in C++
    //virtual functions cannot be called from constructors
    //these are all protected because LibXmlBaseNode should not be created directly
    ~LibXmlBaseNode();
    bool deregistrationNotify();

  public:
    //platform specific
    //LibXml object access for LibXmlBaseDoc
    xmlNodePtr    libXmlNodePtr()        const {return (const xmlNodePtr) m_oNode;}
    xmlDocPtr     libXmlDocPtr()         const {return m_oNode->doc;}
    xmlListPtr    libXmlParentRoutePtr() const {return m_oParentRoute;}

    //LibXml2 generic trigger and filtering callback (author: Annesley)
    static const xmlChar *xmlParentRouteReporter(const xmlNodePtr oParentRouteNode, const void *user, unsigned int iReportLen);
    static int xmlElementTriggerCallback(xmlNodePtr cur, xmlListPtr parent_route, xmlNodeTriggerCallbackContextPtr xtrigger, int iTriggerOperation, int iTriggerStage, xmlNodeFilterCallbackContextPtr xfilter);
    static int xmlElementFilterCallback( xmlNodePtr cur, xmlListPtr parent_route, xmlNodeFilterCallbackContextPtr xfilter,   int iFilterOperation);
    static void xmlDeregisterNodeFunc(xmlNodePtr oNode);

    bool clearMyCachedStylesheets(const IXmlQueryEnvironment *pQE); //pQE as MM

    //namespaces
    //W3C namespace definitions: http://www.w3.org/TR/REC-xml-names/#defaulting
    //XmlBaseNode: const bool isNamespace(const char *sHREF) const;
    const char *namespaceHREF(const bool bDuplicate = true) const;
    const char *namespacePrefix(const bool bReconcileDefault = RECONCILE_DEFAULT, const bool bDuplicate = true) const;

    //the node traverse XPath caller
    XmlNodeList<const IXmlBaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0) const;
    void getObject(const IXmlQueryEnvironment *pQE, const char *sXPath, XmlNodeList<IXmlBaseNode> **ppNodeList, char **psResult, int *piResult, bool *pbResult, iXPathType *piType) const;
    IXmlBaseNode *hardlinkRouteToAncestor(const IXmlBaseNode *pAncestor, const char *sLockingReason = 0) const;

    //information
    //these are common functions to all node types: LibXmlNode, LibXslNode, etc.
    const char *value(const IXmlQueryEnvironment *pQE) const;
    const char *valueDynamic(  const IXmlQueryEnvironment *pQE, const char *sValue, const unsigned int iRecursions = 1) const;
    bool        isValue(const char *sValue) const; //fast
    const char *attributeValueDirect(const XmlAdminQueryEnvironment *pQE, const char *sName, const char *sNamespace = NAMESPACE_NONE, const char *sDefault = NULL) const;
    const char *nameAxisName(const IXmlQueryEnvironment *pQE) const; //name::?
    const char *currentParentRoute() const;
    void       *persistentUniqueMemoryLocationID() const {return (void*) m_oNode;}
    bool  is(const IXmlBaseNode *p2, const bool bHardlinkAware, const bool bParentRouteAware = true) const;
    size_t position() const;
    const char *xml( const IXmlQueryEnvironment *pQE, const int iMaxDepth = 0, const bool bNoEscape = false) const;
    const char *localName(const bool bDuplicate = true) const; //internal use only
    const char *toString() const;  //IReportable
    int   validityCheck(const char *sContext = 0) const;
    iAreaXMLIDPolicy xmlIDPolicy() const;
    bool isHardLinked() const;
    bool isHardLink() const;
    bool isSoftLink() const;
    bool isOriginalHardLink() const;
    bool isDeviant() const;
    bool hasDescendantDeviants(const IXmlBaseNode *pOriginal = 0) const; //a hardlink that has sub-tree deviants, optionally checking for a specific original
    bool hasDeviants(const IXmlBaseNode *pHardlink = 0)   const; //a node that has deviants, optionally in relation to a specific hardlink
    bool isTransient() const;

    //merging
    percentage     similarity( const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const;
    vector<TXml*> *differences(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const;

    //namespace and definitions support
    void addNamespaceDefinition(       const char *sHREF, const char *sPrefix);
    const StringMap<const char*> *namespaceDefinitions() const;
    void setNamespace(const char *sHREF);
    bool setNamespaceRoot(IXmlBaseNode *pFromNode = NULL);

    //update
    IXmlBaseNode *copyChild(        const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const bool bDeepClone = SHALLOW_CLONE, const IXmlBaseNode *pBeforeNode = 0, const int iDestinationPosition = 0, const iXMLIDPolicy iCopyXMLIDPolicy = xmlid_default);
    IXmlBaseNode *moveChild(        const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = NULL, const int iDestinationPosition = 0, const iXMLIDPolicy iCopyXMLIDPolicy = xmlid_default);
    IXmlBaseNode *deviateNode(                 const IXmlQueryEnvironment *pQE, IXmlBaseNode *pHardlink, const IXmlBaseNode *pNewNode, const char *sAttributeName = 0, const iDeviationType iType = deviation_replacement);
    IXmlBaseNode *hardlinkChild(    const IXmlQueryEnvironment *pQE, IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = NULL, const int iDestinationPosition = 0);
    IXmlBaseNode *softlinkChild(    const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, const IXmlBaseNode *pBeforeNode = NULL);
    IXmlBaseNode *mergeNode(        const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default); //must be implemented, should throw CapabilityNotSupported(this)
    IXmlBaseNode *replaceNodeCopy(  const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNodeToMove, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default); //must be implemented, should throw CapabilityNotSupported(this)
    bool removeAndDestroyNode(           const IXmlQueryEnvironment *pQE);
    bool removeAndDestroyChildren(       const IXmlQueryEnvironment *pQE);
    IXmlBaseNode *parseXMLInContextNoAddChild( const IXmlQueryEnvironment *pQE, const char *sXML,  const bool bRemoveRedundantNamespaceDeclarations = false, const bool bTranslateEntities = false) const;
    IXmlBaseNode *parseHTMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sHTML, const bool bRemoveRedundantNamespaceDeclarations = false, const bool bTranslateEntities = false) const;
    const char *value(const IXmlQueryEnvironment *pQE, const char *sValue);
    void setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace = NULL) const;
    void removeAttribute(const IXmlQueryEnvironment *pQE, const char *sName) const;
    void touch(const IXmlQueryEnvironment *pQE);

    IXmlBaseNode *createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent = 0, const char *sNamespace = 0, const bool bTransient = false, const char *sReason = 0);
    void createTextNode(const IXmlQueryEnvironment *pQE, const char *sContent = 0);
    void localName(const IXmlQueryEnvironment *pQE, const char *sName);
    IXmlBaseNode *copyNode(const IXmlQueryEnvironment *pQE, const bool bDeepClone = DEEP_CLONE, const bool bEvaluateValues = COPY_ONLY, iXMLIDPolicy iXMLIDMovePolicy = xmlid_default) const;

    void createHardlinkPlaceholderIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const;
    void createDeviationPlaceholdersIn(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const;
    const IXmlBaseNode *softlinkTarget(const IXmlQueryEnvironment *pQE) const;

  protected:
    void transform(const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      IXmlBaseNode *pOutput,
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
  };
}

#endif
