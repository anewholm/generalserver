//platform agnostic
#include "MaskContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslDoc.h"
#include "Database.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include <pthread.h>
#include <algorithm>

#include "Utilities/container.c"

namespace general_server {
  MaskContext::MaskContext(): m_bEnabled(true), m_pOwnerQE(0) {}

  MaskContext::MaskContext(const MaskContext& mc):
    vector<const IXmlArea*>(mc), //base copy constructors are manual
    m_bEnabled(mc.m_bEnabled),
    m_pOwnerQE(mc.m_pOwnerQE)
  {}

  void MaskContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}

  IXmlMaskContext *MaskContext::inherit() const {
    //just use the copy constructor to make a complete copy
    return clone_with_resources();
  }
  
  IXmlMaskContext *MaskContext::clone_with_resources() const {
    //use the copy constructor to make a complete copy
    return new MaskContext(*this);
  }

  IXmlQueryEnvironment::accessResult MaskContext::applyCurrentAreaMask(const IXmlBaseNode *pNode) const {
    IXmlQueryEnvironment::accessResult ret = IXmlQueryEnvironment::RESULT_INCLUDE;
    const IXmlArea *pArea;

    if (m_bEnabled) {
      pArea = (size() ? back() : 0);
      if (pArea) {
        //contains will apply the mask type
        //i.e. auto-inclusion of @attributes, TEXT, etc.
        if (!pArea->contains(m_pOwnerQE, pNode)) {
          ret = IXmlQueryEnvironment::RESULT_DENY;
          IFDEBUG(
            //if (m_pTransformContext && m_pTransformContext->traceFlags()) Debug::report("node-mask blocked a node [%s]", pNode->localName(NO_DUPLICATE));
          );
        }
      }
    }

    return ret;
  }
}
