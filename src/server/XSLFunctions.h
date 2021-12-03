//platform agnostic file
//XML Database class wraps the XmlDoc
//late bound namespace XSL style commands
//  to expose DOM Level 3 to the XSL process
//  mostly to make changes to the source document during a transformation
//Indexes and other external high level functions for a DOM document
#ifndef _XSLFUNCTIONS_H
#define _XSLFUNCTIONS_H

#include "Utilities/regexpr2.h"
using namespace regex;

#include "Utilities/StringMap.h"
#include "Utilities/StringMultiMap.h"
#include <map>
#include <list>
#include "semaphore.h"
using namespace std;

#include "IXml/IXmlBaseNode.h"       //enums
#include "IXml/IXslXPathFunctionContext.h" //enums
#include "QueryEnvironment/TriggerContext.h"     //Database has-a Database_TRIGGER_CONTEXT
#include "QueryEnvironment/DatabaseAdminQueryEnvironment.h" //Database has-a embedded sub-class XmlAdminQueryEnvironment
#include "TXmlProcessor.h"      //direct inheritance
#include "IReportable.h"        //direct inheritance
#include "Debug.h"              //member
#include "define.h"

#include "Xml/XmlNodeList.h"        //member variable

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class Database;
  class DatabaseNode;
  class DatabaseClass;
  class Server;
  class SecurityContext;

  //these are included in the Database.cpp
  interface_class IXmlDoc;
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlArea;
  interface_class IXslCommandNode;     //XSL extensions

  class EXSLTRegExp: public SERVER_MAIN_XSL_MODULE, public MemoryLifetimeOwner {
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;
  public:
    EXSLTRegExp(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): MemoryLifetimeOwner(pMemoryLifetimeOwner) {}
    const char *xsltModuleNamespace() const {return NAMESPACE_EXSLT_REGEXP;}
    const char *xsltModulePrefix()    const {return NAMESPACE_EXSLT_REGEXP_ALIAS;}
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    
    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_replace(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */
    
    const char *toString() const {return MM_STRDUP("EXSLTRegExp");}
  };

  class EXSLTStrings: public SERVER_MAIN_XSL_MODULE, public MemoryLifetimeOwner {
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;
  public:
    EXSLTStrings(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): MemoryLifetimeOwner(pMemoryLifetimeOwner) {}
    const char *xsltModuleNamespace() const {return NAMESPACE_EXSLT_STRINGS;}
    const char *xsltModulePrefix()    const {return NAMESPACE_EXSLT_STRINGS_ALIAS;}
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    
    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_substringAfterLast(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_substringBeforeLast(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_substringAfter(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_substringBefore(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_list(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_boolean(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_not(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_dynamic(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_endsWith(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_escape(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_unescape(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */
    
    const char *toString() const {return MM_STRDUP("EXSLTStrings");}
  };

  class XSLTCustomUtilities: public SERVER_MAIN_XSL_MODULE, public MemoryLifetimeOwner {
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;
  public:
    XSLTCustomUtilities(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): MemoryLifetimeOwner(pMemoryLifetimeOwner) {}
    const char *xsltModuleNamespace() const {return NAMESPACE_EXSLT_FLOW;}
    const char *xsltModulePrefix()    const {return NAMESPACE_EXSLT_FLOW_ALIAS;}
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    
    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_stack(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_if(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_currentModeName(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_firstSetNotEmpty(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_onError(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */
    
    const char *toString() const {return MM_STRDUP("XSLTCustomUtilities");}
  };
}

#endif
