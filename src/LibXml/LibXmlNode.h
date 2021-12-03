//platform specific file (UNIX)
#ifndef _LIBXMLNODE_H
#define _LIBXMLNODE_H

//std library includes
#include <iostream>
#include <stdarg.h>
using namespace std;

//needed by LibXmlBase.cpp templates
#include "LibXslTransformContext.h" //LibXmlBase::attributeValue()
#include "LibXslDoc.h"              //LibXmlBase::transform()
#include "LibXmlBaseNode.h"         //full for inheritance
#include "Xml/XmlNode.h"                //full for inheritance

namespace general_server {
  class LibXmlDoc;
  class LibXmlNode;
  class Repository;

  class LibXmlNode: public LibXmlBaseNode, public XmlNode {
    friend class LibXmlLibrary; //only the library can instanciate stuff
    friend class LibXmlBaseNode; //PERFORMANCE: instanciation exception: security performance creates on local stack
    friend class LibXslXPathFunctionContext;

  protected:
    //implements the IXmlBaseNode and thus IXmlBaseNode<IXmlDoc> interfaces by
    //  using the LibXmlBaseNode<IXmlDoc> virtual base class
    LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS);
    LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS);
    LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNode *oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS);

  public:
    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXmlBaseNode                *queryInterface(IXmlBaseNode *p)                {return this;}
    const IXmlBaseNode          *queryInterface(const IXmlBaseNode *p) const    {return this;}
    IXmlBaseNode                *queryInterfaceIXmlBaseNode()                   {return this;}
    const IXmlBaseNode          *queryInterfaceIXmlBaseNode() const             {return this;}
    IXmlNamespaced              *queryInterface(IXmlNamespaced *p)              {return this;}
    const IXmlNamespaced        *queryInterface(const IXmlNamespaced *p) const  {return this;}
    IXmlHasNamespaceDefinitions *queryInterface(IXmlHasNamespaceDefinitions *p) {return this;}
    const IXmlHasNamespaceDefinitions *queryInterface(const IXmlHasNamespaceDefinitions *p) const {return this;}

  protected:
    IXmlBaseNode *clone_wrapper_only() const;
    IXmlBaseNode *clone_with_resources() const;
  };
}

#endif
