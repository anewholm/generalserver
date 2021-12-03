//platform agnostic
#include "GrammarContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslDoc.h"
#include "Database.h"
#include "DatabaseClass.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"

namespace general_server {
  GrammarContext::GrammarContext():
    m_bEnabled(true),
    m_pOwnerQE(0)
  {}

  GrammarContext::GrammarContext(const GrammarContext& gram):
    m_bEnabled(gram.m_bEnabled),
    m_pOwnerQE(0)
  {}

  void GrammarContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}

  const char *GrammarContext::toString() const {
    return MM_STRDUP("GrammarContext");
  }
  const char *GrammarContext::type() const {return "GrammarContext";}

  GrammarContext *GrammarContext::clone_with_resources() const {
    return new GrammarContext(*this);
  }

  IXmlGrammarContext *GrammarContext::inherit() const {
    return clone_with_resources();
  }

  bool GrammarContext::maybeXPath(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, const char *sText, const size_t iLen) const {
    return sText && (
         (iLen ? strnchr(sText, '~', iLen) : strchr(sText, '~')) //class names: see the grammar processors
      || (iLen ? strnchr(sText, '`', iLen) : strchr(sText, '`')) //session:node-set(x)
      || (iLen ? strnchr(sText, '^', iLen) : strchr(sText, '^')) //id(...) shorthand
    );
  }
  
  bool GrammarContext::maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, const char *sText) const {
    char c = *sText;
    return sText && (
         (c == '~')  //class names: see the grammar processors
      || (c == '`')  //session:node-set(x)
      || (c == '^')  //id(...) shorthand
    );
  }
  
  const char *GrammarContext::process(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXslXPathFunctionContext *pXPathContext ATTRIBUTE_UNUSED, const char *sXPath, int *piSkip, IXmlLibrary::xmlTraceFlag iTraceFlags) {
    assert(sXPath);

    char *sReplacementXPath = 0;
    char cIdentifier;
    const char *sNameStart  = 0,
               *sNameEnd    = 0,
               *sValue      = 0;
    size_t iNameLen;
    const IXmlBaseNode *pContextNode = 0;

    cIdentifier = sXPath[0];
    sNameStart  = (cIdentifier ? sXPath + 1 : 0);
    switch (cIdentifier) {
      case '!': { //----------------------------------------------- !(thing/@y)/x/z => dyn:evaluate(thing/@y)/x/z
        sReplacementXPath = MM_STRDUP("dyn:evaluate");

        /* ATTRIBUTE_UNUSED
        const IXmlBaseNode *pNewContextNode;
        NOT_COMPLETE("");
        
        pContextNode    = pXPathContext->contextNode(pQE); //pQE as MM
        sValue          = pContextNode->value(pQE);
        pNewContextNode = pContextNode->getSingleNode(pQE, sValue);
        pXPathContext->contextNode(pNewContextNode); //NOT_COMPLETE
        */
        
        *piSkip         = 1;
        break;
      }
      case '`': { //----------------------------------------------- `x session:node-set(x)
        sNameEnd = sNameStart;
        while (isdigit(*sNameEnd)) sNameEnd++;
        iNameLen    = sNameEnd - sNameStart;
        sReplacementXPath = MM_MALLOC_FOR_RETURN(17 + iNameLen + 1 + 1);
        strcpy( sReplacementXPath, "session:node-set(");
        strncpy(sReplacementXPath + 17, sNameStart, iNameLen);
        strcpy( sReplacementXPath + 17 + iNameLen, ")");
        *piSkip = iNameLen + 1;
        break;
      }
      case '~': { //----------------------------------------------- Class expression ~<Name> => id('Class__<Name>')
        //  ~Person == id('Class__Person')
        //  ~* returns all class nodes
        if (*sNameStart == '*') {
          //DatabaseClass::allClassNodes()
          sReplacementXPath = MM_STRDUP("");
          NOT_COMPLETE("");
        } else {
          sNameEnd = sNameStart;
          //https://www.w3.org/TR/REC-xml/#NT-Name
          while (xmlLibrary()->isNameChar(*sNameEnd)) sNameEnd++;
          iNameLen          = sNameEnd - sNameStart;
          sReplacementXPath = DatabaseClass::classIDXPathFromName(sNameStart, iNameLen); 
          *piSkip = iNameLen + 1;
        }
        break;
      }
      case '^': { //----------------------------------------------- ^services short-hand for id(...)
        //TODO: need to escape xml:id
        //#defines should be brought in to $variables
        //e.g. $NAMESPACE_*
        sNameEnd = sNameStart;
        //https://www.w3.org/TR/REC-xml/#NT-Name
        while (xmlLibrary()->isNameChar(*sNameEnd)) sNameEnd++;
        iNameLen          = sNameEnd - sNameStart;
        sReplacementXPath = MM_MALLOC_FOR_RETURN(iNameLen + 6 + 1);
        strcpy( sReplacementXPath + 0, "id('");
        strncpy(sReplacementXPath + 4, sNameStart, iNameLen);
        strcpy( sReplacementXPath + 4 + iNameLen, "')");
        *piSkip = iNameLen + 1;
        break;
      }
      //NOTE: update maybeXPath() with any new replacements
    }

    if (!sReplacementXPath && false) {
      //regex lookup from extendable server configuration nodes
      //PERFORMANCE: cache it all at the beginning
      NOT_COMPLETE("");
    }

    if (iTraceFlags & XML_DEBUG_XPATH_EXPR) {
      if (sReplacementXPath) Debug::report("GrammarContext [%s] => [%s]", sXPath, sReplacementXPath);
      else                   Debug::report("GrammarContext [%s] no replacement", sXPath, rtWarning);
    }
    
    //free up
    //if (sNameStart)   MM_FREE(sNameStart); //pointers in to sXPath
    //if (sNameEnd)     MM_FREE(sNameEnd);   //pointers in to sXPath
    if (pContextNode) delete pContextNode;
    if (sValue)       MM_FREE(sValue);

    return sReplacementXPath;
  }

  char *GrammarContext::testStatic(const char *sNameStart, const char *sTest, const char *sReplacementXPath, int *piSkip) const {
    char *sRet = 0;
    size_t iLen = strlen(sTest);

    if (!strncmp(sNameStart, sTest, iLen)) {
      *piSkip = iLen + 1;
      sRet = MM_STRDUP(sReplacementXPath);
    }
    return sRet;
  }
}
