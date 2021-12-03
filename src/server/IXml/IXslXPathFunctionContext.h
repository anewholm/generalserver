//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXSLXPATHFUNCTIONCONTEXT_H
#define _IXSLXPATHFUNCTIONCONTEXT_H

#include <ctime>
using namespace std;

#include "define.h"
#include "IXml/IXmlQueryEnvironment.h" //direct inheritance
#include "MemoryLifetimeOwner.h"       //direct inheritance
#include "Utilities/VirtualWrapperClass.h"
#include "Xml/XmlNodeList.h"

namespace general_server {
  interface_class IXmlLibrary;
  interface_class IXslDoc;
  interface_class IXslCommandNode;
  interface_class IXslModuleManager;
  class Server;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- IXslXPathFunctionContext ---------------------------
  //-------------------------------------------------------------------------------------------
  enum iXPathParamType {
    XPATH_UNDEFINED = 0,
    XPATH_NODESET = 1,
    XPATH_BOOLEAN = 2,
    XPATH_NUMBER = 3,
    XPATH_STRING = 4,
    XPATH_POINT = 5,
    XPATH_RANGE = 6,
    XPATH_LOCATIONSET = 7,
    XPATH_USERS = 8,
    XPATH_XSLT_TREE = 9 //An XSLT value tree, non modifiable
  };

  interface_class IXslXPathFunctionContext: implements_interface IMemoryLifetimeOwner {
    //construction: only IXmlLibrary should create these
  public:
    virtual ~IXslXPathFunctionContext() {} //virtual public destructors allow proper destruction of derived object

    //environment access
    //virtual IXslTransformContext *transformContext() const = 0;
    //virtual const IXmlQueryEnvironment *ownerQE() const = 0;

    //------------------------------ execution context const information
    virtual iXPathParamType xtype()                  const = 0;
    virtual const char   *xpath()                    const = 0; //xpathCtxt:  full xpath statment
    virtual const char   *xpathCurrent()             const = 0; //xpathCtxt:  current part of the xpath being evaluated
    virtual int     valueCount()               const = 0; //parserCtxt: current function parameters count
    virtual const char   *functionName()             const = 0; //parserCtxt: current function name
    virtual const char   *functionNamespace()        const = 0; //parserCtxt: current function NS
    virtual IXmlBaseNode *stackNode(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iTab = 0) const = 0; //parserCtxt: current node for the outer context of this function resolution
    virtual IXmlBaseNode *contextNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //parserCtxt: checks stackNode(), returns currentNode() if not
    virtual IXmlBaseNode *currentNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //parserCtxt: context->node
    virtual const IXslCommandNode *commandNode(    const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //parserCtxt: context->inst
    virtual const IXmlBaseNode *literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //parserCtxt: context->inst

    //------------------------------ writing
    virtual const IXmlBaseNode *contextNode(const IXmlBaseNode *pNewConextNode) = 0;
    
    //------------------------------ stack function processing
    //function calls use parameter stacks with a specific limited output back on to the stack
    //  no knowledge of context is required beyond the input parameters (deterministic)
    //element calls use full tree interpretation with full variety of actions on any documents
    //  full context knowledge is required
    virtual void                   popIgnoreFromXPathFunctionCallStack() = 0;
    virtual const char            *popInterpretCharacterValueFromXPathFunctionCallStack() = 0;
    virtual double                 popInterpretDoubleValueFromXPathFunctionCallStack()    = 0;
    virtual int                    popInterpretIntegerValueFromXPathFunctionCallStack()   = 0;
    virtual bool                   popInterpretBooleanValueFromXPathFunctionCallStack()   = 0;
    virtual struct tm              popInterpretDateFromXPathFunctionCallStack()           = 0;
    virtual XmlNodeList<IXmlBaseNode> *popInterpretNodeListFromXPathFunctionCallStack()   = 0;
    virtual IXmlBaseNode          *popInterpretNodeFromXPathFunctionCallStack(const bool bThrowOnNot1 = false) = 0;
    //virtual DatabaseClass         *popInterpretDatabaseClassFromXPathFunctionCallStack(IXslXPathFunctionContext *pCtxt) const; //on the DatabaseClass!

    //------------------------------ debugging
    virtual const char *currentXSLCommandXPath() const = 0;
    virtual const char *currentXSLTemplateXPath() const = 0;
    virtual IXslXPathFunctionContext *clone_with_resources() const = 0;
    virtual void handleError(const char *sErrorMessage) = 0;
  };
}

#endif
