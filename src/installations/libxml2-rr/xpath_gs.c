/**
 * xmlXPathDocumentOrder:
 * @node:  the node
 * 
 * @author: Annesley Newholm
 * exposed for the xsl:sort document-order() capacity
 * Returns the populated content of the document order from xmlXPathOrderDocElems()
 */
unsigned long
xmlXPathDocumentOrder(xmlNodePtr node) {
  return ((node != NULL) && (node->type == XML_ELEMENT_NODE) ? -(long)node->content : 0L);
}

/**
 * xmlXPathUnEscapeString:
 * @str:   the string containing \... escape sequences
 *
 * @author: Annesley
 *
 * See the code below for permissable escape sequences
 * IN-PLACE replacement
 * 
 * Returns the current string with escape sequences replaced
 */
void xmlXPathUnEscapeString(xmlChar *str) {
  xmlChar c;

  if (str == NULL) return;
  
  while ((c = *str)) {
    if (c == ESCAPE_CHAR) {
      switch (str[1]) {
        case '0':  *str = '\0';  break;
        case 'a':  *str = '\a';  break;
        case 'b':  *str = '\b';  break;
        case 'f':  *str = '\f';  break;
        case 'n':  *str = '\n';  break;
        case 'r':  *str = '\r';  break;
        case 't':  *str = '\t';  break;
        case 'v':  *str = '\v';  break;
        case '?':  *str = '\?';  break;
        case '\'': *str = '\'';  break;
        case '"':  *str = '"';   break;
        case '\\': *str = '\\';  break;
        case '/':  *str = '/';   break;
        default:   *str = str[1];       /* copy the literal char: unusal */
      }
      /* move the string over the escape char: redutive 
       * TODO: this is not optimal: we should set up a progressive back pointer
       */
      str++;
      xmlStrcpy(str, str+1);
    } else {
      /* in-place replacement, no action needed */
      str++;
    }
  }
}

/**
 * xmlXPathEscapeString:
 * @str:   the string
 *
 * @author: Annesley
 *
 * See the code below for escape sequences
 * free only if changed
 * 
 * Returns input string
 * or the new allocated string with escape sequences
 */
xmlChar *xmlXPathEscapeString(const xmlChar *str) {
  xmlChar c;
  xmlChar *ret, *retpos;
  const xmlChar *strpos;
  size_t retlen;

  if (str == NULL) return NULL;
  
  ret    = (xmlChar*) str; //dual purpose
  retpos = ret;
  strpos = str;
  while ((c = *strpos)) {
    switch (c) {
      /* these chars are not what they are */
      case '\0': c = '0';
      case '\a': c = 'a';
      case '\b': c = 'b';
      case '\f': c = 'f';
      case '\n': c = 'n';
      case '\r': c = 'r';
      case '\t': c = 't';
      case '\v': c = 'v';
      case '\?': c = '?';
      /* these chars are what they are */
      case '\'':
      case '"':
      case '\\':
      case '/':
        if (ret == str) {
          /* lazy malloc 
           * double space to account for full potential escaping
           */
          retlen = retpos - ret;
          ret    = xmlMalloc(retlen + (xmlStrlen(strpos) * 2) + 1);
          memcpy(ret, str, retlen);
          retpos = ret + retlen;
        }
        
        *retpos++ = ESCAPE_CHAR;
      default:
        if (ret != str) *retpos = c;
    }

    retpos++;
    strpos++;
  }
  
  if (ret != str) {
    *retpos = '\0';
    /* either equal or reductive */
    ret = xmlRealloc(ret, retpos - ret + 1);
  }
  
  return ret;
}

/**
 * xmlXPathNextDescendantNatural:
 * @ctxt:  the XPath Parser context
 * @cur:   the current attribute in the traversal
 * @hits:  the current list of XP_TEST_HIT nodes (included by Annesley)
 *
 * @author: Annesley
 *
 * Traversal function for the "descendant" direction
 *   hardlinks are NOT traversed
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextDescendantNatural(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
    xmlNodePtr next, retNode;
    xmlListPtr oldParentRoute;

    if (ctxt == NULL)                                               return(NULL);
    if ((cur == NULL) && (ctxt->context->node == NULL))             return(NULL);
    if ((cur == NULL) && (xmlIsHardLink(ctxt->context->node) == 1)) return(NULL);
    if ((cur == NULL) && ((ctxt->context->node->type == XML_ATTRIBUTE_NODE) || (ctxt->context->node->type == XML_NAMESPACE_DECL))) return(NULL);

    retNode = NULL;
    
    /* prevent deviations and straffing
     * by ignoring the current hardlink route for this node
     */
    oldParentRoute              = ctxt->context->parent_route;
    ctxt->context->parent_route = NULL;
    
    /* ------------------------------ start point => children (notSelf)
     * context node is NOT a hardlink so will be a valid parent to return to
     * now proceed immediately, and only, to the first children traverse
     */
    if (cur == NULL) cur = ctxt->context->node; 

    /* ------------------------------ children first */
    next = xmlReadableChildren(ctxt, cur, SKIP_DTD_AND_ENTITY);
    while ((next != NULL) && (xmlIsHardLink(next) == 1)) {      /* AVOID hardlinks (and softlinks) */
      next = xmlReadableNext(ctxt, next, SKIP_DTD_AND_ENTITY);
    }
    if (next != NULL) retNode = next;

    /* ------------------------------ then siblings
     * give up if we could not descend to the first children 
     */
    if ((retNode == NULL) && (cur != ctxt->context->node)) {
      next = xmlReadableNext(ctxt, cur, SKIP_DTD_AND_ENTITY);
      while ((next != NULL) && (xmlIsHardLink(next) == 1)) {      /* AVOID hardlinks (and softlinks) */
        next = xmlReadableNext(ctxt, next, SKIP_DTD_AND_ENTITY);
      }
      if (next != NULL) retNode = next;
    }

    /* ------------------------------ then up parents and along nexts */
    if ((retNode == NULL) && (cur != ctxt->context->node)) {
      do {
          cur = xmlReadableParent(ctxt, cur, XML_ELEMENT_NODE_ONLY);
          if ((cur == NULL)                    /* unattached or XML_DOCUMENT_NODE */
            || (cur == ctxt->context->node)    /* arrived back at top */
            || (cur->type != XML_ELEMENT_NODE) /* XML_DOCUMENT_NODE */
          ) {
            cur = NULL; /* will break the loop */
          }
          if (cur != NULL) {
            next = xmlReadableNext(ctxt, cur, 1);
            while ((next != NULL) && (xmlIsHardLink(next) == 1)) {      /* AVOID hardlinks */
              next = xmlReadableNext(ctxt, next, 1);
            }
            if (next != NULL) retNode = next;
          }
      } while ((cur != NULL) && (retNode == NULL));
    }

    /* just in case the down moves has created a parent_route */
    if (ctxt->context->parent_route != NULL) xmlListDelete(ctxt->context->parent_route);
    ctxt->context->parent_route = oldParentRoute;
    
    return(retNode);
}


#ifdef LIBXML_DEBUG_ENABLED
/**
 * xmlHardlinksTraversalPolicyName
 * @hardlinks_policy: the integer value
 * 
 * Returns the name of the policy
 */
static const char*
xmlHardlinksTraversalPolicyName(int hardlinks_policy) {
  const char *policy;

  switch (hardlinks_policy) {
    case 0:  policy = "include-all";           break;
    case 1:  policy = "suppress-recursive";    break;
    case 2:  policy = "suppress-repeat";       break;
    case 3:  policy = "suppress-exact-repeat"; break;
    case 4:  policy = "suppress-all";          break;
    case 5:  policy = "disable";               break; /* hardlinks are not remembered here */
    default: policy = "unknown";
  }
  
  return policy;
}
#endif

/**
 * xmlSetHardlinksTraversalPolicy,
 * @context: the XPath context
 * @nodeset: the new hardlinked nodeset for the traversal
 *
 * caller frees the @oldnodeset result if non-zero
 * no copying is done: do not free the input @nodeset!
 * used by traversal functions only
 * 
 * usually set with <database:do-something @hardlink-policy=...
 *
 * Returns the previous nodeset
 */
int
xmlSetHardlinksTraversalPolicy(xmlXPathContextPtr context, int hardlinks_policy) {
  int oldHardlinksPolicy;

  if (context == NULL)       xmlGenericError(xmlGenericErrorContext, "xmlSetHardlinksTraversalPolicy() : context is NULL");
  if (hardlinks_policy == 1) xmlGenericError(xmlGenericErrorContext, "xmlSetHardlinksTraversalPolicy() : suppress-recursive not supported yet");

  oldHardlinksPolicy = context->hardlinks_policy;
  if (oldHardlinksPolicy != hardlinks_policy) {
#ifdef LIBXML_DEBUG_ENABLED
    XML_TRACE_GENERIC3(XML_DEBUG_PARENT_ROUTE, "[parent_route] policy set 0x[%p] [%s] => [%s]", 
      (void*) context, 
      xmlHardlinksTraversalPolicyName(context->hardlinks_policy), 
      xmlHardlinksTraversalPolicyName(hardlinks_policy)
    );
#endif

    /* usually set with <database:do-something @hardlink-policy=... */
    context->hardlinks_policy = hardlinks_policy; 
  }

  return oldHardlinksPolicy;
}

