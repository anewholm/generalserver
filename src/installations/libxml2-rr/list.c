/*
 * list.c: lists handling implementation
 *
 * Copyright (C) 2000 Gary Pennington and Daniel Veillard.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 * Author: Gary.Pennington@uk.sun.com
 */

#define IN_LIBXML
#include "libxml.h"

#include <stdlib.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/list.h>
#include <libxml/globals.h>

/*
 * Type definition are kept internal
 */

struct _xmlLink
{
    struct _xmlLink *next;
    struct _xmlLink *prev;
    void *data;
};

struct _xmlList
{
    xmlLinkPtr sentinel;
    void (*dataDeallocator)(void*);
    int (*linkCompare)(const void *, const void*);
    int unordered;
};

#include "list_gs.c"

/************************************************************************
 *                                    *
 *                Interfaces                *
 *                                    *
 ************************************************************************/

/**
 * xmlLinkDeallocator:
 * @l:  a list
 * @lk:  a link
 *
 * Unlink and deallocate @lk from list @l
 */
static void
xmlLinkDeallocator(xmlLinkPtr lk)
{
    (lk->prev)->next = lk->next;
    (lk->next)->prev = lk->prev;
    xmlFree(lk);
}

/**
 * xmlDataDeallocator:
 * @l:  a list
 * @lk:  a link
 *
 * Deallocate @lk data from list @l
 */
static void
xmlDataDeallocator(xmlListPtr l, xmlLinkPtr lk)
{
    void *data = xmlLinkGetData(lk);
    if((l->dataDeallocator != NULL) && (data != NULL)) l->dataDeallocator(data);
}

/**
 * xmlLinkCompare:
 * @data0:  first data
 * @data1:  second data
 *
 * Compares two arbitrary data
 *
 * Annesley: this is optional now and so is public, not static
 * 
 * Returns -1, 0 or 1 depending on whether data1 is greater equal or smaller
 *          than data0
 */
int 
xmlLinkCompare(const void *data0, const void *data1) {
    if (data0 < data1)       return (-1);
    else if (data0 == data1) return (0);
    return (1);
}

/**
 * xmlListLowerSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the ordered list walking from the beginning
 *
 * Returns the link containing the data or NULL
 */
static xmlLinkPtr
xmlListLowerSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(NULL);
    if (l->unordered == 1) xmlGenericError(xmlGenericErrorContext, "xmlListLowerSearch: list is unordered");
    
    for(lk = l->sentinel->next;lk != l->sentinel && l->linkCompare(lk->data, data) <0 ;lk = lk->next);
    
    return lk;
}

/**
 * xmlListHigherSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the ordered list walking backward from the end
 *
 * Returns the link containing the data or NULL
 */
static xmlLinkPtr
xmlListHigherSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(NULL);
    if (l->unordered == 1) xmlGenericError(xmlGenericErrorContext, "xmlListLowerSearch: list is unordered");
    
    for(lk = l->sentinel->prev;lk != l->sentinel && l->linkCompare(lk->data, data) >0 ;lk = lk->prev);
    
    return lk;
}

/**
 * xmlListLinkSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the list
 *
 * Returns the link containing the data or NULL
 */
static xmlLinkPtr
xmlListLinkSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;
    
    if (l == NULL) return(NULL);
    
    lk = xmlListLowerSearch(l, data);
    
    return (lk == l->sentinel ? NULL : lk);
}

/**
 * xmlListLinkReverseSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the list processing backward
 *
 * Returns the link containing the data or NULL
 */
static xmlLinkPtr
xmlListLinkReverseSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;
    
    if (l == NULL) return(NULL);
    
    lk = xmlListHigherSearch(l, data);
    
    return (lk == l->sentinel ? NULL : lk);
}

/**
 * xmlListCreate:
 * @deallocator:  an optional deallocator function
 * @compare:      an optional comparison function
 *
 * Create a new list
 *
 * Returns the new list or NULL in case of error
 */
