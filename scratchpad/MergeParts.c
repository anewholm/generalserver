  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class IsolatedFirstBestLevelMerge: public XmlMergeStrategy {
    //compares only nodes on the same level as the master in the change node hierarchy
    //node similarity calculated on:
    //  namespace, local-name (required, otherwise zero)
    //  sub-hierarchy, properties
    percentage nodeSimilarity(const IXmlBaseNode *pOldNode, const IXmlBaseNode *pChangeOption) const;
    
  public:
    const XmlNodeList<const IXmlBaseNode> *potentialChangeNodeOptions(const IXmlBaseNode *pOldNode, const IXmlBaseNode *pChangePrevious) const;
    const IXmlBaseNode *bestFit(const IXmlBaseNode *pOldNode, const XmlNodeList<const IXmlBaseNode> *pChangeNodes, percentage& iSimilarity) const;
  };

  const XmlNodeList<const IXmlBaseNode> *IsolatedFirstBestLevelMerge::potentialChangeNodeOptions(const IXmlBaseNode *pOldNode, const IXmlBaseNode *pChangePrevious) const {
    
  }
  
  const IXmlBaseNode *IsolatedFirstBestLevelMerge::bestFit(const IXmlBaseNode *pOldNode, const XmlNodeList<const IXmlBaseNode> *pChangeNodes, percentage& iSimilarity) const {
    
  }
  
  percentage IsolatedFirstBestLevelMerge::nodeSimilarity(const IXmlBaseNode *pOldNode, const IXmlBaseNode *pChangeOption) const {
    
  }
  
  /*
  const IXmlBaseNode *IsolatedFirstBestLevelMerge::bestFit(const IXmlBaseNode *pMaster, XmlNodeList<const IXmlBaseNode> *pChangeOptions) const {
    //selected the first best only
    const IXmlBaseNode *pBestFit = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iChangeOption;
    percentage iMax = 0,
               iCurrent;
    for (iChangeOption = pChangeOptions->begin(); iChangeOption != pChangeOptions->end(); iChangeOption) {
      iCurrent = nodeSimilarity(pMaster, *iChangeOption);
      if (iCurrent > iMax) {
        pBestFit = *iChangeOption;
        iMax = iCurrent;
      }
    }
    return pBestFit;
  }
  
  percentage IsolatedFirstBestLevelMerge::nodeSimilarity(const IXmlBaseNode *pMaster, const IXmlBaseNode *pChange) const {
    size_t iCount = 0,
           iSimilar = 0;
    nodeSimilarityRecursive(pMaster, pChange, iSimilar); //passed by reference
    return (iSimilar / iCount);
  }
  
  int IsolatedFirstBestLevelMerge::nodeSimilarityRecursive(const IXmlBaseNode *pMaster, const IXmlBaseNode *pChange, size_t& iSimilar) const {
    size_t iCount = 1;
    
    //compare the nodes on this level
    
    if (nodeSimilarity()) iSimilar++;
    
    //recurse
    XmlNodeList<const IXmlBaseNode>::const_iterator iChangeOption;
    for (iChangeOption = pChangeOptions->begin(); iChangeOption != pChangeOptions->end(); iChangeOption) {
      iCount += nodeSimilarityRecursive(pMaster, pChange, iSimilar); //passed by reference
    }
    
    return iCount;
  }
  */
  
    IXmlMergeStrategy *pMS = pQE->mergeStrategy();
    percentage iSimilarity;
    
    //----------------------------------------------------- top node treated differently
    //instruction: replaceNodeCopy() = save THIS node
    //because we are saving THIS node, no matter what has changed
    //lower down nodes will get deleted if they have no similarity, and new ones copied
    //e.g. object:Group => interface:Menu will still be saved
    XmlNodeList<const IXmlBaseNode> vPotentialChangeNodeOptions(pNewNode);
    pMS->bestFit(this, &vPotentialChangeNodeOptions, iSimilarity);
    if (iSimilarity == 100) {
      //identical pNewNode hierarchy
      //do nothing
    } else {
      //pNewNode different
      //copy node from new
      syncFromNode(pQE, pNewNode);
    
      //direct change properties also
      ConstPropertiesIterator iPropertyFrom(pQE, pNewNode);
      for (PropertiesIterator iPropertyTo(pQE, this); iPropertyTo; iPropertyTo++) {
        replaceNodeCopyDescendent(pQE, *iPropertyTo, *iPropertyFrom, iXMLIDMovePolicy);
        iPropertyFrom++;
      }

      //descendent nodes and text are searched for, maintained, moved etc.
      //includes processing-nodes and DTDs
      ConstChildrenIterator iChildFrom(pQE, pNewNode, node_type_any);
      for (ChildrenIterator iChildTo(pQE, this); iChildTo; iChildTo++) {
        replaceNodeCopyDescendent(pQE, *iChildTo, *iChildFrom, iXMLIDMovePolicy);
        iChildFrom++;
      }
    }
      IXmlBaseNode *XmlBaseNode::replaceNodeCopyDescendent(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOldNode, const IXmlBaseNode *pNewNode, iXMLIDPolicy iXMLIDMovePolicy) {
    IXmlBaseNode *pBestFit;
    IXmlMergeStrategy *pMS = pQE->mergeStrategy();
    percentage iSimilarity;
    
    //----------------------------------------------------- top node treated differently
    //instruction: replaceNodeCopy() = save THIS node
    //because we are saving THIS node, no matter what has changed
    //lower down nodes will get deleted if they have no similarity, and new ones copied
    //e.g. object:Group => interface:Menu will still be saved
    const XmlNodeList<const IXmlBaseNode> *pPotentialChangeNodeOptions = pMS->potentialChangeNodeOptions(pQE, pOldNode);
    pBestFit = pMS->bestFit(this, pPotentialChangeNodeOptions, iSimilarity);
    
    if (iSimilarity == 100) {
      //identical pNewNode hierarchy
      //do nothing
    } else if (iSimilarity == 0)
      //copy the new node in
    } else {
      //pNewNode different
      //copy node from new
      pOldNode->syncFromNode(pQE, pNewNode);
    
      //direct change properties also
      ConstPropertiesIterator iPropertyFrom(pQE, pNewNode);
      for (PropertiesIterator iPropertyTo(pQE, this); iPropertyTo; iPropertyTo++) {
        replaceNodeCopyDescendent(pQE, *iPropertyTo, *iPropertyFrom, iXMLIDMovePolicy);
        iPropertyFrom++;
      }

      //descendent nodes and text are searched for, maintained, moved etc.
      //includes processing-nodes and DTDs
      ConstChildrenIterator iChildFrom(pQE, pNewNode, node_type_any);
      for (ChildrenIterator iChildTo(pQE, this); iChildTo; iChildTo++) {
        replaceNodeCopyDescendent(pQE, *iChildTo, *iChildFrom, iXMLIDMovePolicy);
        iChildFrom++;
      }
    }
  }