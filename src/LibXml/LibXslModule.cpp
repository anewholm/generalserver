//platform specific file (UNIX)
#include "LibXslModule.h"

#include "LibXmlDoc.h"
#include "LibXmlNode.h"
#include "LibXslDoc.h"
#include "LibXslNode.h"
#include "LibXmlLibrary.h"
#include "Server.h"
#include "Exceptions.h"
#include "Utilities/container.c"

#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

//http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
#include <libxslt/extensions.h>
#include <libxslt/extra.h>
#include <libxslt/templates.h>
//#include "XSLTExtensions.h"

using namespace std;

namespace general_server {
  HRESULT LibXslModule::xmlLibraryRegisterXslCommand(const char *xsltModuleNamespace, const char *sCommandName) const {
    //http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
    xsltRegisterExtModuleElement((const xmlChar*) sCommandName, (const xmlChar*) xsltModuleNamespace, NULL, xslCall);
    return SUCCEEDED(0);
  }

  HRESULT LibXslModule::xmlLibraryRegisterXslFunction(const char *xsltModuleNamespace, const char *sFunctionName) const {
    //http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
    xsltRegisterExtModuleFunction((const xmlChar*) sFunctionName, (const xmlChar*) xsltModuleNamespace, xslFunctionCall);
    return SUCCEEDED(0);
  }

  void LibXslModule::xslFunctionCall(xmlXPathParserContextPtr ctxt, int nargs) {
    //from installations/libxml2-2.7.8/include/libxml/xpath.h
    //GDB: p pQE->currentXSLCommandXPath()
    //GDB: p pQE->currentSourceNodeXPath()
    
    /*
    struct _xmlXPathParserContext {
        const xmlChar *cur;           // the current char being parsed
        const xmlChar *base;          // the full expression

        int error;                    // error code

        xmlXPathContextPtr  context;  // the evaluation context
        xmlXPathObjectPtr     value;  // the current value
        int                 valueNr;  // number of values stacked
        int                valueMax;  // max number of values stacked
        xmlXPathObjectPtr *valueTab;  // stack of values

        xmlXPathCompExprPtr comp;     // the precompiled expression
        int xptr;                     // it this an XPointer expression
        xmlNodePtr         ancestor;  // used for walking preceding axis
    };

    struct _xmlXPathContext {
      xmlDocPtr doc : The current document
      xmlNodePtr  node  : The current node
      int nb_variables_unused : unused (hash table)
      int max_variables_unused  : unused (hash table)
      xmlHashTablePtr varHash : Hash table of defined variables
      int nb_types  : number of defined types
      int max_types : max number of types
      xmlXPathTypePtr types : Array of defined types
      int nb_funcs_unused : unused (hash table)
      int max_funcs_unused  : unused (hash table)
      xmlHashTablePtr funcHash  : Hash table of defined funcs
      int nb_axis : number of defined axis
      int max_axis  : max number of axis
      xmlXPathAxisPtr axis  : Array of defined axis the namespace nod
      xmlNsPtr *  namespaces  : Array of namespaces
      int nsNr  : number of namespace in scopectxt
      void *  user  : function to free extra variables
      int contextSize : the context size
      int proximityPosition : the proximity position extra stuff for
      int xptr  : is this an XPointer context?
      xmlNodePtr  here  : for here()
      xmlNodePtr  origin  : for origin() the set of namespace decla
      xmlHashTablePtr nsHash  : The namespaces hash table
      xmlXPathVariableLookupFunc  varLookupFunc : variable lookup func
      void *  varLookupData : variable lookup data Possibility to lin
      void *  extra : needed for XSLT The function name and U
      const xmlChar * function
      const xmlChar * functionURI : function lookup function and data
      xmlXPathFuncLookupFunc  funcLookupFunc  : function lookup func
      void *  funcLookupData  : function lookup data temporary namespac
      xmlNsPtr *  tmpNsList : Array of namespaces
      int tmpNsNr : number of namespaces in scope error rep
      void *  userData  : user specific data block
      xmlStructuredErrorFunc  error : the callback in case of errors
      xmlError  lastError : the last error
      xmlNodePtr  debugNode : the source node XSLT dictionary
      xmlDictPtr  dict  : dictionary if any
      int flags : flags to control compilation Cache for
      void *  cache

      //Annesley additions
      xmlNodeFilterCallbackContextPtr xfilter;
      void *emo;
      void *externalContext;
    }
    struct _xmlXPathObject {
      xmlXPathObjectType  type
      xmlNodeSetPtr nodesetval
      int boolval
      double  floatval
      xmlChar * stringval
      void *  user
      int index
      void *  user2
      int index2
    }
    */
    assert(ctxt && ctxt->context);

    const IXmlQueryEnvironment *pQE         = 0;
    IXslXPathFunctionContext *pWrappedXPathContext = 0;
    const char *sResult                     = 0;
    XmlNodeList<const IXmlBaseNode> *pNodes = 0;
    UNWIND_EXCEPTION_DECLARE;

    //abstract in to IXML asap
    //emo(), functionNamespace() and functionName() are direct pointers in to an existing persistent object: do not free
    if (pQE   = (const IXmlQueryEnvironment*) ctxt->context->emo) { //optional: no EMO, no party
      UNWIND_EXCEPTION_TRY {
        assert( ctxt
          && ctxt->context
          && ctxt->context->emo
          && ctxt->context->function
          && ctxt->context->functionURI
        );
        //wrap this new context specific to the current situation
        //send it through the QueryEnvironment, which details the entire (changing) situation
        //we have to send nargs through because the valueTab includes the outer context node-sets during predicate resolution
        pWrappedXPathContext = new LibXslXPathFunctionContext(pQE, pQE, ctxt, nargs); //pQE as MM

        if (!pQE->emoContext()) throw NoEMOAvailableForExtension(pQE, (const char*) ctxt->context->function); //pQE as MM
        //GDB: p pQE->currentXSLCommandXPath()
        //GDB: p pQE->currentSourceNodeXPath()
        sResult = pQE->emoContext()->runXslModuleFunction(pQE, pWrappedXPathContext, (const char*) ctxt->context->functionURI, (const char*) ctxt->context->function, &pNodes);
        if (pNodes) {
          //node set returned
          assert(!sResult);
          xmlXPathReturnNodeSet(ctxt, LibXmlLibrary::xPathNodeSetCreate(pQE, pNodes, CREATE_IFNULL)); //pQE as MM
        } else if (sResult) {
          //TODO: does the LibXML2 system xmlFree(sResult) xpath return?
          xmlXPathReturnString(ctxt, (xmlChar*) sResult);
          //TextNode option (causes issues with existing boolean checking of strings etc.):
          //oNodes = xmlXPathNodeSetCreate(NULL, NULL);
          //xmlXPathNodeSetAdd(oNodes, xmlNewDocText(ctxt->context->doc, (const xmlChar*) sResult), NULL);
          //xmlXPathReturnNodeSet(ctxt, oNodes);
        } else {
          xmlXPathReturnFalse(ctxt);
        }
      } UNWIND_EXCEPTION_END_STATIC(pQE);
    }

    //free up
    //GDB: p pQE->currentXSLCommandXPath()
    //the nodes returned are always tmp nodes in the tmp area of the main Database
    //the tmp node document() is the main Database document_const() and does not require deletion
    //TODO: and tmp() nodes are cleaned up separately (at end of transform?)
    //if (pNode->document()) delete pDoc; //shared
    //it is the responsibility of the CREATOR of pNodes to delete them in the case of an EXCEPTION
    //  and the responsibility of the caller in the case of no exception
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_NOEXCEPTION(pNodes); //nodes array
    if (pWrappedXPathContext) delete pWrappedXPathContext;

    UNWIND_EXCEPTION_THROW;
  }

