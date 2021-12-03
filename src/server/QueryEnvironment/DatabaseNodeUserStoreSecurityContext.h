//platform agnostic file
#ifndef _DBNODEUSERSTORESECURITYCONTEXT_H
#define _DBNODEUSERSTORESECURITYCONTEXT_H

#include "SecurityContext.h" //direct inheritance
#include "Xml/XmlAdminQueryEnvironment.h" //DatabaseNodeUserStoreSecurityContext has-a XmlAdminQueryEnvironment

namespace general_server {
  class DatabaseNode;
  interface_class IXmlBaseNode;
  interface_class IXmlQueryEnvironment;

  //---------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------- DatabaseNodeUserStoreSecurityContext
  //---------------------------------------------------------------------------------------
  class DatabaseNodeUserStoreSecurityContext: public SecurityContext {
    IXmlBaseNode *m_pUserStore;
    XmlAdminQueryEnvironment m_ibqe_read_userstore;

    friend class Database; //only Database can create DatabaseNodeUserStoreSecurityContext
    DatabaseNodeUserStoreSecurityContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlBaseNode *pUserStore);
    DatabaseNodeUserStoreSecurityContext(const DatabaseNodeUserStoreSecurityContext& sec);
    ~DatabaseNodeUserStoreSecurityContext() {}

  public:
    //environment
    IXmlQueryEnvironment::accessResult checkSecurity(const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) const;
    IXmlSecurityContext *inherit() const;
    IXmlSecurityContext *clone_with_resources() const;

    //info
    const char *toString()   const;
    const char *userStore()  const;
    const char *userUUID()   const;
    const char *type()       const;
    IXmlBaseNode *userNode() const;

    //login
    const IXmlBaseNode *login(const char *sUsername, const char *sHashedPassword);
    const IXmlBaseNode *login(const char *sUUID);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
