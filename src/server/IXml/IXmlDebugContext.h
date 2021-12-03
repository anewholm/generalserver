//platform agnostic file
#ifndef _IXMLDEBUGCONTEXT_H
#define _IXMLDEBUGCONTEXT_H

#include "define.h"

#include "IXml/IXmlQueryEnvironment.h" //enums
#include "IXml/IXmlSecurityContext.h"  //enums

#include "Utilities/StringMap.h"
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;
  interface_class IXmlLibrary;

  interface_class IXmlDebugContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    //server events
    virtual IXmlQueryEnvironment::accessResult informAndWait(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCur, const IXmlQueryEnvironment::accessOperation iFilterOperation, IXmlQueryEnvironment::accessResult iIncludeNode) = 0;

    //client debug flow controls
    virtual IXmlBaseNode *setBreakpoint(IXmlBaseNode *pBreakpointNode) = 0;
    virtual void clearBreakpoint(IXmlBaseNode *pBreakpointNode) = 0;
    virtual void stepInit()  = 0;
      virtual void stepOver() = 0;
      virtual void stepInto() = 0;
      virtual void stepOut()  = 0;
      virtual void stepContinue()  = 0;
    virtual void stepEnd()  = 0; //cancel step completely, allowing a new thread to step
  };
}

#endif