/**
 * xmlCheckRecursion:
 * @list:    the sequential node store
 * @hardlinks_policy: checking for unique in the list and returning NULL if not
 *   0 = no check
 *   1 = TODO: check for exact node pointer in the exact same context as before
 *   2 = check for node and any hardlinks of the node
 *   3 = check for exact node pointer, in any ancestor context
 *   4 = reject all hardlinks
 *   5 = disabled: returns input
 *
 * carries out an infinite recursion check
 * used by:
 *   stateful xmlXPathParserContext->traversal_state during a single complex xmlXPathNext* traversal function
 *     directly use xmlListPushBack and xmlCheckRecursion function with ->traversal_state in this case
 *     note that actually xmlPopIfHardLinked is not used by traversal_state because it does not pop, but manually progresses instead
 *   more stateful xmlXPathContext->parent_route during an XSLT across multiple xpath statements and traverses
 *     use xmlReadable* and xmlParentRoute* in this case which uses parent_route
 *
 * Application options (for search):
 *   includeAllHardlinks = 0;
 *   suppressRecursiveHardlinks = 1;
 *   suppressRepeatHardlinks = 2;
 *   suppressExactRepeatHardlinks = 3;
 *   suppressAllHardlinks = 4;
 *   disableHardlinkRecording = 5
 *
 * Returns 1 if recursion detected
 * Will also throw an error
 */
int
xmlCheckRecursion(xmlListPtr parent_route, xmlNodePtr new_node, int hardlinks_policy) {
  int recursion_found;

  if (parent_route == NULL) return (0);
  if (new_node     == NULL) return (0);
  if (hardlinks_policy == suppressRecursiveHardlinks) xmlGenericError(xmlGenericErrorContext, "xmlCheckRecursion() : suppress recursive does not work yet!!!");
  if (hardlinks_policy == disableHardlinkRecording) return(0);
  if (hardlinks_policy == suppressAllHardlinks) return(1);

  recursion_found = 0;
  switch (hardlinks_policy) {
    case suppressRepeatHardlinks: /* 2 */
      /* uses the standard linkCompare = xmlAreEqualOrHardLinkedListCompare */
      if (xmlListUnorderedSearch(parent_route, new_node) != NULL) recursion_found = 1;
      break;
    case suppressExactRepeatHardlinks: /* 3 */
      /* direct pointer compare */
      if (xmlListPointerCompareSearch(parent_route, new_node) == 1) recursion_found = 1;
      break;
    default:
      xmlGenericError(xmlGenericErrorContext, "xmlCheckRecursion() : hardlink policy [%i] not understood", hardlinks_policy);
  }

  if (recursion_found == 1) 
    XML_TRACE_GENERIC2(XML_DEBUG_PARENT_ROUTE, "[parent_route] recursion detected adding [%p] with policy [%s]", (void*) new_node, xmlHardlinksTraversalPolicyName(hardlinks_policy));

  return(recursion_found);
}



/**
 * xmlXPathNextParentNatural:
 * @ctxt:  the XPath Parser context
 * @cur:  the current node in the traversal
 *
 * Traversal function for the "parent" direction
 * The parent axis contains the parent of the context node, if there is one.
 * This natural axis ignores the accrued parent_route
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextParentNatural(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  xmlNodePtr retNode;
  xmlListPtr oldParentRoute;
  
  oldParentRoute = ctxt->context->parent_route;
  ctxt->context->parent_route = NULL;
  retNode = xmlXPathNextParent(ctxt, cur, hits); /* will not create a parent_route */
  ctxt->context->parent_route = oldParentRoute;
  
  return retNode;
}


/**
 * xmlXPathNextTop:
 * @ctxt: the XPath Parser context
 * @cur:  the current attribute in the traversal
 * @hits:  the current list of XP_TEST_HIT nodes (included by Annesley)
 *
 * @author: Annesley
 *
 * Traversal function for the "top" direction
 * very similar to the xmlXPathNextDescendant
 * contains only the top successful hits on each branch of the descendant axis
 * this can only be ascertained from the new @hits parameter which contains the current hits
 *   hardlinks are traversed
 *   softlinks are NOT traversed
 *   identical nodes are not returned
 *   same node hardlinks are returned
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextTop(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits) {
    xmlNodePtr retNode;

    if (ctxt == NULL) return(NULL);
    if ((cur == NULL) && (ctxt->context->node == NULL)) return(NULL);
    if ((ctxt->context->node != NULL) && (ctxt->context->node->type == XML_ATTRIBUTE_NODE)) return(NULL);
    if ((ctxt->context->node != NULL) && (ctxt->context->node->type == XML_NAMESPACE_DECL)) return(NULL);
    
    retNode = NULL;
    
    /* ------------------------------ start point (notSelf)
     * make sure we have the correct start parent hardlink so that we can spot the end traverse
     * now proceed immediately, and only, to the first children traverse 
     * 
     * ctxt->context->node might be a hardlink
     * we need to spot when we have ->parent moved back to it
     * AND it is logically PART of the parent_route even though not "traversed"
     */
    if (cur == NULL) cur = ctxt->context->node;

    /* ------------------------------ children first
     * naturally including and remembering hardlinks for correct parent selection after traversal from child
     * we do not descend to children of hits
     * we do not return same (exact) hit twice, i.e. descendants of a hardlink
     * we do return hardlinks of the same hit node, i.e. actual hardlinks
     * do not descend on softlinks
     */
    if (! XP_ALREADY_HIT(cur)) {
      retNode = xmlReadableChildren(ctxt, cur, SKIP_DTD_AND_ENTITY & NO_SOFTLINK);
    }

    /* ------------------------------ then siblings
     * cur is only the context->node when this is the first traversal
     * try only children if it is the first traversal
     * DO NOT check siblings if we fail to descend on the context node children
     */
    if ((retNode == NULL) && (cur != ctxt->context->node)) {
      retNode = xmlReadableNext(ctxt, cur, SKIP_DTD_AND_ENTITY);
    }
    
    /* ------------------------------ then up parents and along nexts
     * need to pop back up the right parent path though because of hardlinks 
     */
    if ((retNode == NULL) && (cur != ctxt->context->node)) {
      do {
        /* permanent move up: set the new cur */
        cur = xmlReadableParent(ctxt, cur, ANY_NODE);
        if (cur == ctxt->context->node) cur = NULL; /* arrived back at start context node */
        /* check across */
        if (cur != NULL) retNode = xmlReadableNext(ctxt, cur, SKIP_DTD_AND_ENTITY);
      } while ((cur != NULL) && (retNode == NULL));
    }
    
    return(retNode);
}


/**
 * xmlXPathNextAncestors:
 * @ctxt:  the XPath Parser context
 * @cur:   the current node in the traversal
 * @hits:   the current list of XP_TEST_HIT nodes (not used)
 * @paths: a stateful list for this traversal node collection
 *
 * @author: Annesley
 *
 * Traversal function for the "ancestors" direction
 * includes all hardlinks of all ancestors and their ancestors
 *
 * there is no route back down from a parent to the child it came from
 * unlike the descendant situation
 * so we need ctxt->traversal_state to traverse this axis
 *
 * Returns the next element following that axis
 * parent_route is NULL for these returns
 */
