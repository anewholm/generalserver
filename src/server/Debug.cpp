//platform agnostic file
#include "Debug.h"

#include "IXml/IXslNode.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlQueryEnvironment.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXmlLibrary.h"
#include "Exceptions.h"

#include "Utilities/strtools.h"
#include <iostream>
#include <string.h>
#include "Utilities/StringVector.h"
using namespace std;

#ifdef WITH_DEBUG //warning IFDEBUG(): embedding a directive within macro arguments is not portable
#include "LibXslTransformContext.h"
#endif

namespace general_server {
  ostream&    Debug::m_fOut = cout;
  FILE       *Debug::m_pLogFile = 0;
  bool        Debug::m_bLogOpenFailed = false;
  const char *Debug::m_sIgnore = NULL;

  Debug::Debug(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pLib(pLib)
  {}
  
  //--------------------------------------------------- XSL Module
  StringMap<IXslModule::XslModuleCommandDetails>  Debug::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Debug::m_XSLFunctions;
  const char *Debug::xsltModuleNamespace()  const {return NAMESPACE_DEBUG;}
  const char *Debug::xsltModulePrefix()     const {return NAMESPACE_DEBUG_ALIAS;}

  const StringMap<IXslModule::XslModuleCommandDetails> *Debug::xslCommands() const {
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("server-message",   XMC(Debug::xslCommand_serverMessage));
      m_XSLCommands.insert("GDB-break",        XMC(Debug::xslCommand_GDBBreak));
      m_XSLCommands.insert("template-stack",   XMC(Debug::xslCommand_templateStack));
      m_XSLCommands.insert("local-vars",       XMC(Debug::xslCommand_localVars));
      m_XSLCommands.insert("terminate",        XMC(Debug::xslCommand_terminate));
      m_XSLCommands.insert("xslt-set-trace",   XMC(Debug::xslCommand_xsltSetTrace));
      m_XSLCommands.insert("xslt-clear-trace", XMC(Debug::xslCommand_xsltClearTrace));
      m_XSLCommands.insert("xml-set-debug",    XMC(Debug::xslCommand_xmlSetDebug));
      m_XSLCommands.insert("xml-clear-debug" , XMC(Debug::xslCommand_xmlClearDebug));
      m_XSLCommands.insert("NOT_COMPLETE",     XMC(Debug::xslCommand_NOT_COMPLETE));
      m_XSLCommands.insert("NOT_CURRENTLY_USED", XMC(Debug::xslCommand_NOT_CURRENTLY_USED));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Debug::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("GDB-break", XMF(Debug::xslFunction_gdbBreak));
    }
    return &m_XSLFunctions;
  }

