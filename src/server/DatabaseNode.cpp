//platform agnostic file
#include "DatabaseNode.h"

#include "Xml/XmlAdminQueryEnvironment.h"
#include "Utilities/container.c"
using namespace std;

namespace general_server {
  DatabaseNode::DatabaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const Database *pDatabase, const IXmlBaseNode *pNode, const bool bOurs):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pDatabase((Database*) pDatabase),
    m_pNode((IXmlBaseNode*) pNode), //this is really a factory: conversion here because DatabaseNode manages the m_pNode
    m_bOurs(bOurs)
  {
    assert(m_pNode);
    assert(m_pDatabase);
  }

  DatabaseNode::DatabaseNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, Database *pDatabase, IXmlBaseNode *pNode, const bool bOurs):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pDatabase(pDatabase),
    m_pNode((IXmlBaseNode*) pNode), //this is really a factory: conversion here because DatabaseNode manages the m_pNode
    m_bOurs(bOurs)
  {
    assert(m_pNode);
    assert(m_pDatabase);
  }

  DatabaseNode::~DatabaseNode() {
    if (m_bOurs) delete m_pNode;
  }

  DatabaseNode *DatabaseNode::clone_with_resources() const {
    return new DatabaseNode(*this);
  }

  bool DatabaseNode::registerNode() {
    return m_pNode->registerNode();
  }

  bool DatabaseNode::deregisterNode() {
    return m_pNode->deregisterNode();
  }

  const IXmlBaseDoc *DatabaseNode::document_const() const {
    return m_pNode->document();
  }

  void *DatabaseNode::persistentUniqueMemoryLocationID() const {
    return m_pNode->persistentUniqueMemoryLocationID();
  }

  IXmlBaseNode *DatabaseNode::node_transient() const {
    //only nodes that are in a transient area already, i.e. have a transient-area ancestor
    //or nodes that are directly labelled transient
    //for performance reasons it is always good to directly label transient nodes
    IXmlBaseNode *pTransientNode = 0;

    if (m_pNode->isTransient()) pTransientNode = m_pNode;
    else throw NonTransientNode(this, MM_STRDUP("node_transient()"), m_pNode);

    return pTransientNode;
  }

  IXmlBaseNode *DatabaseNode::node_admin(XmlAdminQueryEnvironment *pQE) {
    //those with a valid XmlAdminQueryEnvironment can access the node direct
    return m_pNode;
  }

  const Database *DatabaseNode::db() const {return m_pDatabase;}
  Database       *DatabaseNode::db()       {return m_pDatabase;}

  const IXmlLibrary *DatabaseNode::xmlLibrary() const {return m_pDatabase->xmlLibrary();}
  IFDEBUG(const char *DatabaseNode::x() const {return m_pNode->x();})

  //----------------------------------------------------- read-only direct section
  const char *DatabaseNode::currentParentRoute() const {
    return m_pNode->currentParentRoute();
  }
  
  const char *DatabaseNode::uniqueXPathToNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode, const bool bWarnIfTransient, const bool bForceBaseNode, const bool bEnableIDs, const bool bEnableNameAxis, const bool bThrowIfNotBasePath) const {
    return m_pNode->uniqueXPathToNode(pQE, pBaseNode, bWarnIfTransient, bForceBaseNode, bEnableIDs, bEnableNameAxis, bThrowIfNotBasePath);
  }

  const char *DatabaseNode::fullyQualifiedName() const {
    return m_pNode->fullyQualifiedName();
  }

  bool DatabaseNode::isNamespace(const char *sHREF) const {return m_pNode->isNamespace(sHREF);}

  //node traversal (read-only)
  DatabaseNode *DatabaseNode::firstChild(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) {
    return m_pDatabase->factory_node(m_pNode->firstChild(pQE, iNodeTypes, sName, sLockingReason));
  }
  
  DatabaseNode *DatabaseNode::parentNode(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return m_pDatabase->factory_node(m_pNode->parentNode(pQE, sLockingReason));
  }
  
  XmlNodeList<DatabaseNode> *DatabaseNode::children(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return m_pDatabase->factory_nodes(m_pNode->children(pQE, sLockingReason));
  }

  XmlNodeList<const DatabaseNode> *DatabaseNode::children(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    return m_pDatabase->factory_nodes(node_const()->children(pQE, sLockingReason));
  }

  XmlNodeList<DatabaseNode> *DatabaseNode::ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason) {
    return m_pDatabase->factory_nodes(m_pNode->ancestors(pQE, sLockingReason));
  }

  XmlNodeList<const DatabaseNode> *DatabaseNode::ancestors(const IXmlQueryEnvironment *pQE, const char *sLockingReason) const {
    return m_pDatabase->factory_nodes(node_const()->ancestors(pQE, sLockingReason));
  }

  //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
  const DatabaseNode *DatabaseNode::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint) const {
    return m_pDatabase->factory_node(node_const()->getSingleNode(pQE, sXPath, argchar, argint));
  }

  DatabaseNode *DatabaseNode::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint) {
    return m_pDatabase->factory_node(m_pNode->getSingleNode(pQE, sXPath, argchar, argint));
  }

  XmlNodeList<DatabaseNode> *DatabaseNode::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) {
    return m_pDatabase->factory_nodes(m_pNode->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason));
  }

  XmlNodeList<const DatabaseNode> *DatabaseNode::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) const {
    return m_pDatabase->factory_nodes(node_const()->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason));
  }

  void DatabaseNode::transform(
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    IXmlBaseNode *pOutputNode,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode, const IXmlBaseNode *pIncludeDirectory
  ) const {
    m_pNode->transform(pXslStylesheetNode, pQE, pOutputNode, pParamsInt, pParamsChar, pParamsNodeSet, sWithMode, pIncludeDirectory);
  }

  IXmlBaseDoc *DatabaseNode::transform(
    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
    const IXslStylesheetNode *pXslStylesheetNode,
    const IXmlQueryEnvironment *pQE,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode, const IXmlBaseNode *pIncludeDirectory
  ) const {
    return m_pNode->transform(pMemoryLifetimeOwner, pXslStylesheetNode, pQE, pParamsInt, pParamsChar, pParamsNodeSet, sWithMode, pIncludeDirectory);
  }

  DatabaseNode *DatabaseNode::factory_node(IXmlBaseNode *pNode) {return db()->factory_node(pNode);}
  const DatabaseNode *DatabaseNode::factory_node(const IXmlBaseNode *pNode) const {return db()->factory_node(pNode);}

  //attributes, values, names (read-only)
  //  process curly brackets with pTransformingStylesheet
  const char *DatabaseNode::value(const IXmlQueryEnvironment *pQE) const {
    //caller frees result
    return m_pNode->value(pQE);
  }
  DatabaseNode *DatabaseNode::parseXMLInContextNoAddChild(const IXmlQueryEnvironment *pQE, const char *sXML) {
    return m_pDatabase->factory_node(m_pNode->parseXMLInContextNoAddChild(pQE, sXML));
  }
  const char   *DatabaseNode::attributeValue(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const char *sDefault) const {
    return m_pNode->attributeValue(pQE, sName, sNamespace, sDefault);
  }

  const char   *DatabaseNode::attributeValueDirect(   const XmlAdminQueryEnvironment *pQE, const char *sName, const char *sNamespace, const char *sDefault) const {
    return m_pNode->attributeValueDirect(pQE, sName, sNamespace, sDefault);
  }
  
  int     DatabaseNode::attributeValueInt(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const int iDefault) const {
    return m_pNode->attributeValueInt(pQE, sName, sNamespace, iDefault);
  }

  bool DatabaseNode::attributeValueBoolInterpret(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const bool bDefault) const {
    return m_pNode->attributeValueBoolInterpret(pQE, sName, sNamespace, bDefault);
  }
  
  bool    DatabaseNode::attributeValueBoolDynamicString(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const bool bDefault) const {
    return m_pNode->attributeValueBoolDynamicString(pQE, sName, sNamespace, bDefault);
  }
  DatabaseNode *DatabaseNode::attributeValueNode(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace) const {
    return m_pDatabase->factory_node(m_pNode->attributeValueNode(pQE, sName, sNamespace));
  }
  const char   *DatabaseNode::attributeValueUniqueXPath(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace, const IXmlBaseNode *pBaseNode, const bool bForceBaseNode) const {
    return m_pNode->attributeValueUniqueXPath(pQE, sName, sNamespace, pBaseNode, bForceBaseNode);
  }
  bool DatabaseNode::is(const DatabaseNode *p2, const bool bHardlinkAware) const {
    return m_pNode->is(p2->m_pNode, bHardlinkAware);
  }
  bool DatabaseNode::is(const IXmlBaseNode *p2, const bool bHardlinkAware) const {
    return m_pNode->is(p2, bHardlinkAware);
  }
  bool DatabaseNode::isRootNode() const {
    return m_pNode->isRootNode();
  }

  const char *DatabaseNode::group(const IXmlQueryEnvironment *pQE, const bool bDuplicate) const {
    return m_pNode->group(pQE, bDuplicate);
  }

  const char *DatabaseNode::typeName()  const {return m_pNode->typeName();}
  bool DatabaseNode::isNodeElement()    const {return m_pNode->isNodeElement();}
  bool DatabaseNode::isNodeDocument()   const {return m_pNode->isNodeDocument();}
  bool DatabaseNode::isNodeAttribute()  const {return m_pNode->isNodeAttribute();}
  bool DatabaseNode::isNodeText()       const {return m_pNode->isNodeText();}
  bool DatabaseNode::isNodeNamespaceDecleration() const {return m_pNode->isNodeNamespaceDecleration();}
  bool DatabaseNode::isProcessingInstruction() const {return m_pNode->isProcessingInstruction();}
  bool DatabaseNode::isCDATASection()   const {return m_pNode->isCDATASection();}

  bool DatabaseNode::isHardLink() const {
    return m_pNode->isHardLink();
  }

  bool DatabaseNode::isSoftLink() const {
    return m_pNode->isSoftLink();
  }

  bool DatabaseNode::isTransient() const {
    return m_pNode->isTransient();
  }
  bool DatabaseNode::attributeExists(const IXmlQueryEnvironment *pQE, const char *sName, const char *sNamespace) const {
    return m_pNode->attributeExists(pQE, sName, sNamespace);
  }

  const char *DatabaseNode::xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth) const {
    return m_pNode->xml(pQE, iMaxDepth);
  }

  void DatabaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace) {
    //this is a TXML based change
    //stores the repeatable Transaction
    TXml_SetAttribute t(this, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, m_pNode, sName, sValue, sNamespace);
    t.applyTo(m_pDatabase, NO_RESULTS_NODE, pQE);
  }

  void DatabaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue, const char *sNamespace) {
    char *sValue = itoa(iValue);
    setAttribute(pQE, sName, sValue, sNamespace);
    MMO_FREE(sValue);
  }

  void DatabaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace) {
    char *sValue = itoa(uValue);
    setAttribute(pQE, sName, sValue, sNamespace);
    MMO_FREE(sValue);
  }

  void DatabaseNode::setAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue, const char *sNamespace) {
    setAttribute(pQE, sName, (bValue ? "yes" : "no"), sNamespace);
  }

  IXmlBaseNode *DatabaseNode::setTransient(const IXmlQueryEnvironment *pQE) {
    //by definition this will be a non-transient and therefore probably const
    //so we remove the const
    m_pNode->setTransient(pQE);
    return m_pNode;
  }

  const IXmlBaseNode *DatabaseNode::setTransientArea(const IXmlQueryEnvironment *pQE) {
    //by definition this will be a non-transient and therefore probably const
    //so we remove the const
    m_pNode->setTransientArea(pQE);
    return m_pNode;
  }

  void DatabaseNode::removeAttribute(const IXmlQueryEnvironment *pQE, const char *sName) {
    NOT_COMPLETE("");
  }

  DatabaseNode *DatabaseNode::createChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace) {
    IXmlBaseNode *pResultNode = 0;
    TXml_CreateChildElement t(this, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, m_pNode, sName, sContent, sNamespace);
    t.applyTo(m_pDatabase, &pResultNode, pQE);
    return factory_node(pResultNode);
  }

  DatabaseNode *DatabaseNode::copyChild(const IXmlQueryEnvironment *pQE, const DatabaseNode *pNodeToMove, const bool bDeepClone, const DatabaseNode *pBeforeNode, iXMLIDPolicy iXMLIDMovePolicy) {
    IXmlBaseNode *pResultNode = 0;
    //TODO: TXml_CopyChild iXMLIDMovePolicy?
    TXml_CopyChild t(this, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pNodeToMove->node_const(), m_pNode, pBeforeNode->node_const(), bDeepClone);
    t.applyTo(m_pDatabase, &pResultNode, pQE);
    return factory_node(pResultNode);
  }

  DatabaseNode *DatabaseNode::deviateNode(const IXmlQueryEnvironment *pQE, DatabaseNode *pHardlink, const DatabaseNode *pNewNode, const iDeviationType iType) {
    IXmlBaseNode *pResultNode = 0;
    NOT_COMPLETE("");
    //TXml_DeviateNode t(pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pHardlink->node_const(), m_pNode, pNewNode->node_const(), iType);
    //t.applyTo(m_pDatabase, &pResultNode, pQE);
    return factory_node(pResultNode);
  }

  DatabaseNode *DatabaseNode::hardlinkChild(const IXmlQueryEnvironment *pQE, DatabaseNode *pCommonChild, const DatabaseNode *pBeforeNode) {
    IXmlBaseNode *pResultNode = 0;
    TXml_HardlinkChild t(this, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pCommonChild->node_const(), m_pNode, pBeforeNode ? pBeforeNode->node_const() : 0);
    t.applyTo(m_pDatabase, &pResultNode, pQE);
    return factory_node(pResultNode);
  }

  DatabaseNode *DatabaseNode::softlinkChild(const IXmlQueryEnvironment *pQE, DatabaseNode *pCommonChild, const DatabaseNode *pBeforeNode) {
    IXmlBaseNode *pResultNode = 0;
    TXml_SoftlinkChild t(this, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pCommonChild->node_const(), m_pNode, pBeforeNode ? pBeforeNode->node_const() : NULL);
    t.applyTo(m_pDatabase, &pResultNode, pQE);
    return factory_node(pResultNode);
  }

  DatabaseNode *DatabaseNode::getOrCreateChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace) {
    return db()->factory_node(m_pNode->getOrCreateChildElement(pQE, sName, sContent, sNamespace));
  }

  IXmlBaseNode *DatabaseNode::getOrCreateTransientChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace, const char *sReason) {
    //note that anyone can use this who has the privileges to setTransient()
    //this function is generally programmatic usage by DatabaseNodeServerObjects
    IXmlBaseNode *pNewNode = 0;

    //look for existing node
    pNewNode = m_pNode->child(pQE, 0, node_type_element_only, sName, sNamespace);
    //create if not there
    if (!pNewNode) pNewNode = createTransientChildElement(pQE, sName, sContent, sNamespace, sReason);
    //ensure transient
    //this will be true if the parent node, m_pNode, is a @gs:transient-area
    if (!pNewNode->isTransient()) pNewNode->setTransient(pQE);

    return pNewNode;
  }

  IXmlBaseNode *DatabaseNode::createTransientChildElement(const IXmlQueryEnvironment *pQE, const char *sName, const char *sContent, const char *sNamespace, const char *sReason) {
    //note that anyone can use this who has the privileges to setTransient()
    //this function is generally programmatic usage by DatabaseNodeServerObjects
    return m_pNode->createChildElement(pQE, sName, sContent, sNamespace, CREATE_TRANSIENT, sReason);
  }

  void DatabaseNode::touch(const IXmlQueryEnvironment *pQE, const char *sReason) {
    TXml_TouchNode t(this, pQE, sReason, NO_TXML_BASENODE, m_pNode);
    t.applyTo(m_pDatabase, NO_RESULTS_NODE, pQE);
  }

  size_t DatabaseNode::childElementCount(const IXmlQueryEnvironment *pQE) const {
    return m_pNode->childElementCount(pQE);
  }

  const char *DatabaseNode::xmlID(const IXmlQueryEnvironment *pQE) const {
    return m_pNode->xmlID(pQE);
  }

  unsigned long DatabaseNode::documentOrder() const {
    return m_pNode->documentOrder();
  }

  bool DatabaseNode::in(const XmlNodeList<const IXmlBaseNode> *pVec, const bool bHardlinkAware) const {
    return m_pNode->in(pVec, bHardlinkAware);
  }

  const char *DatabaseNode::localName(const bool bDuplicate) const {
    return m_pNode->localName(bDuplicate);
  }

  bool DatabaseNode::removeAndDestroyNode(const IXmlQueryEnvironment *pQE, const char *sReason) {
    TXml_RemoveNode t(this, pQE, sReason, NO_TXML_BASENODE, m_pNode);
    t.applyTo(m_pDatabase, NO_RESULTS_NODE, pQE);
  }

  void DatabaseNode::inheritanceTransform(const char *sClassName,
    const IXmlQueryEnvironment *pQE,
    IXmlBaseNode *pOutputNode,
    const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet,
    const char *sWithMode, const IXmlBaseNode *pIncludeDirectory
  ) const {
    assert(sClassName);
    assert(pOutputNode);
    assert(pQE);

    const DatabaseNode *pStylesheetNode = 0;
    const IXslStylesheetNode *pStylesheetXSLNode = 0;

    UNWIND_EXCEPTION_BEGIN {
      if ( (pStylesheetNode    = getSingleNode(m_pDatabase->m_pIBQE_dbAdministrator, "~%s/inheritance_render", sClassName))
        && (pStylesheetXSLNode = pStylesheetNode->node_const()->queryInterface((IXslStylesheetNode*) 0))
      ) {
        transform(pStylesheetXSLNode, pQE, pOutputNode, pParamsInt, pParamsChar, pParamsNodeSet, sWithMode, pIncludeDirectory);
      } else throw NoStylesheet(this);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pStylesheetNode) delete pStylesheetNode;

    UNWIND_EXCEPTION_THROW;
  }

  const IXmlBaseNode *DatabaseNode::generateDynamicStylesheet(
    const IXmlQueryEnvironment *pQE
  ) const {
    //generate a DSXL (dynamic XSL) Stylesheet
    //note that the input-node == the stylesheet
    assert(pQE);

    const IXslStylesheetNode *pStylesheetXSLNode = 0;
    IXmlBaseNode       *pTmpNode                 = 0;

    //passing pQE through as the stylesheet means that the params will get re-evaluated?
    //global <xsl:param => repository:filesystempath-to-nodes($gs_dynamic_url) => here again
    IXmlQueryEnvironment *pNewQE = pQE->inherit();

    UNWIND_EXCEPTION_BEGIN {
      if (pTmpNode = db()->createTmpNode()) {
        if (pStylesheetXSLNode = m_pNode->queryInterface((const IXslStylesheetNode*) 0)) {
          //The stylesheet should include ~XSLStylesheet/name::code_render
          //in order to process the gs_code_render_dynamic
          
          transform(pStylesheetXSLNode, pNewQE, pTmpNode,
            NO_PARAMS_INT, NO_PARAMS_CHAR, NO_PARAMS_NODE,
            "gs_code_render_dynamic"
          );
        } else throw NoStylesheet(this);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pNewQE) delete pNewQE;
    
    UNWIND_EXCEPTION_THROW;
    
    return pTmpNode->firstChild(pQE);
  }

  DatabaseNode *DatabaseNode::copyNode(const IXmlQueryEnvironment *pQE, const bool bDeepClone, const bool bEvaluateValues, iXMLIDPolicy iXMLIDMovePolicy) const {
    return m_pDatabase->factory_node(m_pNode->copyNode(pQE, bDeepClone, bEvaluateValues, iXMLIDMovePolicy));
  }

  void DatabaseNode::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace) {
    m_pNode->setAttribute(pQE, sName, sValue, sNamespace);
  }

  void DatabaseNode::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const int iValue, const char *sNamespace) {
    m_pNode->setAttribute(pQE, sName, iValue, sNamespace);
  }

  void DatabaseNode::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const size_t uValue, const char *sNamespace) {
    m_pNode->setAttribute(pQE, sName, uValue, sNamespace);
  }

  void DatabaseNode::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue, const char *sNamespace) {
    m_pNode->setAttribute(pQE, sName, bValue, sNamespace);
  }
  
  percentage DatabaseNode::similarity( const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const {
    return m_pNode->similarity(pQE, pWithNode);
  }
  
  vector<TXml*> *DatabaseNode::differences(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pWithNode) const {
    return m_pNode->differences(pQE, pWithNode);
  }
  
  DatabaseNode *DatabaseNode::mergeNode(const IXmlQueryEnvironment *pQE, const DatabaseNode *pNewNode, const iXMLIDPolicy iCopyXMLIDPolicy) {
    //returns this!
    //mergeNode() in-place morphing
    TXml *pTXml;
    vector<TXml*> *pvTXmls  = 0;
    vector<TXml*>::iterator iTXml;
    
    UNWIND_EXCEPTION_BEGIN {
      pvTXmls = differences(pQE, pNewNode->m_pNode);
      for (iTXml = pvTXmls->begin(); iTXml != pvTXmls->end(); iTXml++) {
        pTXml = *iTXml;
        Debug::reportObject(pTXml);
        pTXml->applyTo(m_pDatabase, NO_RESULTS_NODE, pQE);
      }
    } UNWIND_EXCEPTION_END;
    
    //free up
    if (pvTXmls) vector_element_destroy(pvTXmls);
    
    UNWIND_EXCEPTION_THROW;

    return this;
  }
  
  size_t DatabaseNode::position() const {
    return m_pNode->position();
  }

  const char *DatabaseNode::toString() const {
    return m_pNode->toString();
  }
}