xmlNodePtr
xmlXPathNextAncestors(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  if (ctxt == NULL) return(NULL);
  if (ctxt->context == NULL) return(NULL);
  if ((cur == NULL) && (ctxt->context->node == NULL)) return(NULL);
  
#if 0
  xmlNodePtr retNode;
  
  /* Use the parent_route instead
   * here we cna maintain a functional parent_route whilst traversing
   * this will be reset after the traverse
   * and any nodes selected will have the correct parent_route intact
   */
  
  /* ---------------------------- initial step (notSelf) */
  if (cur == NULL) {
    cur = ctxt->context->node;
    /* ensure an initial blank parent_route */
    if (ctxt->context->parent_route == NULL) ctxt->context->parent_route = xmlListCreate(NULL, (xmlListDataCompare) xmlAreEqualOrHardLinkedListCompare);
    else                                     xmlListClear(ctxt->context->parent_route);
  }
  
  /* ---------------------------- parents first 
   * obeying parent_route which is blank initially
   */
  retNode = xmlReadableParent(ctxt->context, cur, ANY_NODE);
  
  /* ---------------------------- then sibling (hardlinks) */
  if ((retNode == NULL) && (xmlIsHardLinked(cur) == 1)) {
    next = cur->hardlink_info->next;
    cur  = xmlParentRouteAcross(ctxt, cur, next); /* will add this hardlink to the parent_route */
    cur  = xmlReadableContextNode(ctxt, cur);
  }
  
  ...
#endif
  
  if (cur == NULL) {
    if (ctxt->traversal_state != NULL) xmlGenericError(xmlGenericErrorContext, "xmlXPathNextAncestors: traversal_state populated at start\n");
    ctxt->traversal_state = xmlListCreate(NULL, NULL);
    cur = ctxt->context->node;
  }

  /* ------------------------------ remember unique hardlinks
   * remember these because they all need to be traversed
   * there is no route back down from a parent to this hardlink
   * so we need state to traverse this axis
   * the uniqueness also doubles as infinite recusrion checking
   */
  if (xmlIsHardLinked(cur) == 1) {
    if (xmlCheckRecursion(ctxt->traversal_state, cur, suppressExactRepeatHardlinks) == 0) {
      cur = xmlFirstHardLink(cur);
      xmlListPushBack(ctxt->traversal_state, cur); 
    }
  }

  /* ------------------------------ natural parent traverse
   * xmlXPathNextAncestor() will pop parents according to the cur parent_route
   * we do not care about the parent_route directly because we are traversing ALL parent_routes
   * this simply moves up to the singular only natural parent
   */
  if (cur != NULL) cur = xmlReadableParent(ctxt, cur, ANY_NODE);

  /* ------------------------------ progress recorded hardlinks to their last hardlink
   * if the cur branch has reached NULL
   * re-traverse each hardlinked node completely, in reverse order
   * we want to continue to the end of the hardlink chain anyway, even if a xmlXPathNextAncestor() cannot be read
   * xmlPopIfHardLinked(...) is not used here because we manually access the traversal_state
   */
  if ((cur == NULL) && (ctxt->traversal_state != NULL)) {
    cur = xmlListReverseWalk(ctxt->traversal_state, xmlXPathNextAncestorsOrSelfWalker, NULL, NULL);
    if (cur != NULL) cur = xmlReadableParent(ctxt, cur, ANY_NODE);
  }

  /* ------------------------------ empty the node-set memory at the end */
  if (cur == NULL) {
    if (ctxt->traversal_state != NULL) {
      xmlListDelete(ctxt->traversal_state);
      ctxt->traversal_state = NULL;
    }
  }

  return(cur);
}

/**
 * xmlXPathNextXPath:
 * @ctxt:  the XPath Parser context
 * @cur:  the current attribute in the traversal
 *
 * @author: Annesley
 *
 * Traversal function for the non-standard "xpath" direction
 * the xpath axis contains all the nodes from the dynamic evaluation of the text from the current node
 *   xpath-to-nodeset/xpath::xpath-selection-from-nodeset
 *   it is equivalent to 
 *   dyn:evaluate(xpath-to-nodeset)/self::xpath-selection-from-nodeset
 * e.g. ./gs:query-string/@xpath-to.destination/xpath::*[1]
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextXPath(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  xmlNodePtr next;
  int i;

  if (ctxt == NULL) return(NULL);
  if (ctxt->context->parent_route == NULL) return(NULL);

  LIBXML_NOT_COMPLETE("xmlXPathNextXPath");
  
  /* -------------------------- start 
   * we use traversal_state as our only state mechanism
   * but it really serves here only to remember where we are in context->parent_route
   */
  if (cur == NULL) {
    if (ctxt->traversal_state != NULL) xmlGenericError(xmlGenericErrorContext, "xmlXPathNextXPath: ctxt->traversal_state already populated at start");
    
    /* evaluate xpath! */
    ctxt->traversal_state = xmlListCreate(NULL, NULL);
    
    /* set new parent_route also!!! */
    
    cur = ctxt->context->node;
  }
  
  /* -------------------------- next */
  i = xmlListSize(ctxt->traversal_state);
  if (i < xmlListSize(ctxt->context->parent_route)) {
    next = xmlListItem(ctxt->context->parent_route, i);
  } else next = NULL;
  
  /* -------------------------- finish */
  if (next == NULL) {
    if (ctxt->traversal_state != NULL) {
      xmlListDelete(ctxt->traversal_state);
      ctxt->traversal_state = NULL;
    }
  }
  
  return(next);
}

/**
 * xmlXPathNextHardlink:
 * @ctxt:  the XPath Parser context
 * @cur:  the current attribute in the traversal
 *
 * @author: Annesley
 *
 * Traversal function for the "hardlink" direction
 * the hardlink axis contains the hardlinks(s) of this hardlinked node
 * starting with the first in the chain
 * in ANY namespace
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextHardlink(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  xmlNodePtr hardlink;

  if (cur == NULL) {
    if ( (ctxt == NULL)
      || (ctxt->context == NULL)
      || (ctxt->context->node == NULL)
      || (ctxt->context->node->type != XML_ELEMENT_NODE)
    ) return(NULL);

    hardlink = xmlFirstHardLink(ctxt->context->node); /* returns input if no hardlinks */
  } else {
    hardlink = cur->hardlink_info ? cur->hardlink_info->next : NULL;
  }

  return(hardlink);
}


/**
 * xmlXPathNextParentRoute:
 * @ctxt:  the XPath Parser context
 * @cur:  the current attribute in the traversal
 *
 * @author: Annesley
 *
 * Traversal function for the "parent-route" direction
 * the hardlink axis contains the parent_route hardlinks(s) of this hardlinked node
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextParentRoute(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  int i;
  xmlNodePtr next;

  if (ctxt == NULL) return(NULL);
  if (ctxt->context->parent_route == NULL) return(NULL);
  if (xmlListIsEmpty(ctxt->context->parent_route) == 1) return(NULL);

  /* -------------------------- start 
   * we use traversal_state as our only state mechanism
   * but it really serves here only to remember where we are in context->parent_route
   */
  if (cur == NULL) {
    if (ctxt->traversal_state != NULL) xmlGenericError(xmlGenericErrorContext, "xmlXPathNextParentRoute: ctxt->traversal_state already populated at start");
    ctxt->traversal_state = xmlListCreate(NULL, NULL);
    cur = ctxt->context->node;
  }
  
  /* -------------------------- next */
  i = xmlListSize(ctxt->traversal_state);
  if (i < xmlListSize(ctxt->context->parent_route)) {
    next = xmlListItem(ctxt->context->parent_route, i); /* expensive list traversal */
    xmlListPushBack(ctxt->traversal_state, next);       /* i++ */
    /* needs to be correct for further steps 
     * we also send the original through so that xmlParentRouteUp() does not strange
     */
    next = xmlParentRouteUp(ctxt->context, cur, xmlFirstHardLink(next));  
  } else next = NULL; /* end */
  
  
  /* -------------------------- finish */
  if (next == NULL) {
    if (ctxt->traversal_state != NULL) {
      xmlListDelete(ctxt->traversal_state);
      ctxt->traversal_state = NULL;
    }
  }
  
  return(next);
}


/**
 * xmlXPathNextParents:
 * @ctxt:  the XPath Parser context
 * @cur:  the current attribute in the traversal
 *
 * @author: Annesley
 *
 * Traversal function for the "parents" direction
 * the parents axis contains all the parent(s) of this node's hardlinks
 * the parents are not hardlinked, this node is hardlinked under all the normal parents
 * in ANY namespace
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextParents(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
  int position = 0;
  xmlNodePtr hardlink, parent = NULL;
  
  if (ctxt == NULL) return (NULL);
  if (ctxt->context->node == NULL) return (NULL);
  if (ctxt->context->node->type != XML_ELEMENT_NODE) return(NULL);

  /* -------------------------- start 
   * we use traversal_state as our only state mechanism
   * but it really serves here only to remember where we are in context->parent_route
   */
  if (cur == NULL) {
    if (ctxt->traversal_state != NULL) xmlGenericError(xmlGenericErrorContext, "xmlXPathNextParents: ctxt->traversal_state already populated at start");
    ctxt->traversal_state = xmlListCreate(NULL, NULL);
  } else {
    /* pop down from last up below */
    xmlReadableChildren(ctxt, cur, ANY_NODE);
  }
  
  /* -------------------------- loop to next hardlink */
  hardlink = ctxt->context->node;
  position = xmlListSize(ctxt->traversal_state);
  if (hardlink->hardlink_info == NULL) {
    /* if the context node is not a hardlink then the axis contains only the context node parent */
    if (position != 0) hardlink = NULL;
  } else {
    /* TODO: deviations? */
    hardlink = xmlFirstHardLink(hardlink); /* returns input if no hardlinks */
    for (; position != 0; position--) hardlink = hardlink->hardlink_info->next;
  }
  if (hardlink) {
    parent = xmlReadableParent(ctxt, hardlink, ANY_NODE); 
    xmlListPushBack(ctxt->traversal_state, hardlink);
  }

  /* -------------------------- finish */
  if (parent == NULL) {
    xmlListDelete(ctxt->traversal_state);
    ctxt->traversal_state = NULL;
  }

  return(parent);
}

/**
 * @author: Annesley
 * xmlXPathNextNamedChild:
 * @ctxt:  the XPath Parser context
 * @cur:  the current attribute in the traversal
 *
 * Traversal function for the "name" direction
 * the name axis contains the nodes with one or more attributes called @*:name
 * in ANY namespace
 * name::* will still work as it will traverse everything with a name attribute
 * the order of nodes on this axis is implementation-defined; the axis will
 * be empty unless the context node is an element
 *
 * Returns the next element following that axis
 */
