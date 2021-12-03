//platform independent
#include "XmlMergeStrategy.h"

namespace general_server {
  XmlMergeStrategy::XmlMergeStrategy( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode, const mergeFlags iMergeFlags):
    m_iNewNode(pNewNode),
    m_iExistingNode(pExistingNode),
    m_iMergeFlags(iMergeFlags),
    m_iNextTraversal(descendToChildren)
  {
    assert(m_pNewNode);
    assert(m_pExistingNode);
  }
  
  void XmlMergeStrategy::syncLocalName( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const {}
  void XmlMergeStrategy::syncNamespace( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const {}
  void XmlMergeStrategy::syncProperties(const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const {}
  void XmlMergeStrategy::syncContent(   const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const {}
  
  unsigned int XmlMergeStrategy::nodeSimilarity(const IXmlBaseNode *pExistingNode, unsigned int &iCount100) const {}
  unsigned int XmlMergeStrategy::nodeSimilarity_recursive(const IXmlBaseNode *pNewNodeDescendent, const IXmlBaseNode *pExistingNodeDescendent, unsigned int &iCount100) const {}
  unsigned int XmlMergeStrategy::propertySimilarity(const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const {}
  bool         XmlMergeStrategy::testLocalName( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const {}
  bool         XmlMergeStrategy::testNamespace( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const {}
  bool         XmlMergeStrategy::testContent(   const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const {}
  
  const XmlNodeList<const IXmlBaseNode> *XmlMergeStrategy::potentialChangeNodeOptions(const char *sXPath) const {}
  
  void XmlMergeStrategy::begin(const IXmlBaseNode *pExisting = NULL, const IXmlBaseNode *pNew = NULL) {
    if (pExisting) m_pExisting = pExisting;
    if (pNew)      m_pNew      = pNew;
  }
  
  void XmlMergeStrategy::operator++() {
    m_iNewNode++;
    m_iExistingNode++;
  }
  
  bool XmlMergeStrategy::bool() const {return pNew == NULL;}

  mergeAction XmlMergeStrategy::merge() {
    for (begin(); *this; this->operator++()) mergeNode();
  }
  
  void XmlMergeStrategy::mergeNode() {
    IXmlBaseNode *pBestFit = 0;
    mergeAction iMergeAction = bestFit(&pBestFit);
    
    //TODO: change these to function pointers?
    switch (iMergeAction) {
      case moveExistingAndSkip: {
        //full match    on an EXISTING node: move it into position and skip to next NEW
        if (!pBestFit->is(m_pExisting)) moveFitToExisting();
        m_iNewNode.skipThisBranch();
        m_iExistingNode.skipThisBranch();
        break;
      }
      case syncToMovedExistingAndDescend: {
        //partial match on an EXISTING node: move it into position and merge NEW sub-hierarchy
        if (!pBestFit->is(m_pExisting)) moveFitToExisting();
        break;
      }
      case copyBeforeExisting: {
        //no match      on an EXISTING node: so copy the NEW node
        copyNewToBeforeExisting();
        m_iNewNode.skipThisBranch();
        m_iExistingNode.skipThisBranch();
        break;
      }
    }
  }
}
