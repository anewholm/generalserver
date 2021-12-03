/*
 * Summary: interfaces for tree triggers
 *
 * Author: Annesley Newholm
 *
 * IXmlQueryEnvironment:
 *    OP_EXECUTE = 1,
 *    OP_WRITE   = 2,
 *    OP_READ    = 4,
 *    OP_ADD     = 8
 */

#ifndef __XML_TRIGGER_H__
#define __XML_TRIGGER_H__

#define TRIGGER_STAGE_BEFORE 0
#define TRIGGER_STAGE_AFTER  1

#define TRIGGER_OP_EXECUTE 1
#define TRIGGER_OP_WRITE   2
#define TRIGGER_OP_READ    4
#define TRIGGER_OP_ADD     8

#define TRIGGER_CONTINUE 1
#define TRIGGER_STOP     0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xmlNodeTriggerCallbackContext xmlNodeTriggerCallbackContext;
typedef xmlNodeTriggerCallbackContext *xmlNodeTriggerCallbackContextPtr;

/* to avoid tree.h inclusion because cyclic */
struct _xmlList;
struct _xmlNode;
struct _xmlNs;
struct _xmlXPathContext;

typedef int (*xmlElementTriggerCallbackFunc)(struct _xmlNode*, struct _xmlList*, xmlNodeTriggerCallbackContextPtr, int triggerOperation, int triggerStage, xmlNodeFilterCallbackContextPtr xfilter);

struct _xmlNodeTriggerCallbackContext {
  xmlElementTriggerCallbackFunc xmlElementTriggerCallback; /* the callback */
  void *param;          /* usually a C++ object to call */
  struct _xmlXPathContext *xpathCtxt;
};

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