xmlNodePtr
xmlXPathNextNamedChild(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, xmlNodeSetPtr hits ATTRIBUTE_UNUSED) {
    xmlChar *sNameAxisName;

    if (ctxt == NULL) return(NULL);
    if (ctxt->context->node == NULL) return(NULL);
    if ( (ctxt->context->node->type != XML_ELEMENT_NODE) && (ctxt->context->node->type != XML_DOCUMENT_NODE)) return(NULL);

    /* --------------------------------- start point (notSelf) */
    if (cur == NULL) cur = xmlReadableChildren(ctxt, ctxt->context->node, XML_ELEMENT_NODE_ONLY);
    
    /* --------------------------------- next sibling */
    else cur = xmlReadableNext(ctxt, cur, XML_ELEMENT_NODE_ONLY);

    do {
      /* check for a name attribute */
      sNameAxisName = xmlXPathNameAxisName(cur, NULL, ctxt->context);
      if (sNameAxisName == NULL) cur = xmlReadableNext(ctxt, cur, XML_ELEMENT_NODE_ONLY);
      else xmlFree(sNameAxisName);
    } while ((sNameAxisName == NULL) && (cur != NULL));
    
    return(cur);
}


/**
 * xmlEvalXPathStringNs:
 * @ctxt:  the XPath context
 * @comp:  the compiled XPath expression
 * @nsNr:  the number of namespaces in the list
 * @nsList:  the list of in-scope namespaces to use
 *
 * @author: Annesley
 * 
 * Process the expression using XPath, allowing to pass a namespace mapping
 * context and get a string
 *
 * Returns the computed string value or NULL, must be deallocated by the caller.
 */
static xmlChar *
xmlEvalXPathString(xmlXPathContextPtr xpathCtxt, xmlXPathCompExprPtr comp) {
    xmlChar *ret = NULL;
    xmlXPathObjectPtr res;

    res = xmlXPathCompiledEval(comp, xpathCtxt);
    if (res != NULL) {
        if (res->type != XPATH_STRING)
            res = xmlXPathConvertString(res);
        if (res->type == XPATH_STRING) {
            ret = res->stringval;
            res->stringval = NULL;
        } else {
            xmlGenericError(xmlGenericErrorContext, "xmlEvalXPathStringNs: string() function didn't return a String");
        }
        xmlXPathFreeObject(res);
    }

    return(ret);
}

/**
 * xmlTemplateValueProcess:
 * @ctxt:  the XPath context
 * @str:  the attribute template node value
 * 
 * @author: Annesley
 *
 * Process the given string, allowing to pass a namespace mapping
 * context and return the new string value.
 *
 * Returns the computed string value or NULL, must be deallocated by the caller.
 */
xmlChar *
xmlTemplateValueProcess(xmlXPathContextPtr ctxt, const xmlChar *str) {
    xmlChar *ret = NULL;
    const xmlChar *cur;
    xmlChar *expr, *val;

    if (str == NULL) return(NULL);
    if (*str == 0)   return(xmlStrndup((xmlChar *)"", 0));

    cur = str;
    while (*cur != 0) {
        if (*cur == '{') {
            if (*(cur+1) == '{') {      /* escaped '{' */
                cur++;
                ret = xmlStrncat(ret, str, cur - str);
                cur++;
                str = cur;
                continue;
            }
            ret = xmlStrncat(ret, str, cur - str);
            str = cur;
            cur++;
            while ((*cur != 0) && (*cur != '}')) cur++;
            if (*cur == 0) {
                xmlGenericError(xmlGenericErrorContext, "xmlTemplateValueProcess: unmatched '{' in [%s]\n", str);
                ret = xmlStrncat(ret, str, cur - str);
                return(ret);
            }
            str++;
            expr = xmlStrndup(str, cur - str);
            if (expr == NULL)
                return(ret);
            else if (*expr == '{') {
                ret = xmlStrcat(ret, expr);
                xmlFree(expr);
            } else {
                xmlXPathCompExprPtr comp;
                /* TODO: keep precompiled form around */
                comp = xmlXPathCompile(expr);
                val  = xmlEvalXPathString(ctxt, comp);
                xmlXPathFreeCompExpr(comp);
                xmlFree(expr);
                if (val != NULL) {
                    ret = xmlStrcat(ret, val);
                    xmlFree(val);
                }
            }
            cur++;
            str = cur;
        } else if (*cur == '}') {
            cur++;
            if (*cur == '}') {  /* escaped '}' */
                ret = xmlStrncat(ret, str, cur - str);
                cur++;
                str = cur;
                continue;
            } else {
                xmlGenericError(xmlGenericErrorContext, "xmlTemplateValueProcess: unmatched '}' in [%s]\n", str);
            }
        } else
            cur++;
    }
    if (cur != str) ret = xmlStrncat(ret, str, cur - str);

    return(ret);
}


/**
 * xmlTriggerCall,
 * xmlCreateNodeTriggerCallbackContext
 * xmlCopyNodeTriggerCallbackContext
 * xmlFreeNodeTriggerCallbackContext:
 * @xtrigger: the trigger context
 * @cur:      the current node
 * @parent_route: node context
 *
 * @author: Annesley
 *
 * Generic low-level direct triggering
 * Generally used for:
 * These are mostly designed to push design up to the next layer
 * rather than hardcoded or parameterised implementation here
 * hooks:
 *   the security hook fires before attemps to browse a node, for any reason. OP is passed in and through
 *   read triggers fire before and receive the context and xpath context
 *     useful for [re]populating dynamic content
 *     useful examining the xpath query and pre-populating the result only with required contents (and child count)
 *     sub-functions return xmlNodeListPtr not xmlDocPtr
 *     <repository:employers       database:on-before-read="database:replace-children(., postgres:get-query-results())" />    <-- xpath context input
 *     <repository:namespace-cache database:on-before-read="database:replace-children(., server:serialise('namespace-cache'))" />
 *     <repository:cron            database:on-before-read="database:replace-children(., repository:rx('/etc/crontab', ../rx:cron-tab))" />
 *     xpath: repository:employers/postgres:employer[@eid = 9]
 *   write triggers fire before and after write attempts
 *     the before trigger can cancel the write attempt
 *     useful for releasing watches on content
 *     @database:on-after-write="database:transform(., )"
 *     @database:on-after-write="database:move-node(., ../repository:archived)"
 *     @database:on-after-write="database:set-attribute(@count, @count + 1)"
 *
 * xmlNodeTriggerCallbackContextPtr:
 *   xtrigger->onBeforeRead(ctxt, cur)
 *   xtrigger->onBeforeWrite(ctxt, cur) -> [save the previous values that are important here if necessary]
 *   ...
 *
 * TODO: cacheing: xmlNodePtr should include these to allow cached speed up of checking
 *   int bCached_applySecurityCheck = 1;
 *   int bCached_applyTriggerCheck  = 1;
 */
xmlNodeTriggerCallbackContextPtr xmlCreateNodeTriggerCallbackContext(xmlElementTriggerCallbackFunc xmlElementTriggerCallback, void *param, xmlXPathContextPtr xpathCtxt) {
  xmlNodeTriggerCallbackContextPtr ret;

  ret = (xmlNodeTriggerCallbackContextPtr) xmlMalloc(sizeof(xmlNodeTriggerCallbackContext));
  if (ret == NULL) {
      xmlXPathErrMemory(NULL, "creating trigger context\n");
      return(NULL);
  }
  memset(ret, 0 , (size_t) sizeof(xmlNodeTriggerCallbackContext));
  ret->xmlElementTriggerCallback = xmlElementTriggerCallback;
  ret->param                     = param;
  ret->xpathCtxt                 = xpathCtxt;

  return(ret);
}
xmlNodeTriggerCallbackContextPtr xmlCopyNodeTriggerCallbackContext(xmlNodeTriggerCallbackContextPtr xtrigger) {
  if (xtrigger == NULL) return (NULL);
  return xmlCreateNodeTriggerCallbackContext(
    xtrigger->xmlElementTriggerCallback,
    xtrigger->param,
    xtrigger->xpathCtxt
  );
}
void xmlFreeNodeTriggerCallbackContext(xmlNodeTriggerCallbackContextPtr xtrigger) {
  xmlFree(xtrigger);
}

int
xmlTriggerCall(xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr parent, xmlListPtr parent_route, int triggerOperation, int triggerStage, xmlNodeFilterCallbackContextPtr xfilter)
{
  int result = TRIGGER_CONTINUE; /* result interpreted according to trigger context */
  if (xtrigger) result = xtrigger->xmlElementTriggerCallback(parent, parent_route, xtrigger, triggerOperation, triggerStage, xfilter);

  return result;
}

