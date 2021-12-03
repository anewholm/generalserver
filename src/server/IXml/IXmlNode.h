//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXMLNODE_H
#define _IXMLNODE_H

#include "define.h"

namespace general_server {
  //aheads
  interface_class IXmlBaseNode;

  interface_class IXmlNode {
  public:
    virtual ~IXmlNode() {} //virtual public destructors allow proper destruction of derived object

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode *queryInterface(IXmlBaseNode *p) = 0;
  };
}

#endif
