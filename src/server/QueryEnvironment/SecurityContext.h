//platform agnostic file
#ifndef _SECURITYCONTEXT_H
#define _SECURITYCONTEXT_H

#include "define.h"

#include "IXml/IXmlSecurityContext.h" //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance
#include <vector>

using namespace std;

namespace general_server {
  interface_class IXmlBaseDoc;

  //---------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------- SecurityContext
  //---------------------------------------------------------------------------------------
  class SecurityContext: implements_interface IXmlSecurityContext, virtual public MemoryLifetimeOwner {
    //SecurityContext is an IXml* level object. Database uses it.
    //  However, it is not for making changes to users, DatabaseNodes should be used for that
    //  and SecurityContext returns const IXml* information
    //implementing IXmlFilterContext means that this SecurityContext can be passed directly to any XML call
    //which then calls back to this object requesting filtering for each element
  public:
    enum securityStrategy {
      unixSecurityStrategy = 1,
      accessControlList
    };

  protected:
    const char *m_sUserName;
    const char *m_sMasqueradeUserId;
    vector<int>   m_groupids;
    const IXmlQueryEnvironment *m_pOwnerQE;
    bool m_bWriteable; //SECURITY: defaults to true, does not override main security!

    //only sub-classes can use this class
    SecurityContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    SecurityContext(const SecurityContext& sec);
    ~SecurityContext();
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}

  public:
    bool login_trusted(const char *sUserId);
    void logout();
    void masquerade(const char *sUserId);
    void unmasquerade();
    bool setReadOnly();
    bool setWriteable();
    bool writeable() const;

    //control: not allowed in SecurityContext
    bool enabled() const {return true;}
    bool enable()        {return true;}
    bool disable()       {assert(false);return true;}

    //info
    const char  *username()            const;
    bool   isUser(const char *sUserId) const;
    bool   isRoot()            const;
    bool   hasGroup(const char *sGroupId) const;
    bool   isLoggedIn()                   const;

    //utilities
    SecurityContext::securityStrategy parseSecurityStrategy(const char *sSecurityStrategy);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
