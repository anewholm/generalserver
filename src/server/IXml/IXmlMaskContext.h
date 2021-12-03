//platform agnostic file
#ifndef _IXMLMASKCONTEXT_H
#define _IXMLMASKCONTEXT_H

#include "define.h"
#include "Utilities/StringMap.h"
#include "IXml/IXmlQueryEnvironment.h"          //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //inheritance
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  interface_class IXslTransformContext;
  interface_class IXmlQueryEnvironment;

  interface_class IXmlMaskContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    virtual size_t size() const = 0;
    virtual const IXmlArea *back() const = 0;

    virtual IXmlMaskContext *inherit() const = 0;

    virtual void push_back(const IXmlArea *pArea) = 0;
    virtual void pop_back() = 0;
    virtual IXmlQueryEnvironment::accessResult applyCurrentAreaMask(const IXmlBaseNode *pNode) const = 0;
  };
}

#endif