  void LibXslModule::xslCall(xsltTransformContextPtr ctxt, xmlNodePtr node, xmlNodePtr inst, xsltElemPreCompPtr comp) {
    //-----------------------------------------------
    //--------------------------------------- C++ libxml init and custom extensions
    //-----------------------------------------------
    /* xsltTransformContextPtr: http://xmlsoft.org/xslt/html/libxslt-xsltInternals.html#xsltTransformContext
      * xmlXPathParserContext:   http://xmlsoft.org/html/libxml-xpath.html#xmlXPathParserContext
      * xmlXPathObject:          http://xmlsoft.org/html/libxml-xpath.html#xmlXPathObject
      * xmlXPathObjectType:      http://xmlsoft.org/html/libxml-xpath.html#xmlXPathObjectType
      */

    //Doc: http://xmlsoft.org/xslt/html/libxslt-xsltInternals.html#xsltTransformContext
    //ctxt = transform context
    //  ctxt->style    = the *parent* stylesheet
    //  ctxt->document = current source document, can be NULL
    //  ctxt->node     = current source node being processed
    //  ctxt->output   = output document
    //  ctxt->insert   = output node
    //  ctxt->inst     = the instruction in the stylesheet (Database:transform command node)
    //  ctxt->_profile = user defined data
    //  ctxt->sec      = security preferences
    //USE CONTEXT INSTEAD: node = ctxt->node = current source node being processed
    //USE CONTEXT INSTEAD: inst = ctxt->inst = the instruction in the stylesheet (Database:transform command node)
    //comp = this function pointer (?)
    assert(ctxt);

    IXmlBaseNode *pInsertResultNode = 0;
    const IXmlQueryEnvironment *pQE = 0;

    //abstract in to IXML asap
    //pInsertResultNode is always zero from runXslModuleCommand(...)s
    UNWIND_EXCEPTION_BEGIN_STATIC(NO_OWNER) {
      //emo is optional
      //GDB:
      //  ((LibXslTransformContext*) pQE->transformContext())->m_ctxt
      //  ((LibXslTransformContext*) pQE->transformContext())->m_ctxt->xpathCtxt
      if (!ctxt->emo) throw NoEMOAvailableForExtension(NO_OWNER, (const char*) ctxt->inst->name);
      if (pQE = (const IXmlQueryEnvironment*) ctxt->emo) { 
        assert( ctxt && ctxt->document && ctxt->document->doc
          && ctxt->node
          && ctxt->output->doc && ctxt->insert
          && ctxt->inst && ctxt->inst->ns && ctxt->inst->ns->href && ctxt->inst->name
        );

        pInsertResultNode = pQE->emoContext()->runXslModuleCommand(pQE, (const char*) ctxt->inst->ns->href, (const char*) ctxt->inst->name);
        if (pInsertResultNode) NOT_COMPLETE(""); //LibXslModule::xslCall() node return?
      }
    } UNWIND_EXCEPTION_END_STATIC(NO_OWNER);

    //free up
    //if (pQE) delete pQE; //NOT_OURS;
    //if (pInsertResultNode) delete pInsertResultNode->document(); //NOT_OURS: pQE owns these
    if (pInsertResultNode) delete pInsertResultNode;

    UNWIND_EXCEPTION_THROW;
  }
}
