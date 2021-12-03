//platform agnostic file
#ifndef _ADMINSECURITYCONTEXT_H
#define _ADMINSECURITYCONTEXT_H

#include "define.h"

#include "IXml/IXmlSecurityContext.h" //direct inheritance
#include <vector>

using namespace std;

namespace general_server {
  interface_class IXmlBaseDoc;

  //---------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------- AdminSecurityContext
  //---------------------------------------------------------------------------------------
  class AdminSecurityContext: implements_interface IXmlSecurityContext {
    friend class XmlAdminQueryEnvironment; //only XmlAdminQueryEnvironment can use this bypass class
    friend class DatabaseAdminQueryEnvironment;
    friend class BrowserQueryEnvironment;       //only BrowserQueryEnvironment can use this bypass class

    const IXmlBaseDoc *m_pDoc;
    AdminSecurityContext(const IXmlBaseDoc *pDoc = NULL): m_pDoc(pDoc) {}
    AdminSecurityContext(const AdminSecurityContext& sec ATTRIBUTE_UNUSED) {}

    //control: not allowed in SecurityContext
    bool enabled() const {assert(false); return true;}
    bool enable()        {assert(false); return true;}
    bool disable()       {assert(false); return true;}

  public:
    IXmlQueryEnvironment::accessResult checkSecurity(const IXmlBaseNode *pCur ATTRIBUTE_UNUSED, const IXmlQueryEnvironment::accessOperation iAccessOperation ATTRIBUTE_UNUSED) const {return IXmlQueryEnvironment::RESULT_INCLUDE;}
    IXmlSecurityContext *inherit() const {return clone_with_resources();}
    IXmlSecurityContext *clone_with_resources() const {return new AdminSecurityContext(*this);}
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE ATTRIBUTE_UNUSED) {}
    IXmlQueryEnvironment *queryEnvironment() const {return 0;}
    const IXmlBaseDoc *document() const {return m_pDoc;}
    bool setReadOnly()        {return true;}
    bool setWriteable()       {return true;}
    bool writeable()    const {return true;}

    //info
    const char *toString() const {return MM_STRDUP(type());}
    const char *type()     const {return "AdminSecurityContext";}

    //--------------------------- info
    virtual const char  *username()                       const {return MM_STRDUP(ROOT_USERNAME);}
    virtual const char  *userStore()                    const {return 0;}
    virtual IXmlBaseNode *userNode()                    const {return 0;}
    virtual const char  *userUUID()                     const {return MM_STRDUP(ROOT_UUID);}
    virtual bool   isUser(const char *sUserId)    const {return sUserId && _STREQUAL(sUserId, ROOT_USERNAME);}
    virtual bool   isRoot()                       const {return true;}
    virtual bool   hasGroup(const char *sGroupId ATTRIBUTE_UNUSED) const {return true;}
    virtual bool   isLoggedIn()                   const {return true;}

    //--------------------------- security setup
    //logging in
    virtual const IXmlBaseNode *login(const char *sUsername ATTRIBUTE_UNUSED, const char *sHashedPassword ATTRIBUTE_UNUSED) {return 0;}
    virtual const IXmlBaseNode *login(const char *sUUID ATTRIBUTE_UNUSED) {return 0;}
    virtual bool                login_trusted(const char *sUserId ATTRIBUTE_UNUSED) {return false;}
    virtual void                logout() {}
    virtual void                masquerade(const char *sUserId ATTRIBUTE_UNUSED) {}
    virtual void                unmasquerade() {}
  };
}

#endif
