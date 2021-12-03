//platform specific file (UNIX)
#ifndef _LIBXMLNODEMASK_H
#define _LIBXMLNODEMASK_H

#include "define.h"

#include "IXml/IXmlNodeMask.h"   //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance

#include <vector>
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlQueryEnvironment;

  //TODO: move LibXmlNodeMask to Xml layer
  class LibXmlNodeMask: public IXmlNodeMask, public MemoryLifetimeOwner {
    //NOTE: only xpath node-mask supported at the moment
    const char *m_sNodeMask; //xpath
    iMaskType m_iMaskType;

  public:
    LibXmlNodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const iMaskType iMaskTyper = maskAutoIncludeAttributesAndText);
    LibXmlNodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const char *sNodeMaskType);
    ~LibXmlNodeMask();

    XmlNodeList<const IXmlBaseNode> *calculateFor(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode) const;
    XmlNodeList<const IXmlBaseNode> *calculateFor(const IXmlQueryEnvironment *pQE, const XmlNodeList<const IXmlBaseNode> *pNodes) const;

    iMaskType maskType() const {return m_iMaskType;}
    void maskType(iMaskType iMaskTyper) {m_iMaskType = iMaskTyper;}

    const char *toString() const;
  };
}

#endif
