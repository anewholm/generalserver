//platform specific file (UNIX)
#ifndef _LIBXSLXPATHFUNCTIONCONTEXT_H
#define _LIBXSLXPATHFUNCTIONCONTEXT_H

//this file has been split out because of the LibXmlBase.cpp dependency on it
//  LibXsl.cpp -> LibXsl.h -> LibXml.h -> LibXmlBase.cpp which uses LibXslTransformContext directly in code
#include "Xml/XslDoc.h"       //full definition of XslTransformContext for inheritance
#include "LibXslModule.h" //various LibXsl libraries
#include "Utilities/VirtualWrapperClass.h"

#include <libxml/xmlgrammarprocessor.h>
#include <libxml/xmlprocessor.h>

using namespace std;

namespace general_server {
  class LibXslXPathFunctionContext: public XslXPathFunctionContext {
    xmlXPathParserContextPtr m_ctxt;
    const bool m_bOurs;
    const int m_nargs;

    //workers
    const char *xmlXPathObjectType(xmlXPathObjectPtr oXPathObject) const;
    //context setup stages
    void setContextNode(               xmlXPathContextPtr oXCtxt, const IXmlBaseNode *pCurrentNode) const;
    void registerExternalContext(      xmlXPathContextPtr oXCtxt, void *oExternalContext) const;
    void registerXPathFunctionLookup(  xmlXPathContextPtr oXCtxt) const;
    void registerXSLTVariableLookup(   xmlXPathContextPtr oXCtxt, void *oExternalContext) const;
    void registerXSLTFunctionLookup(   xmlXPathContextPtr oXCtxt) const;
    void registerXPathNamespaces(      xmlXPathContextPtr oXCtxt, const IXmlBaseDoc *pDoc) const;
    void setQueryEnvironmentComponents(xmlXPathContextPtr oXCtxt, const IXmlQueryEnvironment *pQE) const;

    //construction
    friend class LibXslModule; friend class LibXmlLibrary; //only LibXmlLibrary and LibXslModule can create these
    LibXslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0);
    LibXslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, xmlXPathParserContextPtr ctxt, const int nargs = 0);

  public:
    LibXslXPathFunctionContext(const LibXslXPathFunctionContext& lxxc); //copy constructor: clone_with_resources(*this)
    ~LibXslXPathFunctionContext();
    LibXslXPathFunctionContext *clone_with_resources()  const;
    IXslXPathFunctionContext   *inherit() const;

    static const xmlChar *xmlExternalGrammarProcessorCallback(xmlGrammarProcessorCallbackContextPtr oCtxt, xmlNodePtr *context, xmlListPtr *parent_route, int *piSkip, int iTraceFlags);
    static int            xmlExternalXPathProcessorCallback(  xmlXPathProcessingCallbackContextPtr  oCtxt, xmlNodePtr cur, xmlListPtr oParentRoute, int test_type, const xmlChar *sLocalName, const xmlChar *sPrefix, const char ***pvsGroupings);

    //platform specific accessors
    xmlXPathContextPtr         libXmlXPathContext()       const {return m_ctxt->context;}
    xmlXPathParserContextPtr   libXmlXPathParserContext() const {return m_ctxt;}

    //xpath evaluation environment
    iXPathParamType xtype()         const;
    const char *xpath()             const;          //parserCtxt: full xpath statment
    const char *xpathCurrent()      const;          //parserCtxt: current part of the xpath being evaluated
    IXmlBaseNode *stackNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iTab = 0) const; //parserCtxt: node for the outer context of this function resolution
    IXmlBaseNode *contextNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner)     const;          //parserCtxt: checks stackNode(), returns currentNode() if not
    IXmlBaseNode *currentNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner)     const;          //parserCtxt: context->node
    const IXslCommandNode *commandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner)  const;    //parserCtxt: context->inst
    const IXmlBaseNode *literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const; //parserCtxt: context->inst
    int   valueCount()        const;          //parserCtxt: current function parameters count
    const char *functionName()      const;          //xpathCtxt:  current function name (used by EMO for late binding)
    const char *functionNamespace() const;          //xpathCtxt:  current function NS   (used by EMO for late binding)

    //writeable
    const IXmlBaseNode *contextNode(const IXmlBaseNode *pNewConextNode);
    
    //parameter access
    void                   popIgnoreFromXPathFunctionCallStack() ;
    const char            *popInterpretCharacterValueFromXPathFunctionCallStack() ;
    double                 popInterpretDoubleValueFromXPathFunctionCallStack() ;
    int                    popInterpretIntegerValueFromXPathFunctionCallStack() ;
    bool                   popInterpretBooleanValueFromXPathFunctionCallStack() ;
    struct tm              popInterpretDateFromXPathFunctionCallStack() ;
    XmlNodeList<IXmlBaseNode> *popInterpretNodeListFromXPathFunctionCallStack() ;
    //DatabaseClass         *popInterpretDatabaseClassFromXPathFunctionCallStack() ; //on the DatabaseClass!

    //debugging
    const char *currentParentRoute()      const;
  };
}

#endif
