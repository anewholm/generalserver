//platform agnostic
#include "RepositorySaveSecurityContext.h"

#include "IXml/IXmlBaseNode.h"
#include "Exceptions.h"

namespace general_server {
  RepositorySaveSecurityContext::RepositorySaveSecurityContext(): m_pOwnerQE(0) {}
  RepositorySaveSecurityContext::RepositorySaveSecurityContext(const RepositorySaveSecurityContext& sec):
    m_pOwnerQE(sec.m_pOwnerQE)
  {}
  
  IXmlQueryEnvironment::accessResult RepositorySaveSecurityContext::checkSecurity(const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) const {
    //usually during xmlsave(...)
    const IXmlNamespaced *pCurType = pCur->queryInterface((IXmlNamespaced*) 0);
    
    //saving should never involve writing
    if (iAccessOperation != IXmlQueryEnvironment::OP_READ) throw NodeWriteableAccessDenied(m_pOwnerQE, MM_STRDUP("during RepositorySaveSecurityContext"));
    
    return (
            //exclusions
            (pCur->isNodeAttribute() && pCurType->isNamespace(NAMESPACE_REPOSITORY))      //@repository:*
         || (pCur->isNodeNamespaceDecleration() && pCur->isValue(NAMESPACE_REPOSITORY))   //@xmlns:repository
         || (pCur->isNodeAttribute() && pCur->isName(NAMESPACE_OBJECT, "cpp-component"))  //@object:cpp-component
         || (pCur->isTransient())                                                         //@gs:transient
      ? IXmlQueryEnvironment::RESULT_DENY
      : IXmlQueryEnvironment::RESULT_INCLUDE
    );
  }
}
