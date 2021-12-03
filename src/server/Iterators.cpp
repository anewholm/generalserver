//platform agnostic file
#include "Iterators.h"

#include "IXml/IXmlQueryEnvironment.h"
#include "IXml/IXmlBaseNode.h"

namespace general_server {
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  DOMIterator::DOMIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes):
    m_pQE(pQE),
    m_pOriginalNode(pOriginalNode),
    m_pCurrentNode(startNode()),
    m_iNodeTypes(iNodeTypes)
  {
    assert(m_pQE);
    assert(m_pOriginalNode);
    assert(m_pCurrentNode);
  }

  DOMIterator::DOMIterator(const DOMIterator& iFrom):
    m_pQE(iFrom.m_pQE),
    m_pOriginalNode(iFrom.m_pOriginalNode),
    m_pCurrentNode(iFrom.m_pCurrentNode->clone_with_resources()),
    m_iNodeTypes(iFrom.m_iNodeTypes)
  {}

  const IXmlLibrary *DOMIterator::xmlLibrary() const {return m_pQE->xmlLibrary();}
  
  DOMIterator::~DOMIterator() {
    if (m_pCurrentNode) delete m_pCurrentNode;
  }

  DOMIterator::operator bool() const {
    return m_pCurrentNode;
  }

  IXmlBaseNode *DOMIterator::operator*() const {
    return m_pCurrentNode;
  }
  
  void DOMIterator::operator++(int) {
    if (m_pCurrentNode) setNewNode(m_pCurrentNode->getSingleNode(m_pQE, "%s::%s[1]", forwardAxis())); //, xmlLibrary()->nodeTypeXPath(m_iNodeTypes)));
    //TODO: else throw ...
  }

  void DOMIterator::operator--(int) {
    if (m_pCurrentNode) setNewNode(m_pCurrentNode->getSingleNode(m_pQE, "%s::%s[1]", reverseAxis())); //, xmlLibrary()->nodeTypeXPath(m_iNodeTypes)));
    //TODO: else throw ...
  }
  
  IXmlBaseNode *DOMIterator::setNewNode(IXmlBaseNode *pNewNode) {
    if (m_pCurrentNode) delete m_pCurrentNode;
    m_pCurrentNode = pNewNode;
    return m_pCurrentNode;
  }
 
  IXmlBaseNode *DOMIterator::startNode() const {
    return m_pOriginalNode->clone_with_resources();
  }
  
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  ConstDOMIterator::ConstDOMIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes):
    m_pQE(pQE),
    m_pOriginalNode(pOriginalNode),
    m_pCurrentNode(startNode()),
    m_iNodeTypes(iNodeTypes)
  {
    assert(m_pQE);
    assert(m_pOriginalNode);
    assert(m_pCurrentNode);
  }

  ConstDOMIterator::ConstDOMIterator(const ConstDOMIterator& iFrom):
    m_pQE(iFrom.m_pQE),
    m_pOriginalNode(iFrom.m_pOriginalNode),
    m_pCurrentNode(iFrom.m_pCurrentNode->clone_with_resources()),
    m_iNodeTypes(iFrom.m_iNodeTypes)
  {}
  
  ConstDOMIterator::~ConstDOMIterator() {
    if (m_pCurrentNode) delete m_pCurrentNode;
  }

  void ConstDOMIterator::operator=(const ConstDOMIterator& iFrom) {
    if (m_pCurrentNode) delete m_pCurrentNode;
    m_pQE           = iFrom.m_pQE;
    m_pOriginalNode = iFrom.m_pOriginalNode;
    m_pCurrentNode  = iFrom.m_pCurrentNode->clone_with_resources();
  }

  ConstDOMIterator::operator bool() const {
    return m_pCurrentNode;
  }

  const IXmlBaseNode *ConstDOMIterator::operator*() const {
    return m_pCurrentNode;
  }
  
  void ConstDOMIterator::operator++(int) {
    if (m_pCurrentNode) setNewNode(m_pCurrentNode->getSingleNode(m_pQE, "%s::*[1]", forwardAxis()));
    //TODO: else throw ...
  }

  void ConstDOMIterator::operator--(int) {
    if (m_pCurrentNode) setNewNode(m_pCurrentNode->getSingleNode(m_pQE, "%s::*[1]", reverseAxis()));
    //TODO: else throw ...
  }
  
  const IXmlBaseNode *ConstDOMIterator::setNewNode(const IXmlBaseNode *pNewNode) {
    if (m_pCurrentNode) delete m_pCurrentNode;
    m_pCurrentNode = pNewNode;
    return m_pCurrentNode;
  }
 
  const IXmlBaseNode *ConstDOMIterator::startNode() const {
    return m_pOriginalNode->clone_with_resources();
  }
  
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ */
  const char *FollowingSiblingIterator::m_sForwardAxis = "following-sibling";
  const char *FollowingSiblingIterator::m_sReverseAxis = "preceeding-sibling";
  FollowingSiblingIterator::FollowingSiblingIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes): DOMIterator(pQE, pOriginalNode, iNodeTypes) {}

  const char *ChildrenIterator::m_sForwardAxis = "following-sibling";
  const char *ChildrenIterator::m_sReverseAxis = "preceeding-sibling";
  ChildrenIterator::ChildrenIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes): DOMIterator(pQE, pOriginalNode, iNodeTypes) {}
  IXmlBaseNode *ChildrenIterator::startNode() const {return m_pOriginalNode->firstChild(m_pQE, m_iNodeTypes);}

  const char *ConstChildrenIterator::m_sForwardAxis = "following-sibling";
  const char *ConstChildrenIterator::m_sReverseAxis = "preceeding-sibling";
  ConstChildrenIterator::ConstChildrenIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes): ConstDOMIterator(pQE, pOriginalNode, iNodeTypes) {}
  const IXmlBaseNode *ConstChildrenIterator::startNode() const {return m_pOriginalNode->firstChild(m_pQE, m_iNodeTypes);}

  const char *DescendantIterator::m_sForwardAxis = "descendant";
  const char *DescendantIterator::m_sReverseAxis = "ancestor";
  DescendantIterator::DescendantIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes): DOMIterator(pQE, pOriginalNode, iNodeTypes) {}
  IXmlBaseNode *DescendantIterator::startNode() const {return m_pOriginalNode->firstChild(m_pQE, m_iNodeTypes);}

  const char *ConstDescendantIterator::m_sForwardAxis = "descendant";
  const char *ConstDescendantIterator::m_sReverseAxis = "ancestor";
  ConstDescendantIterator::ConstDescendantIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pOriginalNode, const iDOMNodeTypeRequest iNodeTypes): ConstDOMIterator(pQE, pOriginalNode, iNodeTypes) {}
  const IXmlBaseNode *ConstDescendantIterator::startNode() const {return m_pOriginalNode->firstChild(m_pQE, m_iNodeTypes);}

  const char *PropertiesIterator::m_sForwardAxis = "following-sibling";
  const char *PropertiesIterator::m_sReverseAxis = "preceeding-sibling";
  PropertiesIterator::PropertiesIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOriginalNode): DOMIterator(pQE, pOriginalNode, node_type_any) {}
  IXmlBaseNode *PropertiesIterator::startNode() const {return m_pOriginalNode->getSingleNode(m_pQE, "@*[1]");}

  const char *ConstPropertiesIterator::m_sForwardAxis = "following-sibling";
  const char *ConstPropertiesIterator::m_sReverseAxis = "preceeding-sibling";
  ConstPropertiesIterator::ConstPropertiesIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pOriginalNode): ConstDOMIterator(pQE, pOriginalNode, node_type_any) {}
  const IXmlBaseNode *ConstPropertiesIterator::startNode() const {return m_pOriginalNode->getSingleNode(m_pQE, "@*[1]");}
}
 