xmlListPtr
xmlListCreate(xmlListDeallocator deallocator, xmlListDataCompare compare)
{
    xmlListPtr l;
    if (NULL == (l = (xmlListPtr )xmlMalloc( sizeof(xmlList)))) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for list");
        return (NULL);
    }
    /* Initialize the list to NULL */
    memset(l, 0, sizeof(xmlList));

    /* Add the sentinel */
    if (NULL ==(l->sentinel = (xmlLinkPtr )xmlMalloc(sizeof(xmlLink)))) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for sentinel");
        xmlFree(l);
        return (NULL);
    }
    l->sentinel->next = l->sentinel;
    l->sentinel->prev = l->sentinel;
    l->sentinel->data = NULL;

    /* dataDeallocator is optional: check before using */
    if (deallocator != NULL) l->dataDeallocator = deallocator;
    
    /* linkCompare is REQUIRED
     * default to xmlLinkCompare for basic pointer value comparison
     */
    l->linkCompare = (compare ? compare : xmlLinkCompare);

#ifdef LIBXML_RR
    if (compare == NULL) l->unordered = 1;
#endif
    
    return l;
}

/**
 * xmlListSearch:
 * @l:  a list
 * @data:  a search value
 *
 * Search the list for an existing value of @data
 *
 * Returns the value associated to @data or NULL in case of error
 */
void *
xmlListSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;
    if (l == NULL)
        return(NULL);
    lk = xmlListLinkSearch(l, data);
    if (lk)
        return (lk->data);
    return NULL;
}

/**
 * xmlListUnorderedSearch:
 * @l:  a list
 * @data:  a search value
 *
 * @author: Annesley Newholm
 *
 * xmlLists are ORDERED. the normal search alogrithums expect an ordered list
 * Search the entire un-ordered list for an existing value of @data
 *
 * Returns the value associated to @data or NULL in case of error
 */
void *
xmlListUnorderedSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(NULL);

    for(lk = l->sentinel->next;(lk != l->sentinel) && (l->linkCompare(lk->data, data) != 0) ;lk = lk->next);

    return (lk == l->sentinel ? NULL : lk->data);
}

/**
 * xmlListPointerCompareSearch:
 * @l:  a list
 * @data:  a search value
 *
 * @author: Annesley Newholm
 *
 * Search the entire list for an existing value of @data
 * direct pointer comparison ONLY
 *
 * Returns:
 *   0 - not found
 *   1 - found
 */
int
xmlListPointerCompareSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(0);

    for(lk = l->sentinel->next;(lk != l->sentinel) && (lk->data != data) ;lk = lk->next);

    return (lk == l->sentinel ? 0 : 1);
}

/**
 * xmlListReverseSearch:
 * @l:  a list
 * @data:  a search value
 *
 * Search the list in reverse order for an existing value of @data
 *
 * Returns the value associated to @data or NULL in case of error
 */
void *
xmlListReverseSearch(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;
    
    if (l == NULL) return(NULL);
    
    lk = xmlListLinkReverseSearch(l, data);

    return (lk ? lk->data : NULL);
}

/**
 * xmlListInsert:
 * @l:  a list
 * @data:  the data
 *
 * Insert data in the ordered list at the beginning for this value
 *
 * Returns 0 in case of success, 1 in case of failure
 */
int
xmlListInsert(xmlListPtr l, void *data)
{
    xmlLinkPtr lkPlace, lkNew;

    if (l == NULL) return(1);
    
    lkPlace = xmlListLowerSearch(l, data);
    /* Add the new link */
    lkNew = (xmlLinkPtr) xmlMalloc(sizeof(xmlLink));
    if (lkNew == NULL) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for new link");
        return (1);
    }
    lkNew->data = data;
    lkPlace = lkPlace->prev;
    lkNew->next = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev = lkPlace;
    return 0;
}

/**
 * xmlListAppend:
 * @l:  a list
 * @data:  the data
 *
 * Insert data in the ordered list at the end for this value
 *
 * Returns 0 in case of success, 1 in case of failure
 */
