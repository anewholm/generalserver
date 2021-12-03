/*
 * Summary: lists interfaces
 * Description: this module implement the list support used in
 * various place in the library.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Gary Pennington <Gary.Pennington@uk.sun.com>
 */

#ifndef __XML_LINK_INCLUDE__
#define __XML_LINK_INCLUDE__

#include <libxml/xmlversion.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xmlLink xmlLink;
typedef xmlLink *xmlLinkPtr;

typedef struct _xmlList xmlList;
typedef xmlList *xmlListPtr;

#include "list_gs.h"

/**
 * xmlListDeallocator:
 * @lk:  the data to deallocate
 *
 * Callback function used to free data from a list.
 */
typedef void (*xmlListDeallocator) (void *data);
/**
 * xmlListDataCompare:
 * @data0: the first data
 * @data1: the second data
 *
 * Callback function used to compare 2 data.
 *
 * Returns 0 is equality, -1 or 1 otherwise depending on the ordering.
 */
typedef int  (*xmlListDataCompare) (const void *data0, const void *data1);
/**
 * xmlListWalker:
 * @data: the data found in the list
 * @user: extra user provided data to the walker
 *
 * Callback function used when walking a list with xmlListWalk().
 *
 * Returns 0 to stop walking the list, 1 otherwise.
 */
typedef void* (*xmlListWalker) (const void *data, const void *user1, const void *user2);
typedef xmlChar* (*xmlListReporter) (const void *data, const void *user, const size_t report_len);

/* Creation/Deletion */
XMLPUBFUN xmlListPtr XMLCALL
                xmlListCreate           (xmlListDeallocator deallocator,
                                         xmlListDataCompare compare);
XMLPUBFUN int XMLCALL
                xmlLinkCompare          (const void *data0, 
                                         const void *data1);
XMLPUBFUN void XMLCALL
                xmlListDelete           (xmlListPtr l);

/* Basic Operators */
XMLPUBFUN int XMLCALL
                xmlListPointerCompare   (xmlListPtr l1, 
                                         xmlListPtr l2);
XMLPUBFUN int XMLCALL
                xmlListCompareItem      (xmlListPtr l, 
                                         void *data, 
                                         int i);
XMLPUBFUN int XMLCALL
                xmlListCompare          (xmlListPtr l1,
                                         xmlListPtr l2);
XMLPUBFUN void * XMLCALL
                xmlListSearch           (xmlListPtr l,
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListPointerCompareSearch
                                        (xmlListPtr l, 
                                         void *data);
XMLPUBFUN void * XMLCALL
                xmlListUnorderedSearch  (xmlListPtr l,
                                         void *data);
XMLPUBFUN void * XMLCALL
                xmlListReverseSearch    (xmlListPtr l,
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListStrafe           (xmlListPtr l, 
                                         int i, 
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListInsert           (xmlListPtr l,
                                         void *data) ;
XMLPUBFUN int XMLCALL
                xmlListAppend           (xmlListPtr l,
                                         void *data) ;
XMLPUBFUN int XMLCALL
                xmlListRemoveFirst      (xmlListPtr l,
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListRemoveLast       (xmlListPtr l,
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListRemoveAll        (xmlListPtr l,
                                         void *data);
XMLPUBFUN void XMLCALL
                xmlListClear            (xmlListPtr l);
XMLPUBFUN int XMLCALL
                xmlListIsEmpty            (xmlListPtr l);
XMLPUBFUN xmlLinkPtr XMLCALL
                xmlListFront            (xmlListPtr l);
XMLPUBFUN xmlLinkPtr XMLCALL
                xmlListEnd              (xmlListPtr l);
XMLPUBFUN void* XMLCALL
                xmlListItem             (xmlListPtr l, int i);
XMLPUBFUN int XMLCALL
                xmlListSize             (xmlListPtr l);

XMLPUBFUN void* XMLCALL
                xmlListPopFront         (xmlListPtr l);
XMLPUBFUN void* XMLCALL
                xmlListPopBack          (xmlListPtr l);
XMLPUBFUN int XMLCALL
                xmlListPushFront        (xmlListPtr l,
                                         void *data);
XMLPUBFUN int XMLCALL
                xmlListPushBack         (xmlListPtr l,
                                         void *data);

/* Advanced Operators */
XMLPUBFUN void XMLCALL
                xmlListReverse          (xmlListPtr l);
XMLPUBFUN void XMLCALL
                xmlListSort             (xmlListPtr l);
XMLPUBFUN xmlChar* XMLCALL
                xmlListReport           (xmlListPtr l, 
                                         xmlListReporter reporter, 
                                         const xmlChar *delimiter, 
                                         const void *user);
XMLPUBFUN xmlListPtr XMLCALL
                xmlListWalkTo           (xmlListPtr lFrom, 
                                         xmlListPtr lTo, 
                                         xmlListWalker walker, 
                                         const void *user1, 
                                         const void *user2);
XMLPUBFUN void* XMLCALL
                xmlListWalk             (xmlListPtr l,
                                         xmlListWalker walker,
                                         const void *user1,
                                         const void *user2);
XMLPUBFUN void* XMLCALL
                xmlListReverseWalk      (xmlListPtr l,
                                         xmlListWalker walker,
                                         const void *user1,
                                         const void *user2);
XMLPUBFUN void XMLCALL
                xmlListMerge            (xmlListPtr l1,
                                         xmlListPtr l2);
XMLPUBFUN xmlListPtr XMLCALL
                xmlListDup              (const xmlListPtr old);
XMLPUBFUN int XMLCALL
                xmlListCopy             (xmlListPtr cur,
                                         const xmlListPtr old);
/* Link operators */
XMLPUBFUN void * XMLCALL
                xmlLinkGetData          (xmlLinkPtr lk);

/* xmlListUnique() */
/* xmlListSwap */

#ifdef __cplusplus
}
#endif

#endif /* __XML_LINK_INCLUDE__ */
