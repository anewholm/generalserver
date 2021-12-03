//platform agnostic file
#ifndef _IXMLGRAMMARCONTEXT_H
#define _IXMLGRAMMARCONTEXT_H

#include "define.h"
#include "Utilities/StringMap.h"
#include "IXml/IXmlQueryEnvironment.h"          //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //inheritance
#include "IXml/IXmlLibrary.h"                   //xmlTraceFlag
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  interface_class IXslTransformContext;
  interface_class IXmlQueryEnvironment;

  interface_class IXmlGrammarContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    virtual ~IXmlGrammarContext() {}

    virtual bool maybeXPath(const IXmlQueryEnvironment *pQE, const char *sText, const size_t iLen = 0) const = 0;
    virtual bool maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE, const char *sText) const = 0;

    virtual IXmlGrammarContext *inherit() const = 0;
    virtual const char *process(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPathContext, const char *sXPath, int *iSkip, IXmlLibrary::xmlTraceFlag iTraceFlags) = 0;
  };
}

#endif