int xmlListAppend(xmlListPtr l, void *data)
{
    xmlLinkPtr lkPlace, lkNew;

    if (l == NULL) return(1);
    
    lkPlace = xmlListHigherSearch(l, data);
    
    /* Add the new link */
    lkNew = (xmlLinkPtr) xmlMalloc(sizeof(xmlLink));
    if (lkNew == NULL) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for new link");
        return (1);
    }
    lkNew->data = data;
    lkNew->next = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev = lkPlace;
    
    return 0;
}

/**
 * xmlListDelete:
 * @l:  a list
 *
 * Deletes the list and its associated data
 */
void xmlListDelete(xmlListPtr l) {
    if (l == NULL) return;

    xmlListClear(l);
    xmlFree(l->sentinel);
    xmlFree(l);
}

/**
 * xmlListRemoveFirst:
 * @l:  a list
 * @data:  list data
 *
 * Remove the first instance associated to data in the list
 *
 * Returns 1 if a deallocation occured, or 0 if not found
 */
int
xmlListRemoveFirst(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(0);
    
    /*Find the first instance of this data */
    lk = xmlListLinkSearch(l, data);
    if (lk != NULL) {
        xmlDataDeallocator(l, lk);
        xmlLinkDeallocator(lk);
        return 1;
    }
    
    return 0;
}

/**
 * xmlListRemoveLast:
 * @l:  a list
 * @data:  list data
 *
 * Remove the last instance associated to data in the list
 *
 * Returns 1 if a deallocation occured, or 0 if not found
 */
int
xmlListRemoveLast(xmlListPtr l, void *data)
{
    xmlLinkPtr lk;

    if (l == NULL) return(0);
    
    /*Find the last instance of this data */
    lk = xmlListLinkReverseSearch(l, data);
    if (lk != NULL) {
        xmlDataDeallocator(l, lk);
        xmlLinkDeallocator(lk);
        return 1;
    }
    
    return 0;
}

/**
 * xmlListRemoveAll:
 * @l:  a list
 * @data:  list data
 *
 * Remove the all instance associated to data in the list
 *
 * Returns the number of deallocation, or 0 if not found
 */
int
xmlListRemoveAll(xmlListPtr l, void *data)
{
    xmlLinkPtr  lk, next;
    int count = 0;

    if (l == NULL) return 0;
    
    lk = l->sentinel->next;
    while(lk != l->sentinel) {
        next = lk->next;
        if (xmlLinkGetData(lk) == data) {
          xmlDataDeallocator(l, lk);
          xmlLinkDeallocator(lk);
        }
        lk = next;
    }
    
    l->unordered = 0;
    
    return count;
}

/**
 * xmlListClear:
 * @l:  a list
 *
 * Remove the all data in the list
 */
void
xmlListClear(xmlListPtr l)
{
    xmlLinkPtr  lk, next;

    if (l == NULL) return;
    
    lk = l->sentinel->next;
    while(lk != l->sentinel) {
        next = lk->next;
        xmlDataDeallocator(l, lk);
        xmlLinkDeallocator(lk);
        lk = next;
    }
}

/**
 * xmlListIsEmpty:
 * @l:  a list
 *
 * Is the list empty ?
 *
 * Returns 1 if the list is empty, 0 if not empty and -1 in case of error
 */
int
xmlListIsEmpty(xmlListPtr l) {
  return (l == NULL) || (l->sentinel->next == l->sentinel);
}

/**
 * xmlListFront:
 * @l:  a list
 *
 * Get the first element in the list
 *
 * Returns the first element in the list, or NULL
 */
xmlLinkPtr
xmlListFront(xmlListPtr l)
{
    if (l == NULL)
        return(NULL);
    return (l->sentinel->next);
}

/**
 * xmlListEnd:
 * @l:  a list
 *
 * Get the last element in the list
 *
 * Returns the last element in the list, or NULL
 */
