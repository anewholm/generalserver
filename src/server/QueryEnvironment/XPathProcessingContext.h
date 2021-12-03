//platform agnostic file
#ifndef _XPATHPROCESSINGCONTEXT_H
#define _XPATHPROCESSINGCONTEXT_H

#include "define.h"
#include "IXml/IXmlXPathProcessingContext.h" //direct inheritance
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
  class XPathProcessingContext: public IXmlXPathProcessingContext {
    bool m_bEnabled;
    const IXmlQueryEnvironment *m_pOwnerQE;

    //only these can create this
    friend class QueryEnvironment;
    friend class CompilationEnvironment;
    friend class XmlAdminQueryEnvironment;
    friend class DatabaseAdminQueryEnvironment;
    XPathProcessingContext();
    XPathProcessingContext(const XPathProcessingContext& trig);
    XPathProcessingContext *clone_with_resources() const;
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}

    const IXmlLibrary *xmlLibrary() const {return m_pOwnerQE->xmlLibrary();}
    IXmlQueryEnvironment::xprocessorResult checkName(const IXmlBaseNode *pCur, const char *sPrefix, const char *sLocalName) const;
    StringVector *templateSuggestionsForLocalName(const char *sClause) const;

  public:
    const char   *toString() const;
    const char   *type() const;
    IXmlXPathProcessingContext *inherit() const;

    //trigger control
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
