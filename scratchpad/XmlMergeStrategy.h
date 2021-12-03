//platform agnostic file
#ifndef _XMLMERGESTRATEGY_H
#define _XMLMERGESTRATEGY_H

#include "define.h" //includes platform define also

#include "IXmlMergeStrategy.h" //direct inheritance
#include "Iterators.h"         //member variable

namespace general_server {
  class XmlMergeStrategy: public IXmlMergeStrategy {
    ConstDescendantIterator m_iNewNode;
    DescendantIterator      m_iExistingNode;
    const mergeFlags m_iMergeFlags;
    
  protected:
    virtual void syncLocalName( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const;
    virtual void syncNamespace( const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const;
    virtual void syncProperties(const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const;
    virtual void syncContent(   const IXmlBaseNode *pNewNode, IXmlBaseNode *pExistingNode) const;
    
    virtual unsigned int nodeSimilarity(const IXmlBaseNode *pExistingNode, unsigned int &iCount100) const;
    virtual unsigned int nodeSimilarity_recursive(const IXmlBaseNode *pNewNodeDescendent, const IXmlBaseNode *pExistingNodeDescendent, unsigned int &iCount100) const;
    virtual unsigned int propertySimilarity(const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const;
    virtual bool         testLocalName( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const;
    virtual bool         testNamespace( const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const;
    virtual bool         testContent(   const IXmlBaseNode *pNewNode, const IXmlBaseNode *pExistingNode) const;
    
    virtual const XmlNodeList<const IXmlBaseNode> *potentialChangeNodeOptions(const char *sXPath) const;
    
    virtual void init(const IXmlBaseNode *pExisting, const IXmlBaseNode *pNew, const mergeFlags iMergeFlags = 0);
    virtual void begin();
    virtual void operator++();
    virtual void end() const;

  public:
    virtual mergeAction merge();
    virtual mergeAction mergeNode();
  };
}

#endif