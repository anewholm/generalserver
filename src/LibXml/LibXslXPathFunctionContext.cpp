//platform agnostic file
#include "LibXslXPathFunctionContext.h"

#include "libxslt/variables.h"
#include "Server.h"
#include "Debug.h"
#include "LibXmlLibrary.h"
#include "LibXmlBaseNode.h"
#include "LibXslDoc.h"
#include "LibXmlNode.h"
#include "IXml/IXmlGrammarContext.h"
#include "IXml/IXmlXPathProcessingContext.h"

#include "Utilities/StringVector.h"
#include "Utilities/clone.c"

using namespace std;

namespace general_server {
  //construction
  LibXslXPathFunctionContext::LibXslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, xmlXPathParserContextPtr ctxt, const int nargs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XslXPathFunctionContext(pMemoryLifetimeOwner, pQE), 
    m_ctxt(ctxt), 
    m_bOurs(false),
    m_nargs(nargs) 
  {}

  LibXslXPathFunctionContext::LibXslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const char *sXPath): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XslXPathFunctionContext(pMemoryLifetimeOwner, pQE), 
    m_bOurs(true),
    m_nargs(0) 
  {
    assert(m_pOwnerQE);

    xmlXPathContextPtr oXCtxt          = 0;
    xsltTransformContextPtr oTCtxt     = 0;
    const LibXmlBaseDoc *pDocType      = 0;
    const IXmlBaseDoc   *pDoc          = 0;   //required by QE at construction
    const IXslTransformContext *pTCtxt;       //QE singleton server free
    const LibXslTransformContext *pTCtxtType;
    
    //create a blank new xpath context
    //TODO: pDoc       = (pCurrentNode ? pCurrentNode->document() : m_pOwnerQE->singularDocumentContext());
    pDoc       = m_pOwnerQE->singularDocumentContext();
    pDocType   = dynamic_cast<const LibXmlBaseDoc*>(pDoc);
    oXCtxt     = xmlXPathNewContext(pDocType->libXmlDocPtr()); /* TODO; new LibXslXPathFunctionContext() more than 1000 of these */
    m_ctxt     = xmlXPathNewParserContext((const xmlChar*) sXPath, oXCtxt);
    
    //inherit the xpath from the transform context with current parent_route etc.
    //TODO: move these actions in to their respective QE components
    //  for each component, e.g. TransformContext, run ->attachToXPathContext()
    if (pTCtxt = m_pOwnerQE->transformContext()) {
      pTCtxtType    = dynamic_cast<const LibXslTransformContext*>(pTCtxt);
      oTCtxt        = pTCtxtType->libXmlContext();
      oXCtxt->hardlinks_policy = oTCtxt->xpathCtxt->hardlinks_policy;
      registerExternalContext(oXCtxt, oTCtxt);
      registerXSLTFunctionLookup(oXCtxt);
      registerXSLTVariableLookup(oXCtxt, oTCtxt);
      //xsltRegisterAllElement(oTCtxt);
    }

    //setup stages
    registerXPathFunctionLookup(oXCtxt);
    setQueryEnvironmentComponents(oXCtxt, m_pOwnerQE);
    oXCtxt->gp         = xmlCreateGrammarProcessorCallbackContext(LibXslXPathFunctionContext::xmlExternalGrammarProcessorCallback,  (void*) m_pOwnerQE);
    oXCtxt->xprocessor = xmlCreateXPathProcessingCallbackContext( LibXslXPathFunctionContext::xmlExternalXPathProcessorCallback,    (void*) m_pOwnerQE);
    
    registerXPathNamespaces(oXCtxt, pDoc);
    if (pCurrentNode) setContextNode(oXCtxt, pCurrentNode);

    //base native checks (probably already caught above)
    IFDEBUG(assert(m_ctxt->context));       //LibXML xmlXPathParserContext requires an xmlXPathContextPtr
  }

  LibXslXPathFunctionContext::LibXslXPathFunctionContext(const LibXslXPathFunctionContext& lxxc):
    MemoryLifetimeOwner(lxxc.mmParent()), //addChild()
    XslXPathFunctionContext(lxxc.mmParent(), lxxc.m_pOwnerQE),
    m_ctxt(base_object_clone<xmlXPathParserContext>(lxxc.m_ctxt)),
    m_bOurs(true), //this one is now cloned so needs to be destroyed also
    m_nargs(lxxc.m_nargs)
  {}

  LibXslXPathFunctionContext::~LibXslXPathFunctionContext() {
    if (m_bOurs) {
      xmlXPathFreeContext(m_ctxt->context);
      xmlXPathFreeParserContext(m_ctxt);
    }
  }

  int LibXslXPathFunctionContext::xmlExternalXPathProcessorCallback(xmlXPathProcessingCallbackContextPtr oCtxt, xmlNodePtr cur, xmlListPtr oParentRoute, int test_type, const xmlChar *sLocalName, const xmlChar *sPrefix, const char ***pvsGroupings) {
    //STATIC
    //caller frees non-zero *pvsGroupings
    assert(oCtxt && oCtxt->param);
    int ret = 0; //IXmlQueryEnvironment::XP_TEST_HIT_NO;
    const StringVector *pvGroupings = 0;
    StringVector::const_iterator iGrouping;
    int i;
    const char **vsGroupings;

    const IXmlQueryEnvironment *pQE  = 0; //required: non-const will cause volatile methods to be accessed
    IXmlXPathProcessingContext *pXP  = 0; //optional

    if (pQE = (const IXmlQueryEnvironment*) oCtxt->param) {
      if (pXP = pQE->xpathProcessingContext()) {
        if (pXP->enabled()) {
          //PREFORMANCE: stack decleration for performance, only when we are asked to analyse
          const LibXmlNode oCur(pQE, cur, oParentRoute, pQE->singularDocumentContext(), NO_REGISTRATION, "grammar processing check"); //pQE as MM
          
          switch (test_type) {
            case IXmlQueryEnvironment::NODE_TEST_NAME: {
              ret = pXP->checkName(&oCur, (const char*) sLocalName, (const char*) sPrefix);
              break;
            }
            case IXmlQueryEnvironment::TEMPLATE_GROUP_NAMES: {
              //Groupings are done on local-name only
              pvGroupings = pXP->templateSuggestionsForLocalName((const char*) sLocalName);
              break;
            }
          }
        }
      }
    }
    
    //if requested: construct a zero terminated string array at pvsGroupings
    if (pvsGroupings) {
      ret = (pvGroupings ? pvGroupings->size() : 0);
      vsGroupings = (const char**) MM_MALLOC_FOR_CALLER((ret + 1) * sizeof(const char*));
      if (pvGroupings) {
        for (iGrouping = pvGroupings->begin(); iGrouping != pvGroupings->end(); iGrouping++) {
          i = distance(pvGroupings->begin(), iGrouping);
          vsGroupings[i] = *iGrouping;
        }
      }
      vsGroupings[ret] = NULL;
      *pvsGroupings = vsGroupings;
    }
    
    //free up
    if (pvGroupings) delete pvGroupings; //contents are pointers in to the DatabaseClass

    return ret;
  }
  
  const xmlChar *LibXslXPathFunctionContext::xmlExternalGrammarProcessorCallback(xmlGrammarProcessorCallbackContextPtr oCtxt, xmlNodePtr *context_node, xmlListPtr *parent_route, int *piSkip, int iTraceFlags) {
    //STATIC
    assert(oCtxt && oCtxt->param);
    assert(oCtxt->xpathParserCtxt);
    assert(context_node); //we need to be able to reply with a context even if there is none
    assert(parent_route); //we need to be able to reply with a context even if there is none
    //but not assert(*new_context) because there might not be any currently
    
    const char *sReplacementXPath    = 0;
    const IXmlQueryEnvironment *pQE; //required: non-const will cause volatile methods to be accessed
    IXmlGrammarContext         *pGP; //optional

    pQE = (IXmlQueryEnvironment*) oCtxt->param;
    if (oCtxt->xpathParserCtxt->cur && (pGP = pQE->grammarContext())) {
      LibXslXPathFunctionContext oXPathContext(pQE, pQE, oCtxt->xpathParserCtxt); //PERFORMANCE //pQE as MM
      sReplacementXPath = pGP->process(pQE, &oXPathContext, (const char*) oCtxt->xpathParserCtxt->cur, piSkip, (IXmlLibrary::xmlTraceFlag) iTraceFlags);
      //Blank response is valid as SKIP() may be required, optionally with a context node change
      if (!sReplacementXPath) {
        //GP failed to find a replacement
        Debug::report("GP failed to find a replacement [%s]", (const char*) oCtxt->xpathParserCtxt->cur);
      }
    }

    //free up
    //if (pQE)  delete pQE;  //NOT_OURS
    //if (pGP)  delete pGP;  //NOT_OURS
    //if (pDoc) delete pDoc; //NOT_OURS

    return (const xmlChar*) sReplacementXPath;
  }
  
  void LibXslXPathFunctionContext::registerExternalContext(xmlXPathContextPtr oXCtxt, void *oExternalContext) const {
    oXCtxt->externalContext = oExternalContext;
    oXCtxt->extra           = oExternalContext;
  }

  void LibXslXPathFunctionContext::registerXPathFunctionLookup(xmlXPathContextPtr oXCtxt) const {
    xmlXPathRegisterFuncLookup(oXCtxt, (xmlXPathFuncLookupFunc) xsltXPathFunctionLookup, (void *) oXCtxt);
  }
  
  void LibXslXPathFunctionContext::registerXSLTFunctionLookup(xmlXPathContextPtr oXCtxt) const {
    xsltRegisterAllFunctions(oXCtxt);
  }

  void LibXslXPathFunctionContext::registerXSLTVariableLookup(xmlXPathContextPtr oXCtxt, void *oExternalContext) const {
    xmlXPathRegisterVariableLookup(oXCtxt, xsltXPathVariableLookup, oExternalContext);
  }

  void LibXslXPathFunctionContext::setContextNode(xmlXPathContextPtr oXCtxt, const IXmlBaseNode *pCurrentNode) const {
    //a transform context without a current node is possible
    //but probably cause problems later...
    const LibXmlBaseNode *pCurrentNodeType = dynamic_cast<const LibXmlBaseNode*>(pCurrentNode);
    if (!pCurrentNodeType) throw TransformWithoutCurrentNode(this);
    
    oXCtxt->node         = pCurrentNodeType->libXmlNodePtr();
    oXCtxt->parent_route = xmlListDup(pCurrentNodeType->libXmlParentRoutePtr());
  }

  void LibXslXPathFunctionContext::registerXPathNamespaces(xmlXPathContextPtr oXCtxt, const IXmlBaseDoc *pDoc) const {
    const StringMap<const char*> *pNamespaceDefinitions;
    StringMap<const char*>::const_iterator iNamespaceDefinition;

    //register all NameSpaces from the target document
    //  otherwise we will not be able to query user custom added prefix namespaces
    //  because the xpath loads the standard prefixes which are then used to indicate the nodes in each step
    //  so what if a prefix changes in the tree as xpath decends, how would the node prefix be?
    //  it would need to use the original, global xpath knowledge of namespace prefixes...
    //  can xpath then query anything that does not have a globally loaded and declared namespace with prefix: NO
    //TODO: check to see if the oXCtxt->nsHash is already populated and then ignore?
    pNamespaceDefinitions = pDoc->allPrefixedNamespaceDefinitions();
    for (iNamespaceDefinition = pNamespaceDefinitions->begin(); iNamespaceDefinition != pNamespaceDefinitions->end(); iNamespaceDefinition++) {
      //xmlXPathRegisterNs() will accept several prefixes for the same namespace
      //but do we? what if a prefix meaning changes during descent?
      //but the xpath MUST have meaning, and namespace decleration is not possible DURING xpath descent
      //especially as full namespace URI prefix is not possible
      //THEREFORE: xpath prefixes MUST have one meaning
      //default namespaces and xpath: http://search.cpan.org/~shlomif/XML-LibXML-2.0016/lib/XML/LibXML/Node.pod
      xmlXPathRegisterNs(oXCtxt, (xmlChar*) iNamespaceDefinition->first, (xmlChar*) iNamespaceDefinition->second);
    }
    if (!oXCtxt->nsHash || oXCtxt->nsHash->nbElems == 0) throw CannotRunXPathWithoutNamespaceCache(this);
  }

  void LibXslXPathFunctionContext::setQueryEnvironmentComponents(xmlXPathContextPtr oXCtxt, const IXmlQueryEnvironment *pQE) const {
    oXCtxt->emo      = (void*) pQE;
    oXCtxt->xfilter  = xmlCreateNodeFilterCallbackContext( LibXmlBaseNode::xmlElementFilterCallback,  (void*) pQE);
    oXCtxt->xtrigger = xmlCreateNodeTriggerCallbackContext(LibXmlBaseNode::xmlElementTriggerCallback, (void*) pQE, oXCtxt);
  }

  LibXslXPathFunctionContext *LibXslXPathFunctionContext::clone_with_resources() const {
    //cloning happens in the copy constructor
    return new LibXslXPathFunctionContext(*this);
  }

  IXslXPathFunctionContext *LibXslXPathFunctionContext::inherit() const {return clone_with_resources();}

  const char *LibXslXPathFunctionContext::currentParentRoute() const {
    //caller frees result
    const char *ret = 0;
    IXslTransformContext *pTCtxt;

    if (pTCtxt = m_pOwnerQE->transformContext()) ret = pTCtxt->currentParentRoute();

    //free up
    //if (pTCtxt) delete pTCtxt; //pointer in to pQE

    return ret;
  }

  int LibXslXPathFunctionContext::valueCount() const {
    //m_ctxt->valueNr contains all parameters AND context nodes in one;
    return m_nargs;
  }

  const char *LibXslXPathFunctionContext::functionName() const {
    return (const char *) m_ctxt->context->function;
  }

  const char *LibXslXPathFunctionContext::functionNamespace() const {
    return (const char *) m_ctxt->context->functionURI;
  }

  iXPathParamType LibXslXPathFunctionContext::xtype() const {
    return m_ctxt->value ? (iXPathParamType) m_ctxt->value->type : XPATH_UNDEFINED;
  }

  const char *LibXslXPathFunctionContext::xpath() const {
    //do not free!
    //full xpath statement
    return (const char*) m_ctxt->base;
  }

  const char *LibXslXPathFunctionContext::xpathCurrent() const {
    //do not free!
    //current part of the xpath being evaluated
    return (const char*) m_ctxt->cur;
  }

  void LibXslXPathFunctionContext::popIgnoreFromXPathFunctionCallStack()  {
    //const char *sValue = (const char*) xmlXPathPopString(m_ctxt); //will cause a string coversion of node-sets
    xmlXPathPop(m_ctxt);
  }

  bool LibXslXPathFunctionContext::popInterpretBooleanValueFromXPathFunctionCallStack()  {
    //caller frees result
    bool bValue = false;
    bValue = (bool) xmlXPathPopBoolean(m_ctxt);
    if (xmlXPathCheckError(m_ctxt)) {
      throw XPathFunctionWrongArgumentType(this, MM_STRDUP("boolean"), MM_STRDUP("something else"));
    }
    return bValue;
  }

  const char *LibXslXPathFunctionContext::popInterpretCharacterValueFromXPathFunctionCallStack()  {
    //caller frees result
    const char *sValue = 0;
    
    sValue = (const char*) xmlXPathPopString(m_ctxt);
    if (xmlXPathCheckError(m_ctxt)) {
      if (sValue) xmlFree((void*) sValue);
      sValue = 0;
      throw XPathFunctionWrongArgumentType(this, MM_STRDUP("string"), MM_STRDUP("something else"));
    }
    
    return sValue;
  }

  int LibXslXPathFunctionContext::popInterpretIntegerValueFromXPathFunctionCallStack()  {
    return (int) popInterpretDoubleValueFromXPathFunctionCallStack();
  }

  double LibXslXPathFunctionContext::popInterpretDoubleValueFromXPathFunctionCallStack()  {
    //caller frees result
    double dValue = 0;
    dValue = xmlXPathPopNumber(m_ctxt);
    if (xmlXPathCheckError(m_ctxt)) {
      dValue = 0;
      throw XPathFunctionWrongArgumentType(this, MM_STRDUP("double"), MM_STRDUP("something else"));
    }
    return dValue;
  }

  const char *LibXslXPathFunctionContext::xmlXPathObjectType(xmlXPathObjectPtr oXPathObject) const {
    //do not free result
    const char *sRet = 0;
    if (oXPathObject) {
      switch (oXPathObject->type) {
        case XPATH_UNDEFINED:   sRet = "XPATH_UNDEFINED"; break;
        case XPATH_NODESET:     sRet = "XPATH_NODESET"; break;
        case XPATH_BOOLEAN:     sRet = "XPATH_BOOLEAN"; break;
        case XPATH_NUMBER:      sRet = "XPATH_NUMBER"; break;
        case XPATH_STRING:      sRet = "XPATH_STRING"; break;
        case XPATH_POINT:       sRet = "XPATH_POINT"; break;
        case XPATH_RANGE:       sRet = "XPATH_RANGE"; break;
        case XPATH_LOCATIONSET: sRet = "XPATH_LOCATIONSET"; break;
        case XPATH_USERS:       sRet = "XPATH_USERS"; break;
        case XPATH_XSLT_TREE:   sRet = "XPATH_XSLT_TREE"; break;
      }
    }
    return sRet;
  }

  struct tm LibXslXPathFunctionContext::popInterpretDateFromXPathFunctionCallStack()  {
    //caller frees result
    //getdate_r() requires the DATEMSK environmental variable and is reentrant
    //  DATEMSK=datemsk is stored in the source folder and contains things like
    //  %A  #day of the week
    //strptime(const char *s, const char *format, struct tm *tm)
    //  http://man7.org/linux/man-pages/man3/strptime.3.html
    //  we can dynamically set our own accepted formats in the code with this one
    //  no dependencies!
    struct tm tGMT;
    const char *sValue;

    //get the string
    sValue = (const char*) xmlXPathPopString(m_ctxt);
    if (xmlXPathCheckError(m_ctxt)) {
      if (sValue) xmlFree((void*) sValue);
      sValue = 0;
      throw XPathFunctionWrongArgumentType(this, MM_STRDUP("date string"), MM_STRDUP("something else"));
    }

    //convert to date..
    if (sValue) {
      //throw FailedToParseDate(this, sValue);
      tGMT = m_pOwnerQE->xmlLibrary()->parseDate(sValue);
      MMO_FREE(sValue);
    }

    return tGMT;
  }

  XmlNodeList<IXmlBaseNode> *LibXslXPathFunctionContext::popInterpretNodeListFromXPathFunctionCallStack()  {
    //caller frees result
    XmlNodeList<IXmlBaseNode> *pNodes;
    xmlNodeSetPtr oNodes;
    xmlListPtr oParentRoute;
    xmlNodePtr oNode;
    const IXmlBaseDoc *pSourceDoc = 0;

    if (xtype() != XPATH_NODESET) {
      throw XPathFunctionWrongArgumentType(this, MM_STRDUP("XPATH_NODESET"), MM_STRDUP(xmlXPathObjectType(*m_ctxt->valueTab)));
    } else {
      pNodes     = new XmlNodeList<IXmlBaseNode>(this);
      oNodes     = xmlXPathPopNodeSet(m_ctxt);
      pSourceDoc = m_pOwnerQE->singularDocumentContext();
      if (oNodes) {
        for (int n = 0; n < oNodes->nodeNr; n++) {
          oNode        = oNodes->nodeTab[n];
          oParentRoute = (oNodes->parent_routeTab ? oNodes->parent_routeTab[n] : NULL);
          pNodes->push_back(
            LibXmlLibrary::factory_node(this, oNode, (IXmlBaseDoc *) pSourceDoc, oParentRoute)
          );
        }
      }
    }

    //clear up node-set only, we keep the nodes
    if (oNodes) xmlFree(oNodes);
    //one pSourceDoc is shared amongst the returned nodes
    //and one is freed by the caller function when deleting the nodes
    //so that we do not have to pass back a pointer to the () owning document wrapper
    //if (pSourceDoc) delete pSourceDoc;

    return pNodes;
  }

  IXmlBaseNode *LibXslXPathFunctionContext::contextNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees result
    //this is the inner stack node first and then the current node if not
    IXmlBaseNode *pNode = stackNode(pMemoryLifetimeOwner);
    if (!pNode)   pNode = currentNode(pMemoryLifetimeOwner);
    return pNode;
  }
  
  const IXmlBaseNode *LibXslXPathFunctionContext::contextNode(const IXmlBaseNode *pNewConextNode) {
    const LibXmlBaseNode *pNewConextNodeType;
    xmlNodePtr new_context_node;
    //xmlNodeSetPtr nodeset;
    
    NOT_COMPLETE("");
    
    if (pNewConextNodeType = dynamic_cast<const LibXmlBaseNode *>(pNewConextNode)) {
      new_context_node = pNewConextNodeType->m_oNode;
      if (m_ctxt->valueNr) {
        //TODO: need to set the node in the current parsing, not the initial or stack node
        //replace it with /dyn:evaluate(.) ?
        //nodeset = m_ctxt->valueTab[m_ctxt->valueNr-1]->nodesetval;
      } else {
        m_ctxt->context->node = new_context_node;
      }
    }

    return pNewConextNode;
  }

  IXmlBaseNode *LibXslXPathFunctionContext::currentNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees result
    //this is the outer transformation context node
    //i.e. current()
    IXmlBaseNode      *pNode      = 0;
    const IXmlBaseDoc *pSourceDoc = 0;

    if (m_ctxt->context && m_ctxt->context->node) {
      pSourceDoc = m_pOwnerQE->singularDocumentContext();
      pNode      = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->context->node, (IXmlBaseDoc *) pSourceDoc, m_ctxt->context->parent_route);
    }

    return pNode;
  }
  
  const IXmlBaseNode *LibXslXPathFunctionContext::literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees result
    const IXmlBaseNode *pLiteralCommandNode = 0;
    IXslTransformContext  *pTCtxt = 0;

    if (pTCtxt = m_pOwnerQE->transformContext()) pLiteralCommandNode = pTCtxt->literalCommandNode(pMemoryLifetimeOwner);

    //free up
    //if (pTCtxt) delete pTCtxt; //pointer in to pQE

    return pLiteralCommandNode;
  }

  const IXslCommandNode *LibXslXPathFunctionContext::commandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees result
    const IXslCommandNode *pCommandNode = 0;
    IXslTransformContext  *pTCtxt = 0;

    if (pTCtxt = m_pOwnerQE->transformContext()) pCommandNode = pTCtxt->commandNode(pMemoryLifetimeOwner);

    //free up
    //if (pTCtxt) delete pTCtxt; //pointer in to pQE

    return pCommandNode;
  }

  IXmlBaseNode* LibXslXPathFunctionContext::stackNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iTab) const {
    //caller frees result
    //called by xslFunction_stack() which places pNode in the return with it's temporary document
    //LibXslModule frees this node return
    IXmlBaseNode *pNode      = 0;
    const IXmlBaseDoc  *pSourceDoc = 0;
    xmlNodeSetPtr aNodeset   = 0;
    xmlListPtr aParentRoute  = 0;

    //there is only one outer-context stack node ever
    //but LibXml2 only returns node sets
    if (m_ctxt->valueNr <= iTab) {
      //return NULL: the tab is not that big!
    } else {
      aNodeset = m_ctxt->valueTab[iTab]->nodesetval;
      if (aNodeset && aNodeset->nodeNr) {
        pSourceDoc   = m_pOwnerQE->singularDocumentContext();
        aParentRoute = (aNodeset->parent_routeTab ? aNodeset->parent_routeTab[iTab] : NULL);
        pNode        = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, aNodeset->nodeTab[0], (IXmlBaseDoc *) pSourceDoc, aParentRoute);
      }
    }

    //free up
    //DO NOT free individual node pointers because they are NOT_OURS
    //TODO: xmlFreeNodeSet(aNodeset);
    //one pSourceDoc is shared amongst the returned nodes
    //and one is freed by the caller function when deleting the nodes
    //so that we do not have to pass back a pointer to the () owning document wrapper
    //if (pSourceDoc) delete pSourceDoc;

    return pNode;
  }
}
