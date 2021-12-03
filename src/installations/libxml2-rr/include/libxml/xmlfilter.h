/*
 * Summary: interfaces for tree security
 *
 * Author: Annesley Newholm
 *
 * IXmlQueryEnvironment:
 *    OP_EXECUTE = 1,
 *    OP_WRITE   = 2,
 *    OP_READ    = 4,
 *    OP_ADD     = 8
 */

#ifndef __XML_SECURITY_H__
#define __XML_SECURITY_H__

#define FILTER_OP_EXECUTE 1
#define FILTER_OP_WRITE   2
#define FILTER_OP_READ    4
#define FILTER_OP_ADD     8

#define FILTER_NODE_INCLUDE 1
#define FILTER_NODE_EXCLUDE 0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xmlNodeFilterCallbackContext xmlNodeFilterCallbackContext;
typedef xmlNodeFilterCallbackContext *xmlNodeFilterCallbackContextPtr;

/* to avoid tree.h inclusion because cyclic */
struct _xmlList;
struct _xmlNode;
struct _xmlNs;

typedef int (*xmlElementFilterCallbackFunc)(struct _xmlNode*, struct _xmlList*, xmlNodeFilterCallbackContextPtr, int filterOperation);

struct _xmlNodeFilterCallbackContext {
  xmlElementFilterCallbackFunc xmlElementFilterCallback; /* the callback */
  void *param;          /* usually a C++ object to call */
};

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

