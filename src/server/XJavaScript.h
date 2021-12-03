//platform agnostic file
#ifndef _XJAVASCRIPT_H
#define _XJAVASCRIPT_H

//standard library includes
using namespace std;

#include "define.h"
#include "LibXslModule.h"
#include "Utilities/regexpr2.h"
using namespace regex;

namespace general_server {
  class XJavaScript: public SERVER_MAIN_XSL_MODULE, public MemoryLifetimeOwner {
  private:
    //SERVER_MAIN_XSL_MODULE only
    static StringMap<IXslModule::XslModuleCommandDetails> m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

  public:
    XJavaScript(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): MemoryLifetimeOwner(pMemoryLifetimeOwner) {}
    const StringMap<IXslModule::XslModuleCommandDetails> *xslCommands() const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;
    const char *xslModuleManagerName() const;
    
    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_parameters(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_parameterChecks(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */
    
    const char *toString() const {return MM_STRDUP("XJavaScript");}
  };
}

#endif