/**
 * xmlXPathProcessingCall,
 * xmlCreateXPathProcessingCallbackContext
 * xmlCopyXPathProcessingCallbackContext
 * xmlFreeXPathProcessingCallbackContext:
 * @xprocessor: the xpath processor context
 * @ctxt:     the xpath context
 * @cur:      the current node
 * @test:     the test, e.g. NAME_TEXT
 *
 * @author: Annesley
 *
 * XPath strategy
 * Generally used for:
 *   Inheritance
 *   e.g. object:Person inherits from object:User
 * 
 * returns XP_TEST_HIT or 0
 */
xmlXPathProcessingCallbackContextPtr xmlCreateXPathProcessingCallbackContext(xmlXPathProcessingCallbackFunc xmlXPathProcessingCallback, void *param) {
  xmlXPathProcessingCallbackContextPtr ret;

  ret = (xmlXPathProcessingCallbackContextPtr) xmlMalloc(sizeof(xmlXPathProcessingCallbackContext));
  if (ret == NULL) {
      xmlXPathErrMemory(NULL, "creating xpath processing context\n");
      return(NULL);
  }
  memset(ret, 0 , (size_t) sizeof(xmlXPathProcessingCallbackContext));
  ret->xmlXPathProcessingCallback = xmlXPathProcessingCallback;
  ret->param                      = param;

  return(ret);
}

xmlXPathProcessingCallbackContextPtr xmlCopyXPathProcessingCallbackContext(xmlXPathProcessingCallbackContextPtr xprocessor) {
  return xmlCreateXPathProcessingCallbackContext(
    xprocessor->xmlXPathProcessingCallback,
    xprocessor->param
  );
}

void xmlFreeXPathProcessingCallbackContext(xmlXPathProcessingCallbackContextPtr xprocessor) {
  xmlFree(xprocessor);
}

int
xmlXPathProcessingCall(xmlXPathProcessingCallbackContextPtr xprocessor, xmlNodePtr cur, xmlListPtr parent_route, int test, const xmlChar *localname, const xmlChar *prefix, const char ***pvsGroupings)
{
   /* EXECUTION phase: return 
    *   XP_TEST_HIT (1) or
    *   XP_TEST_HIT_NS (2) 
    * if the xprocessor should change the result to a hit 
    * 
    * COMPILATION phase:
    *   xsl:template name groupings for fast lookup
    */
  int result = 0;
  if (xprocessor) result = xprocessor->xmlXPathProcessingCallback(xprocessor, cur, parent_route, test, localname, prefix, pvsGroupings);

  return result;
}

/**
 * xmlFilterCheck,
 * xmlCreateNodeFilterCallbackContext
 * xmlCopyNodeFilterCallbackContext
 * xmlFreeNodeFilterCallbackContext:
 * @xfilter:  the filter context
 * @cur:      the current node
 * @parent_route: node context
 *
 * @author: Annesley
 *
 * Generic low-level direct filtering
 * Generally used for:
 *   Test to see if node access is allowed within the current security context
 *   the namespace xmlsecurity defines the node access restrictions
 *   UNIX style: xmlsecurity:ownername="root" xmlsecurity:group="group_1" xmlsecurity:permissions="777"
 *   other security styles can be added that relate to other attributes
 */
xmlNodeFilterCallbackContextPtr xmlCreateNodeFilterCallbackContext(xmlElementFilterCallbackFunc xmlElementFilterCallback, void *param) {
  xmlNodeFilterCallbackContextPtr ret;

  ret = (xmlNodeFilterCallbackContextPtr) xmlMalloc(sizeof(xmlNodeFilterCallbackContext));
  if (ret == NULL) {
      xmlXPathErrMemory(NULL, "creating filter context\n");
      return(NULL);
  }
  memset(ret, 0 , (size_t) sizeof(xmlNodeFilterCallbackContext));
  ret->xmlElementFilterCallback = xmlElementFilterCallback;
  ret->param                    = param;

  return(ret);
}

xmlNodeFilterCallbackContextPtr xmlCopyNodeFilterCallbackContext(xmlNodeFilterCallbackContextPtr xfilter) {
  return xmlCreateNodeFilterCallbackContext(
    xfilter->xmlElementFilterCallback,
    xfilter->param
  );
}

void xmlFreeNodeFilterCallbackContext(xmlNodeFilterCallbackContextPtr xfilter) {
  xmlFree(xfilter);
}

int
xmlFilterCheck(xmlNodeFilterCallbackContextPtr xfilter, xmlNodePtr cur, xmlListPtr parent_route, int securityOperation)
{
  int result = FILTER_NODE_INCLUDE; /* default to include in the case of no valid filter */
  if (xfilter) result = xfilter->xmlElementFilterCallback(cur, parent_route, xfilter, securityOperation);

  return result;
}

/**
 * xmlDeviateToOriginal:
 * @context: contains current parent_route list for checking for valid deviants
 * @cur:     the current deviant being checked for an original
 *
 * Returns the cur or original node
 */
xmlNodePtr
xmlDeviateToOriginal(xmlXPathContextPtr context, xmlNodePtr deviant) {
  xmlDeviationPtr deviation;
  xmlNodePtr deviations_holder;

  if (context == NULL) return(deviant);
  if (deviant == NULL) return(NULL);
                                                        
  if ((deviant->parent != NULL) && ((deviant->type == XML_ELEMENT_NODE) || (deviant->type == XML_ATTRIBUTE_NODE))) {
    /* XML_ATTRIBUTE_NODE deviations are held on the parent elements */
    deviations_holder = (deviant->type == XML_ELEMENT_NODE ? deviant : deviant->parent);
    if (deviations_holder->hardlink_info != NULL) {
      if (context->parent_route != NULL) {
        /* if we have not traversed any hardlinks, we have no deviations */
        if (xmlListIsEmpty(context->parent_route) == 0) {
          deviation = deviations_holder->hardlink_info->originals;
          while (deviation) {
            /* ok, so if this original is for a traversed hardlink then we need to serve it
             * NOTE: not all "originals" on this node are for this node!
             * e.g. XML_ATTRIBUTE_NODE originals can be stored here too
             */
            if (deviation->deviant == deviant) {
              if (xmlListUnorderedSearch(context->parent_route, deviation->hardlink) != NULL)
                deviant = deviation->original;
            }
            deviation = deviation->next;
          }
        }
      }
    }
  }

  return (deviant);
}

/**
 * xmlIsDeviant:
 * @cur:     the current deviant being checked for original
 *
 * Returns true if the node has an original
 */
int
xmlIsDeviant(xmlNodePtr cur) {
  return ((cur != NULL) && (cur->hardlink_info != NULL) && (cur->hardlink_info->originals != NULL));
}

/**
 * xmlHasDescendantDeviants:
 * @cur:      the current hardlink being checked for a decendent deviants
 * @original: an optional specific original to check for
 *
 * Returns true if some nodes in the sub-tree of this hardlink have deviants
 */
int
xmlHasDescendantDeviants(xmlNodePtr hardlink, xmlNodePtr original) {
  xmlDeviationPtr deviation;
  int ret = 0;

  if (hardlink == NULL) return(0);

  if ((hardlink->hardlink_info != NULL) && (hardlink->hardlink_info->descendant_deviants != NULL)) {
    if (original == NULL) ret = 1;
    else {
      deviation = hardlink->hardlink_info->descendant_deviants;
      while ((deviation != NULL) && (ret == 0)) {
        if (original == deviation->original) ret = 1;
      }
    }
  }

  return(ret);
}


/**
 * xmlHasOriginals:
 * @cur:  the test node
 * @hardlink: OPTIONAL to check for a specific hardlink case
 *
 * hardlink_info->next and hardlink_info->prev represent a double linked list of other hardlinks
 * this returns true if the node is hardlinked but NOT the original
 */
int
xmlHasOriginals(xmlNodePtr cur, xmlNodePtr hardlink) {
  int ret = 0;
  xmlDeviationPtr deviant;
  xmlNodePtr deviations_holder;
  
  if (cur == NULL) return 0;
  if ( (cur->type == XML_ELEMENT_NODE)
    && (hardlink == NULL) 
    && (cur->hardlink_info != NULL)
    && (cur->hardlink_info->originals != NULL)
  )
    return 1;

  deviations_holder = cur;
  if (cur->type == XML_ATTRIBUTE_NODE) deviations_holder = cur->parent;
  
  if (deviations_holder->hardlink_info != NULL) {
    deviant = deviations_holder->hardlink_info->originals;
    while ((deviant != NULL) && (ret == 0)) {
      if ((deviant->original == cur)
        && (
          (deviant->hardlink == hardlink)
          || (hardlink == NULL)
        )) ret = 1;
      deviant = deviant->next;
    }
  }

  return ret;
}


/**
 * xmlHasDeviants:
 * @cur:      the current node being checked for deviants
 * @hardlink: OPTIONAL specific hardlink to check for
 *
 * Returns true if the node has deviants
 */
