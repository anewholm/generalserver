//platform agnostic file
#ifndef _XMLNODEMAP_H
#define _XMLNODEMAP_H

//std library includes
#include <map>
using namespace std;

//lots of these needed because of all-in-one template declerations
#include "IXml/IXmlNodeList.h" //direct inheritance
#include "IXml/IXmlBaseNode.h" //direct inheritance

namespace general_server {
  template<class T> class XmlNodeMap: public map<const void*, T> {
  public:
    typename XmlNodeMap<T>::iterator       find(const IXmlBaseNode *pNode);
    typename XmlNodeMap<T>::const_iterator find_const(const IXmlBaseNode *pNode) const;
    typename XmlNodeMap<T>::iterator insert(const IXmlBaseNode *pNode, T obj);
  };

  template<class T> typename XmlNodeMap<T>::iterator XmlNodeMap<T>::find(const IXmlBaseNode *pNode) {
    return map<const void*, T>::find(pNode->persistentUniqueMemoryLocationID());
  }

  template<class T> typename XmlNodeMap<T>::const_iterator XmlNodeMap<T>::find_const(const IXmlBaseNode *pNode) const {
    return map<const void*, T>::find_const(pNode->persistentUniqueMemoryLocationID());
  }

  template<class T> typename XmlNodeMap<T>::iterator XmlNodeMap<T>::insert(const IXmlBaseNode *pNode, T obj) {
    pair<typename XmlNodeMap<T>::iterator, bool> pInserted = map<const void*, T>::insert(pair<const void*, T>(pNode->persistentUniqueMemoryLocationID(), obj));
    return pInserted.first;
  }
}

#endif
