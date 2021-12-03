//platform agnostic file
#include "XSLFunctions.h"

//full definitions of the ahead declerations in Database.h
#include "IXml/IXmlSecurityContext.h"
#include "QueryEnvironment/QueryEnvironment.h"
#include "DatabaseClass.h"
#include "QueryEnvironment/BrowserQueryEnvironment.h"
#include "QueryEnvironment/TriggerContext.h"
#include "QueryEnvironment/MaskContext.h"
#include "IXml/IXmlNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlArea.h"
#include "TXml.h"
#include "RegularX.h"
#include "QueryEnvironment/DatabaseNodeUserStoreSecurityContext.h"
#include "Server.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"

#include "LibXmlBaseNode.h"
#include "LibXslDoc.h"

#include <time.h>
using namespace std;

namespace general_server {
  StringMap<IXslModule::XslModuleFunctionDetails> EXSLTRegExp::m_XSLFunctions;
  StringMap<IXslModule::XslModuleFunctionDetails> EXSLTStrings::m_XSLFunctions;
  StringMap<IXslModule::XslModuleFunctionDetails> XSLTCustomUtilities::m_XSLFunctions;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- EXSLTRegExp ----------------------------------------
  //-------------------------------------------------------------------------------------------
  const StringMap<IXslModule::XslModuleFunctionDetails> *EXSLTRegExp::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("replace", XMF(EXSLTRegExp::xslFunction_replace));
      //m_XSLFunctions.insert("match",   XMF(EXSLTRegExp::xslFunction_match));
      //m_XSLFunctions.insert("test",    XMF(EXSLTRegExp::xslFunction_test));
    }
    return &m_XSLFunctions;
  }

  const char *EXSLTRegExp::xslFunction_replace(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //LibXSLT EXSLT does not support regexp namespace
    //  xsltRegisterExtModuleFunction()
    //flags are only i and g
    //http://www.cplusplus.com/reference/string/string/
    //http://easyethical.org/opensource/spider/regexp%20c++/greta2.htm
    /*
      enum REGEX_FLAGS
      {
          NOFLAGS       = 0x0000,
          NOCASE        = 0x0001, // ignore case
          GLOBAL        = 0x0002, // match everywhere in the string
          MULTILINE     = 0x0004, // ^ and $ can match internal line breaks
          SINGLELINE    = 0x0008, // . can match newline character
          RIGHTMOST     = 0x0010, // start matching at the right of the string
          NOBACKREFS    = 0x0020, // only meaningful when used with GLOBAL and substitute
          FIRSTBACKREFS = 0x0040, // only meaningful when used with GLOBAL
          ALLBACKREFS   = 0x0080, // only meaningful when used with GLOBAL
          NORMALIZE     = 0x0100, // Preprocess patterns: "\\n" => "\n", etc.
          EXTENDED      = 0x0200, // ignore whitespace in pattern
      };
    */
    assert(pQE);
    assert(pQE->transformContext());
    assert(pXCtxt);

    static const char *sFunctionSignature = "regexp:replace(input, regexp, flags, replacement)"; UNUSED(sFunctionSignature);
    const char *sReplacement       = 0, 
               *sRegularExpression = 0, 
               *sInput             = 0, 
               *sFlags             = 0,
               *sFlag;
    char cFlag;

    //basic_string<char>, char_traits<char>, allocator<char>
    //using strings because we want to use substitute()
    string stInput;
    string stRegularExpression;
    string stReplacement;
    REGEX_FLAGS iFlags = NOFLAGS;
    subst_results mResultsS;

    UNWIND_EXCEPTION_BEGIN {
      //parameters (backwards) sInput, sRegularExpression, sReplacement [, sFlags]
      switch (pXCtxt->valueCount()) {
        case 4: 
          sReplacement        = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 3:
          //flags is 3rd as standard EXSLT
          //TODO: process string flags in to values
          //i,g
          sFlags              = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 2:
          sRegularExpression  = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          sInput              = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); 
          break;
        default: throw XPathArgumentCountWrong(this, sFunctionSignature);
      }
      
      if (sInput && sRegularExpression) {
        //match, regexp class definitions in regexpr2.h:789
        //using rpattern, instead of rpattern_c because we are using strings
        stInput             = sInput;
        stRegularExpression = sRegularExpression;
        stReplacement       = (sReplacement ? sReplacement: ""); //defaults to empty
        
        if (sFlags) {
          sFlag = sFlags;
          while (cFlag = *sFlag++) {
            if (cFlag == 'g') iFlags |= GLOBAL;
            if (cFlag == 'i') iFlags |= NOCASE;
            if (cFlag == 'n') iFlags |= NORMALIZE;
            if (cFlag == 'm') iFlags |= MULTILINE;
            if (cFlag == 's') iFlags |= SINGLELINE;
          }
        }
        
        rpattern rgx1(stRegularExpression, stReplacement, iFlags, regex::MODE_DEFAULT);
        rgx1.substitute(stInput, mResultsS);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sInput)             MM_FREE(sInput);
    if (sRegularExpression) MM_FREE(sRegularExpression);
    if (sFlags)             MM_FREE(sFlags);
    if (sReplacement)       MM_FREE(sReplacement);

    UNWIND_EXCEPTION_THROW;

    return MM_STRDUP(stInput.c_str());
  }

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- EXSLTStrings (str) ---------------------------------
  //-------------------------------------------------------------------------------------------
  const StringMap<IXslModule::XslModuleFunctionDetails> *EXSLTStrings::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("substring-after-last",  XMF(EXSLTStrings::xslFunction_substringAfterLast));
      m_XSLFunctions.insert("substring-before-last", XMF(EXSLTStrings::xslFunction_substringBeforeLast));
      m_XSLFunctions.insert("substring-after",       XMF(EXSLTStrings::xslFunction_substringAfter));
      m_XSLFunctions.insert("substring-before",      XMF(EXSLTStrings::xslFunction_substringBefore));
      m_XSLFunctions.insert("list",                  XMF(EXSLTStrings::xslFunction_list));
      m_XSLFunctions.insert("boolean",               XMF(EXSLTStrings::xslFunction_boolean));
      m_XSLFunctions.insert("not",                   XMF(EXSLTStrings::xslFunction_not));
      m_XSLFunctions.insert("dynamic",               XMF(EXSLTStrings::xslFunction_dynamic));
      m_XSLFunctions.insert("ends-with",             XMF(EXSLTStrings::xslFunction_endsWith));
      m_XSLFunctions.insert("escape",                XMF(EXSLTStrings::xslFunction_escape));
      m_XSLFunctions.insert("unescape",              XMF(EXSLTStrings::xslFunction_unescape));
    }
    return &m_XSLFunctions;
  }

  const char *EXSLTStrings::xslFunction_endsWith(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //always returns something, maybe a MM_STRDUP blank string
    //the [default] argument will be returned also if the input evaluates to an empty string
    assert(pQE);
    assert(pQE->transformContext());
    assert(pXCtxt);

    static const char *sFunctionSignature = "str:ends-with(string, end)"; UNUSED(sFunctionSignature);
    const char *sEnd    = 0,
               *sString = 0;
    bool bEndsWith      = false;
    size_t iString, iEnd;

    UNWIND_EXCEPTION_BEGIN {
      sEnd    = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sString = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      iEnd    = strlen(sEnd);
      iString = strlen(sString);

      bEndsWith = sString && sEnd
        && iString
        && iString > iEnd
        && _STREQUAL(sString + iString - iEnd, sEnd);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sEnd)    MM_FREE(sEnd);
    if (sString) MM_FREE(sString);

    UNWIND_EXCEPTION_THROW;

    return (bEndsWith ? MM_STRDUP("yes") : 0);
  }

  const char *EXSLTStrings::xslFunction_dynamic(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //always returns something, maybe a MM_STRDUP blank string
    //the [default] argument will be returned also if the input evaluates to an empty string
    assert(pQE);
    assert(pXCtxt);

    static const char *sFunctionSignature = "str:dynamic('str{xpath}str' [, default, recursive, write-source-on-error])";
    const char *sDynamicString    = 0,
               *sEmptyResult      = 0,
               *sDefault          = 0,
               *sResult           = 0;
    const IXmlBaseNode *pFromNode = 0;
    unsigned int iRecursions      = 1;
    bool bWriteSourceOnError      = true;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 4: bWriteSourceOnError = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();   ATTRIBUTE_FALLTHROUGH;
        case 3: iRecursions         = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack();   ATTRIBUTE_FALLTHROUGH;
        case 2: sDefault            = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: sDynamicString      = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        default: throw XPathArgumentCountWrong(this, sFunctionSignature);
      }
      
      //context node
      //if it is THE attribute then suggest sending the process to the parent node with an apply-templates
      if (pQE->transformContext()) pFromNode = pQE->transformContext()->sourceNode(pQE); //pQE as MM
      else                         pFromNode = pXCtxt->contextNode(pQE);                 //pQE as MM

      if (pFromNode) {
        try {
          sResult = pFromNode->valueDynamic(pQE, sDynamicString, iRecursions); //caller ONLY frees result IF not sDynamicString
        } catch (ExceptionBase &eb) {
          if (bWriteSourceOnError) {
            sResult = sDynamicString;
            Debug::warn("Failed to str:dynamic(). writing source instead");
          } else throw DynamicValueCalculation(eb, sDynamicString);
        }
      }

      //default return value
      if (!sResult || !*sResult) {
        if (sResult != sDynamicString) sEmptyResult = sResult;
        if (sDefault) sResult = sDefault;
        else          sResult = MM_STRDUP("");
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sDefault       && sResult != sDefault)       MM_FREE(sDefault);
    if (sDynamicString && sResult != sDynamicString) MM_FREE(sDynamicString);
    if (sEmptyResult   && sResult != sEmptyResult)   MM_FREE(sEmptyResult);
    if (pFromNode)                                   MMO_DELETE(pFromNode);

    UNWIND_EXCEPTION_THROW;

    return sResult;
  }

  const char *EXSLTStrings::xslFunction_substringAfterLast(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    assert(pQE);
    assert(pXCtxt);

    //reverse popping
    static const char *sFunctionSignature = "str:substring-after-last(str, delimeter [, not-found])"; UNUSED(sFunctionSignature);
    const char *sNotFound = 0,
               *sAfter    = 0,
               *sString   = 0,
               *sPos;
    char *sResult = 0;

    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 3) sNotFound = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sAfter  = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sString = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();

      if (sPos = strrstr(sString, sAfter)) sResult = MM_STRDUP(sPos + 1);
      else {
        if (sNotFound) sResult = MM_STRDUP(sNotFound);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sNotFound) MM_FREE(sNotFound);
    if (sAfter)    MM_FREE(sAfter);
    if (sString)   MM_FREE(sString);

    UNWIND_EXCEPTION_THROW;

    return sResult;
  }

  const char *EXSLTStrings::xslFunction_substringBeforeLast(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    assert(pQE);
    assert(pXCtxt);

    //reverse popping
    static const char *sFunctionSignature = "str:substring-before-last(str, delimeter [, not-found])"; UNUSED(sFunctionSignature);
    char *sResult = 0;
    const char *sNotFound = 0,
               *sBefore   = 0,
               *sString   = 0,
               *sPos;

    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 3) sNotFound = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sBefore = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sString = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();

      if (sPos = strrstr(sString, sBefore)) sResult = strndup(sString, sPos - sString);
      else {
        if (sNotFound) sResult = MM_STRDUP(sNotFound);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sNotFound) MM_FREE(sNotFound);
    if (sBefore)   MM_FREE(sBefore);
    if (sString)   MM_FREE(sString);

    UNWIND_EXCEPTION_THROW;

    return sResult;
  }

  const char *EXSLTStrings::xslFunction_substringAfter(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    assert(pQE);
    assert(pXCtxt);

    //the default parameter is returned if the delimeter is not found
    //the normal xsl:substring(str, delimeter) returns an empty string if not found
    static const char *sFunctionSignature = "str:substring-after(str, delimeter [, not-found])"; UNUSED(sFunctionSignature);
    const char *sNotFound = 0,
               *sAfter    = 0,
               *sString   = 0,
               *sPos;
    char *sResult = 0;

    //reverse popping
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 3) sNotFound = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sAfter    = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sString   = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();

      if (sPos = strstr(sString, sAfter)) sResult = MM_STRDUP(sPos + 1);
      else {
        if (sNotFound) sResult = MM_STRDUP(sNotFound);
        else           sResult = MM_STRDUP(sString);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sNotFound) MM_FREE(sNotFound);
    if (sAfter)    MM_FREE(sAfter);
    if (sString)   MM_FREE(sString);

    UNWIND_EXCEPTION_THROW;

    return sResult;
  }

  const char *EXSLTStrings::xslFunction_substringBefore(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    assert(pQE);
    assert(pXCtxt);

    //if the delimeter is not found: the INPUT or the not-found parameter is returned
    //the normal xsl:substring(str, delimeter) returns an empty string if not found
    static const char *sFunctionSignature = "str:substring-before(str, delimeter [, not-found])"; UNUSED(sFunctionSignature);
    const char *sNotFound = 0,
               *sBefore   = 0,
               *sString   = 0,
               *sPos;
    char *sResult = 0;

    //reverse popping
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 3) sNotFound = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sBefore   = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      sString   = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();

      if (sPos = strstr(sString, sBefore)) sResult = strndup(sString, sPos - sString);
      else {
        if (sNotFound) sResult = MM_STRDUP(sNotFound);
        else           sResult = MM_STRDUP(sString);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sNotFound) MM_FREE(sNotFound);
    if (sBefore)   MM_FREE(sBefore);
    if (sString)   MM_FREE(sString);

    UNWIND_EXCEPTION_THROW;

    return sResult;
  }

  const char *EXSLTStrings::xslFunction_list(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //e.g.
    //  str:list(.../object:Database) => dynamics,general_server
    //  str:list(class:derived-classes()) => CSS,ObjectClass,HTMLObject,BaseObject
    //GDB: *((LibXslXPathFunctionContext*) pXCtxt)->m_ctxt
    static const char *sFunctionSignature = "str:list(node-set [, string delimiter=[,], value-xpath=local-name()])";
    string stList;
    const char *sValueXPath = 0,
               *sDelimiter  = 0,
               *sValue      = 0;
    char cValueXPathType    = 0;
    XmlNodeList<IXmlBaseNode> *pvNodeList = 0;
    XmlNodeList<IXmlBaseNode>::const_iterator iNode;
    const IXmlBaseNode *pNode;

    switch (pXCtxt->valueCount()) {
      case 3: sValueXPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
      case 2: sDelimiter  = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
      case 1: pvNodeList  = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
        break;
      default: throw XPathArgumentCountWrong(this, sFunctionSignature);
    }

    //xpath type classification if any
    if (sValueXPath) {
      if      (*sValueXPath == '@')               cValueXPathType = '@';
      //else if (_STREQUAL(sValueXPath, "local-name()")) cValueXPathType = 0;
      else if (_STREQUAL(sValueXPath, "name()"))  cValueXPathType = 'n';
      else if (_STREQUAL(sValueXPath, "xpath()")) cValueXPathType = 'x';
      //else default case 0: local-name()
    }

    for (iNode = pvNodeList->begin(); iNode != pvNodeList->end(); iNode++) {
      pNode = *iNode;
      //get the value for this node
      switch (cValueXPathType) {
        case 0: { //local-name() e.g. class:Person => Person
          sValue = pNode->localName();
          break;
        }
        case '@': { //e.g. @name
          sValue = pNode->attributeValue(pQE, sValueXPath+1);
          break;
        }
        case 'x': { //xpath
          sValue = pNode->uniqueXPathToNode(pQE);
          break;
        }
        default: NOT_COMPLETE("str:list x");
      }
      if (sValue) {
        if (stList.size()) stList += (sDelimiter ? sDelimiter : ",");
        stList += sValue;
        MM_FREE(sValue);
      }
    }

    //free up
    if (sValueXPath) MM_FREE(sValueXPath);
    if (sDelimiter)  MM_FREE(sDelimiter);
    //if (sValue)      MM_FREE(sValue); //freed in the loop
    if (pvNodeList)  delete pvNodeList->element_destroy();

    return MM_STRDUP(stList.c_str());
  }

  const char *EXSLTStrings::xslFunction_not(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //GDB: *((LibXslXPathFunctionContext*) pXCtxt)->m_ctxt
    static const char *sFunctionSignature = "str:not(object [, boolean default if resolves to empty string])";
    const char *sValue = 0;
    bool bRet = true; //by default str:not(@attribute) will return true() if the attribute is not present
    bool bDefaultIfResolvesToEmptyString = bRet;

    //uses xpath.c xmlXPathCastToString(xmlXPathObjectPtr val)
    //to convert fragments etc.
    switch (pXCtxt->valueCount()) {
      case 2: bDefaultIfResolvesToEmptyString = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
      case 1: sValue                          = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        break;
      default: throw XPathArgumentCountWrong(this, sFunctionSignature);
    }

    if (!sValue || !*sValue) bRet = bDefaultIfResolvesToEmptyString;
    else                     bRet = !pQE->xmlLibrary()->textEqualsTrue(sValue);

    if (sValue && strlen(sValue) > 20) Debug::report("str:not value is excessively long\n  [%s]", pXCtxt->xpathCurrent());

    //free up
    if (sValue) MM_FREE(sValue);

    //TODO: allow return of booleans from xpath functions
    return (bRet ? MM_STRDUP("yes") : 0);
  }

  const char *EXSLTStrings::xslFunction_escape(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //GDB: *((LibXslXPathFunctionContext*) pXCtxt)->m_ctxt
    static const char *sFunctionSignature = "str:escape([string])";
    const char *sEscape      = 0,
               *sValue       = 0;
    const IXmlBaseNode *pContextNode = 0;

    switch (pXCtxt->valueCount()) {
      case 1: sValue       = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        break;
      case 0:
        pContextNode = pXCtxt->contextNode(pQE);
        sValue       = pContextNode->value(pQE);
        break;
      default: throw XPathArgumentCountWrong(this, sFunctionSignature);
    }

    sEscape = pQE->xmlLibrary()->escape(sValue);
      
    //free up
    if (sValue && sEscape != sValue) MM_FREE(sValue);
    if (pContextNode) MM_DELETE(pContextNode);

    return sEscape;
  }

  const char *EXSLTStrings::xslFunction_unescape(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //GDB: *((LibXslXPathFunctionContext*) pXCtxt)->m_ctxt
    static const char *sFunctionSignature = "str:unescape([string])";
    const char *sUnEscape    = 0,
               *sValue       = 0;
    const IXmlBaseNode *pContextNode = 0;

    switch (pXCtxt->valueCount()) {
      case 1: sValue       = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        break;
      case 0:
        pContextNode = pXCtxt->contextNode(pQE);
        sValue       = pContextNode->value(pQE);
        break;
      default: throw XPathArgumentCountWrong(this, sFunctionSignature);
    }

    sUnEscape = pQE->xmlLibrary()->unescape(sValue);
      
    //free up
    if (sValue && sUnEscape != sValue) MM_FREE(sValue);
    if (pContextNode) MM_DELETE(pContextNode);

    return sUnEscape;
  }

  const char *EXSLTStrings::xslFunction_boolean(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //GDB: *((LibXslXPathFunctionContext*) pXCtxt)->m_ctxt
    //string("1") indicates true
    //string("")  indicates false
    assert(pQE);
    assert(pXCtxt);

    //XSL boolean():
    //  a number is true if and only if it is neither positive or negative zero nor NaN,
    //  a node-set is true if and only if it is non-empty,
    //  a string is true if and only if its length is non-zero
    //XSL not():
    //  The not function returns true if its argument is false, and false otherwise
    //  not('0')     = false
    //  not('false') = false
    //  not('')      = true
    //  not(0)       = true
    //  not(empty node-set) = true
    //Database:boolean()
    //  processes the value of tree fragments and node-sets by their text value
    //  converts numbers also to text first
    //  valid true strings:
    //    '1'    = true
    //    'yes'  = true
    //    'on'   = true
    //    'true' = true
    //  everything else is false:
    //    '0'    = false
    //    'off'  = false
    //    'false'= false
    //    ''     = false (unless a true() default is sent through as the second argument)
    //Default modifier:
    //  str:boolean(@missing-attribute, true())  = true()
    //  str:boolean(@missing-attribute, false()) = false()

    static const char *sFunctionSignature = "str:boolean(object [, boolean default if resolves to empty string])";
    const char *sValue = 0;
    bool bRet = false; //by default str:boolean(@attribute) will return false() if the attribute is not present
    bool bDefaultIfResolvesToEmptyString = false;

    //uses xpath.c xmlXPathCastToString(xmlXPathObjectPtr val)
    //to convert fragments etc.
    switch (pXCtxt->valueCount()) {
      case 2: {
        if (pXCtxt->xtype() != XPATH_BOOLEAN) throw XPathBooleanArgumentRequired(this, sFunctionSignature);
        bDefaultIfResolvesToEmptyString = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
        ATTRIBUTE_FALLTHROUGH;
      }
      case 1: {
        sValue = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        break;
      }
      default: throw XPathArgumentCountWrong(this, sFunctionSignature);
    }

    if (!sValue || !*sValue) bRet = bDefaultIfResolvesToEmptyString;
    else                     bRet = pQE->xmlLibrary()->textEqualsTrue(sValue);

    if (sValue && strlen(sValue) > 20) {
      Debug::report("str:boolean value is excessively long in command [%s] on [%s]\n  [%s]\n", (pXCtxt->xpathCurrent() ? pXCtxt->xpathCurrent() : ""), (pQE->currentXSLCommandXPath() ? pQE->currentXSLCommandXPath() : ""), sValue);
    }

    //free up
    if (sValue) MM_FREE(sValue);

    //TODO: allow return of booleans from xpath functions
    return (bRet ? MM_STRDUP("yes") : 0);
  }


  //-------------------------------------------------------------------------------------------
  //-------------------------------------- XSLTCustomUtilities --------------------------------
  //-------------------------------------------------------------------------------------------
  const StringMap<IXslModule::XslModuleFunctionDetails> *XSLTCustomUtilities::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("if",                    XMF(XSLTCustomUtilities::xslFunction_if));
      m_XSLFunctions.insert("first-set-not-empty",   XMF(XSLTCustomUtilities::xslFunction_firstSetNotEmpty));
      m_XSLFunctions.insert("stack",                 XMF(XSLTCustomUtilities::xslFunction_stack));
      m_XSLFunctions.insert("on-error",              XMF(XSLTCustomUtilities::xslFunction_onError));
      m_XSLFunctions.insert("current-mode-name",     XMF(XSLTCustomUtilities::xslFunction_currentModeName));
    }
    return &m_XSLFunctions;
  }
  
  const char *XSLTCustomUtilities::xslFunction_currentModeName(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    return (pQE->transformContext() ? pQE->transformContext()->currentModeName() : 0);
  }

  const char *XSLTCustomUtilities::xslFunction_onError(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    assert(pQE);
    assert(pXCtxt);

    NOT_COMPLETE("xslFunction_onError"); //XSLTCustomUtilities::xslFunction_onError()

    static const char *sFunctionSignature = "flow:on-error(whatever)"; UNUSED(sFunctionSignature); UNUSED(sFunctionSignature);
    const char *sWhatever = 0;

    UNWIND_EXCEPTION_BEGIN {
      sWhatever = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      /*
      if (pvStylesheetNode = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack()) {
      }
      (*pNodes)->push_back(pNode);
      */
      cout << "XSLTCustomUtilities::xslFunction_onError\n";
    } UNWIND_EXCEPTION_END;

    //free up
    if (sWhatever) MM_FREE(sWhatever);

    UNWIND_EXCEPTION_THROW;

    return MM_STRDUP("whatever");
  }

  const char *XSLTCustomUtilities::xslFunction_if(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //note that both true and false will be evaluated before this function is called
    //this is NOT lazy evaluation
    assert(pQE);
    assert(pXCtxt);

    static const char *sFunctionSignature = "flow:if((bool) test, (node-set) true [, (node-set) false])";
    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0,
                                    *pvFalse   = 0,
                                    *pvTrue    = 0;
    bool bTest;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3: pvFalse = (XmlNodeList<const IXmlBaseNode>*) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: pvTrue  = (XmlNodeList<const IXmlBaseNode>*) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          break;
        default: throw XPathArgumentCountWrong(this, sFunctionSignature);
      }
      if (!pvFalse) pvFalse = new XmlNodeList<const IXmlBaseNode>(pQE); //pQE as MM
      bTest = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();

      if (bTest) pNewNodes = pvTrue;
      else       pNewNodes = pvFalse;
    } UNWIND_EXCEPTION_END;

    //free up
    if (pvTrue  && pvTrue  != pNewNodes) delete pvTrue->element_destroy();
    if (pvFalse && pvFalse != pNewNodes) delete pvFalse->element_destroy();
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *XSLTCustomUtilities::xslFunction_firstSetNotEmpty(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //flow:first-set-not-empty will ignore non node-set parameters
    assert(pQE);
    assert(pXCtxt);

    static const char *sFunctionSignature = "flow:first-set-not-empty((node-set) a, (node-set) b [, (node-set) x ...])";
    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    XmlNodeList<const IXmlBaseNode> *pNodeSet  = 0;

    UNWIND_EXCEPTION_BEGIN {
      //iterate input node-sets checking for non-empty
      for (int i = pXCtxt->valueCount(); i > 0; i--) {
        if (pXCtxt->xtype() == XPATH_NODESET) {
          pNodeSet = (XmlNodeList<const IXmlBaseNode>*) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          if (pNodeSet && pNodeSet->size()) {
            if (pNewNodes) delete pNewNodes->element_destroy();
            pNewNodes = pNodeSet;
          } else delete pNodeSet; //has no elements
        } else {
          Debug::report("non node-set argument for [%s]", sFunctionSignature);
          pXCtxt->popIgnoreFromXPathFunctionCallStack();
        }
      }

      //if all parameters are empty:
      //return an empty node set so that the return is correct
      if (!pNewNodes) pNewNodes = new XmlNodeList<const IXmlBaseNode>(pQE); //pQE as MM
    } UNWIND_EXCEPTION_END;

    //free up
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);
    
    UNWIND_EXCEPTION_THROW;
    
    *pNodes = pNewNodes;
    return 0;
  }
  
  const char *XSLTCustomUtilities::xslFunction_stack(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    assert(pQE);
    assert(pXCtxt);

    //returns the outer context node in the current context node-set for the tabbed contexts surrounding the function.
    //for example:
    //  f1(f2(f3(flow:stack(1))))
    //  would return the node-set f2() was currently processing
    //similar to current() which returns the very outer context node that the xpath eval is running on
    static const char *sFunctionSignature = "flow:stack((int) tab)"; UNUSED(sFunctionSignature);
    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(pQE); //pQE as MM
    IXmlBaseNode *pNode = 0;
    int iTab;

    UNWIND_EXCEPTION_BEGIN {
      iTab = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack();
      //the associated temporary IXmlDoc of this node will be released in LibXslModule
      if (pNode = pXCtxt->stackNode(pQE, iTab)) pNewNodes->push_back(pNode); //pQE as MM
    } UNWIND_EXCEPTION_END;
    
    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }
}
