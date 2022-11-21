//platform agnostic file
#include "LibXslTransformContext.h"

#include "libxslt/variables.h"
#include "libxslt/transform.h"
#include "libxml/hash.h"

#include "Server.h"
#include "Debug.h"
#include "LibXmlLibrary.h"
#include "LibXmlDoc.h"
#include "LibXmlNode.h"
#include "LibXslNode.h"

#include "Utilities/strtools.h"
#include "Utilities/clone.c"

using namespace std;

namespace general_server {
  LibXslTransformContext::LibXslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, xsltTransformContextPtr ctxt): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XslTransformContext(pMemoryLifetimeOwner, pOwnerQE),
    m_ctxt(ctxt),
    m_bOurs(false),
    m_bInherited(false)
  {}
  
  LibXslTransformContext::LibXslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, const IXmlBaseDoc *pSourceDoc, const IXslDoc *pStylesheet, xsltTransformContextPtr ctxt):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XslTransformContext(pMemoryLifetimeOwner, pOwnerQE),
    m_bOurs(true),
    m_bInherited(false),

    m_pSourceDoc(pSourceDoc),
    m_pStylesheet(pStylesheet)
  {
    assert(m_pSourceDoc);
    assert(m_pStylesheet);
    //assert(m_pOwnerQE); //XslTransformContext does this

    m_ctxt = (ctxt ? ctxt : xsltNewTransformContext(
      dynamic_cast<const LibXslDoc*>(    pStylesheet)->libXslStylesheetPtr(),
      dynamic_cast<const LibXmlBaseDoc*>(pSourceDoc )->libXmlDocPtr()
    ));
    
    //this will cause the transform context to be sent through to the error handler
    xsltSetTransformErrorFunc(m_ctxt, m_ctxt, LibXmlLibrary::transformErrorFunc);

    //non parent inheritance transform context
    //  no globalVars to inherit
    //the EMO on the xpath context it is needed by external functions used in the triggers, e.g. repository:rx(...)
    //  and also the external function lookup registration XSLT_REGISTER_FUNCTION_LOOKUP()
    //  xsltRegisterExtModuleFunction(sFunctionName, xsltModuleNamespace, xslFunctionCall);
    //  we directly use the xpathCtxt from the LibXML transform context here
    //  rather than creating the IXml transform Context wrapper and then accessing it
    //m_ctxt->xfilter  freed in xsltFreeTransformContext() below
    //m_ctxt->xtrigger freed in xsltFreeTransformContext() below
    xmlXPathContextPtr oXPathContext = m_ctxt->xpathCtxt;

    //transform context
    m_ctxt->emo      = (void*) m_pOwnerQE; //-> static LibXslModule::xslFunctionCall
    m_ctxt->xfilter  = xmlCreateNodeFilterCallbackContext( LibXmlBaseNode::xmlElementFilterCallback,  (void*) m_pOwnerQE);
    m_ctxt->xtrigger = xmlCreateNodeTriggerCallbackContext(LibXmlBaseNode::xmlElementTriggerCallback, (void*) m_pOwnerQE, oXPathContext);

    //xpath context
    oXPathContext->emo         = m_ctxt->emo;
    oXPathContext->xfilter     = m_ctxt->xfilter;
    oXPathContext->xtrigger    = m_ctxt->xtrigger;
    oXPathContext->gp          = xmlCreateGrammarProcessorCallbackContext(LibXslXPathFunctionContext::xmlExternalGrammarProcessorCallback,  (void*) m_pOwnerQE);
    oXPathContext->xprocessor  = xmlCreateXPathProcessingCallbackContext( LibXslXPathFunctionContext::xmlExternalXPathProcessorCallback,    (void*) m_pOwnerQE);
    oXPathContext->externalContext = m_ctxt;
  }

  LibXslTransformContext::LibXslTransformContext(const LibXslTransformContext& lxtc):
    //exceptions only...
    XslTransformContext(lxtc.mmParent(), lxtc.m_pOwnerQE),
    MemoryLifetimeOwner(lxtc.mmParent()), //addChild()
    m_bOurs(lxtc.m_bOurs),
    m_ctxt(base_object_clone<xsltTransformContext>(lxtc.m_ctxt)),
    m_bInherited(   lxtc.m_bInherited),
    m_pSourceDoc(   lxtc.m_pSourceDoc),
    m_pStylesheet(  lxtc.m_pStylesheet),
    m_paramsInt(    lxtc.m_paramsInt.begin(),  lxtc.m_paramsInt.end(),  1),      //copying dependent on contents, thus a copy constructor template is not possible
    m_paramsChar(   lxtc.m_paramsChar.begin(), lxtc.m_paramsChar.end(), "true"), //copying dependent on contents, thus a copy constructor template is not possible
    m_paramsNodeSet(lxtc.m_paramsNodeSet),     //direct copy constructor clone_with_resources()
    m_iTraceFlags(  lxtc.m_iTraceFlags)
  {}

  LibXslTransformContext::~LibXslTransformContext() {
    //params
    /*
     * caller frees the params (often locally declared)
    //free only second value of int params
    StringMap<const size_t>::const_iterator         iParamsInt;
    if (m_paramsInt) {
      for (iParamsInt = m_paramsInt.begin(); iParamsInt != m_paramsInt.end(); iParamsInt++) {
        MMO_FREE(iParamsInt->second);
      }
    }
    if (m_paramsInt)  delete m_paramsInt;
    if (m_paramsChar) delete m_paramsChar;
    if (m_paramsNodeSet) delete m_paramsNodeSet;
    */

    //context
    if (m_bOurs && m_ctxt) {
      m_ctxt->globalVars = 0; //dont free the shared parents globalVars!
      //TODO: xsltFreeTransformContext(m_ctxt);
    }
  }

  IXmlBaseDoc *LibXslTransformContext::transform() const {
    //output may be directly to the node, thus returning nothing
    //NOTE: that any LibXslModule custom commands that try to write / read during this transform will block if we lock here
    //this XSLT may mark stylesheet cache entrie(s) for removal, but it won't free the compiled stylesheet until next transform
    IXmlBaseDoc *pNewDoc      = 0;
    const char **pcParams     = 0;
    const char **pcNodeSets   = 0;
    xmlDocPtr  oRes           = 0;
    const IXmlBaseNode *pFirstChild = 0;
    size_t i;

    assert(m_ctxt && m_ctxt->xfilter && m_ctxt->xfilter->param);
    assert(ownerQE() == m_ctxt->xfilter->param);

    IFDEBUG(
      //Debug::reportObject(ownerQE());
    );

    UNWIND_EXCEPTION_BEGIN {
      //these params become global and first template params
      pcParams   = assembleStoredBasicParams();
      pcNodeSets = assembleStoredNodeSetParams();
      
      try {
        oRes = xsltApplyStylesheetUserSecure(m_ctxt->style, m_ctxt->document->doc, pcParams, pcNodeSets, 0, 0, m_ctxt);
        //RESOURCE HUNGRY: IFDEBUG(m_pSourceDoc->validityCheck("check doc after transform()");)
      } catch (ExceptionBase &eb) {
        //need to OURS free this m_ctxt
        throw XSLTException(eb, this);
      }

      //-------------------------------------------- output post-processing
      if (oRes) {
        //new document created by the xsltApplyStylesheetUser(...)
        pNewDoc = LibXmlLibrary::factory_document(mmParent(), m_pOwnerQE->xmlLibrary(), "results", oRes);
        if (pNewDoc->hasMultipleChildElements(m_pOwnerQE)) throw MultipleRootNodes(this, MM_STRDUP("doc results"));
        //sanitise output: unneccessary because of omitXmlDeclaration = true
        //pNewDoc->removeProcessingInstructions();
        pNewDoc->copyNamespaceCacheFrom(m_pSourceDoc);
      } else {
        //no oRes when we asked for one
        UNWIND_IF_NOEXCEPTION(throw NoOutputDocument(this));
      }
    } UNWIND_EXCEPTION_END;

    //-------------------------------------------- free up
    if (pFirstChild) delete pFirstChild;
    if (m_bInherited) {
      //reset these anyways
      //new global vars will have been added
      //  and the parent will now have access to them
      //  but the parent will release them for us
      //the context will be freed by the exception eventually
      //  the exception will bubble up back through the original transform
      //  trapping and freeing it's context also
      m_ctxt->globalVars = 0;
    }
    UNWIND_IF_NOEXCEPTION(
      //xsltFreeTransformContext(m_ctxt); //singleton freed by QE
      //xmlFreeNodeFilterCallbackContext(m_ctxt->xfilter); //freed in xsltFreeTransformContext()
    );
    if (pcParams)   {
      i = 0;
      while (pcParams[i++]) MMO_FREE(pcParams[i++]);
      MMO_FREE(pcParams);
    }
    if (pcNodeSets) {
      i = 0;
      while (pcNodeSets[i++]) xmlXPathFreeNodeSet((xmlNodeSetPtr) pcNodeSets[i++]);
      MMO_FREE(pcNodeSets);
    }
    //we are statically cacheing these so no free here. freed in static destruction
    //or by node changes -> cache refresh -> re-compile
    //if (oXSLDoc2) xsltFreeStylesheet(oXSLDoc2);

    UNWIND_EXCEPTION_THROW;

    return pNewDoc;
  }

  void LibXslTransformContext::transform(const IXmlBaseNode *pInitialOutputNode) const {
    //output may be directly to the node, thus returning nothing
    //NOTE: that any LibXslModule custom commands that try to write / read during this transform will block if we lock here
    //this XSLT may mark stylesheet cache entrie(s) for removal, but it won't free the compiled stylesheet until next transform
    const char **pcParams     = 0;
    const char **pcNodeSets   = 0;
    xmlDocPtr  oRes           = 0;
    const IXmlBaseNode *pFirstChild = 0;
    const IXmlHasNamespaceDefinitions *pFirstChildType = 0;
    bool bTopElemVisited = false;
    size_t i;

    assert(m_ctxt && m_ctxt->xfilter && m_ctxt->xfilter->param);
    assert(ownerQE() == m_ctxt->xfilter->param);

    IFDEBUG(
      //Debug::reportObject(ownerQE());
    );

    UNWIND_EXCEPTION_BEGIN {
      //these params become global and first template params
      pcParams        = assembleStoredBasicParams();
      pcNodeSets      = assembleStoredNodeSetParams();
      bTopElemVisited = updateOutputNode(pInitialOutputNode);

      try {
        oRes = xsltApplyStylesheetUserSecure(m_ctxt->style, m_ctxt->document->doc, pcParams, pcNodeSets, 0, 0, m_ctxt);
        //RESOURCE HUNGRY: IFDEBUG(m_pSourceDoc->validityCheck("check doc after transform()");)
      } catch (ExceptionBase &eb) {
        //need to OURS free this m_ctxt
        throw XSLTException(eb, this);
      }

      //-------------------------------------------- output post-processing
      if (oRes) {
        //oRes when we asked for one
        if (m_ctxt->document->doc != oRes) xmlFreeDoc(oRes);
        UNWIND_IF_NOEXCEPTION(throw NoOutputDocument(this, "we did not ask for an oRes Doc"));
      } else {
        //we cannot check the output node count because there may have already been some
        //if (output node->hasMultipleChildElements()) throw MultipleRootNodes(this, MM_STRDUP("output results"));
        //else output went directly to pOutputNode: m_ctxt->output
        if (!bTopElemVisited) {
          //we have output to a new area, far away from the parent transform output node
          //probably the tmp area of the main Database
          //or a new document?
          //m_ctxt->topElemVisited means that the output went to the current parent context output node
          if (pFirstChild = pInitialOutputNode->firstChild(m_pOwnerQE)) {
            if (pFirstChildType = pFirstChild->queryInterface((const IXmlHasNamespaceDefinitions*) 0)) {
              //TODO: more than one top level node from the transform?
              //pFirstChildType->setNamespaceRoot();
            } else throw InterfaceNotSupported(this, MM_STRDUP("IXmlHasNamespaceDefinitions*"));
          } //no output: this is ok, no error
        }
      }
    } UNWIND_EXCEPTION_END;

    //-------------------------------------------- free up
    if (pFirstChild) delete pFirstChild;
    if (m_bInherited) {
      //reset these anyways
      //new global vars will have been added
      //  and the parent will now have access to them
      //  but the parent will release them for us
      //the context will be freed by the exception eventually
      //  the exception will bubble up back through the original transform
      //  trapping and freeing it's context also
      m_ctxt->globalVars = 0;
    }
    UNWIND_IF_NOEXCEPTION(
      //xsltFreeTransformContext(m_ctxt); //singleton freed by QE
      //xmlFreeNodeFilterCallbackContext(m_ctxt->xfilter); //freed in xsltFreeTransformContext()
    );
    if (pcParams)   {
      i = 0;
      while (pcParams[i++]) MMO_FREE(pcParams[i++]);
      MMO_FREE(pcParams);
    }
    if (pcNodeSets) {
      i = 0;
      while (pcNodeSets[i++]) xmlXPathFreeNodeSet((xmlNodeSetPtr) pcNodeSets[i++]);
      MMO_FREE(pcNodeSets);
    }
    //we are statically cacheing these so no free here. freed in static destruction
    //or by node changes -> cache refresh -> re-compile
    //if (oXSLDoc2) xsltFreeStylesheet(oXSLDoc2);

    UNWIND_EXCEPTION_THROW;
  }

  const char *LibXslTransformContext::toString() const {
    stringstream ssOut;
    const IXmlBaseNode *pInstructionNode = instructionNode(this);
    const IXmlBaseNode *pSourceNode      = sourceNode(this);
    const char *sSSName      = (m_pStylesheet    ? m_pStylesheet->name() : MM_STRDUP("(no stylesheet: error!)"));
    const char *sCommandNode = (pInstructionNode ? pInstructionNode->localName(NO_DUPLICATE) : "(no instruction node)");
    const char *sSourceNode  = (pSourceNode      ? pSourceNode->localName(NO_DUPLICATE) : "(no source node: creating a new document)");

    ssOut << "TransformContext of [" << sSourceNode << "]:";
    ssOut << " (" << (m_bInherited ? "inherited" : "new") << ")\n";
    //ssOut << "  output: " << sSSName << "\n";
    //ssOut << "  insert: " << sSSName << "\n";
    ssOut << "  stylesheet: " << sSSName << "\n";
    ssOut << "  mode: " << (m_ctxt->mode ? (const char*) m_ctxt->mode : "(none)") << "\n";
    ssOut << "-- globalVars:\n" << globalVars()->toString();
    //these are not relevant in a new transform
    //the parameters of <Database:transform and Database:transform( are calculated for us
    //before the transform starts, by the parent process
    //ssOut << "-- localVars:\n"  << localVars()->toString();
    ssOut << "-- params:\n";
    if (m_paramsChar.size())    ssOut << m_paramsChar.toString(true) << "\n";
    if (m_paramsInt.size())     ssOut << m_paramsInt.toString(true)  << "\n";
    if (m_paramsNodeSet.size()) ssOut << m_paramsNodeSet.toStringFromObjects(true) << "\n";

    //free up
    if (sSSName)          MMO_FREE(sSSName);
    if (pInstructionNode) MMO_DELETE(pInstructionNode);
    if (pSourceNode)      MMO_DELETE(pSourceNode);
    //if (sCommandNode) MMO_FREE(sCommandNode); //NO_DUPLICATE
    //if (sSourceNode)  MMO_FREE(sSourceNode); //NO_DUPLICATE

    return MM_STRDUP(ssOut.str().c_str());
  }

  IXslTransformContext *LibXslTransformContext::inherit(const IXslDoc *pChangeStylesheet) const {
    xsltTransformContextPtr oNewTCtxt;
    xmlXPathContextPtr oNewXPathContext;

    //we can inherit the setup for the same stylesheet
    if (!pChangeStylesheet) pChangeStylesheet = m_pStylesheet;

    //------------------------------------------------ native parent transform context inheritance
    //the standard libXslt has been removed: "Global parameter %s already defined"
    //pass through parent context globalVars so that we can maintain nodes in the params
    //this only happens when a parent stylesheet is sent through
    //which is only when we are running our custom config:transform or config:append-child transform=...
    //xsltApplyStylesheetUser(... params) => context->globalVars
    //local additional params variable will override the m_ctxt->globalVars during start of xsltApplyStylesheetUser(...)
    oNewTCtxt = xsltNewTransformContext(
      dynamic_cast<const LibXslDoc*>(pChangeStylesheet)->libXslStylesheetPtr(),
      m_ctxt->document->doc
    );
    oNewTCtxt->globalVars          = m_ctxt->globalVars; //xmlHashCopy(m_ctxt->globalVars, xmlCopyGlobalVar);
    /*
    oNewTCtxt->type                = m_ctxt->type;
    oNewTCtxt->document            = m_ctxt->document;
    oNewTCtxt->state               = m_ctxt->state;
    oNewTCtxt->dict                = m_ctxt->dict;
    */

    //continue from last position in output
    oNewTCtxt->output              = m_ctxt->output;
    oNewTCtxt->insert              = m_ctxt->insert;
    /*
    oNewTCtxt->node                = m_ctxt->node;
    oNewTCtxt->parent_route        = xmlListDup(m_ctxt->parent_route);
    */

    //custom additions for XslModule, security and triggers
    oNewTCtxt->emo                 = m_ctxt->emo;     //XslModule link for our C++ extensions
    oNewTCtxt->xfilter             = xmlCopyNodeFilterCallbackContext(m_ctxt->xfilter);   //security pass through
    oNewTCtxt->xtrigger            = xmlCopyNodeTriggerCallbackContext(m_ctxt->xtrigger); //trigger pass through

    //xpath mirror settings
    oNewXPathContext               = oNewTCtxt->xpathCtxt;
    oNewXPathContext->emo          = oNewTCtxt->emo;
    oNewXPathContext->xfilter      = oNewTCtxt->xfilter;  //should always be the same
    oNewXPathContext->xtrigger     = oNewTCtxt->xtrigger; //should always be the same
    //xpath context point back to transform context
    oNewXPathContext->externalContext = oNewTCtxt;
    //point to same foreign namespace stack
    oNewXPathContext->namespaces   = m_ctxt->xpathCtxt->namespaces;
    oNewXPathContext->nsHash       = m_ctxt->xpathCtxt->nsHash;

    //sharing this will cause everything to be already defined
    //as it overrides the new style context needed
    //oNewTCtxt->style         = m_ctxt->style;

    /*
    if (oNewTCtxt->style) {
      //xsltInternals.h
      oNewTCtxt->style->nsAliases     = m_ctxt->style->nsAliases;
      oNewTCtxt->style->attributeSets = m_ctxt->style->attributeSets;
      oNewTCtxt->style->nsHash        = m_ctxt->style->nsHash;
      oNewTCtxt->style->nsDefs        = m_ctxt->style->nsDefs;
      oNewTCtxt->style->principal     = m_ctxt->style;
    }
    */

    //don't need the template stack
    //sub-transform is standalone
    /*
    oNewTCtxt->templ    = m_ctxt->templ;
    oNewTCtxt->templNr  = m_ctxt->templNr;
    oNewTCtxt->templMax = m_ctxt->templMax;
    oNewTCtxt->templTab = m_ctxt->templTab;
    oNewTCtxt->xpathCtxt = m_ctxt->xpathCtxt;
    */

    //don't need local variables, inputs will have already been evaluated
    //enabling these will cause a double free of the stack objects
    //this updates varsNr, varsMax, varsTab and vars
    //xsltCopyStackElemList(oNewTCtxt, *m_ctxt->varsTab);
    /*
    oNewTCtxt->varsNr  = m_ctxt->varsNr;
    oNewTCtxt->varsMax = m_ctxt->varsMax;
    oNewTCtxt->varsTab = m_ctxt->varsTab;
    oNewTCtxt->vars    = m_ctxt->vars;
    */

    //enabling the doclist will cause shared keytable free problems
    //oNewTCtxt->docList        = m_ctxt->docList;

    //causes SIGSEGV faults when enabled
    //oNewTCtxt->cache        = m_ctxt->cache;
    
    //------------------------------------------------ new object
    IXslTransformContext *pNewTCtxt = new LibXslTransformContext(m_pOwnerQE, m_pOwnerQE, m_pSourceDoc, pChangeStylesheet, oNewTCtxt); //pQE as MM

    return pNewTCtxt;
  }

  void LibXslTransformContext::setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) {
    m_iTraceFlags = iFlags;
    m_ctxt->traceCode = &m_iTraceFlags;
  }

  IXmlLibrary::xsltTraceFlag LibXslTransformContext::traceFlags() const {
    return (IXmlLibrary::xsltTraceFlag) m_iTraceFlags;
  }

  void LibXslTransformContext::clearXSLTTraceFlags() {
    m_iTraceFlags = IXmlLibrary::XSLT_TRACE_NONE;
    m_ctxt->traceCode = &m_iTraceFlags;
  }

  void LibXslTransformContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {
    xmlXPathContextPtr oNewXPathContext = m_ctxt->xpathCtxt;
    XslTransformContext::setQueryEnvironment(pOwnerQE);

    //transform context settings
    m_ctxt->emo                 = (void*) m_pOwnerQE;     //XslModule link for our C++ extensions
    m_ctxt->xfilter->param      = (void*) m_pOwnerQE;
    m_ctxt->xtrigger->param     = (void*) m_pOwnerQE;

    //xpath mirror settings
    oNewXPathContext->emo      = (void*) m_pOwnerQE;
    //oNewXPathContext->xfilter  = oNewTCtxt->xfilter; //should always be the same
    //oNewXPathContext->xtrigger = oNewTCtxt->xtrigger; //should always be the same
  }

  void LibXslTransformContext::globalVariableUpdate(const char *sName, const char *sValue) {
    const char *params[4];
    params[0] = sName;
    params[1] = sValue;
    params[2] = 0;
    params[3] = 0;
    
    NOT_CURRENTLY_USED("");
    
    xsltEvalUserParams(m_ctxt, (const char**) &params, NULL); //@eval = 1
  }

  XmlNodeList<const IXmlBaseNode> *LibXslTransformContext::templateStack(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //return a bunch of xsl:template nodes
    XmlNodeList<const IXmlBaseNode> *pTemplateNodes = new XmlNodeList<const IXmlBaseNode>(this);
    for (int i = 0; i < m_ctxt->templNr; i++)
      pTemplateNodes->push_back(
        LibXmlLibrary::factory_node(
          pMemoryLifetimeOwner,
          m_ctxt->templTab[i]->elem,
          m_pStylesheet->queryInterface((const IXmlBaseDoc*) 0)
        )
      );
    return pTemplateNodes;
  }

  StringMap<const char*> *LibXslTransformContext::globalVars() const {
    _xmlHashEntry *pItem;
    _xsltStackElem *pPayload;
    StringMap<const char*> *pmVars = new StringMap<const char*>();
    const char *sName;

    if (m_ctxt->globalVars && m_ctxt->globalVars->table) {
      for (int i = 0; i < m_ctxt->globalVars->size; i++) {
        pItem = &m_ctxt->globalVars->table[i];
        if (pItem->name) {
          if (!*pItem->name) sName = MM_STRDUP("<blank name>");
          else               sName = MM_STRDUP((const char*) pItem->name);
          pPayload = (_xsltStackElem*) pItem->payload;
          pmVars->insert(sName, (pPayload && pPayload->name ? MM_STRDUP((const char*) pPayload->name) : 0));
        }
       }
    }

    return pmVars;
  }
  StringMap<const char*> *LibXslTransformContext::localVars() const {
    xsltStackElem *pItem;
    StringMap<const char*> *pmVars = new StringMap<const char*>();
    const char *sName;

    if (m_ctxt->varsTab) {
      for (int i = 0; i < m_ctxt->varsNr; i++) {
        pItem = m_ctxt->varsTab[i];
        while (pItem) {
          if (pItem->name) {
            if (!*pItem->name) sName = MM_STRDUP("<blank name>");
            else               sName = MM_STRDUP((const char*) pItem->name);
            pmVars->insert(sName, (pItem->select ? MM_STRDUP((const char*) pItem->select) : 0));
          }
          pItem = pItem->next;
        }
      }
    }

    return pmVars;
  }

  //documents and nodes (free these copies NOT_OURS)
  const IXmlBaseDoc *LibXslTransformContext::sourceDoc()  const {
    return m_pSourceDoc;
  }
  
  const IXslDoc *LibXslTransformContext::commandDoc() const {
    return m_pStylesheet;
  }
  
  const IXmlBaseNode *LibXslTransformContext::sourceNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees non-zero result
    //input XML document
    return m_ctxt->node ? LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->node, m_pSourceDoc, m_ctxt->xpathCtxt->parent_route) : 0;
  }

  const IXslCommandNode *LibXslTransformContext::commandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees non-zero result
    //commandNode will check all ancestors until it finds a node with an @xml:id
    //stylesheet and current xsl command, INCLUDING extension-prefix-elements
    const IXmlBaseNode *pCommandNode;
    const IXslCommandNode *pCommandNodeType = 0;

    if (m_ctxt->inst) {
      //NOTE: this ->inst does not seem to have a ->doc as the commandDoc()...
      pCommandNode     = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->inst, commandDoc());
      pCommandNodeType = pCommandNode->queryInterface((const IXslCommandNode*) 0);
    }

    return pCommandNodeType;
  }

  const IXmlBaseNode *LibXslTransformContext::literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //xsl command lists include literals
    //caller frees non-zero result
    //commandNode will check all ancestors until it finds a node with an @xml:id
    //stylesheet and current xsl command, INCLUDING extension-prefix-elements
    const IXmlBaseNode *pLiteralCommandNode = 0;

    if (m_ctxt->inst) {
      /* NOTE: this inst[ruction] is in its OWN XSL document
       * the xsl:stylesheet has a parent in to the main document
       * e.g. xsl:stylesheet => (doc change!) class:DatabaseElement
       * this is to allow xsl:include/@xpath to work
       */
      pLiteralCommandNode = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->inst, (IXmlBaseDoc*) m_pStylesheet->queryInterface((const IXmlBaseDoc*) 0));
    }

    return pLiteralCommandNode;
  }

  const IXslCommandNode *LibXslTransformContext::templateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //caller frees non-zero result
    //stylesheet and current xsl template
    const IXmlBaseNode *pCommandNode;
    const IXslCommandNode *pCommandNodeType = 0;

    if (m_ctxt->templ && m_ctxt->templ->elem) {
      pCommandNode     = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->templ->elem, (IXmlBaseDoc*) m_pStylesheet->queryInterface((const IXmlBaseDoc*) 0));
      pCommandNodeType = pCommandNode->queryInterface((const IXslCommandNode*) 0);
    }

    return pCommandNodeType;
  }

  const IXmlBaseNode *LibXslTransformContext::instructionNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //stylesheet and current instruction, INCLUDING direct output nodes, e.g. html:div
    //caller frees non-zero result
    //stylesheet and current xsl command
    const IXmlBaseNode *pInstructionNode = 0;

    if (m_ctxt->inst) {
      pInstructionNode = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->inst, (IXmlBaseDoc*) m_pStylesheet->queryInterface((const IXmlBaseDoc*) 0));
    }
    return pInstructionNode;
  }

  IXmlBaseNode *LibXslTransformContext::outputNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const {
    //output XML node
    //can be the document if no output node has been created yet
    IXmlBaseNode *pOutputNode = 0;
    
    //if we are at document level then we have not started the root output yet
    if (m_ctxt->insert) {
      //TODO: re-use the doc from somewhere? this resource is not freed?
      IXmlBaseDoc *pDoc = LibXmlLibrary::factory_document(pMemoryLifetimeOwner, m_pOwnerQE->xmlLibrary(), "output doc", m_ctxt->insert->doc, NOT_OURS);
      pOutputNode = LibXmlLibrary::factory_node(pMemoryLifetimeOwner, m_ctxt->insert, pDoc);
    }
    
    return pOutputNode;
  }

  const char *LibXslTransformContext::currentParentRoute() const {
    return (const char*) xmlListReport(m_ctxt->xpathCtxt->parent_route, (xmlListReporter) LibXmlBaseNode::xmlParentRouteReporter, BAD_CAST " => ", NULL);
  }

  const char *LibXslTransformContext::currentModeName() const {
    return MM_STRDUP((const char*) m_ctxt->mode);    
  }

  void LibXslTransformContext::setMode(const char *sWithMode) {
    if (sWithMode && *sWithMode) {
      //mode with characters (not "")
      //initial xsl:template @mode: actually check this properly
      if ((strchr(sWithMode, ' ') || strchr(sWithMode, ':'))) throw InvalidXMLName(this, sWithMode);
      m_ctxt->mode = (const xmlChar*) sWithMode;
    } else {
      //clear the mode. note that a "" blank string will also clear the mode here
      //because a "" blank string is a *VALID* mode in LibXML and will cause confusion!
      m_ctxt->mode = 0;
    }
  }

  bool LibXslTransformContext::clearOutputNode() {
    //clear it instead
    
    NOT_CURRENTLY_USED("clearOutputNode");
    
    m_ctxt->output    = 0;
    m_ctxt->insert    = 0;
    m_ctxt->topElemVisited = 0;

    return false;
  }

  bool LibXslTransformContext::updateOutputNode(const IXmlBaseNode *pOutputNode) const {
    //PRIVATE
    //TODO: if this context is inherited from the parent context then bTopElemVisited should be set to true
    //this prevents doc initial output like:
    //  processing instructions
    //  xml decleration
    const LibXmlBaseNode *pOutputNodeType;
    xmlNodePtr oOutputNode;

    assert(pOutputNode);
    
    //assign and check
    pOutputNodeType = dynamic_cast<const LibXmlBaseNode*>(pOutputNode);
    oOutputNode     = pOutputNodeType->libXmlNodePtr();

    //to prevent re-output of document level stuff
    //are we are continuing the same output?
    //DO NOT need to re-assess namespaces
    //<Database:transform     => direct output in to existing document flow at current node, thus all namespaces are on ancestors
    //Database:transform(...) => direct output to tmp area and then potential copy back in to document flow. namespace re-assessment needed
    m_ctxt->topElemVisited = (oOutputNode == m_ctxt->insert);

    //are we continuing from the inherited parent context output node?
    if (!m_ctxt->topElemVisited) {
      //output to a new specific area
      //m_ctxt->node = input node
      m_ctxt->output    = pOutputNodeType->libXmlDocPtr(); //output document
      m_ctxt->insert    = oOutputNode;                     //output node
    }

    return m_ctxt->topElemVisited;
  }
  
  void LibXslTransformContext::setStartNode(const IXmlBaseNode *pStartNode) {
    //libxslt:transform.c has been changed statically to pay attention to initialContextNode setting!!
    const LibXmlBaseNode *pStartNodeType;
    if (pStartNodeType = dynamic_cast<const LibXmlBaseNode*>(pStartNode)) {
      m_ctxt->initialContextDoc         = pStartNodeType->libXmlDocPtr();
      m_ctxt->initialContextNode        = pStartNodeType->libXmlNodePtr();
      m_ctxt->initialContextParentRoute = pStartNodeType->libXmlParentRoutePtr();
    }
  }

  void LibXslTransformContext::addParams(const StringMap<const size_t> *pParamsInt) {
    //caller frees everything
    //TODO: LibXslTransformContext::addParams() not cumulative");
    m_paramsInt.insert(pParamsInt);
  }

  void LibXslTransformContext::addParams(const StringMap<const char*> *pParamsChar) {
    //caller frees everything
    //TODO: LibXslTransformContext::addParams() not cumulative");
    m_paramsChar.insert(pParamsChar);
  }

  void LibXslTransformContext::addParams(const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet) {
    //caller frees everything
    //TODO: LibXslTransformContext::addParams() not cumulative");
    m_paramsNodeSet.insert(pParamsNodeSet);
  }

  const char **LibXslTransformContext::assembleStoredBasicParams() const {
    //PRIVATE function
    //stored intrinsic type parameters
    //node type params are directly added to the context immediately
    StringMap<const size_t>::const_iterator         iParamsInt;
    StringMap<const char* >::const_iterator         iParamsChar;
    char *sValue                    = 0;
    const char **params             = 0;
    size_t param                    = 0;

    //-------------------------------------------- compile simple primitive parameters array
    params              = (const char **) MM_MALLOC((m_paramsInt.size() + m_paramsChar.size() + 1) * 2 * sizeof(char*)); //stores names and values (*2)
    for (iParamsInt = m_paramsInt.begin(); iParamsInt != m_paramsInt.end(); iParamsInt++) {
      sValue = MM_MALLOC(48); //malloc freed later
      sprintf(sValue, "%lu", iParamsInt->second);
      params[param++] = iParamsInt->first;
      params[param++] = sValue; //TODO: free these?
    }
    for (iParamsChar = m_paramsChar.begin(); iParamsChar != m_paramsChar.end(); iParamsChar++) {
      sValue = m_pOwnerQE->xmlLibrary()->escape(iParamsChar->second);
      params[param++] = iParamsChar->first;
      params[param++] = strquote(sValue); //TODO: free these?
      if (sValue != iParamsChar->second) MM_FREE(sValue);
    }
    params[param] = 0; //zero complete array

    return params;
  }

  const char **LibXslTransformContext::assembleStoredNodeSetParams() const {
    //PRIVATE function
    //stored intrinsic type parameters
    //node type params are directly added to the context immediately
    StringMap<const XmlNodeList<const IXmlBaseNode>*>::const_iterator iParamsNodeSet;
    const char **params = NULL;
    size_t param = 0;

    //-------------------------------------------- compile simple primitive parameters array
    params              = (const char **) MMO_MALLOC((m_paramsNodeSet.size() + 1) * (sizeof(xmlNodeSetPtr) + sizeof(char*))); //stores names and values (*2), private method malloc
    for (iParamsNodeSet = m_paramsNodeSet.begin(); iParamsNodeSet != m_paramsNodeSet.end(); iParamsNodeSet++) {
      params[param++] = iParamsNodeSet->first;
      params[param++] = (const char*) LibXmlLibrary::xPathNodeSetCreate(this, iParamsNodeSet->second);
    }
    params[param] = 0; //zero complete array

    return params;
  }

  IXslTransformContext::iHardlinkPolicy LibXslTransformContext::hardlinkPolicy(const iHardlinkPolicy iNewHardlinkPolicy) {
    return (iHardlinkPolicy) xmlSetHardlinksTraversalPolicy(m_ctxt->xpathCtxt, iNewHardlinkPolicy);
  }

  IXslTransformContext::iHardlinkPolicy LibXslTransformContext::hardlinkPolicy(const char *sHardlinkPolicy) {
    iHardlinkPolicy iNewHardlinkPolicy;
    if      (_STREQUAL(sHardlinkPolicy, "include-all"))           iNewHardlinkPolicy = includeAllHardlinks;
    else if (_STREQUAL(sHardlinkPolicy, "suppress-recursive"))    iNewHardlinkPolicy = suppressRecursiveHardlinks;
    else if (_STREQUAL(sHardlinkPolicy, "suppress-repeat"))       iNewHardlinkPolicy = suppressRepeatHardlinks;
    else if (_STREQUAL(sHardlinkPolicy, "suppress-exact-repeat")) iNewHardlinkPolicy = suppressExactRepeatHardlinks;
    else if (_STREQUAL(sHardlinkPolicy, "suppress-all"))          iNewHardlinkPolicy = suppressAllHardlinks;
    else if (_STREQUAL(sHardlinkPolicy, "disable"))               iNewHardlinkPolicy = disableHardlinkRecording;
    else throw UnknownHardlinkPolicy(this, sHardlinkPolicy);
    return hardlinkPolicy(iNewHardlinkPolicy);
  }

  void LibXslTransformContext::continueTransformation(const IXmlBaseNode *pNewOutputNode) const {
    //<database:our-thing>
    //  <xsl:stuff...> => continue with this
    //</database:our-thing>
    //
    //m_ctxt->inst = <database:our-thing>
    //xsltApplySequenceConstructor() is called by things like xsltIf() to apply the child XSL commands
    //we made xsltApplySequenceConstructor() public
    if (pNewOutputNode) updateOutputNode(pNewOutputNode);
    if (m_ctxt->inst->children) xsltApplySequenceConstructor(m_ctxt, m_ctxt->node, m_ctxt->inst->children, NULL);
  }

  void LibXslTransformContext::xsltCommand_element() const {
    NOT_COMPLETE("xsltCommand_element");
    //xsltStyleItemElementPtr comp = 0;
    //xsltElement(xsltTransformContextPtr ctxt, xmlNodePtr node, xmlNodePtr inst, xsltStylePreCompPtr castedComp)
  }
}
