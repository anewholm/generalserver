//platform agnostic file
#ifndef _IXMLNODEMAP_H
#define _IXMLNODEMAP_H

#include "IXml/IXmlBaseNode.h" //defines

namespace general_server {
  interface_class IXmlQueryEnvironment;

  interface_class IXmlNodeMap {
  public:
    virtual ~IXmlNodeMap() {}
  };
}

#endif