xmlLinkPtr
xmlListEnd(xmlListPtr l)
{
    if (l == NULL)
        return(NULL);
    return (l->sentinel->prev);
}

/**
 * xmlListSize:
 * @l:  a list
 *
 * Get the number of elements in the list
 *
 * Returns the number of elements in the list or -1 in case of error
 */
int
xmlListSize(xmlListPtr l)
{
    xmlLinkPtr lk;
    int count=0;

    if (l == NULL)
        return(-1);
    /* TODO: keep a counter in xmlList instead */
    for(lk = l->sentinel->next; lk != l->sentinel; lk = lk->next, count++);
    return count;
}

/**
 * xmlListItem:
 * xmlListLink: PRIVATE
 * 
 * @l:  a list
 * @i:  index of item to retrieve
 *
 * Get the element in the list by index
 * A Negative index will run backwards: last item index is -1
 *
 * Returns the element in the list
 * or NULL of out-of-range
 */
static xmlLinkPtr
xmlListLink(xmlListPtr l, int i) {
    xmlLinkPtr lk;
    
    if (l == NULL) return(NULL);
    
    if (i < 0) for (lk = l->sentinel->prev; (lk != l->sentinel) && (i < LAST_LISTITEM); lk = lk->prev, i++);
    else       for (lk = l->sentinel->next; (lk != l->sentinel) && (i > 0); lk = lk->next, i--);
    
    return (lk == l->sentinel ? NULL : lk);
}

void*
xmlListItem(xmlListPtr l, int i) {
    xmlLinkPtr lk = xmlListLink(l, i);
    return (lk == NULL ? NULL : lk->data);
}

/**
 * xmlListPopFront:
 * @l:  a list
 *
 * Removes the first element in the list and returns it
 */
void*
xmlListPopFront(xmlListPtr l)
{
    void *p = NULL;
    
    if (!xmlListIsEmpty(l)) {
        p = l->sentinel->next->data;
        xmlLinkDeallocator(l->sentinel->next);
    }
    
    return p;
}

/**
 * xmlListPopBack:
 * @l:  a list
 *
 * Removes the last element in the list and returns it
 */
void*
xmlListPopBack(xmlListPtr l)
{
    void *p = NULL;
    
    if (!xmlListIsEmpty(l)) {
        p = l->sentinel->prev->data;
        xmlLinkDeallocator(l->sentinel->prev);
    }
    
    return p;
}

/**
 * xmlListPushFront:
 * @l:  a list
 * @data:  new data
 *
 * add the new data at the beginning of the list
 *
 * Returns 0 if successful, 1 otherwise
 */
int
xmlListPushFront(xmlListPtr l, void *data)
{
    xmlLinkPtr lkPlace, lkNew;

    if (l == NULL) return(1);
    
    lkPlace = l->sentinel;
    
    /* if the list is not currently empty then another element will cause unordered */
    if (xmlListIsEmpty(l) == 0) l->unordered = 1;
    
    /* Add the new link */
    lkNew = (xmlLinkPtr) xmlMalloc(sizeof(xmlLink));
    if (lkNew == NULL) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for new link");
        return (1);
    }
    lkNew->data   = data;
    lkNew->next   = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev   = lkPlace;
    
    return 0;
}

/**
 * xmlListPushBack:
 * @l:  a list
 * @data:  new data
 *
 * add the new data at the end of the list
 *
 * Returns 0 if successful, 1 in the case of failure
 */
int
xmlListPushBack(xmlListPtr l, void *data)
{
    xmlLinkPtr lkPlace, lkNew;

    if (l == NULL) return(1);
    
    lkPlace = l->sentinel->prev;

    /* if the list is empty then the second element will cause unordered */
    if (xmlListIsEmpty(l) == 0) l->unordered = 1;
    
    /* Add the new link */
    if (NULL ==(lkNew = (xmlLinkPtr )xmlMalloc(sizeof(xmlLink)))) {
        xmlGenericError(xmlGenericErrorContext, "Cannot initialize memory for new link");
        return (1);
    }
    lkNew->data = data;
    lkNew->next = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev = lkPlace;
    
    return 0;
}

