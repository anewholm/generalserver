//platform agnostic
#include "SecurityStrategy.h"

#include "IXml/IXmlBaseNode.h"
#include "SecurityContext.h"
#include "Xml/XmlAdminQueryEnvironment.h"

namespace general_server {
  //-------------------------------------------------------------------------------------------------------------
  //------------------------------------- UNIX ------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------
  UnixSecurityStrategy::UnixSecurityStrategy(int iOwnerId ATTRIBUTE_UNUSED, int iOwnerGroupId ATTRIBUTE_UNUSED, int iPermissions ATTRIBUTE_UNUSED) {
    //TODO: allow writing of security attributes on to a node through the SecurityStrategy
    NOT_COMPLETE("");
    NOT_CURRENTLY_USED("");
  }
  const char *UnixSecurityStrategy::m_sName = "unix";

  IXmlQueryEnvironment::accessResult UnixSecurityStrategy::checkNodeAccess(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const SecurityContext *pSecurityContext, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) {
    const XmlAdminQueryEnvironment ibqe_read_security(pMemoryLifetimeOwner, pCur->document());
    /* IXmlQueryEnvironment::accessOperation:
     *   enum accessOperation {
     *     OP_EXECUTE = 1,
     *     OP_WRITE   = 2,
     *     OP_READ    = 4,
     *     OP_ADD     = 8
     *   };
     *
     * thus our unix bits are xwra
     */

    //http://en.wikipedia.org/wiki/File_system_permissions
    IXmlQueryEnvironment::accessResult ret = IXmlQueryEnvironment::RESULT_INCLUDE;
    //applicable attributes from the xmlsecurity namespace include:
    const char *sOwnerName      = 0; //xmlsecurity:ownername
    const char *sOwnerGroupName = 0; //xmlsecurity:ownergroupid
    const char *sPermissions    = 0; //xmlsecurity:permissions
    char cPermOwner, cPermGroup, cPermOther;
    char sPermToUse[2];
    unsigned int uPermToUse;
    bool bSystemAttribute;

    if (pSecurityContext->isRoot()) return IXmlQueryEnvironment::RESULT_INCLUDE; //admin override

    switch (pCur->nodeType()) {
      case NODE_ATTRIBUTE: {
        bSystemAttribute = pCur->isNamespace(NAMESPACE_XMLSECURITY) 
                        || pCur->isNamespace(NAMESPACE_REPOSITORY)
                        || pCur->isName(NAMESPACE_OBJECT, "cpp-component");
        if (bSystemAttribute) ret = IXmlQueryEnvironment::RESULT_DENY;
        break;
      }
      case NODE_ELEMENT: {
        if (sOwnerName = pCur->attributeValueDirect(&ibqe_read_security, "ownername", NAMESPACE_XMLSECURITY)) {
          //security info has been set on this node
          ret = IXmlQueryEnvironment::RESULT_DENY; //if security information is incomplete then deny access
          sOwnerGroupName = pCur->attributeValueDirect(&ibqe_read_security, "ownergroupid", NAMESPACE_XMLSECURITY);
          sPermissions    = pCur->attributeValueDirect(&ibqe_read_security, "permissions", NAMESPACE_XMLSECURITY);
          if (sPermissions && strlen(sPermissions) == 3) {
            //permission masks
            cPermOwner = sPermissions[0];
            cPermGroup = sPermissions[1];
            cPermOther = sPermissions[2];

            //user status selection
            memset(sPermToUse, 0, 2);
            if (     pSecurityContext->isUser(sOwnerName))        *sPermToUse = cPermOwner;
            else if (pSecurityContext->hasGroup(sOwnerGroupName)) *sPermToUse = cPermGroup;
            else                                                  *sPermToUse = cPermOther;

            //apply mask
            //GS mask is rwxa = 4 bits = a nibble = 0 -> F (0 -> 15)
            if (isxdigit(*sPermToUse)) {
              uPermToUse = strtol(sPermToUse, NULL, 16);
              if (uPermToUse & iAccessOperation) ret = IXmlQueryEnvironment::RESULT_INCLUDE;
            }
          }
        }
        break;
      }
      default: {}
    }

    //free up
    //if (sOwnerName)      MMO_FREE(sOwnerName)       //direct pointer in to xmlNode structure
    //if (sOwnerGroupName) MMO_FREE(sOwnerGroupName); //direct pointer in to xmlNode structure
    //if (sPermissions)    MMO_FREE(sPermissions);    //direct pointer in to xmlNode structure

    return ret;
  }


  //-------------------------------------------------------------------------------------------------------------
  //------------------------------------- AccessControlList ------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------
  const char *AccessControlList::m_sName = "acl";
  IXmlQueryEnvironment::accessResult AccessControlList::checkNodeAccess(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const SecurityContext *pSecurityContext, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iAccessOperation) {
    const XmlAdminQueryEnvironment ibqe_read_security(pMemoryLifetimeOwner, pCur->document());
    IXmlQueryEnvironment::accessResult ret = IXmlQueryEnvironment::RESULT_INCLUDE;
    const char *sACL;

    if (pSecurityContext->isRoot()) return IXmlQueryEnvironment::RESULT_INCLUDE; //admin override

    if (pCur->isNodeElement()) {
      if (sACL = pCur->attributeValueDirect(&ibqe_read_security, "acl", NAMESPACE_XMLSECURITY)) {
        switch (iAccessOperation) {
          case IXmlQueryEnvironment::OP_EXECUTE: ATTRIBUTE_FALLTHROUGH;
          case IXmlQueryEnvironment::OP_WRITE:   ATTRIBUTE_FALLTHROUGH;
          case IXmlQueryEnvironment::OP_READ:    ATTRIBUTE_FALLTHROUGH;
          case IXmlQueryEnvironment::OP_ADD: 
            NOT_COMPLETE("ACLs");
        }
      }
    }

    return ret;
  }
}
