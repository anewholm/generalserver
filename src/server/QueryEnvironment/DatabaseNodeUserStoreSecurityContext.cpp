//platform agnostic
#include "DatabaseNodeUserStoreSecurityContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXslTransformContext.h"
#include "SecurityStrategy.h"
#include "Database.h"

namespace general_server {
  //---------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------- DatabaseNodeUserStoreSecurityContext
  //---------------------------------------------------------------------------------------
  DatabaseNodeUserStoreSecurityContext::DatabaseNodeUserStoreSecurityContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlBaseNode *pUserStore):
    SecurityContext(pMemoryLifetimeOwner),
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_ibqe_read_userstore(pMemoryLifetimeOwner, pUserStore ? pUserStore->document() : 0),
    m_pUserStore(pUserStore)
  {
    assert(m_pUserStore);
  }

  DatabaseNodeUserStoreSecurityContext::DatabaseNodeUserStoreSecurityContext(const DatabaseNodeUserStoreSecurityContext& sec):
    m_pUserStore(sec.m_pUserStore),
    m_ibqe_read_userstore(sec.m_ibqe_read_userstore),
    SecurityContext(sec),
    MemoryLifetimeOwner(sec.mmParent()) //addChild()
  {}

  IXmlSecurityContext *DatabaseNodeUserStoreSecurityContext::inherit() const {
    //just use the copy constructor to make a complete copy
    return clone_with_resources();
  }
  IXmlSecurityContext *DatabaseNodeUserStoreSecurityContext::clone_with_resources() const {
    //use the copy constructor to make a complete copy
    return new DatabaseNodeUserStoreSecurityContext(*this);
  }

  IXmlQueryEnvironment::accessResult DatabaseNodeUserStoreSecurityContext::checkSecurity(const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) const {
    //all known security strategies in use must return true to allow this access
    //TODO: performance: check the whole Database at startup for SecurityStrategys in use and then limit them here for performance
    //TODO: performance: get LibXML only to call here if there are non-xml:id attributes on the node
    //TODO: performance: write a low level hasAttributesWithNamespace(namespace) function to check for Database or xmlsecurity namespace first
    IXmlQueryEnvironment::accessResult ret = IXmlQueryEnvironment::RESULT_DENY;

    if (!m_bWriteable && iAccessOperation != IXmlQueryEnvironment::OP_READ) 
      throw NodeWriteableAccessDenied(this, MM_STRDUP("read-only DatabaseNodeUserStoreSecurityContext"));
    
    if (isRoot()) {
      //ignore root access
      ret = IXmlQueryEnvironment::RESULT_INCLUDE;
    } else {
      //Database_SECURITY_STRATEGY always = all
      if ( (UnixSecurityStrategy::checkNodeAccess(mmParent(), this, pCur, iAccessOperation) == IXmlQueryEnvironment::RESULT_INCLUDE)
        && (AccessControlList::checkNodeAccess(   mmParent(), this, pCur, iAccessOperation) == IXmlQueryEnvironment::RESULT_INCLUDE)
      ) {
        ret = IXmlQueryEnvironment::RESULT_INCLUDE;
      }
    }

    IFDEBUG(
      //if (ret == IXmlQueryEnvironment::RESULT_DENY)
      //  Debug::report("security blocked a node [%s] [%s]", pCur->localName(NO_DUPLICATE), pQE->transformContext()->currentXSLCommandXPath());
    );

    return ret;
  }

  const char *DatabaseNodeUserStoreSecurityContext::toString() const {
    //caller frees result
    string st;
    st += type();
    st += " (";
    st += (m_sUserName ? m_sUserName : "anonymous");
    st += ")";
    return MM_STRDUP(st.c_str());
  }

  const char *DatabaseNodeUserStoreSecurityContext::type() const {return "DatabaseNodeUserStoreSecurityContext";}

  const IXmlBaseNode *DatabaseNodeUserStoreSecurityContext::login(const char *sUsername, const char *sHashedPassword) {
    //caller frees result
    //callers to this function REQUIRE a user store to check against
    //do not use ServerSecurityContext for this
    //a return value of zero indicates failure
    bool bLoginOk = false;
    const char   *sUserName           = 0;
    const char   *sUserHashedPassword = 0;
    const IXmlBaseNode *pUserNode     = 0;

    //don't use the name axis here because we want an exact match
    if (pUserNode = m_pUserStore->getSingleNode(&m_ibqe_read_userstore, "object:User[@name='%s']", sUsername)) {
      if (sUserHashedPassword = pUserNode->attributeValueDirect(&m_ibqe_read_userstore, "password-hash")) {
        if (!strcmp(sHashedPassword, sUserHashedPassword)) {
          //login ok
          if (sUserName = pUserNode->attributeValueDirect(&m_ibqe_read_userstore, "name"))
            bLoginOk = login_trusted(sUserName); //will MM_STRDUP(sUserName)
        }
      }
    }
    if (!bLoginOk) throw LoginFailed(this);

    //free up
    //if (sUserName)           MM_FREE(sUserName); //attributeValueDirect
    //if (sUserHashedPassword) MM_FREE(sUserName); //attributeValueDirect

    return pUserNode;
  }
  
  const IXmlBaseNode *DatabaseNodeUserStoreSecurityContext::login(const char *sUUID) {
    //caller frees result
    //callers to this function REQUIRE a user store to check against
    //do not use AnonymousSecurityContext or ServerSecurityContext for this
    //a return value of zero indicates failure
    const char *sUserName         = 0;
    const IXmlBaseNode *pUserNode = 0;
    bool bLoginOk = false;

    //hardcoded cases
    if      (_STREQUAL(sUUID, ROOT_UUID))      bLoginOk = login_trusted(ROOT_USERNAME);      //will MM_STRDUP(sUserId)
    else if (_STREQUAL(sUUID, ANONYMOUS_UUID)) bLoginOk = login_trusted(ANONYMOUS_USERNAME); //will MM_STRDUP(sUserId)
    else if (pUserNode = m_pUserStore->getSingleNode(&m_ibqe_read_userstore, "object:User[@guid='%s']", sUUID)) {
      //general case
      if (sUserName = pUserNode->attributeValueDirect(&m_ibqe_read_userstore, "name"))
        bLoginOk = login_trusted(sUserName); //will MM_STRDUP(sUserName)
    }
    if (!bLoginOk) throw LoginFailed(this);

    //free up
    //if (sUserName) MM_FREE(sUserName); //attributeValueDirect

    return pUserNode;
  }

  const char *DatabaseNodeUserStoreSecurityContext::userStore() const {
    return m_pUserStore ? m_pUserStore->uniqueXPathToNode(&m_ibqe_read_userstore) : 0;
  }

  const char *DatabaseNodeUserStoreSecurityContext::userUUID() const {
    //caller frees result
    const char *sUUID             = 0;
    const IXmlBaseNode *pUserNode = 0;

    //anonymous or root users do not require a user node
    if      (_STREQUAL(m_sUserName, ROOT_USERNAME))      sUUID = MM_STRDUP(ROOT_UUID);
    else if (_STREQUAL(m_sUserName, ANONYMOUS_USERNAME)) sUUID = MM_STRDUP(ANONYMOUS_UUID);
    else {
      if (m_sUserName && m_pUserStore) {
        if (pUserNode = m_pUserStore->document()->nodeFromID(&m_ibqe_read_userstore, m_sUserName)) {
          sUUID = pUserNode->attributeValueDirect(&m_ibqe_read_userstore, "guid"); //no MM_STRDUP
          if (sUUID) sUUID = MM_STRDUP(sUUID);
        }
      }
    }
    
    //free up
    if (pUserNode) delete pUserNode;

    return sUUID;
  }
  
  IXmlBaseNode *DatabaseNodeUserStoreSecurityContext::userNode() const {
    return (m_sUserName && m_pUserStore ? m_pUserStore->document()->nodeFromID(&m_ibqe_read_userstore, m_sUserName) : 0);
  }
}