/**
 * xmlLinkGetData:
 * @lk:  a link
 *
 * See Returns.
 *
 * Returns a pointer to the data referenced from this link
 */
void *
xmlLinkGetData(xmlLinkPtr lk)
{
    if (lk == NULL)
        return(NULL);
    return lk->data;
}

/**
 * xmlListReverse:
 * @l:  a list
 *
 * Reverse the order of the elements in the list
 */
void
xmlListReverse(xmlListPtr l)
{
    xmlLinkPtr lk;
    xmlLinkPtr lkPrev;

    if (l == NULL)
        return;
    lkPrev = l->sentinel;
    for (lk = l->sentinel->next; lk != l->sentinel; lk = lk->next) {
        lkPrev->next = lkPrev->prev;
        lkPrev->prev = lk;
        lkPrev = lk;
    }
    /* Fix up the last node */
    lkPrev->next = lkPrev->prev;
    lkPrev->prev = lk;
}

/**
 * xmlListCompareItem:
 * @l:     a list
 * @data:  compare item
 * @index: LAST_LISTITEM (-1) indicates last item
 * 
 * @author: Annesley
 *
 * Compare data with an item in the list using the standard xmlLinkCompare
 * NOTE: that linkCompare() returns:
 *   -1 - less than
 *    0 - equal
 *    1 - greater than
 * 
 * returns:
 *   1 - equal or if both NULL
 *   0 - not equal
 */
int
xmlListCompareItem(xmlListPtr l, void *data, int i) {
  xmlLinkPtr lk = xmlListLink(l, i);
  return ((lk != NULL) && (l->linkCompare(lk->data, data) == 0) ? 1 : 0);
}

/**
 * xmlListPointerCompare:
 * @l1:  a list
 * @l2:  a list
 * 
 * @author: Annesley
 *
 * Compare data in each list. Order is important, do not use linkCompare
 * 
 * returns 1 if equal OR if both NULL
 */
int
xmlListPointerCompare(xmlListPtr l1, xmlListPtr l2) {
    xmlLinkPtr l1k, l2k;

    if ((xmlListIsEmpty(l1) == 1) && (xmlListIsEmpty(l2) == 1)) return (1);
    if ((l1 == NULL) || (l2 == NULL)) return 0;
    
    l2k = l2->sentinel->next;
    for (l1k = l1->sentinel->next; 
            (l1k != l1->sentinel) 
         && (l2k != l2->sentinel)
         && (l1k->data == l2k->data); /* pointer compare */
         l1k = l1k->next
    ) {
      l2k = l2k->next;
    }
    
    return ((l1k == l1->sentinel) && (l2k == l2->sentinel) ? 1 : 0);
}

/**
 * xmlListCompare:
 * @l1:  a list
 * @l2:  a list
 * 
 * @author: Annesley
 *
 * Compare data in each list. Order is important.
 * 
 * returns 1 if equal OR if both NULL
 */
int
xmlListCompare(xmlListPtr l1, xmlListPtr l2) {
    xmlLinkPtr l1k, l2k;

    if ((xmlListIsEmpty(l1) == 1) && (xmlListIsEmpty(l2) == 1)) return (1);
    if ((l1 == NULL) || (l2 == NULL)) return 0;
    
    l2k = l2->sentinel->next;
    for (l1k = l1->sentinel->next; 
            (l1k != l1->sentinel) 
         && (l2k != l2->sentinel)
         && (l1->linkCompare(l1k->data, l2k->data) == 0); /* equal using linkCompare */
         l1k = l1k->next
    ) {
      l2k = l2k->next;
    }
    
    return ((l1k == l1->sentinel) && (l2k == l2->sentinel) ? 1 : 0);
}

/**
 * xmlListSort:
 * @l:  a list
 *
 * Sort all the elements in the list
 */
