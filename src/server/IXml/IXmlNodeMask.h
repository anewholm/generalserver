//platform agnostic file
#ifndef _IXMLNODEMASK_H
#define _IXMLNODEMASK_H

#include "define.h" //includes platform define also

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
  enum iMaskType {
    //attributes and TEXT children appear automatically with all visible elements (default)
    maskAutoIncludeAttributesAndText = 0, //default
    //alll non-elements including processing instructions, ENTITY and document are included
    maskAutoIncludeAllNonElements,
    //attributes appear automatically on all visible elements
    //TEXT only appears if explicitly stated, |text()
    maskAutoIncludeAttributes, 
    //@attributes, TEXT and other nodes will not appear unless explicitly referenced
    maskExplicit
  };
  
  interface_class IXmlNodeMask: implements_interface IReportable {
  public:
    virtual XmlNodeList<const IXmlBaseNode> *calculateFor(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode) const = 0;
    virtual XmlNodeList<const IXmlBaseNode> *calculateFor(const IXmlQueryEnvironment *pQE, const XmlNodeList<const IXmlBaseNode> *pNodes) const = 0;

    virtual iMaskType maskType() const = 0;
    virtual void maskType(iMaskType iMaskTyper) = 0;
    
    virtual const char *toString() const = 0;
  };
}

#endif
