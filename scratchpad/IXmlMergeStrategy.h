//platform agnostic file
#ifndef _IXMLMERGESTRATEGY_H
#define _IXMLMERGESTRATEGY_H

#include "define.h" //percentage

#include "XmlNodeList.h"


namespace general_server {
  class IXmlBaseNode;
  
  class IXmlMergeStrategy {
    //NEW node hierarchy replaces => EXISTING node hierarchy where different
    //NEW hierarchy is traversed looking for best matches in the EXISTING
    
  protected:
    enum mergeAction {
      moveExistingAndSkip,           //full match    on an EXISTING node: move it into position and skip to next NEW
      syncToMovedExistingAndDescend, //partial match on an EXISTING node: move it into position and merge NEW sub-hierarchy
      copyBeforeExisting             //no match      on an EXISTING node: so copy the NEW node
    };
    
    enum mergeFlags {
      alwaysSyncTopNode = 1 //sometimes, if the NEW node is very different, e.g. namespace, the strategy can replace the EXISTING, instead of syncing it in-place
    };
    
    //recieves all the percentage fits for potential nodes and decides what to do
    virtual const IXmlBaseNode *bestFit(const XmlNodeList<const IXmlBaseNode> *pNewNodes, percentage& iSimilarity) const = 0;
    virtual void syncLocalName( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const = 0;
    virtual void syncNamespace( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const = 0;
    virtual void syncProperties(const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const = 0;
    virtual void syncContent(   const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const = 0;
    
    //works out similarity between 2 single nodes, can take into account the hierarchy
    //should use bestFit() when calculating similarity
    //final score is the (returned iSimilarity * 100 / iCount100)%
    virtual unsigned int nodeSimilarity(const IXmlBaseNode *pExistingNode, unsigned int &iCount100) const = 0;
    virtual unsigned int nodeSimilarity_recursive(const IXmlBaseNode *pNewNodeDescendent, const IXmlBaseNode *pExistingNodeDescendent, unsigned int &iCount100) const = 0;
    virtual unsigned int propertySimilarity(const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const = 0;
    virtual bool         testLocalName( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const = 0;
    virtual bool         testNamespace( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const = 0;
    virtual bool         testContent(   const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const = 0;
    
    //all EXISTING nodes to be considered as potential replacements for the NEW node
    //e.g. self::*|following-sibling::*
    virtual const char *potentialChangeNodeOptionsXPath() = 0;
    //run the xpath on the current EXISTING node
    virtual const XmlNodeList<const IXmlBaseNode> *potentialChangeNodeOptions(const char *sXPath) const = 0;
    
    //iteration over NEW primarily and EXISTING following
    virtual void init(const IXmlBaseNode *pExisting, const IXmlBaseNode *pNew, const mergeFlags iMergeFlags = 0) = 0;
    virtual void begin() = 0;
    virtual void operator++() = 0;
    virtual void end() const = 0;

  public:
    //actions
    //merge() usually just iterates with a sync()
    virtual mergeAction merge() = 0;
    //sync() usually will just ask for the bestFit() and react to the 
    virtual mergeAction mergeNode() = 0;
  };
}

#endif