void
xmlListSort(xmlListPtr l)
{
    xmlListPtr lTemp;

    if (l == NULL)
        return;
    if(xmlListIsEmpty(l))
        return;

    /* I think that the real answer is to implement quicksort, the
     * alternative is to implement some list copying procedure which
     * would be based on a list copy followed by a clear followed by
     * an insert. This is slow...
     */

    if (NULL ==(lTemp = xmlListDup(l)))
        return;
    xmlListClear(l);
    xmlListMerge(l, lTemp);
    xmlListDelete(lTemp);
    
    l->unordered = 0;
    
    return;
}

/**
 * xmlListReport:
 * @l:         a list
 * @reporter:  a reporting function
 * @delimiter: delimiter between link reports, e.g. a comma
 * @user:      a user parameter passed to the walker function
 *
 * Walk all the element of the first from first to last and
 * compile the report
 * 
 * Returns the string report
 * Caller frees result
 * 
 */
xmlChar *
xmlListReport(xmlListPtr l, xmlListReporter reporter, const xmlChar *delimiter, const void *user) {
    xmlLinkPtr lk;
    xmlChar *report;
    const xmlChar *link_report;
    size_t report_len, link_report_len, delimiter_len;
    unsigned int free_report;

    if ((l == NULL) || (reporter == NULL)) return NULL;
    if (delimiter == NULL) delimiter = BAD_CAST ", ";
    
    report        = xmlStrdup(BAD_CAST "");
    report_len    = 0;
    delimiter_len = xmlStrlen(delimiter);
    for(lk = l->sentinel->next; (lk != l->sentinel); lk = lk->next) {
        link_report = reporter(lk->data, user, report_len);
        free_report = 0;
        if      (link_report == NULL)  link_report = BAD_CAST "(NULL)";
        else if (*link_report == '\0') link_report = BAD_CAST "(EMPTY)";
        else free_report = 1;
        
        /* concatentate */
        link_report_len = xmlStrlen(link_report); /* always more than zero */
        report = xmlRealloc(report,  /* copies the existing data if moved */
            report_len
          + link_report_len 
          + (report_len ? delimiter_len : 0) /* if we have a delimiter and it is not the first link report */
          + 1
        );
        if (report != NULL) {
          if (delimiter_len && report_len) {
            xmlStrcpy(report + report_len, delimiter);
            report_len += delimiter_len;
          }
          xmlStrcpy(report + report_len, link_report);
          report_len += link_report_len;
        }
        if (free_report) xmlFree((void*) link_report);
    }
    
    return report;
}

/**
 * xmlListWalk:
 * @l:  a list
 * @walker:  a processing function
 * @user1:  a user parameter passed to the walker function
 * @user2:  a user parameter passed to the walker function
 *
 * Walk all the element of the first from first to last and
 * apply the walker function to it
 * 
 * xmlListWalker() returns:
 *   NULL    - continue walking
 *   1       - terminate walking and return NULL (error)
 *   poniter - terminate walking, update the _xmlLink value and return the pointer
 * 
 * Returns the xmlListWalker() returned pointer or NULL
 */
void *
xmlListWalk(xmlListPtr l, xmlListWalker walker, const void *user1, const void *user2) {
    xmlLinkPtr lk;
    void *data = NULL;

    if ((l == NULL) || (walker == NULL)) return NULL;
    
    for(lk = l->sentinel->next; (lk != l->sentinel) && (data == NULL); lk = lk->next) {
        data = walker(lk->data, user1, user2);
        if ((data != NULL) && (data != (void*) 1)) {
            /* terminate with update */
            if (lk->data != data) {
              if (l->dataDeallocator) l->dataDeallocator(lk); /* will only call if non-NULL */
              lk->data = data;
            }
        }
    }
    
    return (void*) data;
}

