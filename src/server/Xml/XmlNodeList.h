//platform agnostic file
#ifndef _XMLNODELIST_H
#define _XMLNODELIST_H

//std library includes
#include <algorithm> //sort
#include <vector>
using namespace std;

//lots of these needed because of all-in-one template declerations
#include "IXml/IXmlNodeList.h" //direct inheritance
#include "IXml/IXmlBaseNode.h" //use in template function
#include "Exceptions.h" //use in template function
#include "Debug.h" //use in template function

namespace general_server {
  template<class IXmlNodeType> class XmlNodeList: implements_interface IXmlNodeList, virtual public MemoryLifetimeOwner, public vector<IXmlNodeType*> {
  public:
    IFDEBUG(const char *x() const;)
    
    XmlNodeList(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlNodeType *pInitialNode, const bool bIfNotNull = false);
    XmlNodeList(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);

    bool contains(IXmlNodeType *pNode, const bool bHardlinkAware = HARDLINK_AWARE) const;
    bool is(XmlNodeList<IXmlNodeType> *pNodeList, const bool bHardlinkAware) const; //uses IXmlBaseNode-is()
    const char *xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth = 0) const;
    IXmlNodeType *operator[](const size_t i) const;
    IXmlNodeType *first() const;
    static bool documentOrder(IXmlNodeType *pN1, IXmlNodeType *pN2);
    void removeDuplicates();
    typename XmlNodeList::iterator       find(IXmlNodeType *pNode, const bool bHardlinkAware);
    typename XmlNodeList::const_iterator find_const(IXmlNodeType *pNode, const bool bHardlinkAware) const;
    void sortDocumentOrderAscending();
    void sortDocumentOrderDescending();
    typename XmlNodeList<IXmlNodeType>::iterator push_back(IXmlNodeType *pNode);
    typename XmlNodeList<IXmlNodeType>::iterator push_back_unique(IXmlNodeType *pNode);
    void insert(const XmlNodeList<IXmlNodeType> *pNodes);
    const IXmlNodeList *element_destroy() const;
    void registerNodes();
    XmlNodeList<IXmlNodeType> *copyNodes(const IXmlQueryEnvironment *pQE, const bool bDeepClone = DEEP_CLONE, const bool bEvaluateValues = COPY_ONLY) const;
    XmlNodeList<IXmlNodeType> *clone_with_resources() const;

    const char *toString() const;
    const char *toStringNodes() const;
    const char *uniqueXPathToNodes(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = true, const bool bForceBaseNode = NO_FORCE_BASE_NODE, const bool bEnableIDs = false, const bool bEnableNameAxis = true, const bool bThrowIfNotBasePath = true) const;
  };

