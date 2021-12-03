//platform agnostic file
#include "Xml/XslModule.h"

#include "string.h" //required for UNIX strdup
#include "Server.h"
#include "IXml/IXmlDoc.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "Thread.h"
#include "Exceptions.h"

using namespace std;

namespace general_server {
  //-------------------------------------------------------- XslModule ------------------------------------------------
  //-------------------------------------------------------- XslModule ------------------------------------------------
  //-------------------------------------------------------- XslModule ------------------------------------------------
  void XslModule::registerXslCommands() const {
    //commands registration in the platform specific library
    const StringMap<IXslModule::XslModuleCommandDetails> *mXSLCommands = xslCommands();
    if (mXSLCommands) {
      StringMap<IXslModule::XslModuleCommandDetails>::const_iterator i;
      for (i = mXSLCommands->begin(); i != mXSLCommands->end(); i++) {
        xmlLibraryRegisterXslCommand(xsltModuleNamespace(), i->first);
      }
    }
  }
  void XslModule::registerXslFunctions() const {
    //functions registration in the platform specific library
    const StringMap<IXslModule::XslModuleFunctionDetails> *mXSLFunctions = xslFunctions();
    if (mXSLFunctions) {
      StringMap<IXslModule::XslModuleFunctionDetails>::const_iterator i;
      for (i = mXSLFunctions->begin(); i != mXSLFunctions->end(); i++) {
        xmlLibraryRegisterXslFunction(xsltModuleNamespace(), i->first);
      }
    }
  }

  IXmlBaseNode *XslModule::runXslCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //normal object member function to late bind the correct FN_MODULE_METHOD function according to the function node name
    IXmlBaseNode *pInsertResultNode = 0;
    const StringMap<IXslModule::XslModuleCommandDetails> *mXSLCommands = xslCommands(); //lazy cached in static
    StringMap<IXslModule::XslModuleCommandDetails>::const_iterator iXslCommand;
    FN_MODULE_METHOD pFunc;

    //look for the function for this name
    if (mXSLCommands) {
      iXslCommand = mXSLCommands->find(sFunction);
      if (iXslCommand == mXSLCommands->end()) {
        throw XSLTExtensionNotFound(this, sNamespace, sFunction);
      } else {
        XslModuleCommandDetails xmcd  = iXslCommand->second;
        pFunc = xmcd.fFunctionPointer;
        pInsertResultNode = (xslObject()->*pFunc)(pQE, pCommandNode, pSourceNode, pOutputNode);
      }
    } else throw XSLTExtensionNotFound(this, sNamespace, sFunction);