  IXmlBaseNode *Debug::xslCommand_xsltSetTrace(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:xslt-set-trace [@flags]"; UNUSED(sCommandSignature);
    const char *sFlags                   = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    IXmlLibrary::xsltTraceFlag iFlags = IXmlLibrary::XSLT_TRACE_ALL;

    //required destination parameter
    pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    sFlags           = pCommandNodeType->attributeValueDynamic(pQE, "flags");

    if (sFlags) iFlags = pQE->xmlLibrary()->parseXSLTTraceFlags(sFlags);
    pQE->setXSLTTraceFlags(iFlags);
    pQE->xmlLibrary()->setDebugFunctions();  //turn the output on

    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_xsltClearTrace(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:xslt-clear-trace"; UNUSED(sCommandSignature);
    pQE->clearXSLTTraceFlags();
    pQE->xmlLibrary()->clearDebugFunctions();  //turn the output off
    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_xmlSetDebug(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:xml-set-debug [@flags]"; UNUSED(sCommandSignature);
    const char *sFlags                   = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    IXmlLibrary::xmlTraceFlag iFlags = IXmlLibrary::XML_DEBUG_ALL;

    //required destination parameter
    pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    sFlags           = pCommandNodeType->attributeValueDynamic(pQE, "flags");

    if (sFlags) iFlags = pQE->xmlLibrary()->parseXMLTraceFlags(sFlags);
    pQE->setXMLTraceFlags(iFlags);
    pQE->xmlLibrary()->setDebugFunctions();  //turn the output on

    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_xmlClearDebug(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:xml-clear-debug"; UNUSED(sCommandSignature);
    pQE->clearXMLTraceFlags();
    pQE->xmlLibrary()->clearDebugFunctions();  //turn the output off
    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_serverMessage(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //inputs to this debug() call
    //pCommandNode: e.g. <config:appendChild>
    //pSourceNode:  e.g. <repository:index_xsl> if the transform is currently traversing the process_main_stylesheet
    //pOutputNode:  e.g. <root>

    //attributes for this debug() call
    //@if:          conditional output
    //@type:        the style of debug
    //@level:       the level of things
    //@output:      the debug output
    //@output-node: the debug output node

    static const char *sCommandSignature = "debug:server-message [@if @prefix @indent] @output/@output-node [@type @level @max-depth]";
    const char *sOutput                  = 0;
    const char *sType                    = 0,
               *sLevel                   = 0,
               *sPrefix                  = 0,
               *sIndent                  = 0;
    static const char *sIndentTemplate   = "                                                        ";
    static size_t iIndentTemplateLen     = strlen(sIndentTemplate);
    IReportable::reportType  rt = IReportable::rtInformation;
    IReportable::reportLevel rl = IReportable::rlExpected;
    const IXmlBaseNode *pCommandNodeType = 0,
                       *pNode            = 0;
    size_t iDepth, iIndent;

    IFNDEBUG(return 0;)
    
    if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
      if (pCommandNodeType->attributeValueBoolInterpret(pQE, "if", NAMESPACE_NONE, true)) {
        sType            = pCommandNodeType->attributeValueDynamic(pQE, "type");
        sLevel           = pCommandNodeType->attributeValueDynamic(pQE, "level");
        sPrefix          = pCommandNodeType->attributeValueDynamic(pQE, "prefix",  NAMESPACE_NONE, "Debug");
        iIndent          = pCommandNodeType->attributeValueInt(pQE, "indent", NAMESPACE_NONE, 0);
        rt               = parseReportType(sType);
        rl               = parseReportLevel(sLevel);

        sIndent = strndup(sIndentTemplate, iIndent * 2 > iIndentTemplateLen ? iIndentTemplateLen : iIndent * 2);
        
        if (sOutput = pCommandNodeType->attributeValueDynamic(pQE, "output")) {
          report("%s%s%s%s", sIndent, sPrefix, (*sPrefix ? ": " : ""), sOutput, rt, rl);
          MM_FREE(sOutput);
        }

        if (pNode = pCommandNodeType->attributeValueNode(pQE, "output-node", NAMESPACE_NONE, pSourceNode)) {
          //GDB: *((LibXmlBaseNode*) pNode)->m_oNode
          iDepth = pCommandNodeType->attributeValueInt(pQE, "max-depth", NAMESPACE_NONE, 3);
          if (sOutput = pNode->xml(pQE, iDepth)) {
            report("%s%s%s:\n %s", sIndent, sPrefix, (iDepth == 3 ? " (max-depth=3)" : ""), sOutput, rt, rl);
            MM_FREE(sOutput);
          }
        }

        if (!sOutput) throw AttributeRequired(this, MM_STRDUP("output or valid output-node"), sCommandSignature);
      }
    }

    //free up
    if (sType)   MM_FREE(sType);
    if (sLevel)  MM_FREE(sLevel);
    if (sPrefix) MM_FREE(sPrefix);
    if (sIndent) MM_FREE(sIndent);
    //if (sOutput) MM_FREE(sOutput); //already freed above
    if (pNode)   delete pNode;

    return 0;
  }

  const char *Debug::xslFunction_gdbBreak(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //fake command simply here to allow breaking out to examine current XSLT situation
    static const char *sCommandSignature = "debug:GDB-break()"; UNUSED(sCommandSignature);
    report("place a GDB break here");
    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_GDBBreak(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //fake command simply here to allow breaking out to examine current XSLT situation
    //GDB: ((LibXslTransformContext*) pQE->transformContext())->m_ctxt
    //GDB: ((LibXslTransformContext*) pQE->transformContext())->m_ctxt->xpathCtxt
    static const char *sCommandSignature = "debug:GDB-break"; UNUSED(sCommandSignature);

    IFDEBUG(
      const IXmlBaseNode *pParentSourceNode;
      IXslTransformContext   *pTCtxt = pQE->transformContext();
      xsltTransformContextPtr oTCtxt = (pTCtxt ? ((LibXslTransformContext*) pTCtxt)->libXmlContext() : 0);
      xmlXPathContextPtr      oCtxt  = (oTCtxt ? oTCtxt->xpathCtxt : 0);

      if (pSourceNode) {
        pParentSourceNode = pSourceNode->parentNode(pQE);
        if (pParentSourceNode) {
          report("debug:GDB-break under [%s %s]", pParentSourceNode->localName(NO_DUPLICATE), pParentSourceNode->xmlID(pQE));
          reportObject(pParentSourceNode);
        }
      }

      if (pTCtxt) {
        if (oCtxt) {
          if (!oCtxt->parent_route) report("no parent_route", IReportable::rtWarning);
        } else report("no xpath Context!", IReportable::rtWarning);
        report("mode [%s]", (const char*) oTCtxt->mode);
      } else report("no Transform Context!", IReportable::rtWarning);
    );

    report("place a GDB break here");
    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_terminate(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:terminate"; UNUSED(sCommandSignature);
    IFDEBUG(terminate();)
    IFNDEBUG(throw XSLCommandNotSupported(this, sCommandSignature);)
    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_templateStack(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:template-stack"; UNUSED(sCommandSignature);
    IXslTransformContext *pTransformContext = pQE->transformContext();
    XmlNodeList<const IXmlBaseNode> *pTemplateStackNodes = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iTemplate;
    const IXslTemplateNode *pTemplateNode;

    if (pTransformContext) {
      pTemplateStackNodes = pTransformContext->templateStack(pQE); //pQE as MM
      for (iTemplate = pTemplateStackNodes->begin(); iTemplate != pTemplateStackNodes->end(); iTemplate++) {
        pTemplateNode = (*iTemplate)->queryInterface((const IXslTemplateNode*) 0);
        reportObject(pTemplateNode);
      }
    }

    //free up
    if (pTemplateStackNodes) delete pTemplateStackNodes->element_destroy();

    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_localVars(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:local-vars"; UNUSED(sCommandSignature);
    IXslTransformContext *pTransformContext = pQE->transformContext();
    StringMap<const char*> *pLocalVars = 0;

    if (pTransformContext) {
      pLocalVars = pTransformContext->localVars();
      reportInternal_andFree(pLocalVars->toString());
    }

    //free up
    if (pLocalVars) delete pLocalVars->elements_free();

    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_NOT_COMPLETE(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:NOT_COMPLETE [@because]"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sBecause = 0,
               *sXPath   = 0;

    UNWIND_EXCEPTION_BEGIN {
      //optional
      if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0))
        sBecause = pCommandNodeType->attributeValueDynamic(pQE, "because");

      sXPath = pQE->currentXSLCommandXPath();
      IFDEBUG(Debug::report("NOT_COMPLETE %s [%s]", sBecause, sXPath);)
      NOT_COMPLETE(sBecause); //may throw an exception
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXPath)   MM_FREE(sXPath);
    if (sBecause) MM_FREE(sBecause);
    
    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Debug::xslCommand_NOT_CURRENTLY_USED(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "debug:NOT_CURRENTLY_USED [@because]"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sBecause = 0,
               *sXPath   = 0;

    UNWIND_EXCEPTION_BEGIN {
      //optional
      if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0))
        sBecause = pCommandNodeType->attributeValueDynamic(pQE, "because");

      sXPath = pQE->currentXSLCommandXPath();
      IFDEBUG(Debug::report("NOT_CURRENTLY_USED %s [%s]", sBecause, sXPath);)
      NOT_CURRENTLY_USED(sBecause); //may throw an exception
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXPath)   MM_FREE(sXPath);
    if (sBecause) MM_FREE(sBecause);
    
    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  //--------------------------------------------------- direct reporting
  IReportable::reportType Debug::parseReportType(const char *sReportType) {
    IReportable::reportType rt = IReportable::rtInformation;
    if      (_STREQUAL(sReportType, "error"))   rt = IReportable::rtError;
    else if (_STREQUAL(sReportType, "warning")) rt = IReportable::rtWarning;
    else if (_STREQUAL(sReportType, "debug"))   rt = IReportable::rtDebug;
    return rt;
  }

  IReportable::reportLevel Debug::parseReportLevel(const char *sReportLevel) {
    IReportable::reportLevel rl = IReportable::rlExpected;
    if      (_STREQUAL(sReportLevel, "worrying"))    rl = IReportable::rlWorrying;
    else if (_STREQUAL(sReportLevel, "recoverable")) rl = IReportable::rlRecoverable;
    else if (_STREQUAL(sReportLevel, "fatal"))       rl = IReportable::rlFatal;
    return rl;
  }

  const char *Debug::reportLevelName(IReportable::reportLevel iReportLevel) {
    const char *sName = 0;
    
    switch (iReportLevel) {
      case IReportable::rlExpected:    sName = "expected"; break;
      case IReportable::rlWorrying:    sName = "worrying"; break;
      case IReportable::rlRecoverable: sName = "recoverable"; break;
      case IReportable::rlFatal:       sName = "fatal"; break;
    }
    
    return sName;
  }

  const char *Debug::reportTypeName(IReportable::reportType iReportType) {
    const char *sName = 0;
    
    switch (iReportType) {
      case IReportable::rtDebug:       sName = "debug"; break;
      case IReportable::rtInformation: sName = "information"; break;
      case IReportable::rtWarning:     sName = "warning"; break;
      case IReportable::rtError:       sName = "error"; break;
    }
    
    return sName;
  }
  
  //strings
  void Debug::ignore(const char *sWhat) {
    m_sIgnore = sWhat;
  }
  
  void Debug::report(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3, sArg4);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3, sArg4, sArg5);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3, sArg4, sArg5, sArg6);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const char *sArg7, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3, sArg4, sArg5, sArg6, sArg7);
  }
  
  void Debug::warn(  const char *sMessage, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const char *sArg7) {
    report(sMessage, sArg1, sArg2, sArg3, sArg4, sArg5, sArg6, sArg7, rtWarning);
  }
  
  //various objects / numbers
  void Debug::reportObject(const IReportable *pObject) {
    assert(pObject);
    reportInternal_andFree(pObject->toString());
  }
  
  void Debug::report(const char *sMessage, const int iArg1, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    const char *sArg1 = itoa(iArg1);
    reportInternal(sMessage, rt, rl, sArg1);
    MM_FREE(sArg1);
  }

  void Debug::report(const char *sMessage, const char *sArg1, const int iArg2, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    const char *sArg2 = itoa(iArg2);
    reportInternal(sMessage, rt, rl, sArg1, sArg2);
    MM_FREE(sArg2);
  }
  
  void Debug::report(const char *sMessage, const char *sArg1, const int iArg2, const char *sArg3, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    const char *sArg2 = itoa(iArg2);
    reportInternal(sMessage, rt, rl, sArg1, sArg2, sArg3);
    MM_FREE(sArg2);
  }
  
  void Debug::report(const char *sMessage, const StringVector *pvItems, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    StringVector::const_iterator iItem;
    string sOut;
    for (iItem = pvItems->begin(); iItem != pvItems->end(); iItem++) {
      if (*iItem) {
        if (iItem != pvItems->begin()) sOut += ", ";
        sOut += *iItem;
      }
    }
    reportInternal(sMessage, rt, rl, sOut.c_str());
  }

  void Debug::report(const stringstream &sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage.str().c_str(), rt, rl);
  }

  //internal
  void Debug::reportInternal_andFree(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl) {
    reportInternal(sMessage, rt, rl);
    if (sMessage) MM_FREE(sMessage);
  }
  
  void Debug::reportInternal(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl, const char *sArg1, const char *sArg2, const char *sArg3, const char *sArg4, const char *sArg5, const char *sArg6, const char *sArg7, const char *sArg8, const char *sArg9, const char *sArg10) {
    char *sFullMessage  = 0;
    string stFullMessage;
    static time_t iTime = time(NULL); /* STATIC */
    int iTimeSinceLastCall;
    const char *sTimeSinceLastCall = 0;
    size_t iLen;

    //-------------------------- profiling help
    iTimeSinceLastCall = difftime(time(NULL), iTime);
    sTimeSinceLastCall = itoa(iTimeSinceLastCall);
    iTime              = time(NULL);

    //-------------------------- C SNPRINTF message
    if (!sMessage) sMessage = "<0x0 message>";
    iLen = strlen(sMessage)
        + (sArg1  ? strlen(sArg1)  : 0)
        + (sArg2  ? strlen(sArg2)  : 0)
        + (sArg3  ? strlen(sArg3)  : 0)
        + (sArg4  ? strlen(sArg4)  : 0)
        + (sArg5  ? strlen(sArg5)  : 0)
        + (sArg6  ? strlen(sArg6)  : 0)
        + (sArg7  ? strlen(sArg7)  : 0)
        + (sArg8  ? strlen(sArg8)  : 0)
        + (sArg9  ? strlen(sArg9)  : 0)
        + (sArg10 ? strlen(sArg10) : 0)
        + 1;
    sFullMessage = (char*) MMO_MALLOC(iLen);
    _SNPRINTF10(sFullMessage, iLen, sMessage, sArg1, sArg2, sArg3, sArg4, sArg5, sArg6, sArg7, sArg8, sArg9, sArg10);

    //-------------------------- std::string parts
    if      (rt == IReportable::rtError)   stFullMessage.append("************************ error ************************\n");
    else if (rt == IReportable::rtWarning) stFullMessage.append("*********************** warning ***********************\n");
    stFullMessage.append(sFullMessage);
    if (iTimeSinceLastCall) {
      stFullMessage += " [";
      stFullMessage += sTimeSinceLastCall;
      if (iTimeSinceLastCall > 2) stFullMessage += " (!time)";
      stFullMessage += " sec]";
    }
      
    //-------------------------- pump it out
    if (!m_sIgnore || !strstr(sFullMessage, m_sIgnore)) {
      m_fOut << stFullMessage << "\n";
      log(stFullMessage.c_str(), rt, rl, sTimeSinceLastCall); //log to file also by default
      fflush(stdout);
    
      //TODO: pause
      //no pause at the moment
      //because the API needs moving to api.localhost
      //otherwise it is asking for /api which doesn't exist
      //and the target is in the path which also doesn't work
      IFDEBUG(if (rt > IReportable::rtInformation) SHARED_BREAKPOINT("Debug", sFullMessage);)
    }

    //free up
    if (sTimeSinceLastCall)       MMO_FREE(sTimeSinceLastCall);
    if (sFullMessage != sMessage) MMO_FREE(sFullMessage);
  }
  
  void Debug::startProgressReport()    {m_fOut << "[";}
  void Debug::progressReport()         {m_fOut << ".";}
  void Debug::completeProgressReport() {m_fOut << "]\n";}

  void Debug::log(const char *sMessage, const IReportable::reportType rt, const IReportable::reportLevel rl, const char *sTimeSinceLastCall) {
    const char *sLogFile = "/var/log/generalserver/debug.log";
    string stMessage;
#ifdef WITH_LOGDIR
#define LOGFILE_DEBUG WITH_LOGDIR "/debug.log"
    sLogFile = LOGFILE_REQUESTS;
#endif

    if (!m_bLogOpenFailed) {
      //---------------------------------- lazy open
      if (!m_pLogFile) {
        _FOPEN(m_pLogFile, sLogFile, "w");
        if (m_pLogFile) Debug::report("opened log file [%s]", sLogFile);
        else {
          m_bLogOpenFailed = true;
          report("failed to open log file [%s]", sLogFile, IReportable::rtError);
        }
      }
      
      //---------------------------------- construct
      stMessage += "<";
      stMessage += reportTypeName(rt);
      stMessage += " level=\"";
      stMessage += reportLevelName(rl);
      stMessage += "\"";
      stMessage += " time-since-last=\"";
      stMessage += (sTimeSinceLastCall ? sTimeSinceLastCall : "unknown");
      stMessage += "\"><![CDATA[";
      stMessage += escapeForCDATA(sMessage ? sMessage : "<0x0 message>");
      stMessage += "]]></";
      stMessage += reportTypeName(rt);
      stMessage += ">";
      
      //---------------------------------- write
      if (m_pLogFile) {
        fwrite(stMessage.c_str(), sizeof(char), stMessage.size(), m_pLogFile);
        fwrite("\n", sizeof(char), 1, m_pLogFile);
        fflush(m_pLogFile);
      }
    }
  }

  const char *Debug::escapeForCDATA(const char *sString) {
    const char *sOut = sString;
    
    if (sString) {
      while (strstr(sString, "]]>")) {
        NOT_COMPLETE("escapeForCDATA");
      }
    }
    
    return sOut;
  }
}
