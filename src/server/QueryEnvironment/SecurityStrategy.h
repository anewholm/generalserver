//platform agnostic file
#ifndef _SECURITYSTRATEGY_H
#define _SECURITYSTRATEGY_H

#include "define.h"

#include "IXml/IXmlQueryEnvironment.h" //enums

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  class Database;
  class DatabaseNode;
  class SecurityContext;

  //--------------------------------------------------------- abstract Security Strategum
  interface_class ISecurityStrategy {
  public:
    //statically implemented
    //virtual const int checkNodeAccess(const SecurityContext *pSecurityContext, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) = 0;
    //virtual const char *name() = 0;
  };

  //--------------------------------------------------------- concrete NodeStoreSecurityContext
  class UnixSecurityStrategy: implements_interface ISecurityStrategy {
    //http://en.wikipedia.org/wiki/File_system_permissions
  public:
    UnixSecurityStrategy(int iOwnerId, int iOwnerGroupId, int iPermissions);

    static IXmlQueryEnvironment::accessResult checkNodeAccess(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const SecurityContext *pSecurityContext, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation);
    static const char *m_sName;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  //--------------------------------------------------------- concrete NodeStoreSecurityContext
  class AccessControlList: implements_interface ISecurityStrategy {
  public:
    static IXmlQueryEnvironment::accessResult checkNodeAccess(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const SecurityContext *pSecurityContext, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation);
    static const char *m_sName;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
