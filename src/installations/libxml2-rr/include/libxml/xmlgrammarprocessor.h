/*
 * Summary: interfaces for external grammar processors
 *
 * Author: Annesley Newholm
 *
 * e.g. ~Person == id('Class__Person')
 * e.g. #x == id(x)
 */

#ifndef __XML_GRAMMAR_PROCESSOR_H__
#define __XML_GRAMMAR_PROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xmlGrammarProcessorCallbackContext xmlGrammarProcessorCallbackContext;
typedef xmlGrammarProcessorCallbackContext *xmlGrammarProcessorCallbackContextPtr;

/* to avoid tree.h inclusion because cyclic */
struct _xmlNode;
struct _xmlXPathParserContext;

typedef const xmlChar* (*xmlGrammarProcessorCallbackFunc)(xmlGrammarProcessorCallbackContextPtr ctxt, xmlNodePtr *new_context, xmlListPtr *parent_route, int *skip, int iTraceFlags);

struct _xmlGrammarProcessorCallbackContext {
  xmlGrammarProcessorCallbackFunc xmlGrammarProcessorCallback; /* the callback */
  void *param;          /* usually a C++ object to call */
  struct _xmlXPathParserContext *xpathParserCtxt;
};

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