int
xmlHasDeviants(xmlNodePtr cur, xmlNodePtr hardlink) {
  int ret = 0;
  xmlDeviationPtr deviant;
  xmlNodePtr deviations_holder;
  
  if (cur == NULL) return 0;
  if ( (cur->type == XML_ELEMENT_NODE)
    && (hardlink == NULL) 
    && (cur->hardlink_info != NULL)
    && (cur->hardlink_info->deviants != NULL)
  ) 
      return 1;
    
  deviations_holder = cur;
  if (cur->type == XML_ATTRIBUTE_NODE) deviations_holder = cur->parent;
  
  if (deviations_holder->hardlink_info != NULL) {
    deviant = deviations_holder->hardlink_info->deviants;
    while ((deviant != NULL) && (ret == 0)) {
      if ((deviant->deviant == cur)
        && (
          (deviant->hardlink == hardlink)
          || (hardlink == NULL)
        )) ret = 1;
      deviant = deviant->next;
    }
  }

  return ret;
}

/**
 * xmlDeviateToDeviant:
 * @context: contains current parent_route list for checking for valid deviants
 * @cur:     the current deviant being checked for a new node
 *
 * All traversals come here to check for deviation
 * The hardlink structure must contain a deviants list if there is a deviant
 * Deviations on an element do not necessarily concern THAT element!!
 * in the case
 *
 * Returns the cur or replacement node
 */
xmlNodePtr
xmlDeviateToDeviant(xmlXPathContextPtr context, xmlNodePtr cur) {
  /* if cur is an @attribute of a hardlink then its @cur->parent will be the original, not the hardlink
   * thus, if the caller wishes to check for the @cur->parent hardlink then:
   *   it needs to be added to the parent_route first
   */
  int found;
  xmlDeviationPtr deviation;
  xmlNodePtr deviations_holder, new_cur; /* parent in the case of XML_ATTRIBUTE_NODE */

  if (context == NULL) return(cur);
  if (cur == NULL)     return(NULL);
  /* TODO: XML_TEXT_NODE deviations? they do not have the hardlink_info yet */
  if ((cur->type != XML_ELEMENT_NODE) && (cur->type != XML_ATTRIBUTE_NODE)) return (cur);

  /* XML_ATTRIBUTE_NODE deviations are held on the parent elements
   * looking for hardlink_info->deviants for this original (cur)
   */
  deviations_holder = (cur->type == XML_ATTRIBUTE_NODE ? cur->parent : cur);
  new_cur = cur;
  
  if ( (deviations_holder != NULL) 
    && (deviations_holder->hardlink_info != NULL) 
    && (deviations_holder->hardlink_info->deviants != NULL)
  ) {
#ifdef LIBXML_DEBUG_ENABLED
    XML_TRACE_GENERIC5(XML_DEBUG_PARENT_ROUTE, "[parent_route] deviants found on 0x[%p] [%s]%s%s. [%u] in parent_route:",
      (void*) deviations_holder,
      (deviations_holder->name != NULL ? (const char*) deviations_holder->name : "<no name>"),
      (cur->type == XML_ATTRIBUTE_NODE ? " for @" : ""),
      (cur->type == XML_ATTRIBUTE_NODE ? (const char*) cur->name : ""),
      (context->parent_route != NULL ? xmlListSize(context->parent_route) : 0)
    );
#endif
  
    if ( (context->parent_route != NULL)
      && (xmlListIsEmpty(context->parent_route) == 0)
    ) {
      if ((deviations_holder->hardlink_info != NULL) && (deviations_holder->hardlink_info->deviants != NULL)) {
#ifdef LIBXML_DEBUG_ENABLED
        /*
        xmlNodePtr traversed_hardlink;
        int i;
        for (i = 0; i < context->parent_route->nodeNr; i++) {
          traversed_hardlink = context->parent_route->nodeTab[i];
          XML_TRACE_GENERIC4(XML_DEBUG_PARENT_ROUTE, "  parent_route hardlink 0x[%p] [%s] under natural [%s] %s",
            (void*) traversed_hardlink,
            (traversed_hardlink->name != NULL ? (const char*) traversed_hardlink->name : "<no name>"),
            ((traversed_hardlink->parent != NULL) && (traversed_hardlink->parent->name != NULL) ? (const char*) traversed_hardlink->parent->name : "<no name>"),
            ((traversed_hardlink->hardlink_info != NULL) && (traversed_hardlink->hardlink_info->descendant_deviants != NULL)? "(with DEVIANTSSSS)" : "(no descendant_deviants registered)")
          );
        }
        */
#endif

        /* scan ALL the deviations, even when one matches
        * even a correct deviation may be overridden by a later one
        * the closest hardlink takes precedence
        * deviations hardlink are in document order
        */
        deviation = deviations_holder->hardlink_info->deviants;
        while ((deviation != NULL)) {
          /* ok, so if this deviant is for a traversed hardlink then we need to serve it
            * NOTE: not all "deviants" on this node are for this node!
            * e.g. XML_ATTRIBUTE_NODE deviants can be stored here too
            */
          found = 0;
          if (deviation->original == cur) {
            /* this deviation DOES apply to the cur node: check the relevant hardlink with the traversed hardlinks */
            XML_TRACE_GENERIC4(XML_DEBUG_PARENT_ROUTE, "  found relevant deviant of [%s%s]: checking if hardlink 0x[%p] [%s] is in parent_route...",
              (cur->type == XML_ATTRIBUTE_NODE ? "@" : ""),
              (const char*) cur->name,
              (void*) deviation->hardlink,
              (deviation->hardlink->name != NULL ? (const char*) deviation->hardlink->name : "<no name>")
            );
            
            /* cur can be the relevant hardlink */
            if (deviation->hardlink == deviations_holder) {
              new_cur = deviation->deviant;
              found   = 1;
            } else {
              /* search all the traversed hardlinks so far
               * to see if the relevant hardlink has been traversed
               * 
               * DO NOT use the standard search because it will use xmlAreEqualOrHardLinkedListCompare()
               * we want a direct pointer comparison instead
               */
              if (xmlListPointerCompareSearch(context->parent_route, deviation->hardlink) == 1) {
                new_cur = deviation->deviant;
                found   = 1;
              }
            }
            
            /* report */
            if (found == 1) {
              XML_TRACE_GENERIC(XML_DEBUG_PARENT_ROUTE, "  FOUND deviant");
              if (cur->type == XML_ATTRIBUTE_NODE) {
                XML_TRACE_GENERIC3(XML_DEBUG_PARENT_ROUTE, "  @%s=[%s] => [%s]",
                  cur->name,
                  cur->children->content,     /* old content */
                  new_cur->children->content  /* new content */
                );
                /* continue, there may be a more relevant deviant
                 * closer in document order
                 */
              }
            }
          }
          deviation = deviation->next;
        }
      }
    }
  }

  return (new_cur);
}

/**
 * xmlWriteableNode
 * xmlAddableNode:
 * @ctxt:  the XPath parser context with the compiled expression
 * @cur:   the node to check from
 *
 * @author: Annesley
 *
 * Check:
 *   1) filter checks
 *   2) security and
 *   3) trigger checks (last and only fire if necessary)
 */
xmlNodePtr
xmlWriteableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route)
{
  xmlNodePtr ret = NULL;

  /* nodes being freed should not be newly returned 
   * xmlFilterCheck() will throw up if it cannot lock the node as well
   */
  if (MARKED_FOR_DESTRUCTION(cur) == 0) { 
    if ( (xmlFilterCheck(xfilter, cur, parent_route, FILTER_OP_WRITE) == FILTER_NODE_INCLUDE) /* carries out node-mask checks also */
      && (xmlTriggerCall(xtrigger, cur, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter) == TRIGGER_CONTINUE)) 
      ret = cur;
    else xmlNodeWriteAccessDeniedError(cur);
  }

  return(ret);
}

xmlNodePtr
xmlAddableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route)
{
  xmlNodePtr ret = NULL;

  /* nodes being freed should not be newly returned 
   * xmlFilterCheck() will throw up if it cannot lock the node as well
   */
  if (MARKED_FOR_DESTRUCTION(cur) == 0) { 
    if ( (xmlFilterCheck(xfilter, cur, parent_route, FILTER_OP_ADD) == FILTER_NODE_INCLUDE) /* carries out node-mask checks also */
      && (xmlTriggerCall(xtrigger, cur, parent_route, TRIGGER_OP_ADD, TRIGGER_STAGE_BEFORE, xfilter) == TRIGGER_CONTINUE)) 
      ret = cur;
  } else xmlNodeAddableAccessDeniedError(cur);

  return(ret);
}

/**
 * xmlParentRouteUp
 * xmlParentRouteDown
 * xmlStrafe
 * @cur:         output
 * @remove_node: original input before move
 * @add_node:    direction
 * 
 * Routines for controlling parent_route movements and DEBUG
 * We only push/pop hard links, NOT originals
 * Only upwards movements retun a node as they may need to select the correct route for strafe
 */
