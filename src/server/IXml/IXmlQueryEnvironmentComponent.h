//platform agnostic file
#ifndef _IXMLQUERYENVIRONMENTCOMPONENT_H
#define _IXMLQUERYENVIRONMENTCOMPONENT_H

#include "define.h"
#include "IXml/IXmlLibrary.h"
#include "IReportable.h"         //direct inheritance

#include <vector>
using namespace std;

namespace general_server {
  interface_class IXmlQueryEnvironment;
  
  interface_class IXmlQueryEnvironmentComponent: implements_interface IReportable {
  public:
    ~IXmlQueryEnvironmentComponent() {} //virtual destruction
    virtual void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) = 0;
    virtual const IXmlQueryEnvironment *queryEnvironment() const = 0;

    //control
    virtual bool enabled()  const = 0;
    virtual bool enable()         = 0;
    virtual bool disable()        = 0;

    //info
    virtual const char *type()     const = 0;
    virtual const char *toString() const = 0;
  };
}

#endif
