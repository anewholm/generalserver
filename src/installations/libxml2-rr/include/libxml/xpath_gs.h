#define ESCAPE_CHAR '~'

/* ignore_type_policy */
#define ANY_NODE 0
#define SKIP_DTD_AND_ENTITY 1
#define XML_ELEMENT_NODE_ONLY 2
#define NO_SOFTLINK 4

/* xfilter */
XMLPUBFUN xmlNodeFilterCallbackContextPtr XMLCALL
        xmlCreateNodeFilterCallbackContext(
                    xmlElementFilterCallbackFunc xmlElementFilterCallback,
                    void *param);
XMLPUBFUN void XMLCALL
        xmlFreeNodeFilterCallbackContext(xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlNodeFilterCallbackContextPtr XMLCALL
        xmlCopyNodeFilterCallbackContext(xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN int XMLCALL
        xmlFilterCheck(xmlNodeFilterCallbackContextPtr xfilter,
                    xmlNodePtr cur, xmlListPtr parent_route, int securityOperation);
/* grammar processor */
XMLPUBFUN xmlGrammarProcessorCallbackContextPtr XMLCALL
        xmlCreateGrammarProcessorCallbackContext(
                    xmlGrammarProcessorCallbackFunc f,
                    void *param);
XMLPUBFUN void XMLCALL
        xmlXPathFreeGrammarProcessorContext(xmlGrammarProcessorCallbackContextPtr gp);
/* xtrigger */
XMLPUBFUN xmlNodeTriggerCallbackContextPtr XMLCALL
        xmlCreateNodeTriggerCallbackContext(
                    xmlElementTriggerCallbackFunc xmlElementTriggerCallback,
                    void *param,
                    xmlXPathContextPtr xpathCtxt);
XMLPUBFUN void XMLCALL
        xmlFreeNodeTriggerCallbackContext(xmlNodeTriggerCallbackContextPtr xtrigger);
XMLPUBFUN xmlNodeTriggerCallbackContextPtr XMLCALL
        xmlCopyNodeTriggerCallbackContext(xmlNodeTriggerCallbackContextPtr xtrigger);
XMLPUBFUN int XMLCALL
        xmlTriggerCall(xmlNodeTriggerCallbackContextPtr xtrigger,
                    xmlNodePtr cur, xmlListPtr parent_route, int triggerOperation, int triggerStage, xmlNodeFilterCallbackContextPtr xfilter);
/* xprocessing */
XMLPUBFUN xmlXPathProcessingCallbackContextPtr XMLCALL
        xmlCreateXPathProcessingCallbackContext(
                    xmlXPathProcessingCallbackFunc xmlXPathProcessingCallback,
                    void *param);
XMLPUBFUN void XMLCALL
        xmlFreeXPathProcessingCallbackContext(xmlXPathProcessingCallbackContextPtr xprocessing);
XMLPUBFUN xmlXPathProcessingCallbackContextPtr XMLCALL
        xmlCopyXPathProcessingCallbackContext(xmlXPathProcessingCallbackContextPtr xprocessing);
XMLPUBFUN int XMLCALL
        xmlXPathProcessingCall(xmlXPathProcessingCallbackContextPtr xprocessing,
                    xmlNodePtr cur, xmlListPtr parent_route, int test_type, const xmlChar *prefix, const xmlChar *localname, const char ***pvsGroupings);
        
XMLPUBFUN int XMLCALL
        xmlHasDescendantDeviants(xmlNodePtr hardlink, xmlNodePtr original);
XMLPUBFUN int XMLCALL
        xmlHasOriginals(xmlNodePtr cur, xmlNodePtr hardlink);
XMLPUBFUN int XMLCALL
        xmlHasDeviants(xmlNodePtr cur, xmlNodePtr hardlink);
XMLPUBFUN int XMLCALL
        xmlIsDeviant(xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlDeviateToDeviant(xmlXPathContextPtr context,
                          xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlDeviateToOriginal (xmlXPathContextPtr context,
                          xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlWriteableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlAddableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlReadableContextNode(xmlXPathContextPtr context, xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlReadableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlParentRouteUp(xmlXPathContextPtr ctxt, xmlNodePtr from_node, xmlNodePtr to_node);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlParentRouteDown(xmlXPathContextPtr ctxt, xmlNodePtr from_node, xmlNodePtr to_node);
XMLPUBFUN int XMLCALL
        xmlCheckRecursion(xmlListPtr list, xmlNodePtr new_node, int hardlinks_policy);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlVerticalParentRouteAdopt(xmlXPathContextPtr ctxt, xmlNodePtr from_node);

XMLPUBFUN xmlNodePtr XMLCALL xmlReadableNext(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadablePrev(xmlXPathParserContextPtr context, xmlNodePtr cur, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableLastChild(xmlXPathParserContextPtr context, xmlNodePtr parent, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableChildren(xmlXPathParserContextPtr context, xmlNodePtr parent, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableProperties(xmlXPathParserContextPtr context, xmlNodePtr parent);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableParent(xmlXPathParserContextPtr context, xmlNodePtr child, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableDocRoot(xmlXPathParserContextPtr ctxt, xmlNodePtr doc);

XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextNext(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextPrev(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextLast(xmlXPathContextPtr context, xmlNodePtr parent, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextChildren(xmlXPathContextPtr context, xmlNodePtr parent, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextProperties(xmlXPathContextPtr context, xmlNodePtr parent);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextParent(xmlXPathContextPtr context, xmlNodePtr child, int ignore_type_policy);
XMLPUBFUN xmlNodePtr XMLCALL xmlReadableContextDocRoot(xmlXPathContextPtr context, xmlNodePtr doc);

XMLPUBFUN xmlChar * XMLCALL
        xmlXPathNameAxisName(xmlNodePtr cur, const xmlChar *sNameMatch, xmlXPathContextPtr ctxt);
XMLPUBFUN int XMLCALL
        xmlSetHardlinksTraversalPolicy(xmlXPathContextPtr context, int hardlinks_policy);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlPushIfHardLinked(xmlListPtr nodeset, xmlNodePtr cur, int hardlinks_policy);
XMLPUBFUN xmlNodePtr XMLCALL
        xmlPopIfHardLinked(xmlListPtr nodeset, xmlNodePtr cur, int hardlinks_policy);

XMLPUBFUN int XMLCALL
        xmlXPathIdenticalNodeSets(xmlNodeSetPtr ns1, 
                                  xmlNodeSetPtr ns2, 
                                  int check_parent_route, 
                                  xmlXPathParserContextPtr ctxt ATTRIBUTE_UNUSED);
XMLPUBFUN void XMLCALL
        xmlXPathPop(xmlXPathParserContextPtr ctxt);
XMLPUBFUN void XMLCALL
        xmlXPathUnEscapeString(xmlChar *str);
XMLPUBFUN xmlChar* XMLCALL
        xmlXPathEscapeString(const xmlChar *str);
