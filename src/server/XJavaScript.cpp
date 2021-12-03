//platform agnostic file
#include "XJavaScript.h"

#include "Exceptions.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"

#include <string>
using namespace std;

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails> XJavaScript::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> XJavaScript::m_XSLFunctions;

  const StringMap<IXslModule::XslModuleCommandDetails> *XJavaScript::xslCommands() const {
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *XJavaScript::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("parameters",       XMF(XJavaScript::xslFunction_parameters));
      m_XSLFunctions.insert("parameter-checks", XMF(XJavaScript::xslFunction_parameterChecks));
    }
    return &m_XSLFunctions;
  }

  const char  *XJavaScript::xsltModuleNamespace()    const {return NAMESPACE_XJAVASCRIPT;}
  const char  *XJavaScript::xsltModulePrefix()       const {return NAMESPACE_XJAVASCRIPT_ALIAS;}
  const char  *XJavaScript::xslModuleManagerName()   const {return "XJavaScript";}

  const char *XJavaScript::xslFunction_parameters(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "xjs:parameters([parameters text/node, context text/node])";
    assert(pQE);
    assert(pXCtxt);

    const char *sParameterInput = 0, *sContext = 0;
    char *sParameterInputCopy = 0, *sPosition, *sEndPosition;
    string stParameterOutput;
    const IXmlBaseNode  *pInputNode = 0;
    rpattern_c::backref_type br;
    match_results_c mMatchResults;
    static rpattern_c rgxConversion("^\\(\\s*[a-zA-Z0-9_]+\\s*\\)\\s*([a-zA-Z0-9_]+)", NORMALIZE, MODE_DEFAULT);
    static rpattern_c rgxNoConversion("^([a-zA-Z0-9_]+)", NORMALIZE, MODE_DEFAULT);

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      //optional
      switch (pXCtxt->valueCount()) {
        case 2: {
          sContext        = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        }
        case 1: {
          sParameterInput = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        }
        case 0: {
          if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
            sParameterInput = pInputNode->attributeValue(pQE, "parameters");
          } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("attribute context node"), MM_STRDUP("no context node"), sFunctionSignature);
          break;
        }
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }
      
      //iterate through input string comma delimited portions
      if (sParameterInput && *sParameterInput) {
        sParameterInputCopy = MM_STRDUP(sParameterInput);
        sPosition           = sParameterInputCopy;
        while (sPosition) {
          if (sEndPosition = strchr(sPosition, ',')) *sEndPosition++ = '\0'; //null terminate portion
          
          while (*sPosition == ' ') sPosition++; //skip whitespace
          if (*sPosition == '[')    sPosition++; //skip optional area starter
          while (*sPosition == ' ') sPosition++; //skip whitespace
          
          if (_STRNEQUAL(sPosition, "...", 3)) {
            if (stParameterOutput.size()) stParameterOutput.append(",");
            stParameterOutput.append("aFlexibleArgs");
          } else {
            if (*sPosition == '(') br = rgxConversion.match(sPosition, mMatchResults);
            else                   br = rgxNoConversion.match(sPosition, mMatchResults);
            
            if (br.matched && mMatchResults.cbackrefs() > 1) {
              br = mMatchResults.backref(1);
              if (stParameterOutput.size()) stParameterOutput.append(",");
              stParameterOutput.append(br.first, br.second - br.first);
            }
          }

          //next
          sPosition = sEndPosition;
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sParameterInput)     MM_FREE(sParameterInput);
    if (sContext)            MM_FREE(sContext);
    if (sParameterInputCopy) MM_FREE(sParameterInputCopy);
    if (pInputNode)          delete pInputNode;

    UNWIND_EXCEPTION_THROW;
    
    return MM_STRDUP(stParameterOutput.c_str());
  }
  
  const char *XJavaScript::xslFunction_parameterChecks(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //s1FunctionName, s1ArgumentName, uArgument, [bOptional, uDefault, fConversion]
    //i = Function.cfah('removeDisplay','i',i,true);
    //j = Function.cfah('removeDisplay','j',j,true,window.JOO);
    //Function.cp('removeDisplay',arguments,[i,j],false);
    static const char *sFunctionSignature = "xjs:parameter-checks([parameters text/node, context text/node, indent, checkall])";
    assert(pQE);
    assert(pXCtxt);

    const char *sParameterInput = 0, *sContext = 0;
    char *sParameterInputCopy = 0, *sPosition, *sEndPosition;
    const char *sName;
    size_t iNameLen;
    string stParameterOutput, stAllParams;
    bool bOptional = false, bFlexibleArgs = false;
    bool bHasConversion, bHasDefault;
    const IXmlBaseNode  *pInputNode = 0;
    rpattern_c::backref_type br, brConversion, brDefault;
    match_results_c mMatchResults;
    static rpattern_c rgxConversion("^\\(\\s*([a-zA-Z0-9_]+)\\s*\\)\\s*([a-zA-Z0-9_]+)(\\s*=\\s*(.+))?", NORMALIZE, MODE_DEFAULT);
    static rpattern_c rgxNoConversion("^(\\s*)([a-zA-Z0-9_]+)(\\s*=\\s*(.+))?", NORMALIZE, MODE_DEFAULT);
    bool bCheckAll = false; //TODO: pQE->debugContext()->debugMode() ?
    size_t iIndent = 6;
    static const char *sIndent = "                                                ";

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      //optional
      switch (pXCtxt->valueCount()) {
        case 4: bCheckAll       = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 3: iIndent         = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: sContext        = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: {
          sParameterInput = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        }
        case 0: {
          if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
            sParameterInput = pInputNode->attributeValue(pQE, "parameters");
          } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("attribute context node"), MM_STRDUP("no context node"), sFunctionSignature);
          break;
        }
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }
      
      //iterate through input string comma delimited portions
      if (sParameterInput && *sParameterInput) {
        //--------------------------- Function.cfah(...); ---------------------------
        sParameterInputCopy = MM_STRDUP(sParameterInput);
        sPosition           = sParameterInputCopy;
        while (sPosition) {
          while (*sPosition == ' ')  sPosition++; //skip whitespace
          if    (*sPosition == '[') {sPosition++; bOptional = true;} //skip optional area starter
          while (*sPosition == ' ')  sPosition++; //skip whitespace
          
          //null terminate portion
          //optional areas may end with ] which should not be considered part of the DEFAULT
          if (sEndPosition = strchr(sPosition, ',')) *sEndPosition++ = '\0'; 
          else if (bOptional) {
            //last portion, if optional, should end with a ]
            sEndPosition = sPosition + strlen(sPosition) - 1;
            if (*sEndPosition == ']') *sEndPosition = '\0';
            else throw XJavaScriptFormatUnrecignised(this, sParameterInput);
            sEndPosition = 0; //reset to zero to indicate end
          }
          
          if (_STRNEQUAL(sPosition, "...", 3)) {
            bFlexibleArgs = true;
          } else {
            bHasDefault    = false;
            bHasConversion = (*sPosition == '(');
            if (bHasConversion) br = rgxConversion.match(  sPosition, mMatchResults);
            else                br = rgxNoConversion.match(sPosition, mMatchResults);
            if (bHasConversion) {
              brConversion   = mMatchResults.backref(1);
              bHasConversion = (brConversion.second != brConversion.first);
            }
            if (mMatchResults.cbackrefs() > 4) {
              brDefault   = mMatchResults.backref(4);
              bHasDefault = (brDefault.second != brDefault.first);
            }
            
            if (bHasConversion || bHasDefault || bCheckAll) {
              if (br.matched && mMatchResults.cbackrefs() > 2) {
                br       = mMatchResults.backref(2);
                sName    = br.first;
                iNameLen = br.second - br.first;
                
                if (iNameLen) {
                  //j = Function.cfah('removeDisplay','j',j,true,window.JOO);
                  //s1FunctionName, s1ArgumentName, uArgument, [bOptional, uDefault, fConversion]
                  stParameterOutput.append(sIndent, iIndent);
                  stParameterOutput.append(sName, iNameLen);
                  stParameterOutput.append("=Function.cfah('");
                  stParameterOutput.append(sContext ? sContext : "<no context>");
                  stParameterOutput.append("','");
                  stParameterOutput.append(sName, iNameLen);
                  stParameterOutput.append("',");
                  stParameterOutput.append(sName, iNameLen);
                  stParameterOutput.append(",");
                  stParameterOutput.append(bOptional ? "true" : "false");
                  
                  //gather parameters for Function.cp(...)
                  if (stAllParams.size()) stAllParams.append(",");
                  stAllParams.append(sName, iNameLen);

                  //default
                  if (bHasDefault) {
                    stParameterOutput.append(",");
                    stParameterOutput.append(brDefault.first, brDefault.second - brDefault.first);
                  } else if (bHasConversion) stParameterOutput.append(",DEFAULT");
                  
                  //conversion
                  if (bHasConversion) {
                    stParameterOutput.append(",window.");
                    stParameterOutput.append(brConversion.first, brConversion.second - brConversion.first);
                  }
                  
                  stParameterOutput.append(");\n");
                } else throw XJavaScriptFormatUnrecignised(this, sParameterInput);
              } else throw XJavaScriptFormatUnrecignised(this, sParameterInput);
            }
          }

          //next
          sPosition = sEndPosition;
        }
        
        //--------------------------- Function.cp(context, arguments, [arguments], flexible)
        if (bCheckAll) {
          stParameterOutput.append(sIndent, iIndent);
          stParameterOutput.append("Function.cp('");
          stParameterOutput.append(sContext ? sContext : "<no context>");
          stParameterOutput.append("',arguments,[");
          stParameterOutput.append(stAllParams);
          stParameterOutput.append("],");
          stParameterOutput.append(bFlexibleArgs ? "true" : "false");
          stParameterOutput.append(");\n");
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sParameterInput)     MM_FREE(sParameterInput);
    if (sContext)            MM_FREE(sContext);
    if (sParameterInputCopy) MM_FREE(sParameterInputCopy);
    if (pInputNode)          delete pInputNode;

    UNWIND_EXCEPTION_THROW;
    
    return MM_STRDUP(stParameterOutput.c_str());
  }
}