/**
 * xmlListReverseWalk:
 * @l:  a list
 * @walker:  a processing function
 * @user1:  a user parameter passed to the walker function
 * @user2:  a user parameter passed to the walker function
 *
 * Walk all the element of the list in reverse order and
 * apply the walker function to it
 * 
 * xmlListWalker() returns:
 *   NULL    - continue walking
 *   1       - terminate walking and return NULL (error)
 *   poniter - terminate walking, update the _xmlLink value and return the pointer
 * 
 * Returns the xmlListWalker() returned pointer or NULL
 */
void *
xmlListReverseWalk(xmlListPtr l, xmlListWalker walker, const void *user1, const void *user2) {
    xmlLinkPtr lk;
    void *data = NULL;

    if ((l == NULL) || (walker == NULL)) return NULL;
    
    for(lk = l->sentinel->prev; (lk != l->sentinel) && (data == NULL); lk = lk->prev) {
        data = walker(lk->data, user1, user2);
        if ((data != NULL) && (data != (void*) 1)) {
            /* terminate with update */
            if (lk->data != data) {
              if (l->dataDeallocator) l->dataDeallocator(lk); /* will only call if non-NULL */
              lk->data = data;
            }
        }
    }
    
    return (void*) data;
}

/**
 * xmlListMerge:
 * @l1:  the original list
 * @l2:  the new list
 *
 * include all the elements of the second list in the first one and
 * clear the second list
 * 
 * we temporary disable any deallocation because the new list will be freed otherwise
 */
void
xmlListMerge(xmlListPtr l1, xmlListPtr l2)
{
    void (*dealloc)(void*);
    dealloc = l2->dataDeallocator;
    l2->dataDeallocator = NULL;
    
    xmlListCopy(l1, l2);
    xmlListClear(l2);
    
    l2->dataDeallocator = dealloc;
}

/**
 * xmlListDup:
 * @old:  the list
 *
 * Duplicate the list
 *
 * Returns a new copy of the list or NULL in case of error
 */
xmlListPtr
xmlListDup(const xmlListPtr old)
{
    xmlListPtr cur;
    xmlLinkPtr lk;

    if (old == NULL) return(NULL);
    
    /* Hmmm, how to best deal with allocation issues when copying
     * lists. If there is a de-allocator, should responsibility lie with
     * the new list or the old list. Surely not both. I'll arbitrarily
     * set it to be the old list for the time being whilst I work out
     * the answer
     * 
     * Annesley: we make an EXACT unordered copy
     * previously it was using xmlListInsert()
     */
    cur = xmlListCreate(NULL, old->linkCompare);
    if (NULL != cur) {
      for (lk = old->sentinel->next; lk != old->sentinel; lk = lk->next) {
          if (0 != xmlListPushBack(cur, lk->data)) {
              xmlListDelete(cur);
              return (NULL);
          }
      }
    }
    
    /* the xmlListPushBack() will have caused the unorderedflag to be set but it is not true */
    cur->unordered = old->unordered;
    
    return cur;
}

/**
 * xmlListCopy:
 * @cur:  the new list
 * @old:  the old list
 *
 * Move all the element from the old list in the new list
 *
 * Returns 0 in case of success 1 in case of error
 */
int
xmlListCopy(xmlListPtr cur, const xmlListPtr old)
{
    /* Walk the old tree and insert the data into the new one */
    xmlLinkPtr lk;

    if ((old == NULL) || (cur == NULL)) return(1);
    if (old->dataDeallocator) 
      xmlGenericError(xmlGenericErrorContext, "xmlListCopy: old items have a dataDeallocator and will be freed in subsequent xmlListDelete");
    
    if (cur->unordered == 0) { /* ORDERED */
      for(lk = old->sentinel->next; lk != old->sentinel; lk = lk->next) {
          if (0 != xmlListInsert(cur, lk->data)) return (1);
      }
    } else { /* UNORDERED */
      for(lk = old->sentinel->next; lk != old->sentinel; lk = lk->next) {
          if (0 != xmlListPushBack(cur, lk->data)) return (1);
      }
    }
    
    return (0);
}
/* xmlListUnique() */
/* xmlListSwap */
#define bottom_list
#include "elfgcchack.h"
