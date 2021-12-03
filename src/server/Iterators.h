//platform agnostic file
#ifndef _ITERATORS_H
#define _ITERATORS_H

#include "IXml/IXmlBaseNode.h" //iDOMNodeTypeRequest

#include <cstring> //size_t

using namespace std;

/* example usage

for (ChildrenIterator iChildNode(pQE, pNode); iChildNode; iChildNode++) {
  IXmlBaseNode  *pChildNode = *iChildNode
}

*/

namespace general_server {
  class IXmlQueryEnvironment;
  class IXmlBaseNode;
  
  /* ------------------------------------------------------------------------ */
  class DOMIterator {
  protected:
    const IXmlQueryEnvironment *m_pQE;
    IXmlBaseNode *m_pOriginalNode;
    IXmlBaseNode *m_pCurrentNode;
    const iDOMNodeTypeRequest m_iNodeTypes;
    
    virtual IXmlBaseNode *setNewNode(IXmlBaseNode *pNewNode);
    virtual IXmlBaseNode *startNode() const;
    const IXmlLibrary *xmlLibrary() const;

    virtual const char *forwardAxis() = 0;
    virtual const char *reverseAxis() = 0;
    
    DOMIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
    DOMIterator(const DOMIterator& iFrom);
    virtual ~DOMIterator();
    
  public:
    virtual operator bool() const;
    virtual IXmlBaseNode *operator*() const;
    virtual void operator++(int);
    virtual void operator--(int);
  };
  
  /* ------------------------------------------------------------------------ */
  class ConstDOMIterator {
  protected:
    const IXmlQueryEnvironment *m_pQE;
    const IXmlBaseNode *m_pOriginalNode;
    const IXmlBaseNode *m_pCurrentNode;
    const iDOMNodeTypeRequest m_iNodeTypes;
    
    virtual const IXmlBaseNode *setNewNode(const IXmlBaseNode *pNewNode);
    virtual const IXmlBaseNode *startNode() const;
    
    virtual const char *forwardAxis() = 0;
    virtual const char *reverseAxis() = 0;
    
    ConstDOMIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
    ConstDOMIterator(const ConstDOMIterator& iFrom);
    virtual ~ConstDOMIterator();
    
  public:
    virtual operator bool() const;
    virtual const IXmlBaseNode *operator*() const;
    virtual void operator++(int);
    virtual void operator--(int);
    virtual void operator=(const ConstDOMIterator& iFrom);
  };

  /* ------------------------------------------------------------------------ */
  class FollowingSiblingIterator: public DOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
  public:
    FollowingSiblingIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
  };

  class ChildrenIterator: public DOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    IXmlBaseNode *startNode() const;
  public:
    ChildrenIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
  };

  class ConstChildrenIterator: public ConstDOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    const IXmlBaseNode *startNode() const;
  public:
    ConstChildrenIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
  };

  class DescendantIterator: public DOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    IXmlBaseNode *startNode() const;
  public:
    DescendantIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
  };

  class ConstDescendantIterator: public ConstDOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    const IXmlBaseNode *startNode() const;
  public:
    ConstDescendantIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only);
  };

  class PropertiesIterator: public DOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    IXmlBaseNode *startNode() const;
  public:
    PropertiesIterator(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCurrentNode);
  };

  class ConstPropertiesIterator: public ConstDOMIterator {
    static const char *m_sForwardAxis;
    static const char *m_sReverseAxis;
  protected:
    const char *forwardAxis() {return m_sForwardAxis;}
    const char *reverseAxis() {return m_sReverseAxis;}
    const IXmlBaseNode *startNode() const;
  public:
    ConstPropertiesIterator(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode);
  };
}

#endif