static xmlNodePtr
xmlStrafe(xmlListPtr list, xmlNodePtr cur, int index) {
  xmlNodePtr strafe;
  
  if (list == NULL) xmlGenericError(xmlGenericErrorContext, "xmlStrafe: list NULL");
  if (cur == NULL)  xmlGenericError(xmlGenericErrorContext, "xmlStrafe: cur NULL");

  
  /* NOT ALL hardlinked nodes are pushed, only hardlinks (not originals)
   * this is so that natural / id(nodes) that have no parent_route will match their natural position
   * which also has no parent_route
   * 
   * list is declared with xmlLinkCompare = xmlAreEqualOrHardLinkedListCompare()
   * xmlIsHardLink() returns FALSE if it is the original
   * xmlList is extended to accept negative indexes (from the end). last item index is -1 (LAST_LISTITEM)
   */
  strafe = (xmlNodePtr) xmlListItem(list, index);
  
#ifdef LIBXML_DEBUG_ENABLED
  if (xmlIsHardLinked(cur) == 0)                   xmlGenericError(xmlGenericErrorContext, "xmlStrafe: cannot strafe non-hardlinks");
  if (xmlListIsEmpty(list) == 1)                   xmlGenericError(xmlGenericErrorContext, "xmlStrafe: empty list to strafe");
  if (xmlAreEqualOrHardLinked(cur, strafe) == 0)   xmlGenericError(xmlGenericErrorContext, "xmlStrafe: should not strafe non associated hardlinks");
  if (cur != strafe) XML_TRACE_GENERIC1(XML_DEBUG_PARENT_ROUTE, "[parent_route] popped to alternative hardlink => [%p]", (void*) cur);
#endif

  return(strafe);
}

/**
 * xmlVerticalParentRouteAdopt
 * @ctxt:      xpath context whose parent_route may be altered
 * @from_node: usually this will be applied when leaving a node vertically
 *   (parent / children / last / properties)
 * 
 * adopt vertical_parent_route_adopt 
 * we are moving up or down
 * adopting the parent_route includes straffing immediately to the current (soft) hardlink
 * callers should still check the to/from_node
 * and pop/push it to the [new] parent_route if necessary
 */
xmlNodePtr
xmlVerticalParentRouteAdopt(xmlXPathContextPtr ctxt, xmlNodePtr from_node) {
  if (ctxt == NULL)      xmlGenericError(xmlGenericErrorContext, "xmlVerticalParentRouteAdopt: ctxt NULL");
  if (from_node == NULL) xmlGenericError(xmlGenericErrorContext, "xmlVerticalParentRouteAdopt: from_node NULL");
                                                        
  if ((xmlIsHardLinked(from_node) == 1) && (from_node->hardlink_info->vertical_parent_route_adopt != NULL)) {
    if (ctxt->parent_route) xmlListDelete(ctxt->parent_route);
    ctxt->parent_route = xmlListDup(from_node->hardlink_info->vertical_parent_route_adopt);
    from_node = xmlListPopBack(ctxt->parent_route);
  }
  
  return from_node;
}

xmlNodePtr
xmlParentRouteUp(xmlXPathContextPtr ctxt, xmlNodePtr from_node, xmlNodePtr to_node) {
    if (ctxt == NULL)      xmlGenericError(xmlGenericErrorContext, "xmlParentRouteUp: ctxt NULL");
    if (from_node == NULL) xmlGenericError(xmlGenericErrorContext, "xmlParentRouteUp: from_node NULL");
    if (to_node == NULL)   xmlGenericError(xmlGenericErrorContext, "xmlParentRouteUp: to_node NULL");
#ifdef LIBXML_DEBUG_ENABLED
    if (xmlIsHardLink(to_node) == 1) xmlGenericError(xmlGenericErrorContext, "xmlParentRouteUp: to_node is a non-original hardlink, how can we move up to a non-original?");
#endif
                                                        
    /* we always move up to an original hardlink
     * child->parent
     * check if it has a strafe 
     * originals are not placed in the parent_route however, so it may not have a strafe
     */
    if (xmlIsOriginalHardLink(to_node) == 1) { /* returns 0 if NOT a hardlink */
      /* compare last entry with xmlAreEqualOrHardLinkedListCompare() */
      if (xmlListCompareItem(ctxt->parent_route, to_node, LAST_LISTITEM) == 1) {
        to_node = xmlStrafe(ctxt->parent_route, to_node, LAST_LISTITEM);
        xmlListPopBack(ctxt->parent_route);
        XML_TRACE_GENERIC1(XML_DEBUG_PARENT_ROUTE, "[parent_route] removed hardlink [%p]", (void*) to_node);
      }
    }
    
    return to_node;
}

xmlNodePtr
xmlParentRouteDown(xmlXPathContextPtr ctxt, xmlNodePtr from_node, xmlNodePtr to_node) 
{
    /* can return NULL if recursion check fails */
    if (ctxt == NULL)      xmlGenericError(xmlGenericErrorContext, "xmlParentRouteDown: ctxt NULL");
    if (from_node == NULL) xmlGenericError(xmlGenericErrorContext, "xmlParentRouteDown: from_node NULL");
    if (to_node == NULL)   xmlGenericError(xmlGenericErrorContext, "xmlParentRouteDown: to_node NULL");

    /* we only place non-originals in the parent_route */
    if (xmlIsHardLink(from_node) == 1) {
      /* hardlink traversal policies: will report if recursion detected
       * caller must respond to NULL appropriately:
       *   to_node = xmlParentRouteDown(ctxt, from_node, to_node)
       */
      if (xmlCheckRecursion(ctxt->parent_route, from_node, suppressExactRepeatHardlinks) == 1) {
        to_node = NULL;
      } else {
        /* lazy create, it is deleted in xmlXPathParserContextPtr deletion */
        if (ctxt->parent_route == NULL) ctxt->parent_route = xmlListCreate(NULL, (xmlListDataCompare) xmlAreEqualOrHardLinkedListCompare);
        xmlListPushBack(ctxt->parent_route, from_node);
        XML_TRACE_GENERIC1(XML_DEBUG_PARENT_ROUTE, "[parent_route] added hardlink [%p]", (void*) from_node);
      }
    }
    
    return to_node;
}

/**
 * xmlReadableNode:
 * PRIVATE function: DO NOT use.
 * Use the Axis Accessors below
 *
 * @author: Annesley
 * there are also some context specific versions of this function
 *
 * ------------------------------ Public _xmlNode Axis accessors
 * xmlReadableNext
 * xmlReadablePrev
 * xmlReadableLastChild
 * xmlReadableChildren
 * xmlReadableProperties
 * xmlReadableParent
 * xmlReadableDocRoot:
 * @ctxt:  the XPath parser context with the compiled expression
 * @cur:   the node to check from (not checked itself)
 *
 * @author: Annesley
 * each function has various context versions for different locations
 *
 * Match the _xmlNode navigation properties:
 *   node->next
 *   node->prev
 *   node->last
 *   node->children
 *   node->parent
 *
 * Get the next sibling that passes:
 *   1) filter checks
 *   2) security and
 *   3) trigger checks (last and only fire if necessary)
 */
xmlNodePtr
xmlReadableContextNode(xmlXPathContextPtr context, xmlNodePtr cur) {
    if (context == NULL) return(NULL);
    return xmlReadableNode(context->xfilter, context->xtrigger, cur, context->parent_route);
}

xmlNodePtr
xmlReadableNode(xmlNodeFilterCallbackContextPtr xfilter, xmlNodeTriggerCallbackContextPtr xtrigger, xmlNodePtr cur, xmlListPtr parent_route)
{
  xmlNodePtr ret = NULL;
  
  if (cur == NULL) return(NULL);
  /* if (cur->type != XML_ELEMENT_NODE) return (cur); Repsoitory saving suppresses @repository:* attributes */
  
  /* nodes being freed should not be newly returned 
   * xmlFilterCheck() will throw up if it cannot lock the node as well
   */
  if (MARKED_FOR_DESTRUCTION(cur) == 0) { 
    if ( (xmlFilterCheck(xfilter,  cur, parent_route, FILTER_OP_READ) == FILTER_NODE_INCLUDE)  /* carries out node-mask checks also */
      && (xmlTriggerCall(xtrigger, cur, parent_route, TRIGGER_OP_READ, TRIGGER_STAGE_BEFORE, xfilter) == TRIGGER_CONTINUE)) 
      ret = cur;
  }

  return(ret);
}


xmlNodePtr
xmlReadableNext(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextNext(ctxt->context, cur, ignore_type_policy);
}

xmlNodePtr
xmlReadableContextNext(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy)
{
    if (context == NULL) return(NULL);
    if (cur == NULL)     return(NULL);
    
    do {
      cur  = xmlDeviateToOriginal(context, cur);
      cur = cur->next;
      if (cur != NULL) cur = xmlDeviateToDeviant(context, cur);
    } while ((cur != NULL) && (
         (xmlReadableContextNode(context, cur) == NULL)
      || ((ignore_type_policy & SKIP_DTD_AND_ENTITY)   && ((cur->type == XML_ENTITY_DECL) || (cur->type == XML_DTD_NODE)))
      || ((ignore_type_policy & XML_ELEMENT_NODE_ONLY) && (cur->type != XML_ELEMENT_NODE))
    ));
    
    return(cur);
}


xmlNodePtr
xmlReadablePrev(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextPrev(ctxt->context, cur, ignore_type_policy);
}

