//platform specific file (UNIX)
#ifndef _LIBXMLAREA_H
#define _LIBXMLAREA_H

#include "define.h"
#include "IXml/IXmlArea.h"    //direct inheritance
#include "Xml/XmlNodeList.h" //property

namespace general_server {
  interface_class IXmlQueryEnvironment;

  class LibXmlArea: public IXmlArea, virtual public MemoryLifetimeOwner {
    //TODO: move all this to XmlArea
    XmlNodeList<const IXmlBaseNode> m_nodes;        //the source node(s) to which the xpath m_pNodeMask is applied
    const IXmlNodeMask *m_pNodeMask;                //the source node mask that contains the flexible xpath mask idea
    XmlNodeList<const IXmlBaseNode> *m_pvMaskNodes; //holds the resolved nodes in the area
    const bool m_bNoSource;                         //indicates a zero pointer for the node list: thus no mask

    friend class LibXmlLibrary; //only LibXmlLibrary can instanciate LibXmlArea
    LibXmlArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pSourceNodes, const IXmlQueryEnvironment *pQE, const IXmlNodeMask *pNodeMask);
    LibXmlArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, XmlNodeList<const IXmlBaseNode> *pDirectMaskNodes); //set the result directly
    ~LibXmlArea();

  public:
    const char *toString() const;
    const char *toStringNodes() const;
    bool contains(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNode) const;
  };
}

#endif
