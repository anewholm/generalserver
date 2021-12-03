#define NAMESPACE_SEARCH_FOR_DEFAULT 0

static xmlChar *xmlStritoa(int n, xmlChar *prefix, xmlChar *suffix) {
  //caller frees result
  int absn;
  xmlChar *buffer, *pos;
  int len;
  int digits      = 0;
  int prefix_len  = (prefix ? xmlStrlen(prefix) : 0);
  int suffix_len  = (suffix ? xmlStrlen(suffix) : 0);
  int is_negative = (n < 0);

  //calculate size
  absn = abs(n);
  do {digits++;} while ((absn /= 10) > 0);
  len    = prefix_len + is_negative + digits + suffix_len;
  buffer = xmlMalloc(len + 1);
  
  //fill content
  if (prefix_len) xmlStrcpy(buffer, prefix);
  absn  = abs(n);
  pos   = buffer + len - suffix_len;
  *pos-- = '\0'; //zero terminate
  do {*pos-- = (absn % 10) + '0';} while ((absn /= 10) > 0);
  if (is_negative) *pos-- = '-';
  pos++;
  if (suffix_len) xmlStrcpy(buffer + len - suffix_len, suffix); //including zero terminator

  return buffer;
}

/**
 * xmlCreateHardlinkInfo:
 *
 * Creation of a new hardlink info on cur
 * will throw an error if it is already created
 * this is normally lazy creation on demand
 *
 * Returns a pointer to the new hardlink object
 */
xmlHardlinkInfoPtr
xmlCreateHardlinkInfo(xmlNodePtr cur) {
    xmlHardlinkInfoPtr info;

    if (cur->type != XML_ELEMENT_NODE) xmlGenericError(xmlGenericErrorContext, "xmlCreateHardlinkInfo : XML_ELEMENT_NODEonly");
    if (cur->hardlink_info != NULL)    xmlGenericError(xmlGenericErrorContext, "xmlCreateHardlinkInfo : existing hardlink_info != NULL");

    info = xmlMalloc(sizeof(xmlHardlinkInfo));
    if (info == NULL) {
        xmlTreeErrMemory("building hardlink info");
        return(NULL);
    }
    memset(info, 0, sizeof(xmlHardlinkInfo));
    cur->hardlink_info = info;

    return(info);
}

/**
 * xmlHardlinkChild:
 * @parent:  the additonal parent node
 * @cur:  the common single shared child node
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * This is a NEW, non-standard LibXml2 function
 * Add an already linked node additionally to @parent, at the end of the child (or property) list
 * TEXT and ATTRIBUTE nodes are not allowed
 * Only ELEMENTs
 * Annesley: does not unlink the child from its previous parent, use xmlUnlinkNode() for that
 *
 * It is done like this because LibXml2 uses a double linked list for its children
 * which prevents simply pointing to the node from 2 parents because their siblings, prev and next, would be confused
 * The Namespace needs to be bought in to scope (referencing an ancestor @xmlns decleration) for the new location
 *
 * Returns the child or NULL in case of error.
 */
xmlNodePtr
xmlHardlinkChild(xmlNodePtr additional_parent, xmlListPtr parent_route, xmlNodePtr cur, xmlNodePtr before, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
    xmlNodePtr newHardLink = 0, 
               next;
    xmlNsPtr newLocationDefaultNS, nsDef, resolvedNS;

    /* -------------------------------- checks */
    if (additional_parent == NULL) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : additional_parent == NULL");
        return(NULL);
    }

    if (cur == NULL) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : child == NULL");
        return(NULL);
    }

    if ((before != NULL) && (additional_parent != before->parent)) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : before->parent != additional_parent");
        return(NULL);
    }

    if (additional_parent == cur) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : additional_parent == cur");
        return(NULL);
    }

    if (cur->type != XML_ELEMENT_NODE) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : XML_ELEMENT_NODE supported only");
        return(NULL);
    }

    if (cur->doc != additional_parent->doc) {
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : Cross document hard linking not currently supported");
        return(NULL);
    }
    
    if (xmlIsHardLink(cur) == 1) {
        /* TODO: Progressive hardlinking */
        xmlGenericError(xmlGenericErrorContext, "xmlHardlinkChild : Progressive hardlinking (hardlinks to hardlinks) not supported yet");
        return(NULL);
    }

    /* Annesley: triggers, security filtering */
    /* includes xmlTriggerCall(xtrigger, additional_parent, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter) */
    if (xmlWriteableNode(xfilter, xtrigger, additional_parent, parent_route) == NULL) return (NULL);

    /* -------------------------------- all ok, process,
     * same children, prev and next siblings and additional_parent. also same name, and other properties
     * change the prev and next siblings and additional_parent
     * but leave the children the same
     * the Namespace needs to be bought in to scope (referencing an ancestor @xmlns decleration) for the new location
     */
    if ((newHardLink = xmlDocCopyNode(cur, cur->doc, 0))) {
      newHardLink->ns         = cur->ns;
      newHardLink->nsDef      = cur->nsDef;      /* share nsDefs */
      newHardLink->children   = cur->children;   /* share children */
      newHardLink->last       = cur->last;       /* share last child */
      newHardLink->properties = cur->properties; /* share attributes */
      newHardLink->extra      = cur->extra;
      newHardLink->line       = cur->line;

      /* connect the new hard link version of current in to its existing new additional_parent 
       * newHardLink->parent = additional_parent;
       */
      if (before == NULL) newHardLink = xmlAddChild(additional_parent, newHardLink);
      else                newHardLink = xmlAddPrevSibling(before, newHardLink);

      /* Reconcile the namespace for the new scope: we want to maintain the prefix or lack of 
       * we may end up pointing to the same NS
       * the NS def may be declared ON this node
       * in General Server, NULL namespace is illegal. here we allow it
       */
      if (newHardLink->ns != NULL) {
        resolvedNS = NULL;
        if (newHardLink->ns->prefix == NULL) {
          /* source element (cur) is in a default namespace, usually (x)html
           * using xmlSearchNsWithPrefixByHref() would add a prefix to the hardlink
           *   e.g. <li> => <html:li>
           * reuse the default namespace if it is the same
           * (copy the default namespace decl to the hardlink otherwise)
           */
          newLocationDefaultNS = xmlSearchNs(newHardLink->doc, newHardLink, NULL); /* NULL = default NS: no ns->prefix */
          if ((newLocationDefaultNS != NULL) 
            && (xmlStrEqual(newLocationDefaultNS->href, newHardLink->ns->href) == 1)
          ) {
            /* default namespace in new location is the same as the old location so use it 
             * this will be the common situation with General Server
             */
            resolvedNS = newLocationDefaultNS;
          }
        } else {
          /* look for the same prefix AND href to attach to */
          resolvedNS = xmlSearchNsWithPrefixByHref(newHardLink->doc, newHardLink, newHardLink->ns->href);
        }
        
        /* if failed to resolve the namespace 
         * this means that the namespace, default or not, is NOT declared on this node
         * and one with identical HREF and prefix is not available
         * copy it to our new location 
         */
        if (resolvedNS == NULL) {
          resolvedNS = xmlCopyNamespace(newHardLink->ns);
          /* add the copied namespace decl to the element */
          if (newHardLink->nsDef == NULL) newHardLink->nsDef = resolvedNS;
          else {
            nsDef = newHardLink->nsDef;
            while (nsDef->next != NULL) nsDef = nsDef->next;
            nsDef->next = resolvedNS;
          }
        }

        /* node->ns = ns; */
        xmlSetNs(newHardLink, resolvedNS); 
      }
      
      /* Annesley: [create &] update this nodes double linked list: hardlink_info->prev/next
       *   it might not have any children, next or previous, properties, nsDef
       *   so we cannot find the other hardlinks based on those
       * INSERT the hardlink in to the chain, according to hardlink shared from
       *   not just on the end
       * IMPORTANT: the original node is always the first in this list
       */
      if (cur->hardlink_info == NULL) xmlCreateHardlinkInfo(cur); /* initial hardlink */
      xmlCreateHardlinkInfo(newHardLink);

      newHardLink->hardlink_info->prev = cur;
      next = cur->hardlink_info->next;
      if (next != NULL) { /* maybe NULL */
        newHardLink->hardlink_info->next = next;
        next->hardlink_info->prev = newHardLink;
      }
      cur->hardlink_info->next = newHardLink;

      /* Annesley: triggers, security filtering */
      if (additional_parent) xmlTriggerCall(xtrigger, additional_parent, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
    }

    return(newHardLink);
}