xmlNodePtr
xmlReadableContextPrev(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy)
{
    if (context == NULL) return(NULL);
    if (cur == NULL)     return(NULL);

    do {
      cur  = xmlDeviateToOriginal(context, cur);
      cur = cur->prev;
      if (cur != NULL) cur = xmlDeviateToDeviant(context, cur);
    } while ((cur != NULL) && (
         (xmlReadableContextNode(context, cur) == NULL)
      || ((ignore_type_policy & SKIP_DTD_AND_ENTITY)   && ((cur->type == XML_ENTITY_DECL) || (cur->type == XML_DTD_NODE)))
      || ((ignore_type_policy & XML_ELEMENT_NODE_ONLY) && (cur->type != XML_ELEMENT_NODE))
    ));
    
    return(cur);
}


xmlNodePtr
xmlReadableLastChild(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextLast(ctxt->context, cur, ignore_type_policy);
}

xmlNodePtr
xmlReadableContextLast(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy)
{
    xmlNodePtr orig, rout, next;

    if (context == NULL)   return(NULL);
    if (cur == NULL)       return(NULL);
    if ((ignore_type_policy & NO_SOFTLINK) && (xmlIsSoftLink(cur) == 1)) return (NULL);

    cur  = xmlVerticalParentRouteAdopt(context, cur);
    
    orig = cur;
    rout = NULL;
    
    /* initial step down */
    next = cur->last;
    if (next != NULL) {
      next = xmlParentRouteDown(context, cur, next); /* recusion check may cause a NULL response */
      next = xmlDeviateToDeviant(context, next);
      if (next != NULL) rout = next; /* last valid */
    }
    cur = next;
    
    /* we need to take the LAST accessible child in this scenario */
    if ((cur != NULL) && (
        (xmlReadableContextNode(context, cur) == NULL) 
      || ((ignore_type_policy & SKIP_DTD_AND_ENTITY)   && ((cur->type == XML_ENTITY_DECL) || (cur->type == XML_DTD_NODE)))
      || ((ignore_type_policy & XML_ELEMENT_NODE_ONLY) && (cur->type != XML_ELEMENT_NODE))
    )) {
      /* xmlReadableContextPrev(...) loops along the child axis UNTIL it finds an accessible node */
      cur = xmlReadableContextPrev(context, cur, ignore_type_policy);
    }
    
    /* return the parent_route un-touched if there is no next
     * need to move up to the first hardlink and allow strafe back
     * otherwise the xmlParentRouteUp() will complain strange behavior
     */
    if ((cur == NULL) && (rout != NULL)) xmlParentRouteUp(context, rout, xmlFirstHardLink(orig));

    return(cur);
}

xmlNodePtr
xmlReadableChildren(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextChildren(ctxt->context, cur, ignore_type_policy);
}

xmlNodePtr
xmlReadableContextChildren(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy)
{
    xmlNodePtr next;

    if (context == NULL)   return(NULL);
    if (cur == NULL)       return(NULL);
    if ((ignore_type_policy & NO_SOFTLINK) && (xmlIsSoftLink(cur) == 1)) return (NULL);

    cur  = xmlVerticalParentRouteAdopt(context, cur);
    
    /* initial step down 
     * this supports XML_DOCUMENT_NODE child moves 
     * XML_ATTRIBUTE_NODE->children will return NULL
     */
    switch (cur->type) {
      case XML_ATTRIBUTE_NODE: 
        next = NULL;
        break;
      case XML_DOCUMENT_NODE:
      default:
        next = cur->children;
    }

    if (next != NULL) {
      next = xmlParentRouteDown(context, cur, next); /* recusion check may cause a NULL response */
      next = xmlDeviateToDeviant(context, next);
    }
    cur = next;
    
    /* we need to take the LAST accessible child in this scenario */
    if ((cur != NULL) && (
        (xmlReadableContextNode(context, cur) == NULL) 
      || ((ignore_type_policy & SKIP_DTD_AND_ENTITY)   && ((cur->type == XML_ENTITY_DECL) || (cur->type == XML_DTD_NODE)))
      || ((ignore_type_policy & XML_ELEMENT_NODE_ONLY) && (cur->type != XML_ELEMENT_NODE))
    )) {
      /* xmlReadableContextPrev(...) loops along the child axis UNTIL it finds an accessible node */
      cur = xmlReadableContextNext(context, cur, ignore_type_policy);
    }

    return(cur);
}

xmlNodePtr
xmlReadableProperties(xmlXPathParserContextPtr ctxt, xmlNodePtr cur)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextProperties(ctxt->context, cur);
}

xmlNodePtr
xmlReadableContextProperties(xmlXPathContextPtr context, xmlNodePtr cur)
{
    xmlNodePtr next;
    
    if (context == NULL) return(NULL);
    if (cur == NULL)     return(NULL);
    if ((cur->type != XML_ELEMENT_NODE) && (cur->type != XML_PI_NODE)) return(NULL);
                                                        
    cur = xmlVerticalParentRouteAdopt(context, cur);

    /* initial step
     * this is a non-element axis move so no need to pop the parent_route input
     * or indeed to deviate to the original first, properties are part of the deviation
     * there is no security on properties. assumming the owning element read was valid
     */
    next = (xmlNodePtr) cur->properties;
    if (next != NULL) {
      next = xmlParentRouteDown(context, cur, next); /* recusion check may cause a NULL response */
      next = xmlDeviateToDeviant(context, next);
    }

    return(next);
}

xmlNodePtr
xmlReadableParent(xmlXPathParserContextPtr ctxt, xmlNodePtr cur, int ignore_type_policy)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextParent(ctxt->context, cur, ignore_type_policy);
}

xmlNodePtr
xmlReadableContextParent(xmlXPathContextPtr context, xmlNodePtr cur, int ignore_type_policy)
{
    xmlNodePtr next;
    
    if (context == NULL) return(NULL);
    if (cur     == NULL) return(NULL);

    if ((ignore_type_policy & NO_SOFTLINK) == 0) cur = xmlVerticalParentRouteAdopt(context, cur);
                                                        
    cur = xmlDeviateToOriginal(context, cur);
    
    switch (cur->type) {
      case XML_ATTRIBUTE_NODE: {
        next = ((xmlAttrPtr) cur)->parent;
        break;
      }
      default: next = cur->parent;
    }

    if (next != NULL) {
      next = xmlParentRouteUp(context, cur, next);
      next = xmlDeviateToDeviant(context, next);
    }

    /* SECURITY: security checks during upward navigation */
    /* TODO: turning on parent security checking cause an infinite loop in Class derived analysis */
    if (next != NULL) next = xmlReadableContextNode(context, next);
                                                            
    /* parent should always be accessible 
     * unless node-mask!
     */
    if (next == NULL)
      XML_TRACE_GENERIC1(XML_DEBUG_XPATH_STEP, "xmlReadableContextParent: returning NULL parent from [%s]: this might be a node-mask",
        (cur->name ? cur->name : BAD_CAST "<no name>")
      );
    
    /* ignore_type_policy useful for stopping at the XML_DOCUMENT_NODE */
    if ((next != NULL) && ((ignore_type_policy & XML_ELEMENT_NODE_ONLY) && (next->type != XML_ELEMENT_NODE)))
      next = NULL;
    
    return(next);
}

xmlNodePtr
xmlReadableDocRoot(xmlXPathParserContextPtr ctxt, xmlNodePtr cur)
{
    if (ctxt == NULL) return(NULL);
    return xmlReadableContextDocRoot(ctxt->context, cur);
}
xmlNodePtr
xmlReadableContextDocRoot(xmlXPathContextPtr context, xmlNodePtr docNode)
{
    xmlDocPtr doc;
    xmlNodePtr cur;

    if (context == NULL) return(NULL);
    if (docNode == NULL) return(NULL);

    doc = (xmlDocPtr) docNode;
    cur = xmlDocGetRootElement(doc);
    if (cur != NULL) cur = xmlReadableContextNode(context, cur);
    
    return(cur);
}

/*
 * nodes should be excluded by traversal not self checking at list addition
xmlNodePtr
xmlReadableXPathNodeSetAddUnique(xmlXPathContextPtr context, xmlNodeSetPtr set, xmlNodePtr cur)
{
    NOSECURITY_RETURN_VOID(xmlXPathNodeSetAddUnique(set, cur, NULL), cur);
    if ((context == NULL) || (set == NULL) || (cur == NULL)) return(NULL);

    cur = xmlReadableContextNode(context, cur);
    if (cur != NULL) xmlXPathNodeSetAddUnique(set, cur, NULL);

    return(cur);
}
*/

/**
 * xmlXPathPop:
 * @ctxt:  an XPath parser context
 *
 * Pops from the stack, without conversion.
 */
void
xmlXPathPop(xmlXPathParserContextPtr ctxt) {
    xmlXPathObjectPtr obj;

    obj = valuePop(ctxt);
    if (obj == NULL) {xmlXPathSetError(ctxt, XPATH_INVALID_OPERAND);}
    else             xmlXPathReleaseObject(ctxt->context, obj);
}
