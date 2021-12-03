//platform agnostic file
#ifndef _REPOSITORYSAVECONTEXT_H
#define _REPOSITORYSAVECONTEXT_H

#include "define.h"

#include "IXml/IXmlSecurityContext.h" //direct inheritance
#include "IXml/IXmlNamespace.h"
#include <vector>

using namespace std;

namespace general_server {
  interface_class IXmlBaseDoc;

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class RepositorySaveSecurityContext: implements_interface IXmlSecurityContext {
    const IXmlQueryEnvironment *m_pOwnerQE;
    
    //only Repository can use this bypass class
    friend class Repository;
    friend class Directory; 
    RepositorySaveSecurityContext();
    RepositorySaveSecurityContext(const RepositorySaveSecurityContext& sec);

    //control: not allowed in SecurityContext
    bool enabled() const {return true;}
    bool enable()        {return true;}
    bool disable()       {assert(false); return true;}

  public:
    IXmlQueryEnvironment::accessResult checkSecurity(const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) const;
    IXmlSecurityContext *inherit() const {return clone_with_resources();}
    IXmlSecurityContext *clone_with_resources() const {return new RepositorySaveSecurityContext(*this);}
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}
    bool setReadOnly()        {return false;}
    bool setWriteable()       {assert(false); return false;}
    bool writeable()    const {return false;}

    //info
    const char *toString() const {return MM_STRDUP(type());}
    const char *type()     const {return "AdminSecurityContext";}

    //--------------------------- info
    virtual const char  *username()                       const {return 0;}
    virtual const char  *userStore()                    const {return 0;}
    virtual const char  *userUUID()                     const {return 0;}
    virtual bool   isUser(const char *sUserId ATTRIBUTE_UNUSED)    const {return false;}
    virtual bool   isRoot()                       const {return false;}
    virtual bool   hasGroup(const char *sGroupId ATTRIBUTE_UNUSED) const {return false;}
    virtual bool   isLoggedIn()                   const {return false;}
    virtual IXmlBaseNode *userNode()                    const {return 0;}

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