/**
 * xmlLastHardLink:
 * @cur:  the hardlinked (or not) node
 *
 * Returns last node in hardlink chain.
 * Returns input if not hardlinked
 */
xmlNodePtr xmlLastHardLink(xmlNodePtr cur) {
  if ((cur == NULL) || (cur->type != XML_ELEMENT_NODE)) return(cur);
  if (xmlIsHardLinked(cur) == 0) return (cur);

  while (cur->hardlink_info->next != NULL) {
    cur = cur->hardlink_info->next;
  }

  return (cur);
}

/**
 * xmlHardLinkOriginal:
 * @cur:  the hardlinked (or not) node
 *
 * IMPORTANT: the original node is always the first in this list
 * Returns first node in hardlink chain.
 * Returns input if not hardlinked
 */
xmlNodePtr xmlHardLinkOriginal(xmlNodePtr cur) {
  return xmlFirstHardLink(cur);
}

/**
 * xmlFirstHardLink:
 * @cur:  the hardlinked (or not) node
 *
 * IMPORTANT: the original node is always the first in this list
 * Returns first node in hardlink chain.
 * Returns input if not hardlinked
 */
xmlNodePtr xmlFirstHardLink(xmlNodePtr cur) {
  if ((cur == NULL) || (cur->type != XML_ELEMENT_NODE)) return(cur);
  if (xmlIsHardLinked(cur) == 0) return (cur);

  while (cur->hardlink_info->prev != NULL) {
    cur = cur->hardlink_info->prev;
  }

  return (cur);
}

