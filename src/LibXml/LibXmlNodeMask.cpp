//platform independent
#include "LibXmlNodeMask.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlQueryEnvironment.h"
#include "IXml/IXmlMaskContext.h"
#include "IXml/IXslTransformContext.h"

using namespace std;

namespace general_server {
  LibXmlNodeMask::LibXmlNodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const iMaskType iMaskTyper): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sNodeMask(sNodeMask ? MM_STRDUP(sNodeMask) : 0),
    m_iMaskType(iMaskTyper)
  {}

  LibXmlNodeMask::LibXmlNodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const char *sNodeMaskType):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sNodeMask(sNodeMask ? MM_STRDUP(sNodeMask) : 0),
    m_iMaskType(maskAutoIncludeAttributesAndText)
  {
    if (sNodeMaskType && *sNodeMaskType) {
      if      (_STREQUAL(sNodeMaskType, "auto-include-attributes-and-text")) m_iMaskType = maskAutoIncludeAttributesAndText;
      else if (_STREQUAL(sNodeMaskType, "auto-include-all-non-elements"))    m_iMaskType = maskAutoIncludeAllNonElements;
      else if (_STREQUAL(sNodeMaskType, "auto-include-attributes"))          m_iMaskType = maskAutoIncludeAttributes;
      else if (_STREQUAL(sNodeMaskType, "explicit"))                         m_iMaskType = maskExplicit;
      else throw NodeMaskTypeNotRecognised(this, sNodeMaskType);
    }
  }
  
  LibXmlNodeMask::~LibXmlNodeMask() {
    if (m_sNodeMask) MM_FREE(m_sNodeMask);
  }
  
  XmlNodeList<const IXmlBaseNode> *LibXmlNodeMask::calculateFor(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode) const {
    XmlNodeList<const IXmlBaseNode> *pvNodes = 0;
    bool bOldEnabled = false;

    //this read process can fire triggers always use the correct user QE
    //with the correct normal security
    //disable the current node-mask and hardlink suppression policies whilst calculating the new
    if (m_sNodeMask && *m_sNodeMask) {
      if (pQE->maskContext()) bOldEnabled = pQE->maskContext()->disable();
      pvNodes = pNode->getMultipleNodes(pQE, m_sNodeMask);
      if (bOldEnabled && pQE->maskContext()) pQE->maskContext()->enable();
    }

    return pvNodes;
  }

  XmlNodeList<const IXmlBaseNode> *LibXmlNodeMask::calculateFor(const IXmlQueryEnvironment *pQE, const XmlNodeList<const IXmlBaseNode> *pNodes) const {
    XmlNodeList<const IXmlBaseNode> *pvNodes = new XmlNodeList<const IXmlBaseNode>(pNodes->mmParent());
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    bool bOldEnabled = false;

    //this read process can fire triggers
    //always use the correct user QE with the correct normal security
    //disable the current node-mask and hardlink suppression policies whilst calculating the new
    if (m_sNodeMask && *m_sNodeMask) {
      if (pQE->maskContext()) bOldEnabled = pQE->maskContext()->disable();
      for (iNode = pNodes->begin(); iNode != pNodes->end(); iNode++) {
        //NOTE: could REGISTER these in case a node is deleted from the node-mask during the transform
        pvNodes->insert((*iNode)->getMultipleNodes(pQE, m_sNodeMask));
      }
      if (bOldEnabled && pQE->maskContext()) pQE->maskContext()->enable();
    }

    return pvNodes;
  }

  const char *LibXmlNodeMask::toString() const {
    //server free
    return (m_sNodeMask ? m_sNodeMask : "<no mask>");
  }
}
