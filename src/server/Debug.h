//platform agnostic file
#ifndef _DEBUG_H
#define _DEBUG_H

#include "define.h"
//platform specific project headers
#include "LibXslModule.h" //direct inheritance SERVER_MAIN_XSL_MODULE
#include "Utilities/StringMap.h"    //locals
#include "IXml/IXslModule.h"   //types
#include "IReportable.h"  //types

#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

namespace general_server {
  class StringVector;
  
  class Debug: public SERVER_MAIN_XSL_MODULE, public MemoryLifetimeOwner {
    //Debug is all static
    //because we dont need virtual inheritance
    //it is a simple centralised wrapper for cout << operations
    const IXmlLibrary *m_pLib;

    //XslModuleManager
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    static ostream&    m_fOut;
    static FILE       *m_pLogFile;
    static bool        m_bLogOpenFailed;
    static const char *m_sIgnore;
    
  protected:
    static void reportInternal_andFree(const char *sMessage, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void reportInternal(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl, const char *sArg1 = 0, const char *sArg2 = 0, const char *sArg3 = 0, const char *sArg4 = 0, const char *sArg5 = 0, const char *sArg6 = 0, const char *sArg7 = 0, const char *sArg8 = 0, const char *sArg9 = 0, const char *sArg10 = 0);

  public:
    Debug(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib);
    
    //XSL Module command setup
    const char *xsltModuleNamespace()  const;
    const char *xsltModulePrefix()     const;
    const StringMap<IXslModule::XslModuleCommandDetails>   *xslCommands() const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;

    const IXmlLibrary *xmlLibrary() const {return m_pLib;}
    static const char *escapeForCDATA(const char *sString);

    static IReportable::reportType    parseReportType( const char *sReportType);
    static IReportable::reportLevel   parseReportLevel(const char *sReportLevel);
    static const char                *reportTypeName(  IReportable::reportType iReportType);
    static const char                *reportLevelName( IReportable::reportLevel iReportLevel);

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_serverMessage(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_GDBBreak(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_templateStack(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_localVars(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_terminate(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_NOT_COMPLETE(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_NOT_CURRENTLY_USED(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xsltSetTrace(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xsltClearTrace(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xmlSetDebug(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xmlClearDebug(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_gdbBreak(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    //direct reporting
    //strings (strings first, rt and rl afterwards)
    static void ignore(const char *sWhat = NULL);
    static void report(const char *sMessage, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const char *sArg7, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);

    static void warn(  const char *sMessage, const char *sArg1 = NULL, const char *sArg2 = NULL, const char *sArg3 = NULL, const char *sArg4 = NULL, const char *sArg5 = NULL, const char *sArg6 = NULL, const char *sArg7 = NULL);

    //misc objects
    static void reportObject(const IReportable *pObject);
    static void report(const char *sMessage, const StringVector *pvItems, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const int   iArg1, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const int iArg1, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const char *sMessage, const char *sArg1, const int iArg2, const char *sArg3, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    static void report(const stringstream &sMessage, const IReportable::reportType rt = IReportable::rtInformation, const IReportable::reportLevel rl = IReportable::rlExpected);
    
    static void startProgressReport();
    static void progressReport();
    static void completeProgressReport();

    static void log(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl, const char *sTimeSinceLastCall);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
    const char *toString() const {return MM_STRDUP("Debug");}
  };
}

#endif

