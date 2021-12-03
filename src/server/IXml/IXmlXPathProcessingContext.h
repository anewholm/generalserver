//platform agnostic file
#ifndef _IXMLXPATHPROCESSINGCONTEXT_H
#define _IXMLXPATHPROCESSINGCONTEXT_H

#include "define.h"
#include "Utilities/StringMap.h"
#include "IXml/IXmlQueryEnvironment.h"          //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //inheritance
#include "IXml/IXmlLibrary.h"                   //xmlTraceFlag
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  interface_class IXslTransformContext;
  interface_class IXmlQueryEnvironment;
  class StringVector;

  interface_class IXmlXPathProcessingContext: implements_interface IXmlQueryEnvironmentComponent {
  public:
    virtual ~IXmlXPathProcessingContext() {}

    virtual IXmlXPathProcessingContext *inherit() const = 0;
    
    //EXECUTION phase:
    //during xpath evaluation, including compiled xsl:template/@match evaluation
    //we include matches for base classes of the sClause
    virtual IXmlQueryEnvironment::xprocessorResult checkName(const IXmlBaseNode *pCur, const char *sHREF, const char *sLocalName) const = 0;
    
    //COMPILATION phase:
    //fast lookup grouping of compiled templates by base @match name
    //e.g. if @match=object:Person, xsl:template is placed under "Person" key
    //i.e. if the node = object:Person, evaluate all xsl:template/@match for base @match name "Person"
    //we include the xsl:template/@match for base name "User" here as well
    //all relevant templates are then filtered by a full @match xpath evaluation
    virtual StringVector *templateSuggestionsForLocalName(const char *sClause) const = 0;
  };
}

#endif