/**
 * xmlTouchNodeSecure:
 * @cur:  the node
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 */
void
xmlTouchNodeSecure(xmlNodePtr cur, xmlListPtr parent_route, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
    if (cur == NULL) return;
    /* includes xmlTriggerCall(xtrigger, cur, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter); */
    if (xmlWriteableNode(xfilter, xtrigger, cur, parent_route) == NULL) return;
    xmlTriggerCall(xtrigger, cur, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
}


/**
 * xmlUnlinkNodeSecure:
 * @cur:  the node
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 */
void
xmlUnlinkNodeSecure(xmlNodePtr cur, xmlListPtr parent_route, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
    xmlNodePtr parent;

    if (cur == NULL) return;

    /* Annesley: triggers, security filtering */
    parent = cur->parent; /* before it is unlinked! */
    /* includes xmlTriggerCall(xtrigger, cur, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter); */
    if (xmlWriteableNode(xfilter, xtrigger, cur, parent_route) == NULL) return;

    xmlUnlinkNode(cur);

    /* Annesley: triggers, security filtering */
    if (parent) xmlTriggerCall(xtrigger, parent, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
}

#ifdef LIBXML_LOCKING_ENABLED
static xmlMutexPtr xmlLockNodeMutex = NULL;

/**
 * xmlLockNode:
 * @cur:  the node being [un]locked
 * @lock_external: the external requesting reference, usually a pointer
 *
 * @author: Annesley
 * 
 * usage: my_node = xmlLockNode(my_node, wrapper, "why?")
 * when xmlFree() is called on a node it is node->marked_for_destruction = 1
 * and xmlLockNode() will refuse to lock the node instead returning NULL
 *
 * Returns cur if locking successful, NULL if failed
 */
xmlNodePtr
xmlLockNode(xmlNodePtr cur, ATTRIBUTE_UNUSED const void *lock_external, ATTRIBUTE_UNUSED const xmlChar *lock_reason) {
  xmlNodePtr ret = NULL;

  /* MUTEX:
   * required in case another thread:
   *   xmlUnLockNode()
   *   and xmlFreeNode() this node
   * between the marked_for_destruction and the cur->locks increase
   */
#ifdef LIBXML_THREAD_ENABLED
  xmlMutexLock(xmlLockNodeMutex); {
#endif
    if (MARKED_FOR_DESTRUCTION(cur) == 0) {
      cur->locks++;
      ret = cur;
#ifdef LIBXML_DEBUG_ENABLED
      /* lazy initialise 
      * freed in the xmlFreeNode()
      */
      if (lock_reason == 0) lock_reason = BAD_CAST "none given";
      if (cur->lock_externals == NULL) {
        cur->lock_externals = xmlMalloc(sizeof(void*) * cur->locks);
        cur->lock_reasons   = xmlMalloc(sizeof(void*) * cur->locks);
      } else {
        cur->lock_externals = xmlRealloc(cur->lock_externals, sizeof(void*) * cur->locks);
        cur->lock_reasons   = xmlRealloc(cur->lock_reasons,   sizeof(void*) * cur->locks);
      }
      cur->lock_externals[cur->locks-1] = lock_external; /* usually an external node wrapper */
      cur->lock_reasons[  cur->locks-1] = lock_reason;   /* freed by caller, often static const */
#endif
    }
#ifdef LIBXML_THREAD_ENABLED
  } xmlMutexUnlock(xmlLockNodeMutex);
#endif
  
  return ret;
}

/**
 * xmlLockFreezeNode:
 * @cur:  the node being frozen
 * @external_object: the external requesting reference, usually a pointer
 *
 * @author: Annesley
 * 
 * No more locks will be permitted after freeze
 * xmlFree() will do this
 * 
 * Returns @cur
 */
xmlNodePtr
xmlLockFreezeNode(xmlNodePtr cur) {
  cur->marked_for_destruction = 1;
  return cur;
}

/**
 * xmlUnLockNode:
 * @cur:  the node being [un]locked
 * @lock_external: the external requesting reference, usually a pointer
 *
 * @author: Annesley
 * 
 * Always unlocks unless no locks left
 * 
 * Returns number of remaining locks
 */
int
xmlUnLockNode(xmlNodePtr cur, ATTRIBUTE_UNUSED const void *lock_external) {
  unsigned int uLock, destroy_node;
  
  if (cur == NULL)     xmlGenericError(xmlGenericErrorContext, "xmlUnLockNode : node NULL");
  if (cur->locks == 0) xmlGenericError(xmlGenericErrorContext, "xmlUnLockNode : node not locked!");
  
#ifdef LIBXML_THREAD_ENABLED
  xmlMutexLock(xmlLockNodeMutex); {
#endif
#ifdef LIBXML_DEBUG_ENABLED
    /* remove the correct debug reference for this lock 
    * we only need to do this if the lock_external is in the body of the list
    * if it is at the end then the reduction in list length will cause it to be forgotten anyway
    */
    unsigned int iFound = 0;
    for (uLock = 0; uLock < cur->locks; uLock++) {
      if (cur->lock_externals[uLock] == lock_external) iFound = 1;
      if (iFound == 1 && uLock+1 < cur->locks) {
        /* continue to move all lock_externals back one position, maintaining order */
        cur->lock_externals[uLock] = cur->lock_externals[uLock+1];
        cur->lock_reasons[uLock]   = cur->lock_reasons[uLock+1];
      }
    }
    if (iFound == 0) xmlGenericError(xmlGenericErrorContext, "xmlUnLockNode : failed to find external lock debug pointer");
#endif
  
    /* possible final unlock happening on a node marked for destruction 
    * marked_for_destruction indicates that another lock holder has already tried to xmlFreeNode(this)
    * thus, this lock holder is completing the xmlFreeNode(this) instead
    * and another is not expected because this is the final lock
    * new locks cannot be made
    * 
    * MUTEX: 
    * marked_for_destruction is uni-directional
    * that is, it cannot change back from 1 to 0
    * 2 threads could decrease cur->locks simultaneously 
    * and then both calculate destroy_node as 1
    * with the mutex, only 1 thread, the last, could decrease the cur->locks to zero
    */
    --cur->locks;
    destroy_node = ((MARKED_FOR_DESTRUCTION(cur) == 1) && (cur->locks == 0));
#ifdef LIBXML_THREAD_ENABLED
  } xmlMutexUnlock(xmlLockNodeMutex);
#endif
  
  uLock = cur->locks; /* store this because of potential memory release with xmlFreeNode() */
  if (destroy_node == 1) {
    XML_TRACE_GENERIC1(XML_DEBUG_TREE, "xmlUnLockNode : freeing node [%s] marked_for_destruction", 
      (cur->name ? cur->name : BAD_CAST "<no name>")
    );
    xmlFreeNode(cur);
  }
  
  return uLock;
}
#endif /* LIBXML_LOCKING_ENABLED */

/**
 * xmlInitTree:
 * @author: Annesley
 */
void
xmlInitTree() {
#ifdef LIBXML_LOCKING_ENABLED
  xmlLockNodeMutex = xmlNewMutex();
#endif  
}

/**
 * xmlNodeSetContentSecure:
 * @cur:  the node being modified
 * @content:  the new value of the content
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * Replace the content of a node.
 * NOTE: @content is supposed to be a piece of XML CDATA, so it allows entity
 *       references, but XML special chars need to be escaped first by using
 *       xmlEncodeEntitiesReentrant() resp. xmlEncodeSpecialChars().
 *
 * @author: Annesley
 */
void
xmlNodeSetContentSecure(xmlNodePtr cur, xmlListPtr parent_route, const xmlChar *content, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
    if (cur == NULL) return;

    /* only trigger if there is a change */
    if (xmlStrEqual(cur->content, content) == 0) {
      /* Annesley: triggers, security filtering */
      /* includes xmlTriggerCall(xtrigger, cur, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter); */
      if (xmlWriteableNode(xfilter, xtrigger, cur, parent_route) == NULL) return;

      xmlNodeSetContent(cur, content);

      /* Annesley: triggers, security filtering */
      xmlTriggerCall(xtrigger, cur, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
    }
}

/**
 * xmlSetPropSecure:
 * @node:  the node
 * @name:  the attribute name (a QName)
 * @value:  the attribute value
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 */
xmlAttrPtr
xmlSetPropSecure(xmlNodePtr node, xmlListPtr parent_route, const xmlChar *name, const xmlChar *value,
  const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter
) {
    xmlAttrPtr attr;

    /* only trigger if there is a change */
    attr = xmlHasProp(node, name);
    if ((attr == NULL) || (xmlStrEqual(attr->children->content, value) == 0)) {
      /* Annesley: triggers, security filtering */
      /* includes xmlTriggerCall(xtrigger, cur, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter); */
      if (xmlWriteableNode(xfilter, xtrigger, node, parent_route) == NULL) return 0;

      attr = xmlSetProp(node, name, value);

      /* Annesley: triggers, security filtering */
      xmlTriggerCall(xtrigger, node, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
    }
    
    return (attr);
}

/**
 * xmlSetNsPropSecure:
 * @node:  the node
 * @ns:    the namespace
 * @name:  the attribute name (a QName)
 * @value:  the attribute value
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 */
xmlAttrPtr
xmlSetNsPropSecure(xmlNodePtr node, xmlListPtr parent_route, xmlNsPtr ns, const xmlChar *name, const xmlChar *value,
  const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter
) {
    xmlAttrPtr attr;

    /* only trigger if there is a change */
    attr = xmlHasNsProp(node, name, (ns ? ns->href : NULL));
    if ((attr == NULL) || (xmlStrEqual(attr->children->content, value) == 0)) {
      /* Annesley: triggers, security filtering */
      /* includes xmlTriggerCall(xtrigger, cur, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter); */
      if (xmlWriteableNode(xfilter, xtrigger, node, parent_route) == NULL) return 0;

      attr = xmlSetNsProp(node, ns, name, value);
      
      /* Annesley: triggers, security filtering */
      xmlTriggerCall(xtrigger, node, parent_route, TRIGGER_OP_WRITE, TRIGGER_STAGE_AFTER, xfilter);
    }

    return (attr);
}

/**
 * xmlIsSharedDoc:
 * @doc:  the document
 *
 * Shared documents have root nodes that point directly in to other documents
 * So they should not be freed or changed in the same way
 * 
 * 1 = document is shared
 */
int
xmlIsSharedDoc(const xmlDocPtr doc) {
  return ((doc != NULL) && (doc->children != NULL) && (doc->children->doc != doc));
}

const xmlChar *xmlNamespacePrefix(xmlNodePtr cur, const int require_namespaces, const int reconcile_default) {
  /* PERFORMANCE: potentially very expensive process */
  const xmlChar *sDefaultPrefix = 0;
  xmlNsPtr  oReconciledNs       = 0,
            oDefaultNoPrefixNs  = 0;

  /* element default and no-prefix namespace checking
   * this has probably been triggered internally by a fullyQualifiedName() check
   * general server namespace policies:
   *   any given prefix can only be used to indicate the same namespace,
   *   i.e. xmlns:xs cannot appear in the tree twice pointing to 2 different namespaces
   *   top default and stated namespace prefix is gs (always ...general_server)
   *   all XML_ELEMENT_NODEs must have a namespace (gs is default) and thus a valid stated prefix declared somewhere in the tree
   * for XML_ELEMENT_NODE with no prefix: this procedure looks up the tree for an xmlns:<prefix> for that namespace
   * thus allowing us to always provide a valid prefix when we want to
   */
#ifdef LIBXML_DEBUG_ENABLED  
  if ( ((cur->type == XML_ATTRIBUTE_NODE) || (cur->type == XML_ELEMENT_NODE))
    && cur->ns != NULL
    && cur->ns->context != NULL
    && cur->ns->context != cur->doc
  )
    __xmlRaiseError(NULL, NULL, NULL,
      NULL, cur, XML_FROM_VALID,
      XML_NAMESPACE_WRONG_DOC, XML_ERR_ERROR, NULL, 0,
      (const char*) cur->name, NULL, NULL, 0, 0,
      "xmlNamespacePrefix(): namespace from different doc %s", 
      BAD_CAST cur->name
    );
#endif
    
  /* assert(cur->parent || (cur->ns && cur->ns != (void*) 0xffffff && cur->ns->prefix)); */
  switch (cur->type) {
    case XML_TEXT_NODE: {
      /* let us quietly accept this */
      break;
    }
    case XML_ATTRIBUTE_NODE: {
      /* attributes cannot have a default, no prefix namespace
       * the prefix is always stated directly
       */
      oReconciledNs = cur->ns; 
      if (oReconciledNs && oReconciledNs->prefix) sDefaultPrefix = oReconciledNs->prefix;
      break;
    }
    case XML_ELEMENT_NODE: {
      oReconciledNs = cur->ns; /* may already have a valid prefix */
      if (oReconciledNs == NULL) {
        /* we have the NO NAMESPACE (0x0) at all
         * this is NOT the default namespace, but instead a complete absense of namespace
         * THIS IS ILLEGAL in general_server as there should be a xmlns="<general_server>" at the top of all documents
         *   effectively putting everything in the <general_server> namespace when it is not explicitly prefixed
         */
        if (require_namespaces == 1) {
          __xmlRaiseError(NULL, NULL, NULL,
            NULL, cur, XML_FROM_VALID,
            XML_NAMESPACE_REQUIRED, XML_ERR_WARNING, NULL, 0,
            (const char*) cur->name, NULL, NULL, 0, 0,
            "xmlNamespacePrefix(): 0x0 namespace %s", 
            BAD_CAST cur->name
          );
        } else {
          /* assume that it is in the default namespace instead anyway and search for it */
          oDefaultNoPrefixNs = xmlSearchNs(cur->doc, cur, NAMESPACE_SEARCH_FOR_DEFAULT);
          if (oDefaultNoPrefixNs) {
            oReconciledNs = oDefaultNoPrefixNs;
          } else {
            __xmlRaiseError(NULL, NULL, NULL,
              NULL, cur, XML_FROM_VALID,
              XML_NAMESPACE_DEFAULT_NOT_FOUND, XML_ERR_WARNING, NULL, 0,
              (const char*) cur->name, NULL, NULL, 0, 0,
              "xmlNamespacePrefix(): failed to find default namespace: %s", 
              BAD_CAST cur->name
            );
          }
        }
      }
      
      /* we want the normal prefix if declared
       * note that the normal LibXml2::xmlSearchNsByHref(...) will get the one without the prefix if it comes first in the ->nsDef
       */
      if ((oReconciledNs != NULL) && (oReconciledNs->prefix == NULL) && (reconcile_default == 1)) {
        oReconciledNs = xmlSearchNsWithPrefixByHref(cur->doc, cur, oReconciledNs->href);
        if (oReconciledNs == NULL)
          __xmlRaiseError(NULL, NULL, NULL,
            NULL, cur, XML_FROM_VALID,
            XML_NAMESPACE_PREFIXED_NOT_FOUND, XML_ERR_WARNING, NULL, 0,
            (const char*) cur->name, NULL, NULL, 0, 0,
            "xmlNamespacePrefix(): xmlSearchNsWithPrefixByHref() failed to find prefixed namespace: %s", 
            BAD_CAST cur->name
          );
      }

      if (oReconciledNs != NULL) sDefaultPrefix = oReconciledNs->prefix;
      break;
    }
    default: {
      /* other node types DO NOT have namespaces under consideration */
    }
  }

  return sDefaultPrefix;
}

/**
 * xmlSearchNsWithPrefixByHref:
 * @author: Annesley
 * @doc:   the document
 * @node:  the current node
 * @href:  the namespace value
 *
 * Search a Ns aliasing a given URI. Recurse on the parents until it finds
 * the defined namespace or return NULL otherwise.
 * Check the document root node namespace definitions first and use.
 * This also works for unlinked nodes in a doc
 * Returns the namespace pointer or NULL.
 */
xmlNsPtr
xmlSearchNsWithPrefixByHref(xmlDocPtr doc, xmlNodePtr node, const xmlChar * href)
{
    xmlNsPtr cur;
    xmlNodePtr orig = node;
    int is_attr;

    if ((node == NULL) || (href == NULL)) return (NULL);

    if (xmlStrEqual(href, XML_XML_NAMESPACE)) {
        /*
         * Only the document can hold the XML spec namespace.
         */
        if ((doc == NULL) && (node->type == XML_ELEMENT_NODE)) {
            /*
             * The XML-1.0 namespace is normally held on the root
             * element. In this case exceptionally create it on the
             * node element.
             */
            cur = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
            if (cur == NULL) {
                xmlTreeErrMemory("searching namespace");
                return (NULL);
            }
            memset(cur, 0, sizeof(xmlNs));
            cur->type = XML_LOCAL_NAMESPACE;
            cur->href = xmlStrdup(XML_XML_NAMESPACE);
            cur->prefix = xmlStrdup((const xmlChar *) "xml");
            cur->next = node->nsDef;
            node->nsDef = cur;
            return (cur);
        }
        if (doc == NULL) {
            doc = node->doc;
            if (doc == NULL)
                return(NULL);
        }
        /*
        * Return the XML namespace declaration held by the doc.
        */
        if (doc->oldNs == NULL)
            return(xmlTreeEnsureXMLDecl(doc));
        else
            return(doc->oldNs);
    }

    is_attr = (node->type == XML_ATTRIBUTE_NODE);

    /* Annesley: check the root node on the document first */
    node = xmlDocGetRootElement(doc);
    if (node != NULL) {
      cur  = node->nsDef;
      while (cur != NULL) {
          if ((cur->href != NULL) && (href != NULL) &&
              (xmlStrEqual(cur->href, href))) {
              if ((cur->prefix != NULL) &&
                  (   (orig->parent == NULL)
                   || (is_attr && (orig->parent->parent == NULL)) /* allowing disembodied elements / attributes */
                   || (xmlNsInScope(doc, orig, node, cur->prefix) == 1)
                  ))
                  return (cur);
          }
          cur = cur->next;
      }
    }

    /* recurse up the tree looking for nsDefs */
    node = orig;
    while (node != NULL) {
        if ((node->type == XML_ENTITY_REF_NODE) ||
            (node->type == XML_ENTITY_NODE) ||
            (node->type == XML_ENTITY_DECL))
            return (NULL);
        if (node->type == XML_ELEMENT_NODE) {
            /* search the nsDefs of this self_or_ancestor node */
            cur = node->nsDef;
            while (cur != NULL) {
                if ((cur->href != NULL) && (href != NULL) &&
                    (xmlStrEqual(cur->href, href))) {
                    if ((cur->prefix != NULL) &&
                        (xmlNsInScope(doc, orig, node, cur->prefix) == 1))
                        return (cur);
                }
                cur = cur->next;
            }

            /* now search the actual ns of the ancestor for a match as well */
            if (orig != node) {
                cur = node->ns;
                if (cur != NULL) {
                    if ((cur->href != NULL) && (href != NULL) &&
                        (xmlStrEqual(cur->href, href))) {
                        if (((!is_attr) && (cur->prefix != NULL)) &&
                            (xmlNsInScope(doc, orig, node, cur->prefix) == 1))
                            return (cur);
                    }
                }
            }
        }
        node = node->parent;
    }
    return (NULL);
}

/**
 * xmlTreeChangeType:
 * xmlCreateTreeChange:
 * xmlCreateTreeChange*:
 * xmlPushTreeChange*:
 * @node:         a node to affect
 * @related_node: depending on the type of change, the reference node
 *
 * @author: Annesley
 * 
 * Tree changes similar to TXml:
 * Adds the change request on to the list
 * 
 * Returns: void
 */
static xmlTreeChangePtr xmlCreateTreeChange(enum xmlTreeChangeType type, xmlNodePtr node, xmlNodePtr related_node, int position) {
  xmlTreeChangePtr tree_change = xmlMalloc(sizeof(struct _xmlTreeChange));
  tree_change->type         = type;
  tree_change->node         = node;
  tree_change->related_node = related_node;
  tree_change->position     = position;
  return tree_change;
}

static void xmlTreeChangeDeallocator(void *tree_change) {
  xmlFree(tree_change);
}

#ifdef LIBXML_DEBUG_ENABLED
static void *xmlCheckTreeChange(const void *data, const void *user1 ATTRIBUTE_UNUSED, const void *user2 ATTRIBUTE_UNUSED) {
  xmlTreeChangeConstPtr tree_change = data;
  if (tree_change->type > 100) 
    xmlGenericError(xmlGenericErrorContext, "xmlCheckTreeChange: tree_change looks freed");
  return 0;
}
static void xmlCheckTreeChangeList(xmlListPtr tree_changes) {
  xmlListWalk(tree_changes, xmlCheckTreeChange, NULL, NULL);
}
/*
static void *xmlDebugListNode(const void *data, const void *user1 ATTRIBUTE_UNUSED, const void *user2 ATTRIBUTE_UNUSED) {
  xmlMergeMatchConstPtr merge_match = data;
  xmlDebugDumpNode(NULL, merge_match->node, 1);
  return 0;
}
static void xmlDebugListMergeMatchOutput(xmlListPtr nodes) {
  xmlListWalk(nodes, xmlDebugListNode, NULL, NULL);
}
*/
#else
static void xmlCheckTreeChangeList(xmlListPtr tree_changes ATTRIBUTE_UNUSED) {}
#endif

static xmlTreeChangePtr xmlCreateTreeChangeRemoveNode(xmlNodePtr node)                                        {return xmlCreateTreeChange(LibXml_removeNode, node, NULL, 0);}
static xmlTreeChangePtr xmlCreateTreeChangeCopyChild( xmlNodePtr node, xmlNodePtr related_node, int position) {return xmlCreateTreeChange(LibXml_copyChild,  node, related_node, position);}
static xmlTreeChangePtr xmlCreateTreeChangeChangeName(xmlNodePtr node, xmlNodePtr related_node)               {return xmlCreateTreeChange(LibXml_changeName, node, related_node, 0);}
static xmlTreeChangePtr xmlCreateTreeChangeSetValue(  xmlNodePtr node, xmlNodePtr related_node)               {return xmlCreateTreeChange(LibXml_setValue,   node, related_node, 0);}
static xmlTreeChangePtr xmlCreateTreeChangeMoveChild( xmlNodePtr node, xmlNodePtr related_node, int position) {return xmlCreateTreeChange(LibXml_moveChild,  node, related_node, position);}

/* static void xmlPushTreeChangeRemoveNode( xmlListPtr tree_changes, xmlNode *node)                        {xmlListPushBack(tree_changes, xmlCreateTreeChangeRemoveNode(node));} */
static void* xmlCreateTreeChangeRemoveNodeWalker(const void *data, const void *user1 ATTRIBUTE_UNUSED, const void *user2 ATTRIBUTE_UNUSED) {
  return xmlCreateTreeChangeRemoveNode((xmlNodePtr) data);
}
static void xmlPushTreeChangeRemoveNodes(xmlListPtr tree_changes, xmlListPtr node_removals)                         {
  if (tree_changes != NULL) 
    xmlListWalkTo(tree_changes, node_removals, xmlCreateTreeChangeRemoveNodeWalker, NULL, NULL);
}
static void xmlPushTreeChangeCopyChild(  xmlListPtr tree_changes, xmlNodePtr node, xmlNodePtr related_node, int position) {if (tree_changes != NULL) xmlListPushBack(tree_changes, xmlCreateTreeChangeCopyChild(node, related_node, position));}
static void xmlPushTreeChangeChangeName( xmlListPtr tree_changes, xmlNodePtr node, xmlNodePtr related_node)               {if (tree_changes != NULL) xmlListPushBack(tree_changes, xmlCreateTreeChangeChangeName(node, related_node));}
static void xmlPushTreeChangeSetValue(   xmlListPtr tree_changes, xmlNodePtr node, xmlNodePtr related_node)               {if (tree_changes != NULL) xmlListPushBack(tree_changes, xmlCreateTreeChangeSetValue(node, related_node));}
static void xmlPushTreeChangeMoveChild(  xmlListPtr tree_changes, xmlNodePtr node, xmlNodePtr related_node, int position) {if (tree_changes != NULL) xmlListPushBack(tree_changes, xmlCreateTreeChangeMoveChild(node, related_node, position));}
static void xmlMergeTreeChanges(         xmlListPtr tree_changes, xmlListPtr new_changes)                           {
  if (tree_changes != NULL) 
    xmlListMerge(tree_changes, new_changes);
}

/**
 * xmlSimilarity_recursive:
 * xmlOrderedPotentialMatches:
 * xmlPopBestMatch:
 * @existing_node:       the existing_node target hierarchy
 * @new_node:            the new_node hierarchy
 *
 * @author: Annesley
 * 
 * Compares:
 * local-name, namespace
 * properties
 * sub-hierarchy
 * 
 * Returns: the similarity 0-100 of the element and its sub-hierarchy
 * and node changes details
 */
static unsigned int xmlSimilarity_recursive(xmlNodePtr existing_node, xmlNodePtr new_node, xmlListPtr tree_changes, unsigned int *count, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);

static void xmlMergeMatchDeallocator(void *data) {
  xmlMergeMatchPtr merge_match = data;
  if (merge_match == NULL) xmlGenericError(xmlGenericErrorContext, "xmlMergeMatchDeallocator: NULL");
  if (merge_match->tree_changes) xmlListDelete(merge_match->tree_changes);
  xmlFree(merge_match);
}

static xmlListPtr 
xmlMatchPool(xmlNodePtr existing_node, ATTRIBUTE_UNUSED xmlNodePtr new_node, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  xmlListPtr match_pool;
  
  if (existing_node == NULL) return NULL;
    
  /* Strategy 1: every node on the same level 
   * includes XML_ATTRIBUTE_NODE, XML_TEXT_NODE etc.
   * 
   * unsorted because the temporary sorting must be in the context of the current new_node
   * see xmlPopBestMatch() which sorts stuff
   */
  match_pool = xmlListCreate(NULL, NULL);
  do {
    if (xmlReadableNode(xfilter, xtrigger, existing_node, NULL) != NULL)
      xmlListPushBack(match_pool, (void*) existing_node);
  } while ((existing_node = existing_node->next));
    
  return match_pool;
}

xmlListPtr 
xmlCreateTreeChangesList() {
  return xmlListCreate(xmlTreeChangeDeallocator, NULL);
}

static void* xmlListPopulateSimilarity(const void *data, const void *user1, const void *user2, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  xmlNodePtr match_pool_node = (xmlNodePtr) data;
  xmlNodePtr new_node        = (xmlNodePtr) user1;
  xmlNodePtr existing_node   = (xmlNodePtr) user2;
  
  unsigned int subcount = 0;
  unsigned int similairity100;
  xmlListPtr tree_changes;
  xmlMergeMatchPtr merge_match = NULL;

  tree_changes   = xmlListCreate(xmlTreeChangeDeallocator, NULL);
  similairity100 = xmlSimilarity_recursive(match_pool_node, new_node, tree_changes, &subcount, xtrigger, xfilter);
  
  if (similairity100) { /* only push nodes that have any similarity */
    merge_match = xmlMalloc(sizeof(struct _xmlMergeMatch));
    merge_match->count            = subcount;
    merge_match->similarity100    = similairity100;
    merge_match->node             = match_pool_node;
    merge_match->is_existing_node = (existing_node == match_pool_node);
    merge_match->tree_changes     = tree_changes;
  }
  
  return merge_match;
}

static int xmlListMergeMatchSort(const void *data1, const void *data2) {
  xmlMergeMatchConstPtr merge_match1 = data1; 
  xmlMergeMatchConstPtr merge_match2 = data2;
  int sort = 0;
  
  if      (merge_match1->similarity100 > merge_match2->similarity100) sort = 1;
  else if (merge_match2->similarity100 > merge_match1->similarity100) sort = -1;
  else if (merge_match1->is_existing_node) sort = 1;
  else if (merge_match2->is_existing_node) sort = -1;
  return sort;
}

static xmlMergeMatchPtr
xmlPopBestMatchFor(const xmlListPtr match_pool, xmlNodePtr existing_node, xmlNodePtr new_node, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  /* caller frees result */
  xmlListPtr similarity_list;
  xmlMergeMatchPtr best_merge_match;
  
  /* first highest match
   * between the level persistent match_pool and the current new_node on that level
   * we need to traverse the entire list:             match_pool
   * creating a new list according to the similarity: similarity_list
   */
  similarity_list = xmlListCreate(xmlMergeMatchDeallocator, xmlListMergeMatchSort);
  xmlListWalkToSecure(similarity_list, match_pool, xmlListPopulateSimilarity, new_node, existing_node, xtrigger, xfilter);
  /* xmlDebugListMergeMatchOutput(similarity_list); */
  best_merge_match = xmlListPopBack(similarity_list);
  if (best_merge_match) xmlListRemoveAll(match_pool, best_merge_match->node); /* no deallocation */
  
  /* free up */
  xmlListDelete(similarity_list);             /* xmlMergeMatchDeallocator */

  return best_merge_match;
}

static unsigned int 
xmlSimilarity_recursive(xmlNodePtr existing_node, xmlNodePtr new_node, xmlListPtr tree_changes, unsigned int *count, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  xmlAttrPtr att_new_node, att_existing_node;
  xmlNodePtr child_new_node, child_existing_node;
  unsigned int similairity100, property_similairity100, property_count, position, ordering_penalty;
  xmlListPtr match_pool = NULL;
  xmlMergeMatchPtr best_merge_match;
  
  if ((new_node == NULL) && (existing_node == NULL)) return 0; /* count not increased: full match */
  (*count)++;
  if ((new_node == NULL) || (existing_node == NULL)) return 0; /* count increased: zero match */
  if (new_node->type != existing_node->type)         return 0;
    
  /* xmlStrEqual() returns 0 if either input is NULL */
  similairity100 = 0;
  switch (new_node->type) {
    case XML_TEXT_NODE: {
      /* 100: content */
      if (xmlStrEqual(new_node->content, existing_node->content)) similairity100 += 100;
      else xmlPushTreeChangeSetValue(tree_changes, existing_node, new_node);

      XML_TRACE_GENERIC4(XML_DEBUG_TREE, "xmlSimilarity: XML_TEXT_NODE [%u/%u] [%s] == [%s]", 
        similairity100, *count,
        (existing_node->content ? existing_node->content : BAD_CAST ""),
        (new_node->content      ? new_node->content      : BAD_CAST "")
      );
      break;
    }
    case XML_ATTRIBUTE_NODE: {
      /* 60: local-name()
       * 40: content */
      if ((existing_node->ns == new_node->ns) 
        || ((existing_node->ns != NULL) && (new_node->ns != NULL) && (xmlStrEqual(existing_node->ns->href, new_node->ns->href) == 1))
      ) {
        if (xmlStrEqual(existing_node->name, new_node->name) == 1) similairity100 += 60;
        else xmlPushTreeChangeChangeName(tree_changes, existing_node, new_node);
        
        if ((existing_node->children == new_node->children) || (
          (existing_node->children != NULL) && (new_node->children != NULL) && (xmlStrEqual(new_node->children->content, existing_node->children->content) == 1)
        )) similairity100 += 40;
        else xmlPushTreeChangeSetValue(tree_changes, existing_node, new_node);
      }

      XML_TRACE_GENERIC4(XML_DEBUG_TREE, "xmlSimilarity : XML_ATTRIBUTE_NODE [%u/%u] [%s] == [%s]",
        similairity100, *count,
        (existing_node->name ? existing_node->name : BAD_CAST ""),
        (new_node->name      ? new_node->name      : BAD_CAST "")
      );
      break;
    }
    case XML_ELEMENT_NODE: {
      /* 60: local-name()
       * 40: @properties */
      if ((existing_node->ns == new_node->ns) 
        || ((existing_node->ns != NULL) && (new_node->ns != NULL) && (xmlStrEqual(existing_node->ns->href, new_node->ns->href) == 1))
      ) {
        if (xmlStrEqual(existing_node->name, new_node->name) == 1) similairity100 += 60;
        else xmlPushTreeChangeChangeName(tree_changes, existing_node, new_node);
        
        /* ------------------------property similarity 
          * we set the property weighting manually
          * overall count is not increased here
          * match_pool looses each node as it matches, ensuring that the same node is not used twice
          */
        property_similairity100 = 0;
        property_count          = 0;
        position                = 0;
        att_new_node        = new_node->properties;
        att_existing_node   = existing_node->properties;
        match_pool = xmlMatchPool((xmlNodePtr) att_existing_node, (xmlNodePtr) att_new_node, xtrigger, xfilter);
        while (att_new_node) {
          /* security on attributes because we will be skipping the repository and xmlsecurity attributes here */
          if (xmlReadableNode(xfilter, xtrigger, (xmlNodePtr) att_new_node, NULL) != NULL) {
            while ((att_existing_node != NULL) && (xmlReadableNode(xfilter, xtrigger, (xmlNodePtr) att_existing_node, NULL) == NULL)) att_existing_node = att_existing_node->next;
            best_merge_match = xmlPopBestMatchFor(match_pool, (xmlNodePtr) att_existing_node, (xmlNodePtr) att_new_node, xtrigger, xfilter);
            if (best_merge_match == NULL) {
              xmlPushTreeChangeCopyChild(tree_changes, (xmlNodePtr) att_new_node, existing_node, position);
              property_count++;
            } else {
              /* we accept this match, so include the tree_changes and the similarity measure */
              xmlMergeTreeChanges(tree_changes, best_merge_match->tree_changes);
              /* ORDERING: traverse the existing_node only when it is matched 
              * reduce the returned score by 10 if it will require a MOVE
              */
              if (best_merge_match->is_existing_node == 1) {
                ordering_penalty  = 0;
                att_existing_node = att_existing_node->next;
              } else {
                ordering_penalty  = 10;
                xmlPushTreeChangeMoveChild(tree_changes, best_merge_match->node, (xmlNode*) att_existing_node, position);
              }
              property_similairity100 += best_merge_match->similarity100 - ordering_penalty;
              property_count          += best_merge_match->count;  /*- always 1 in this case */
              xmlMergeMatchDeallocator(best_merge_match); /* popped */
            }
          }
          position++;
          att_new_node = att_new_node->next;
        }
        xmlPushTreeChangeRemoveNodes(tree_changes, match_pool);
        if (match_pool) xmlListDelete(match_pool);
        if (property_count == 0) similairity100 += 40;
        else                     similairity100 += (property_similairity100 * 40) / (property_count * 100);
        
        /* ------------------------ children similarity 
          * full weight for children
          * count is increased accordingly
          * match_pool looses each node as it matches, ensuring that the same node is not used twice
          */
        child_new_node      = new_node->children;
        child_existing_node = existing_node->children;
        position            = 0;
        match_pool   = xmlMatchPool(child_existing_node, child_new_node, xtrigger, xfilter);
        while (child_new_node) {
          if (xmlReadableNode(xfilter, xtrigger, child_new_node, NULL) != NULL) {
            while ((child_existing_node != NULL) && (xmlReadableNode(xfilter, xtrigger, child_existing_node, NULL) == NULL)) child_existing_node = child_existing_node->next;
            best_merge_match = xmlPopBestMatchFor(match_pool, child_existing_node, child_new_node, xtrigger, xfilter);
            if (best_merge_match == NULL) {
              xmlPushTreeChangeCopyChild(tree_changes, child_new_node, existing_node, position);
              (*count)++;
            } else {
              /* we accept this match, so include the tree_changes and the similarity measure */
              xmlMergeTreeChanges(tree_changes, best_merge_match->tree_changes);
              /* ORDERING: traverse the existing_node only when it is matched 
              * reduce the returned score by 10 if it will require a MOVE
              */
              if (best_merge_match->is_existing_node == 1) {
                ordering_penalty    = 0;
                child_existing_node = child_existing_node->next;
              } else {
                /* match is not the current existing node so it will have to be moved */
                ordering_penalty    = 10;
                xmlPushTreeChangeMoveChild(tree_changes, best_merge_match->node, child_existing_node, position);
              }
              similairity100 += best_merge_match->similarity100 - ordering_penalty;
              (*count) += best_merge_match->count;
              xmlMergeMatchDeallocator(best_merge_match); /* popped */
            }
            position++;
          }
          child_new_node = child_new_node->next;
        }
        xmlPushTreeChangeRemoveNodes(tree_changes, match_pool);
        if (match_pool) xmlListDelete(match_pool);
        
        XML_TRACE_GENERIC4(XML_DEBUG_TREE, "xmlSimilarity : XML_ELEMENT_NODE [%u/%u] [%s] == [%s]",
          similairity100, *count,
          (existing_node->name ? existing_node->name : BAD_CAST ""),
          (new_node->name      ? new_node->name      : BAD_CAST "")
        );
      }
      break;
    }
    default: {
      /* TODO: XML_DTD_NODE, XML_PI_NODE, ... */
      xmlGenericError(xmlGenericErrorContext, "xmlSimilarity : Node type [%i] not supported yet", new_node->type);
    }
  }     
  
  xmlCheckTreeChangeList(tree_changes);
  
  return similairity100;
}
  
/**
 * xmlSimilarity:
 * @existing_node:       the existing_node target hierarchy
 * @new_node:            the new_node hierarchy
 * @tree_changes:         a passed in list that recieves all the nodes that did not match completely
 *
 * @author: Annesley
 * 
 * Compares:
 * local-name, namespace
 * properties
 * sub-hierarchy
 * 
 * Returns: the similarity 0-100 of the element and its sub-hierarchy
 */
unsigned int 
xmlSimilarity(xmlNodePtr existing_node, xmlNodePtr new_node, xmlListPtr tree_changes, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  unsigned int similairity100;
  unsigned int count = 0;
  
  if (existing_node == NULL) return 0;
  if (new_node == NULL)      return 0;
  
  similairity100 = xmlSimilarity_recursive(existing_node, new_node, tree_changes, &count, xtrigger, xfilter);
  
  return (similairity100 / count);
}

/**
 * xmlThrowNativeError:
 *
 * @author: Annesley
 */
void
xmlThrowNativeError(const xmlChar *sParameter1, const xmlChar *sParameter2, const xmlChar *sParameter3) {
  xmlGenericError(xmlGenericErrorContext, "xmlThrowNativeError: [%s %s %s]", sParameter1, sParameter2, sParameter3); 
}

/**
 * xmlMergeHierarchy:
 * @existing_node:       the existing_node target hierarchy
 * @new_node:            the new_node hierarchy
 * @xtrigger:       xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:        xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 * @leave_existing_node: do not delete nodes in @existing_node that are not found in @new_node
 * @forcesynctop:   always sync the top node rather than replacing it
 *
 * @author: Annesley
 * 
 * @new_node => @existing_node
 * Ensures that all properties, nodes and text from @new_node are set / copied in to the existing_node
 * normally nodes in @existing_node that are not in @new_node are deleted, unless @leave_existing_node
 * 
 * Returns: the existing_node target node if it was sufficiently similar to be synced or @forcesynctop
 * or a new_node node if it was replaced
 */
xmlNodePtr
xmlMergeHierarchy(xmlNodePtr existing_node ATTRIBUTE_UNUSED, xmlNodePtr new_node ATTRIBUTE_UNUSED, const xmlNodeTriggerCallbackContextPtr xtrigger ATTRIBUTE_UNUSED, const xmlNodeFilterCallbackContextPtr xfilter ATTRIBUTE_UNUSED, int leave_existing ATTRIBUTE_UNUSED) {
  LIBXML_NOT_COMPLETE("xmlMergeHierarchy: use high-level mergeNode with TXml instead");
  return 0;
}

/**
 * xmlSoftlinkChild:
 * @additional_parent: where to place the reference (must have no children)
 * @cur:               node to reference
 * @parent_route:      path to cur that will be adopted upon vertical traversal
 * @xtrigger:          xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:           xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 * 
 * Node referencing is like hardlinking
 * but has an explicit hardlink path
 * that is adopted on xpath ->parent, ->children and ->properties moves only
 * not ->next and ->previous
 * e.g. once traversed down on to the reference node, the new parent_route is adopted
 * and parent moves will not return to the original @additional_parent
 * 
 * Returns: 0 on success
 */
xmlNodePtr
xmlSoftlinkChild(xmlNodePtr additional_parent, xmlListPtr parent_route, xmlNodePtr target, xmlNodePtr before, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  xmlNodePtr softlink;
  
  if (additional_parent == NULL) return NULL;
  if (target            == NULL) return NULL;

  if (xmlIsHardLink(target) == 1) {
      printf("xmlSoftlinkChild : Progressive soft(hard)linking (hardlinks to hardlinks) not supported yet.");
      printf("  Temporarily softlinking the original instead. Intended deviations will not work.");
      target = xmlFirstHardLink(target);
  }
  
  softlink = xmlHardlinkChild(additional_parent, parent_route, target, before, xtrigger, xfilter);
  if (softlink != NULL) xmlSetVerticalParentRouteAdoption(softlink, target, parent_route, xtrigger, xfilter, 1);
  
  return softlink;
}

/**
 * xmlSetVerticalParentRouteAdoption:
 * xmlClearVerticalParentRouteAdoption:
 * @cur:      where to place the vertical_parent_route_adopt
 * @vertical_parent_route_adopt: the parent_ruote to adopt on vertical manouvers
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 * @no_security: this function is used by others and can prevent additional security checks
 *
 * @author: Annesley
 * 
 * Seding NULL for vertical_parent_route_adopt will clear the vertical_parent_route_adopt
 * 
 * Returns: 0 on success
 */
int
xmlClearVerticalParentRouteAdoption(xmlNodePtr cur, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter, int no_security) {
  if (cur == NULL) return 1;
  
  /* Annesley: triggers, security filtering */
  /* includes xmlTriggerCall(xtrigger, additional_parent, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter) */
  if (no_security != 1)
    if (xmlWriteableNode(xfilter, xtrigger, cur, NULL) == NULL) return (1);

  /* clear the vertical_parent_route_adopt */
  if ((xmlIsHardLinked(cur) == 1) && (cur->hardlink_info->vertical_parent_route_adopt != NULL)) {
    xmlListDelete(cur->hardlink_info->vertical_parent_route_adopt);
    cur->hardlink_info->vertical_parent_route_adopt = NULL;
  }
  
  return 0;
}

int
xmlSetVerticalParentRouteAdoption(xmlNodePtr softlink, xmlNodePtr target, xmlListPtr vertical_parent_route_adopt, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter, int no_security) {
  if (softlink == NULL) return 1;
  if (target   == NULL) return 2;
  if (softlink->type != XML_ELEMENT_NODE) return 3;
  if (target->type   != XML_ELEMENT_NODE) return 4;
  
  /* Annesley: triggers, security filtering */
  /* includes xmlTriggerCall(xtrigger, additional_parent, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter) */
  if (no_security != 1)
    if (xmlWriteableNode(xfilter, xtrigger, softlink, NULL) == NULL) return (1);

  /* adopt the vertical_parent_route_adopt */
  if (xmlIsHardLinked(softlink) == 0) xmlCreateHardlinkInfo(softlink);
  if (vertical_parent_route_adopt == NULL) {
    softlink->hardlink_info->vertical_parent_route_adopt = xmlListCreate(NULL, NULL);
  } else {
    softlink->hardlink_info->vertical_parent_route_adopt = xmlListDup(vertical_parent_route_adopt);
  }
  /* we need to store the actual hardlink 
   * it will be popped when setting the route
   */
  xmlListPushBack(softlink->hardlink_info->vertical_parent_route_adopt, target);
  
  return (0);
}

/**
 * xmlNewSequencedElement:
 * @parent:   where to place the reference (must have no children)
 * @xtrigger: xmlNodeTriggerCallbackContext strategy pattern trigger object passed in
 * @xfilter:  xmlNodeFilterCallbackContext  strategy pattern security for filtering out or denying access to nodes
 *
 * @author: Annesley
 * 
 * element <prefix>1<suffix>,<prefix>2<suffix>,<prefix>3<suffix>, ...
 * TODO: REENTRANT
 * 
 * Returns: new element on success
 */
xmlNodePtr
xmlNewSequencedElement(xmlNodePtr parent, xmlChar *prefix, xmlChar *suffix, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
  xmlNodePtr index_node;
  int next_index, this_index;
  xmlChar *next_index_string = 0;
  
  if (parent == NULL) return NULL;
  
  /* Annesley: triggers, security filtering */
  /* includes xmlTriggerCall(xtrigger, parent, TRIGGER_OP_WRITE, TRIGGER_STAGE_BEFORE, xfilter) */
  if (xmlWriteableNode(xfilter, xtrigger, parent, NULL) == NULL) return NULL;
  
  /* find next index */
  next_index = 0;
  index_node = parent->children;
  while (index_node != NULL) {
    if (index_node->type == XML_ELEMENT_NODE) {
      this_index = atoi((const char *) index_node->name);
      if (this_index > next_index) next_index = this_index;
    }
    index_node = index_node->next;
  }
  
  /* create new node */
  next_index_string = xmlStritoa(next_index + 1, prefix, suffix);
  index_node        = xmlNewDocNode(parent->doc, parent->ns, next_index_string, NULL);
  index_node        = xmlAddChild(parent, index_node);
  
  //free up
  if (next_index_string != NULL) xmlFree(next_index_string);
  
  return index_node;
}