IFDEBUG(
  template<class IXmlNodeType> const char *XmlNodeList<IXmlNodeType>::x() const {
    //caller frees result
    const char *sXML;
    string stXML;
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;
    
    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      sXML   = (*iVec)->x();
      if (stXML.size()) stXML += "\n\n";
      stXML += sXML;
      MM_FREE(sXML);
    }
    
    return MM_STRDUP(stXML.c_str());
  }
)

  template<class IXmlNodeType> XmlNodeList<IXmlNodeType>::XmlNodeList(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlNodeType *pInitialNode, const bool bIfNotNull):
    MemoryLifetimeOwner(pMemoryLifetimeOwner)
  {
    if (!bIfNotNull || pInitialNode) push_back(pInitialNode);
  }

  template<class IXmlNodeType> XmlNodeList<IXmlNodeType>::XmlNodeList(const IMemoryLifetimeOwner *pMemoryLifetimeOwner):
    MemoryLifetimeOwner(pMemoryLifetimeOwner)
  {}

  template<class IXmlNodeType> const char *XmlNodeList<IXmlNodeType>::xml(const IXmlQueryEnvironment *pQE, const int iMaxDepth) const {
    stringstream sOut;
    IXmlNodeType *pVecNode;
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      pVecNode = *iVec;
      if (iVec != vector<IXmlNodeType*>::begin()) sOut << "\n";
      sOut << pVecNode->xml(pQE, iMaxDepth);
    }

    return MM_STRDUP(sOut.str().c_str());
  }
  
  template<class IXmlNodeType> IXmlNodeType *XmlNodeList<IXmlNodeType>::operator[](const size_t i) const {
    return vector<IXmlNodeType*>::operator[](i);
  }
  
  template<class IXmlNodeType> IXmlNodeType *XmlNodeList<IXmlNodeType>::first() const {
    return vector<IXmlNodeType*>::operator[](0);
  }
  
  template<class IXmlNodeType> XmlNodeList<IXmlNodeType> *XmlNodeList<IXmlNodeType>::clone_with_resources() const {
    //NOT an xmlNodeCopy()!!!
    //in fact doesn't copy any resources because LibXmlBaseNode is actually all pointers
    //IXmlBaseNode::clone_with_resources() => new LibXmlBaseNode(*this)
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;
    XmlNodeList<IXmlNodeType> *pNodes = new XmlNodeList<IXmlNodeType>(mmParent());
    pNodes->reserve(vector<IXmlNodeType*>::size());

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      //GDB: p *((LibXmlBaseNode*)(*iVec))->m_oNode
      try {
        //we dynamically queryInterfaceIXmlBaseNode() here
        //WITHOUT assigning to a variable
        //because we do not know the const of IXmlNodeType
        //queryInterfaceIXmlBaseNode() has const and non-const versions
        pNodes->push_back(
          (*iVec)->queryInterfaceIXmlBaseNode()
          ->clone_with_resources()
          ->queryInterface((IXmlNodeType*) 0)
        );
      } catch (NodeLockFailed ex) {
        Debug::report("NodeLockFailed in an XmlNodeList for [%s], not including node", (*iVec)->queryInterfaceIXmlBaseNode()->localName(NO_DUPLICATE));
      }
    }

    return pNodes;
  }
  
  template<class IXmlNodeType> void XmlNodeList<IXmlNodeType>::sortDocumentOrderAscending() {
    sort(vector<IXmlNodeType*>::begin(), 
         vector<IXmlNodeType*>::end(), 
         XmlNodeList<IXmlNodeType>::documentOrder
    );
  }
  
  template<class IXmlNodeType> void XmlNodeList<IXmlNodeType>::sortDocumentOrderDescending() {
    NOT_COMPLETE("");
  }


  template<class IXmlNodeType> XmlNodeList<IXmlNodeType> *XmlNodeList<IXmlNodeType>::copyNodes(const IXmlQueryEnvironment *pQE, const bool bDeepClone, const bool bEvaluateValues) const {
    //xmlNodeCopy()
    //we do not use copyChild(node-to-copy) because that copies and attaches to this node
    XmlNodeList<IXmlNodeType> *pNodes = new XmlNodeList<IXmlNodeType>(mmParent());
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;
    
    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      //we dynamically queryInterfaceIXmlBaseNode() here
      //WITHOUT assigning to a variable
      //because we do not know the const of IXmlNodeType
      //queryInterfaceIXmlBaseNode() has const and non-const versions
      pNodes->push_back(
        (*iVec)->queryInterfaceIXmlBaseNode()
          ->copyNode(pQE, bDeepClone, bEvaluateValues)
          ->queryInterface((IXmlNodeType*) 0)
      );
    }
    
    return pNodes;
  }

  template<class IXmlNodeType> void XmlNodeList<IXmlNodeType>::registerNodes() {
    typename XmlNodeList<IXmlNodeType>::iterator iVec;
    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++)
      (*iVec)->queryInterfaceIXmlBaseNode()->registerNode();
  }

  template<class IXmlNodeType> bool XmlNodeList<IXmlNodeType>::contains(IXmlNodeType *pNode, const bool bHardlinkAware) const {
    return pNode ? pNode->queryInterfaceIXmlBaseNode()->in(this, bHardlinkAware) : false;
  }

  template<class IXmlNodeType> bool XmlNodeList<IXmlNodeType>::is(XmlNodeList<IXmlNodeType> *pNodeList, const bool bHardlinkAware) const {
    //uses IXmlBaseNode->is()
    //returns 
    //  if input is NULL: false 
    //  2 empty sets:     true
    bool bMatch = true;
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec1, iVec2;
    
    if (pNodeList && vector<IXmlNodeType*>::size() == pNodeList->size()) {
      iVec1 = vector<IXmlNodeType*>::begin();
      iVec2 = pNodeList->begin();
      while (bMatch && iVec1 != vector<IXmlNodeType*>::end()) {
        if (!(*iVec1)->is(*iVec2, bHardlinkAware)) bMatch = false;
        iVec1++;
        iVec2++;
      }
    } else {
      bMatch = false;
    }
    
    return bMatch;
  }

  template<class IXmlNodeType> void XmlNodeList<IXmlNodeType>::removeDuplicates() {
    //hardlink aware: returns true if nodes are hardlinks of each other
    typename XmlNodeList<IXmlNodeType>::iterator iNode, iOther;
    IXmlNodeType *pNode;

    //reverse iteration because we are changing the subject
    //TODO: PERFORMANCE: move the last() element in to the replacement position
    //this will avoid shunting the whole array one place for each duplicate
    iNode = vector<IXmlNodeType*>::end();
    while (iNode != vector<IXmlNodeType*>::begin()) {
      //hardlink aware: returns true if nodes are hardlinks of each other
      iNode--;
      pNode = *iNode;
      iOther = find(pNode, EXACT_MATCH);
      if (iOther != iNode) vector<IXmlNodeType*>::erase(iNode);
    }
  }

  template<class IXmlNodeType> const IXmlNodeList *XmlNodeList<IXmlNodeType>::element_destroy() const {
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;
    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) delete *iVec;
    return this;
  }

  template<class IXmlNodeType> typename XmlNodeList<IXmlNodeType>::iterator XmlNodeList<IXmlNodeType>::find(IXmlNodeType *pNode, const bool bHardlinkAware) {
    //hardlink aware: returns true if nodes are hardlinks of each other
    //we need to use pNode->is(*iNode), not do a pointer compare
    typename XmlNodeList<IXmlNodeType>::iterator iVec;

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      if ((*iVec)->queryInterfaceIXmlBaseNode()->is(pNode->queryInterfaceIXmlBaseNode(), bHardlinkAware)) break;
    }
    return iVec;
  }

  template<class IXmlNodeType> bool XmlNodeList<IXmlNodeType>::documentOrder(IXmlNodeType *pN1, IXmlNodeType *pN2) {
    return pN1->queryInterfaceIXmlBaseNode()->documentOrder() < pN2->queryInterfaceIXmlBaseNode()->documentOrder();
  }

  template<class IXmlNodeType> typename XmlNodeList<IXmlNodeType>::const_iterator XmlNodeList<IXmlNodeType>::find_const(IXmlNodeType *pNode, const bool bHardlinkAware) const {
    //hardlink aware: returns true if nodes are hardlinks of each other
    //we need to use pNode->is(*iNode), not do a pointer compare
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      if ((*iVec)->queryInterfaceIXmlBaseNode()->is(pNode, bHardlinkAware)) break;
    }
    return iVec;
  }

  template<class IXmlNodeType> typename XmlNodeList<IXmlNodeType>::iterator XmlNodeList<IXmlNodeType>::push_back(IXmlNodeType *pNode) {
    if (!pNode) throw XmlNodeListCannotContainNULL(this);
    return vector<IXmlNodeType*>::insert(vector<IXmlNodeType*>::end(), pNode);
  }

  template<class IXmlNodeType> typename XmlNodeList<IXmlNodeType>::iterator XmlNodeList<IXmlNodeType>::push_back_unique(IXmlNodeType *pNode) {
    typename XmlNodeList<IXmlNodeType>::iterator iRet;
    
    if (!pNode) throw XmlNodeListCannotContainNULL(this);
    iRet = vector<IXmlNodeType*>::end();
    if (!contains(pNode)) iRet = vector<IXmlNodeType*>::insert(vector<IXmlNodeType*>::end(), pNode);
    return iRet;
  }

  template<class IXmlNodeType> void XmlNodeList<IXmlNodeType>::insert(const XmlNodeList<IXmlNodeType> *pNodes) {
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;
    for (iVec = pNodes->begin(); iVec != pNodes->end(); iVec++)
      if (!(*iVec)) throw XmlNodeListCannotContainNULL(this);
    vector<IXmlNodeType*>::insert(vector<IXmlNodeType*>::begin(), pNodes->begin(), pNodes->end());
  }

  template<class IXmlNodeType> const char *XmlNodeList<IXmlNodeType>::toString() const {
    stringstream sOut;
    IXmlNodeType *pVecNode;
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      pVecNode = *iVec;
      if (iVec != vector<IXmlNodeType*>::begin()) sOut << "\n";
      sOut << pVecNode->toString();
    }

    return MM_STRDUP(sOut.str().c_str());
  }

  template<class IXmlNodeType> const char *XmlNodeList<IXmlNodeType>::toStringNodes() const {
    return toString();
  }

  template<class IXmlNodeType> const char *XmlNodeList<IXmlNodeType>::uniqueXPathToNodes(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode, const bool bWarnIfTransient, const bool bForceBaseNode, const bool bEnableIDs, const bool bEnableNameAxis, const bool bThrowIfNotBasePath) const {
    //compile valid xpath OR statement
    //e.g. id('idx_13')|/object:Server/repository:databases|id('idx_14')
    stringstream sOut;
    IXmlNodeType *pVecNode;
    typename XmlNodeList<IXmlNodeType>::const_iterator iVec;

    for (iVec = vector<IXmlNodeType*>::begin(); iVec != vector<IXmlNodeType*>::end(); iVec++) {
      pVecNode = *iVec;
      if (iVec != vector<IXmlNodeType*>::begin()) sOut << '|';
        //we dynamically queryInterfaceIXmlBaseNode() here
        //WITHOUT assigning to a variable
        //because we do not know the const of IXmlNodeType
        //queryInterfaceIXmlBaseNode() has const and non-const versions
        sOut << (*iVec)->queryInterfaceIXmlBaseNode()
          ->uniqueXPathToNode(pQE, pBaseNode, bWarnIfTransient, bForceBaseNode, bEnableIDs, bEnableNameAxis, bThrowIfNotBasePath);
    }

    return MM_STRDUP(sOut.str().c_str());
  }
}

#endif
