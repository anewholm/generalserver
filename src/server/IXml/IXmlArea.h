//platform agnostic file
#ifndef _IXMLAREA_H
#define _IXMLAREA_H

#include "define.h" //includes platform define also
#include "IXml/IXmlNodeMask.h"
#include "IXml/IXmlBaseNode.h"

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include "Utilities/StringMap.h"
using namespace std;

#include "IXml/IXmlBaseNode.h"   //enums

namespace general_server {
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  interface_class IXmlArea: implements_interface IReportable {
  public:
    virtual ~IXmlArea() {}
    virtual bool contains(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode) const = 0;
    virtual const char *toString() const = 0;
    virtual const char *toStringNodes() const = 0;
  };
}

#endif