    return pInsertResultNode;
  }

  const char *XslModule::runXslFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //normal object member function to late bind the correct FN_MODULE_METHOD function according to the function node name
    const char *sResult = 0;
    const StringMap<IXslModule::XslModuleFunctionDetails> *mXSLFunctions = xslFunctions(); //lazy cached in static
    StringMap<IXslModule::XslModuleFunctionDetails>::const_iterator iXslFunction;
    FN_MODULE_FUNCTION pFunc;

    //look for the function for this name
    if (mXSLFunctions) {
      iXslFunction = mXSLFunctions->find(sFunction);
      if (iXslFunction == mXSLFunctions->end()) {
        throw XSLTExtensionNotFound(this, sNamespace, sFunction);
      } else {
        XslModuleFunctionDetails xmcd  = iXslFunction->second;
        pFunc = xmcd.fFunctionPointer;
        //xslObject() re-direction not used anymore
        //since DatabaseManager -> Database redirection was removed
        sResult = (xslObject()->*pFunc)(pQE, pXPCtxt, pNodes);
      }
    } else throw XSLTExtensionNotFound(this, sNamespace, sFunction);

    return sResult;
  }
  const char *XslModule::toStringRegistered() const {
    string stContent;
    if (xslCommands()  && xslCommands()->size())  stContent += xslCommands()->toStringKeys();
    if (xslFunctions() && xslFunctions()->size()) {
      if (stContent.size()) stContent += "; ";
      stContent += xslFunctions()->toStringKeys();
    }
    return MM_STRDUP(stContent.c_str());
  }

  //--------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------
  XslModuleManager::XslModuleManager(): m_bExtensionsEnabled(true) {}

  void XslModuleManager::disableExtensions() {m_bExtensionsEnabled = false;}

  void XslModuleManager::registerXslModule(IXslModule *pModule) {
    //register your functions please
    pModule->registerXslCommands();
    pModule->registerXslFunctions();

    //i remember you to pass control to you by namespace
    m_modules.insert(pModule->xsltModuleNamespace(), pModule);
  }

  const StringMap<IXslModule*> *XslModuleManager::modules() const {return &m_modules;}

  void XslModuleManager::registerXslModuleManager(const IXslModuleManager *pModuleManager) {
    StringMap<IXslModule*>::const_iterator iXslModule;

    //assemble prefixes: server request Database ...
    for (iXslModule = pModuleManager->modules()->begin(); iXslModule != pModuleManager->modules()->end(); iXslModule++) {
      registerXslModule(iXslModule->second);
    }
  }

  const char *XslModuleManager::extensionElementPrefixes() const {
    StringMap<IXslModule*>::const_iterator iXslModule;
    string stExtensionElementPrefixes;
    size_t iSize;

    //assemble prefixes: server request Database ...
    for (iXslModule = m_modules.begin(); iXslModule != m_modules.end(); iXslModule++) {
      stExtensionElementPrefixes += iXslModule->second->xsltModulePrefix();
      stExtensionElementPrefixes += ' ';
    }

    //remove trailing space
    if (iSize = stExtensionElementPrefixes.size()) stExtensionElementPrefixes.resize(iSize-1);

    return MM_STRDUP(stExtensionElementPrefixes.c_str());
  }

  const char *XslModuleManager::toStringModules() const {
    //caller frees result
    //TODO: make a method pointer parameterised StringMap::toStringObjects()
    StringMap<IXslModule*>::const_iterator iItem;
    string st;

    st += "ModulesManager:\n";
    for (iItem = m_modules.begin(); iItem != m_modules.end(); iItem++) {
      if (iItem->first) {
        st += "  ";
        st += iItem->second->xsltModulePrefix();
        st += ": ";
        st += iItem->second->toStringRegistered();
        st += "\n" ;
      }
    }
    return MM_STRDUP(st.c_str());
  }

  IXmlBaseNode *XslModuleManager::runXslModuleCommand(const IXmlQueryEnvironment *pQE, const char *sNamespace, const char *sFunction) {
    IXslTransformContext *pTCtxt = 0;
    IXmlBaseNode *pInsertResultNode = 0;
    StringMap<IXslModule*>::const_iterator iXslModule;
    IXslModule *module           = 0;
    iErrorPolicy er              = throw_error;

    const IXmlBaseNode    *pSourceNode  = 0;
    const IXslCommandNode *pCommandNode = 0;
    IXmlBaseNode          *pOutputNode  = 0;

    if (m_bExtensionsEnabled) {
      //look for the module for this namespace
      iXslModule = m_modules.find(sNamespace);
      if (iXslModule == m_modules.end()) {
        throw XSLTExtensionNotFound(this, sNamespace);
      } else {
        module = iXslModule->second;

        pTCtxt = pQE->transformContext();
        assert(pTCtxt);

        //current source document will always be the main Server config document
        //and the source node will be the current node in the config document being processed, e.g. repository:index_xsl
        //  if the transform is currently traversing the template process_main_stylesheet
        pSourceNode   = pTCtxt->sourceNode(pQE); //pQE as MM

        //current command in the current server conversation XSL stylesheet, e.g. <config:append-child destination="//example" />
        //although pStylesheet contains a pointer to pTCtxt, it will not delete it
        //instead we delete it manually in the outer caller method to this
        //isCommandNamespace() will return an IXslCommandNode* for any extension-prefix namespace
        //will throw InterfaceNotSupported if this is not a command node
        try {
          pCommandNode  = pTCtxt->commandNode(pQE); //debug: server: user: etc.  //pQE as MM
        } catch (InterfaceNotSupported &ins) {
          throw XSLTExtensionNotFound(this, sNamespace);
        }

        //current output doc, e.g. <root> -> client for XSLT
        //Note that the output doc will not be present if no root node is yet created
        //XML_DOCUMENT_NODE can be returned here
        pOutputNode   = pTCtxt->outputNode(pQE); //pQE as MM

        try {
          pInsertResultNode = module->runXslCommand(pQE, sNamespace, sFunction, pCommandNode, pSourceNode, pOutputNode);
          //the throw created in this try block will copy the exception
          //and then destroy the local copy
          //passing the copied exception by reference (ExceptionBase &eb) to the catch block
        } catch (ExceptionBase &eb) {
          //eb, local on the stack, will be un-wound automatically during the throw process in this catch
          //so we STRDUP(resources) during the copy constructor
          er = pCommandNode->queryInterfaceIXmlBaseNode()->errorPolicy(); //pQE as MM
          if (er == throw_error) throw XSLElementException(eb);
        }

        //output directly in to output document if requested
        if (pInsertResultNode) {
          if (!pOutputNode) throw NoOutputDocument(this);
          pOutputNode->moveChild(pQE, pInsertResultNode);
        }
      }
    }

    //tidy up
    if (pCommandNode)      delete pCommandNode;
    if (pSourceNode)       delete pSourceNode;
    if (pOutputNode)       delete pOutputNode;
    if (pInsertResultNode) delete pInsertResultNode;
    //if (pTCtxt)            delete pTCtxt; //server free

    //we deal with node returns in this function
    //the caller actually receives it but does nothing
    return 0;
  }

  const char *XslModuleManager::runXslModuleFunction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPCtxt, const char *sNamespace, const char *sFunction, XmlNodeList<const IXmlBaseNode> **pNodes) {
    const char *sResult = 0;
    StringMap<IXslModule*>::const_iterator iXslModule;
    IXslModule *module;

    IXslTransformContext *pTC        = 0;
    const IXmlBaseNode *pCommandNode = 0;
    iErrorPolicy er = throw_error;

    if (m_bExtensionsEnabled) {
      //look for the module for this namespace
      iXslModule = m_modules.find(sNamespace);
      if (iXslModule == m_modules.end()) {
        throw XSLTExtensionNotFound(this, sNamespace);
      } else {
        module = iXslModule->second;

        try {
          sResult = module->runXslFunction(pQE, pXPCtxt, sNamespace, sFunction, pNodes); //pNodes are **output
          //the throw created in this try block will copy the exception
          //and then destroy the local copy
          //passing the copied exception by reference (ExceptionBase &eb) to the catch block
        } catch (ExceptionBase &eb) {
          //eb, local on the stack, will be un-wound automatically during the throw process in this catch
          //so we STRDUP(resources) during the copy constructor
          if (pTC = pQE->transformContext()) {
            if (pCommandNode = pTC->literalCommandNode(pQE)) 
              er = pCommandNode->errorPolicy(); //pQE as MM
          }
          
          //free up
          if (pCommandNode) delete pCommandNode;
          if (er == throw_error) throw XPathException(eb, pXPCtxt);
        }
      }
    }

    //tidy up

    //pNodes are **output and are freed by caller
    return sResult;
  }
}
