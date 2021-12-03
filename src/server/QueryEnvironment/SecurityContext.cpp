//platform agnostic
#include "SecurityContext.h"

#include "IXml/IXmlBaseNode.h"
#include "SecurityStrategy.h"
#include "Exceptions.h"

namespace general_server {
  //---------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------- SecurityContext
  //---------------------------------------------------------------------------------------
  SecurityContext::SecurityContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sUserName(0),
    m_sMasqueradeUserId(0),
    m_pOwnerQE(0),
    m_bWriteable(true)
  {}

  SecurityContext::SecurityContext(const SecurityContext& sec):
    MemoryLifetimeOwner(sec.mmParent()), //addChild()
    m_sUserName(sec.m_sUserName ? MM_STRDUP(sec.m_sUserName) : 0),
    m_sMasqueradeUserId(sec.m_sMasqueradeUserId ? MM_STRDUP(sec.m_sMasqueradeUserId) : 0),
    m_groupids(sec.m_groupids),
    m_pOwnerQE(0),
    m_bWriteable(sec.m_bWriteable)
  {}

  SecurityContext::~SecurityContext() {
    //iUserId 0 is anonymous, but the user does not necessarily exist in the userStore
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    if (m_sUserName)         MMO_FREE(m_sUserName);
    if (m_sMasqueradeUserId) MMO_FREE(m_sMasqueradeUserId);
  }

  void SecurityContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}
  
  bool SecurityContext::setReadOnly()  {
    const bool bWriteable = m_bWriteable; 
    m_bWriteable = false; 
    return bWriteable;
  }
  
  bool SecurityContext::setWriteable() {
    const bool bWriteable = m_bWriteable; 
    m_bWriteable = true; 
    return bWriteable;
  }

  bool SecurityContext::writeable() const {return m_bWriteable;}

  const char  *SecurityContext::username() const {
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    return (m_sUserName ? MM_STRDUP(m_sUserName) : NULL);
  }
  bool   SecurityContext::isUser(const char *sUserId)    const {return _STREQUAL(sUserId, m_sUserName);}
  bool   SecurityContext::isRoot()                       const {return _STREQUAL(ROOT_USERNAME, m_sUserName);}
  bool   SecurityContext::hasGroup(const char *sGroupId ATTRIBUTE_UNUSED) const {return false;}
  bool   SecurityContext::isLoggedIn()                   const {return m_sUserName != NULL;}
  
  SecurityContext::securityStrategy SecurityContext::parseSecurityStrategy(const char *sSecurityStrategy) {
    SecurityContext::securityStrategy ss;

    //passing bad strings to this class ain't gonna be accepted
    if      (!strcmp(sSecurityStrategy, UnixSecurityStrategy::m_sName))    ss = SecurityContext::unixSecurityStrategy;
    else if (!strcmp(sSecurityStrategy, AccessControlList::m_sName))       ss = SecurityContext::accessControlList;
    else throw UnkownSecurityStrategyName(this, sSecurityStrategy);

    return ss;
  }

  bool SecurityContext::login_trusted(const char *sUserId) {
    //iUserId 0 is anonymous, but the user does not necessarily exist in the userStore
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    if (sUserId) {
      if (m_sUserName) MMO_FREE(m_sUserName);
      m_sUserName = MM_STRDUP(sUserId);
    }
    return true;
  }

  void SecurityContext::logout() {
    //iUserId 0 is anonymous, but the user does not necessarily exist in the userStore
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    if (m_sUserName) MMO_FREE(m_sUserName);
    m_sUserName = 0;
  }

  void SecurityContext::masquerade(const char *sUserId) {
    //iUserId 0 is anonymous, but the user does not necessarily exist in the userStore
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    if (m_sMasqueradeUserId) MMO_FREE(m_sMasqueradeUserId);
    m_sMasqueradeUserId = MM_STRDUP(sUserId);
  }

  void SecurityContext::unmasquerade() {
    //iUserId 0 is anonymous, but the user does not necessarily exist in the userStore
    //we MM_STRDUP everything because this is security and we cannot afford SEG faults
    if (m_sMasqueradeUserId) MMO_FREE(m_sMasqueradeUserId);
    m_sMasqueradeUserId = 0;
  }
}
