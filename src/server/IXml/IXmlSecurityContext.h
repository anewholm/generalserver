//platform agnostic file
#ifndef _IXMLSECURITYCONTEXT_H
#define _IXMLSECURITYCONTEXT_H

#include "define.h"

#include "IXml/IXmlQueryEnvironment.h"          //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //direct inheritance

#include "Utilities/StringMap.h"
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;
  interface_class IXmlLibrary;

  interface_class IXmlSecurityContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    virtual IXmlQueryEnvironment::accessResult checkSecurity(const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) const = 0;
    virtual IXmlSecurityContext *inherit() const = 0;

    //--------------------------- info
    virtual const char  *username()               const = 0;
    virtual const char  *userStore()              const = 0;
    virtual const char  *userUUID()               const = 0;
    virtual bool   isUser(const char *UserId)     const = 0;
    virtual bool   isRoot()                       const = 0;
    virtual bool   hasGroup(const char *sGroupId) const = 0;
    virtual bool   isLoggedIn()                   const = 0;
    virtual IXmlBaseNode *userNode()              const = 0;
    virtual bool   setReadOnly()                        = 0;
    virtual bool   setWriteable()                       = 0;
    virtual bool   writeable()                    const = 0;

    //--------------------------- security setup
    //logging in
    virtual const IXmlBaseNode *login(const char *sUsername, const char *sHashedPassword) = 0;
    virtual const IXmlBaseNode *login(const char *sUUID) = 0;
    virtual bool                login_trusted(const char *sUserId) = 0; //direct login
    virtual void                logout() = 0;
    virtual void                masquerade(const char *sUserId) = 0;
    virtual void                unmasquerade() = 0;
  };
}

#endif
