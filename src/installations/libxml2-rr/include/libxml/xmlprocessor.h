/*
 * Summary: interfaces for xpath compilation strategy
 *
 * Author: Annesley Newholm
 */

#ifndef __XML_PROCESSOR_H__
#define __XML_PROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xmlXPathProcessingCallbackContext xmlXPathProcessingCallbackContext;
typedef xmlXPathProcessingCallbackContext *xmlXPathProcessingCallbackContextPtr;

/* to avoid tree.h inclusion because cyclic */
struct _xmlNode;
struct _xmlList;
struct _xmlNs;
struct _xmlXPathContext;

typedef int (*xmlXPathProcessingCallbackFunc)(xmlXPathProcessingCallbackContextPtr ctxt, struct _xmlNode*, struct _xmlList*, int xpath_test_operation, const xmlChar *prefix, const xmlChar *localname, const char ***pvsGroupings);

struct _xmlXPathProcessingCallbackContext {
  xmlXPathProcessingCallbackFunc xmlXPathProcessingCallback; /* the callback */
  void *param;          /* usually a C++ object to call */
};

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

