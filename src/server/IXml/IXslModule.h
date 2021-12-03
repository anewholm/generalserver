//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXSLMODULE_H
#define _IXSLMODULE_H

#include <vector>
using namespace std;

#include "define.h"
#include "Utilities/StringMap.h"

namespace general_server {
  /*
   * XslModuleManager (void*) pointer gets passed in to transform contexts (required of XmlLibrary)
   * XslModuleManager works with XslModule to achieve registration and run, functionality resides in both
   * XslModuleManager decides when each module registerCommands() and registerFunctions()
   *
   * Request (XslModuleManager)->registerXslModule(pModule) record only modules and ask:
   *   module->registerCommands() & module->registerFunctions()
   * XslModuleManager->runModuleCommand()  module->runCommand()
   * XslModuleManager->runModuleFunction() module->runFunction()
   *
   * XslModule runs and registers when told to
   * XslModule->runCommand()
   * XslModule->runFunction()
   * XslModule->registerCommands()  has own store of commands
   * XslModule->registerFunctions() has own store of functions
   *
   */

  //aheads
  interface_class IXmlBaseNode;
  interface_class IXslModule;
  interface_class IXslModuleManager;
  interface_class IXslCommandNode;
  interface_class IXslXPathFunctionContext;
  interface_class IXslTransformContext;
  interface_class IXmlQueryEnvironment;
  interface_class IXmlLibrary;
  class Server;
  class Service;
  template<class IXmlNodeType> class XmlNodeList;

  typedef IXmlBaseNode *(IXslModule::*FN_MODULE_METHOD)   (const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
  typedef const char   *(IXslModule::*FN_MODULE_FUNCTION) (const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- IXslModule ---------------------------------------------
  //-------------------------------------------------------------------------------------------
#define XMF(f) IXslModule::XslModuleFunctionDetails((FN_MODULE_FUNCTION) &f)
#define XMC(f) IXslModule::XslModuleCommandDetails( (FN_MODULE_METHOD)   &f)

  interface_class IXslModule {
  protected:
    //TODO: XslModule extended command meta data
    struct XslModuleFunctionDetails {
      FN_MODULE_FUNCTION fFunctionPointer;
      bool bModuleConst;
      bool bTXml;
      XslModuleFunctionDetails(FN_MODULE_FUNCTION _fFunctionPointer, bool _bModuleConst = true, bool _bTXml = true) {
        fFunctionPointer = _fFunctionPointer;
        bModuleConst     = _bModuleConst;
        bTXml            = _bTXml;
      }
    };

    struct XslModuleCommandDetails {
      FN_MODULE_METHOD fFunctionPointer;
      bool bModuleConst;
      bool bTXml;
      XslModuleCommandDetails(FN_MODULE_METHOD _fFunctionPointer, bool _bModuleConst = true, bool _bTXml = true) {
        fFunctionPointer = _fFunctionPointer;
        bModuleConst     = _bModuleConst;
        bTXml            = _bTXml;
      }
    };

  private:
    //command and function knowledge
    //static lazy cached from (recommended) per-manager static stores
    virtual const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const = 0;
    virtual const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const = 0;

  protected:
    //registration
    virtual HRESULT xmlLibraryRegisterXslFunction(const char *xsltModuleNamespace, const char *sFunctionName) const = 0;
    virtual HRESULT xmlLibraryRegisterXslCommand( const char *xsltModuleNamespace, const char *sCommandName)  const = 0;

  public:
    //registration
    virtual const char *xsltModuleNamespace() const = 0;
    virtual const char *xsltModulePrefix()    const = 0;
    virtual void registerXslCommands()  const = 0;
    virtual void registerXslFunctions() const = 0;
    virtual const char *toStringRegistered() const = 0; //often an is-a, therefore avoid toString() overload conflicts

    //execution: minimal native library functions
    //get a standard context and return
    virtual IXslModule *xslObject() = 0; //usually just returns this, but can redirect
    virtual IXmlBaseNode *runXslCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) = 0;
    virtual const char *runXslFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes) = 0;
  };

  //-------------------------------------------------------------------------------------------------------
  //------------------------------------------ EMO --------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------
  interface_class IXslModuleManager {
    //this object gets passed through to the Library and out again and must resolve extension calls
    //which have been explicitly registered with it

  protected:
    //implementor must call this function at some point during creation
    //remembers the module
    //and calls registerXslCommands() and registerXslFunctions() on the module
    virtual void registerXslModule(IXslModule *pModule) = 0;
    virtual void registerXslModuleManager(const IXslModuleManager *pModuleManager) = 0;

  public:
    //dependency injection accessors
    virtual const IXmlLibrary *xmlLibrary() const = 0;
    virtual const StringMap<IXslModule*> *modules() const = 0;
    virtual const char *toStringModules() const = 0; //often an is-a, therefore avoid toString() overload conflicts
    virtual IXslModuleManager *inherit() const = 0;

    virtual void disableExtensions() = 0;

    //execution from XmlLibrary XSL calls
    //elements and functions
    virtual IXmlBaseNode *runXslModuleCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction) = 0;
    virtual const char   *runXslModuleFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes) = 0;
    //application layer info: request server database etc.
    //see also the IXmlBasDoc::extensionElementPrefixes() which comes from the XML Library
    virtual const char *extensionElementPrefixes() const = 0;
    virtual const char *xslModuleManagerName()     const = 0; //Request or Server
  };
}

#endif
