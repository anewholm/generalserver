//platform agnostic file
#ifndef _IXMLNODELIST_H
#define _IXMLNODELIST_H

#define IFNOTNULL true

#include "define.h"
#include "IReportable.h"       //IReportable direct inheritance
#include "IXml/IXmlBaseNode.h"      //DEEP_CLONE, COPY_ONLY

namespace general_server {
  interface_class IXmlQueryEnvironment;
  interface_class IXmlBaseNode;

  interface_class IXmlNodeList: implements_interface IReportable {
  public:
    IFDEBUG(virtual const char *x() const = 0;)
    
    //construction and destruction
    virtual IXmlNodeList *copyNodes(const IXmlQueryEnvironment *pQE, const bool bDeepClone = DEEP_CLONE, const bool bEvaluateValues = COPY_ONLY) const = 0;
    virtual IXmlNodeList *clone_with_resources() const = 0;
    //virtual void registerNodes(); //this method only works with non-const and therefore prevents the tmeplates from compiling
    virtual ~IXmlNodeList() {}
    virtual const IXmlNodeList *element_destroy() const = 0;

    //manipulation
    virtual void sortDocumentOrderAscending() = 0;
    virtual void sortDocumentOrderDescending() = 0;
    virtual void removeDuplicates() = 0;
    
    //----------- template based inputs
    //IXmlNodeType *first() const;
    //typename XmlNodeList<IXmlNodeType>::iterator push_back(IXmlNodeType *pNode);
    //typename XmlNodeList<IXmlNodeType>::iterator push_back_unique(IXmlNodeType *pNode);
    //virtual void insert(const IXmlNodeList *pNodes); 
    //typename XmlNodeList::iterator       find(IXmlNodeType *pNode);
    //typename XmlNodeList::const_iterator find_const(IXmlNodeType *pNode) const;
    //virtual bool contains(IXmlNodeType *pNode) const = 0;

    //information
    virtual const char *uniqueXPathToNodes(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pBaseNode = 0, const bool bWarnIfTransient = true, const bool bForceBaseNode = NO_FORCE_BASE_NODE, const bool bEnableIDs = false, const bool bEnableNameAxis = true, const bool bThrowIfNotBasePath = true) const = 0;
    virtual const char *toString() const = 0;

  };
}




#endif
