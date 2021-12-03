//platform independent
#include "LibXmlArea.h"

#include "Utilities/container.c"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlMaskContext.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"
using namespace std;

namespace general_server {
  LibXmlArea::LibXmlArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pNodes, const IXmlQueryEnvironment *pQE, const IXmlNodeMask *pNodeMask): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner, pNodes), 
    m_nodes(this),
    m_pNodeMask(pNodeMask), 
    m_pvMaskNodes(0), 
    m_bNoSource(!pNodes) 
  {
    if (pNodes) m_nodes.insert(pNodes);

    //m_pNodeMask is optional
    if (m_pNodeMask) {
      //make calculations now for the relevant nodes
      //nodeMasks are flexible, can be schemas or xpath masks
      //ask the node mask to tell us the list of relevant nodes for this context node
      m_pvMaskNodes = m_pNodeMask->calculateFor(pQE, &m_nodes);
      IFDEBUG(if (m_pvMaskNodes->size() > 50) {
        Debug::report("big node mask! [%s] could slow things down", m_pvMaskNodes->size(), rtWarning);
        Debug::reportObject(m_pNodeMask);
      })
      IFDEBUG(if (m_pvMaskNodes->size() == 0) {
        Debug::report("no nodes in node mask!", rtWarning);
        Debug::reportObject(m_pNodeMask);
      })
    }
  }

  LibXmlArea::LibXmlArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, XmlNodeList<const IXmlBaseNode> *pDirectMaskNodes): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner, pDirectMaskNodes), 
    m_nodes(this),
    m_pNodeMask(0), 
    m_pvMaskNodes(pDirectMaskNodes), 
    m_bNoSource(false) 
  {}

  LibXmlArea::~LibXmlArea() {
    if (m_pvMaskNodes) vector_element_destroy(m_pvMaskNodes);
  }

  const char *LibXmlArea::toString() const {
    stringstream sOut;
    const char *sCount    = 0;

    //parts
    if (m_pvMaskNodes) sCount = itoa(m_pvMaskNodes->size());

    sOut << "mask [" << (m_pNodeMask ? m_pNodeMask->toString() : "no mask") << "] (" << (sCount ? sCount : "all") << ")\n";

    //clean up
    if (sCount)    MM_FREE(sCount);
    //if (sNodeMask) MM_FREE(sNodeMask); //server free

    return MM_STRDUP(sOut.str().c_str());
  }

  const char *LibXmlArea::toStringNodes() const {
    stringstream sOut;
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    const IXmlBaseNode *pNode      = 0;
    const char *sUniqueXPathToNode = 0;
    const char *sString            = 0;

    sString = toString();
    sOut << sString;

    //parts
    if (m_pvMaskNodes) {
      for (iNode = m_pvMaskNodes->begin(); iNode != m_pvMaskNodes->end(); iNode++) {
        pNode = *iNode;
        const XmlAdminQueryEnvironment ibqe(this, pNode->document());
        sUniqueXPathToNode  = pNode->uniqueXPathToNode(&ibqe, NO_BASE_NODE, INCLUDE_TRANSIENT);
        sOut << "  node [" << sUniqueXPathToNode << "]\n";
        MM_FREE(sUniqueXPathToNode);
      }
    }

    //free up
    if (sString) MM_FREE(sString);

    return MM_STRDUP(sOut.str().c_str());
  }

  bool LibXmlArea::contains(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNode) const {
    bool bRet = false;
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    const IXmlBaseNode *pMaskNode;
    bool bOldEnabled;

    if (m_pvMaskNodes) {
      switch (m_pNodeMask ? m_pNodeMask->maskType() : maskAutoIncludeAttributesAndText) {
        case maskAutoIncludeAllNonElements: 
          bRet = !pChildNode->isNodeElement(); 
          ATTRIBUTE_FALLTHROUGH; //failover if not successful
        case maskAutoIncludeAttributesAndText: 
          //default
          if (!bRet) bRet = pChildNode->isNodeText(); 
          ATTRIBUTE_FALLTHROUGH; //failover if not successful  
        case maskAutoIncludeAttributes: 
          if (!bRet) bRet = pChildNode->isNodeAttribute(); 
          ATTRIBUTE_FALLTHROUGH; //failover if not successful
        case maskExplicit: 
          //in() uses is() which is hardlink aware by default
          if (!bRet) bRet = pChildNode->in(m_pvMaskNodes, EXACT_MATCH);
          ATTRIBUTE_FALLTHROUGH; //failover if not successful
      }
      //if (true) cout << "mask nodes [" << m_pvMaskNodes->size() << "] check of [" << pChildNode->fullyQualifiedName() << "] = [" << bRet << "]\n";
    } else if (m_bNoSource) {
      bRet = true;
    } else {
      //no node mask: direct ancestor comparison
      //includes ALL TEXT and @attribute nodes
      //TODO: hardlinks?
      //temporarily disable the node-mask because it may cause the isAncestorOf() call to infinitely loop here again
      bOldEnabled = pQE->maskContext()->disable();
      for (iNode = m_nodes.begin(); iNode != m_nodes.end() && !bRet; iNode++) {
        pMaskNode = *iNode;
        if (pMaskNode->is(pChildNode, EXACT_MATCH) || pMaskNode->isAncestorOf(pQE, pChildNode)) bRet = true;
      }
      if (bOldEnabled) pQE->maskContext()->enable();
    }

    return bRet;
  }
}
