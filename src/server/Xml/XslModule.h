//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XSLMODULE_H
#define _XSLMODULE_H

using namespace std;

#include "define.h"
#include "Utilities/StringMap.h"

#include "IXml/IXslModule.h"     //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXslModule;
  interface_class IXslModuleManager;
  interface_class IXmlQueryEnvironment;
  class Server;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- XslModule ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  class XslModule: implements_interface_static IXslModule, implements_interface IMemoryLifetimeOwner {
  private:
    //recommended static lazy cached function and command lists
    //static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;  //object specific list of XSL commands
    //static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions; //object specific list of XSL functions

  public:
    //registration
    void registerXslCommands()  const;
    void registerXslFunctions() const;
    const char *toStringRegistered() const;

    //note that these are not const because they carry out static lazy cacheing
    virtual const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const {return 0;}
    virtual const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const {return 0;}

    //execution
    virtual IXslModule *xslObject() {return this;}
    IXmlBaseNode *runXslCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    const char   *runXslFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes);
  };

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- XslModuleManager ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  class XslModuleManager: implements_interface_static IXslModuleManager, implements_interface IMemoryLifetimeOwner {
    StringMap<IXslModule*> m_modules;
    bool m_bExtensionsEnabled;

  protected:
    XslModuleManager();
    IXslModuleManager *inherit() const {return (IXslModuleManager*) this;}

    void registerXslModule(IXslModule *pModule);
    void registerXslModuleManager(const IXslModuleManager *pModuleManager);
    const StringMap<IXslModule*> *modules() const;
    const char *toStringModules() const;

  public:
    //execution
    IXmlBaseNode *runXslModuleCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction);
    const char   *runXslModuleFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *extensionElementPrefixes() const;

    void disableExtensions();
  };
}

#endif
