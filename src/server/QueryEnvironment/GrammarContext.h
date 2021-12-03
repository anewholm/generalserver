//platform agnostic file
#ifndef _GRAMMARCONTEXT_H
#define _GRAMMARCONTEXT_H

#include "define.h"
#include "IXml/IXmlGrammarContext.h" //direct inheritance
#include "Utilities/StringMap.h"
#include "Xml/XmlBaseNode.h"
#include "Xml/XmlNodeMap.h"

#include <vector>
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;
  interface_class IXmlLibrary;
  interface_class IXslModuleManager;
  interface_class IXmlQueryEnvironment;
  class Database;

  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  class GrammarContext: public IXmlGrammarContext {
    bool m_bEnabled;
    const IXmlQueryEnvironment *m_pOwnerQE;

    //only these can create this
    friend class Database;
    friend class QueryEnvironment;
    friend class CompilationEnvironment;
    friend class XmlAdminQueryEnvironment;
    friend class DatabaseAdminQueryEnvironment;
    GrammarContext();
    GrammarContext(const GrammarContext& trig);
    GrammarContext *clone_with_resources() const;
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}

    const IXmlLibrary *xmlLibrary() const {return m_pOwnerQE->xmlLibrary();}
    const char *process(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXPathContext, const char *sXPath, int *piSkip, IXmlLibrary::xmlTraceFlag iTraceFlags);
    char *testStatic(const char *sNameStart, const char *sTest, const char *sReplacementXPath, int *piSkip) const;

  public:
    const char   *toString() const;
    const char   *type() const;
    IXmlGrammarContext *inherit() const;

    //trigger control
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}

    bool maybeXPath(const IXmlQueryEnvironment *pQE, const char *sText, const size_t iLen = 0) const;
    bool maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE, const char *sText) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
