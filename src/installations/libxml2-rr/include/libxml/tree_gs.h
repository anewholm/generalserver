#ifndef TREE_GS
#define TREE_GS

#define LIBXML_NOT_CURRENTLY_USED(what) xmlGenericError(xmlGenericErrorContext, "NOT_CURRENTLY_USED: %s", what);
#define LIBXML_NOT_COMPLETE(what)       xmlGenericError(xmlGenericErrorContext, "NOT_COMPLETE: %s",       what);
#define LIBXML_NEEDS_TEST(what)         xmlGenericError(xmlGenericErrorContext, "NEEDS_TEST: %s",         what);

#ifdef LIBXML_LOCKING_ENABLED
#define MARKED_FOR_DESTRUCTION(cur) cur->marked_for_destruction
#define LOCKS(cur) cur->locks
#else
#define MARKED_FOR_DESTRUCTION(cur) 0
#define LOCKS(cur) 0
#endif

/**
 * xmlDeviation:
 *
 * Deviations on a node are valid for a given hardlink ancestor
 * and are additionally stored against the ancestor hardlink in question
 *
 * Deviation entry for a node
 * deviation info is a single linked list for scanning only
 * type is
 *   0 DEVIATION_REPLACEMENT:      replacing this child or property
 *   1 DEVIATION_ADDITIONAL_CHILD: additional  child or property on end
 *   2 DEVIATION_REMOVAL:          skip over the node or property
 */
typedef struct _xmlDeviation xmlDeviation;
typedef xmlDeviation *xmlDeviationPtr;
struct _xmlDeviation {
    struct _xmlNode *original;  /* the node being changed */
    struct _xmlNode *hardlink;  /* for this hardlink only */
    struct _xmlNode *deviant;   /* the new replacement node */
    unsigned short  type;       /* how it replaces: 0,1,2 */
    struct _xmlDeviation *next; /* the next deviation in the list, if any */
};

/**
 * xmlHardlinkInfo:
 *
 * Hardlinked info for a node
 * hardlinks are double linked lists, rather than pointers because that is how LibXML2 does things
 * this info is lazy because of sparse hardlinks
 *
 * if both next and prev are NULL then this node is NOT hardlinked currently
 */
typedef struct _xmlHardlinkInfo xmlHardlinkInfo;
typedef xmlHardlinkInfo *xmlHardlinkInfoPtr;
struct _xmlHardlinkInfo {
    struct _xmlNode *next;                     /* the next hardlink in the chain (if any) */
    struct _xmlNode *prev;                     /* the previous hardlink in the chain (if any) */
    struct _xmlDeviation *descendant_deviants; /* deviants in the hardlink sub-tree */
    struct _xmlDeviation *deviants;            /* deviants of this node */
    struct _xmlDeviation *originals;           /* if this is a deviant, then originals point back to the original */
    xmlListPtr vertical_parent_route_adopt;    /* traversals explicitly load this parent_route */ 
};

/**
 * Annesley: in GS all docs require dictionaries
 * TODO: xmlIsSharedDoc(...) will have 0x0 dict as it does not own the nodes...
 */
#if 0 /* LIBXML_DEBUG_ENABLED */
#define DICT_REQUIRED(str)                                                \
        if (dict == NULL) \
          __xmlRaiseError(NULL, NULL, NULL, \
            NULL, (xmlNodePtr) cur, XML_FROM_CHECK, \
            XML_NO_DICT, XML_ERR_FATAL, NULL, 0, \
            (const char*) cur->name, NULL, NULL, 0, 0, \
            "DICT_REQUIRED(str): no dictionary in doc [%s]",  \
            (cur && cur->doc && cur->doc->name ? cur->doc->name : "<no name>") \
          ); \
        if ((str != NULL) && (xmlDictOwns(dict, (const xmlChar *)(str)) == 0)) \
          __xmlRaiseError(NULL, NULL, NULL, \
            NULL, (xmlNodePtr) cur, XML_FROM_CHECK, \
            XML_NO_DICT, XML_ERR_FATAL, NULL, 0, \
            (const char*) cur->name, NULL, NULL, 0, 0, \
            "DICT_REQUIRED(str): doc dictionary [%s] does not own string [%s]",  \
            (cur && cur->doc && cur->doc->name ? cur->doc->name : "<no name>"), \
            str \
          );
#else
#define DICT_REQUIRED(str)
#endif
        
typedef enum {
  XML_SECURITY_WRITE_ACCESS_DENIED = 100000,
  XML_SECURITY_ADD_ACCESS_DENIED,
  XML_LOCKING_LOCKED_NODE_CANNOT_BE_FREED,
  XML_LOCKING_NODE_HAS_NO_LOCKS,
  
  XML_NO_DICT,
  
  XML_NAMESPACE_WRONG_DOC,
  XML_NAMESPACE_DEFAULT_NOT_FOUND,
  XML_NAMESPACE_PREFIXED_NOT_FOUND,
  XML_NAMESPACE_REQUIRED  
} xmlParserErrorsExtended;

enum xmlTreeChangeType {
  LibXml_removeNode,
  LibXml_copyChild,
  LibXml_changeName,
  LibXml_setValue,
  LibXml_moveChild
};

struct _xmlTreeChange {
  enum xmlTreeChangeType type; /* removeNode,   copyChild,          changeName,   setValue,     moveChild          */
  xmlNodePtr node;         /* exsitingNode, newNode,            exsitingNode, exsitingNode, exsitingNode       */
  xmlNodePtr related_node; /* NULL,         beforeExistingNode, newSyncNode,  newSyncNode,  beforeExistingNode */
  int position;
};
typedef struct _xmlTreeChange* xmlTreeChangePtr;
typedef struct _xmlTreeChange const* xmlTreeChangeConstPtr;

struct _xmlMergeMatch {
  xmlNodePtr node;
  unsigned int count;
  unsigned int similarity100;
  unsigned int is_existing_node;
  xmlListPtr tree_changes;
};
typedef struct _xmlMergeMatch* xmlMergeMatchPtr;
typedef struct _xmlMergeMatch const* xmlMergeMatchConstPtr;

XMLPUBFUN xmlNodePtr XMLCALL
          xmlAddChildSecure    (xmlNodePtr parent, xmlListPtr parent_route, xmlNodePtr cur,
                                const xmlNodeTriggerCallbackContextPtr xtrigger,
                                const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlNodePtr XMLCALL
          xmlCreateHardlinkPlaceholder(const xmlNodePtr hardlink, xmlDocPtr destDoc, 
                                xmlNodePtr output,
                                const xmlNodeTriggerCallbackContextPtr xtrigger,
                                const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN int XMLCALL
          xmlCreateDeviationPlaceholders(const xmlNodePtr hardlink, xmlNodePtr output,
                                const int iIncludeHardlink);
XMLPUBFUN int XMLCALL
          xmlXMLIDPolicy       (xmlNodePtr node);
XMLPUBFUN xmlNodePtr XMLCALL
          xmlHardLinkRouteToAncestor  (xmlNodePtr cur, xmlNodePtr ancestor);
XMLPUBFUN int XMLCALL
          xmlIsSoftLink        (xmlNodePtr cur);
XMLPUBFUN int XMLCALL
          xmlIsHardLink        (xmlNodePtr cur);
XMLPUBFUN int XMLCALL
          xmlIsHardLinked      (xmlNodePtr cur);
XMLPUBFUN int XMLCALL
          xmlIsParentOf        (xmlNodePtr parent, xmlNodePtr cur);
XMLPUBFUN int XMLCALL
          xmlIsOriginalHardLink        (xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlHardlinkChild    (xmlNodePtr parent,
                         xmlListPtr parent_route, 
                         xmlNodePtr cur,
                         xmlNodePtr before,
                         const xmlNodeTriggerCallbackContextPtr xtrigger,
                         const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlLastHardLink       (xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlHardLinkOriginal   (xmlNodePtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlFirstHardLink      (xmlNodePtr cur);
XMLPUBFUN int XMLCALL
    xmlAreDistinctHardLinks(const xmlNodePtr cur1, 
                          const xmlNodePtr cur2);
XMLPUBFUN int XMLCALL
    xmlAreEqualOrHardLinked(xmlNodePtr cur1,
                          xmlNodePtr cur2);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlNewSequencedElement(xmlNodePtr parent, 
                          xmlChar *prefix, 
                          xmlChar *suffix, 
                          const xmlNodeTriggerCallbackContextPtr xtrigger, 
                          const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN int XMLCALL
    xmlSetVerticalParentRouteAdoption(xmlNodePtr cur, 
                          xmlNodePtr target,
                          xmlListPtr vertical_parent_route_adopt, 
                          const xmlNodeTriggerCallbackContextPtr xtrigger, 
                          const xmlNodeFilterCallbackContextPtr xfilter, 
                          int no_security);
XMLPUBFUN int XMLCALL
    xmlClearVerticalParentRouteAdoption(xmlNodePtr cur, 
                          const xmlNodeTriggerCallbackContextPtr xtrigger, 
                          const xmlNodeFilterCallbackContextPtr xfilter, 
                          int no_security);
XMLPUBFUN xmlNodePtr XMLCALL
    xmlSoftlinkChild     (xmlNodePtr additional_parent, 
                          xmlListPtr parent_route,
                          xmlNodePtr cur, 
                          xmlNodePtr before, 
                          const xmlNodeTriggerCallbackContextPtr xtrigger, 
                          const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN int XMLCALL
    xmlDeviateNode       (xmlNodePtr node,
                          xmlNodePtr hardlink,
                          xmlNodePtr deviant,
                          unsigned short type);
XMLPUBFUN void XMLCALL
          xmlTouchNodeSecure(    xmlNodePtr cur, xmlListPtr parent_route, 
                                 const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN void XMLCALL
          xmlUnlinkNodeSecure    (xmlNodePtr cur, xmlListPtr parent_route, 
                                 const xmlNodeTriggerCallbackContextPtr xtrigger,
                                 const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN void XMLCALL
          xmlUnlinkHardLink      (xmlNodePtr cur);
XMLPUBFUN void XMLCALL
          xmlFreeHardlinkInfoIfAlone(xmlNodePtr cur);
XMLPUBFUN xmlAttrPtr XMLCALL
          xmlSetPropSecure(xmlNodePtr node, xmlListPtr parent_route, const xmlChar *name, const xmlChar *value,
                          const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlAttrPtr XMLCALL
          xmlSetNsPropSecure(xmlNodePtr node, xmlListPtr parent_route, xmlNsPtr ns, const xmlChar *name, const xmlChar *value,
                          const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN void XMLCALL
          xmlNodeSetContentSecure(xmlNodePtr cur, xmlListPtr parent_route, const xmlChar *content,
                          const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);
/*
 * Concurrency
 * Annesley: C++ layer can lock nodes to ensure they do not get removed during anaylsis
 * returns the number of locks remaining
 */
#ifdef LIBXML_LOCKING_ENABLED
XMLPUBFUN xmlNodePtr XMLCALL
          xmlLockNode            (xmlNodePtr cur, const void *lock_external, const xmlChar *lock_reason);
XMLPUBFUN int XMLCALL
          xmlUnLockNode          (xmlNodePtr cur, const void *lock_external);
XMLPUBFUN xmlNodePtr XMLCALL
          xmlLockFreezeNode      (xmlNodePtr cur);
#endif

/* NOTE: ANnesley: security implementation */
XMLPUBFUN int XMLCALL
          xmlNodeDumpSecure          (xmlBufferPtr buf,
                          xmlDocPtr doc,
                          xmlNodePtr cur,
                          int level,
                          int format,
                          const xmlNodeTriggerCallbackContextPtr xtrigger,
                          const xmlNodeFilterCallbackContextPtr sec,
                          int maxdepth,
                          int options);
XMLPUBFUN void XMLCALL
          xmlNodeDumpOutputSecure     (xmlOutputBufferPtr buf,
                          xmlDocPtr doc,
                          xmlNodePtr cur,
                          int level,
                          int format,
                          const char *encoding,
                          const xmlNodeTriggerCallbackContextPtr xtrigger,
                          const xmlNodeFilterCallbackContextPtr sec,
                          int maxdepth,
                          int options);

XMLPUBFUN const xmlChar* XMLCALL
          xmlNamespacePrefix (xmlNodePtr cur, const int require_namespaces, const int reconcile_default);
XMLPUBFUN int XMLCALL
          xmlIsSharedDoc     (xmlDocPtr cur);
XMLPUBFUN xmlNodePtr XMLCALL
          xmlAddPrevSiblingSecure(xmlNodePtr cur, xmlListPtr parent_route, xmlNodePtr elem,
                          const xmlNodeTriggerCallbackContextPtr xtrigger,
                          const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlNsPtr XMLCALL
          xmlSearchNsWithPrefixByHref(xmlDocPtr doc,
                          xmlNodePtr node,
                          const xmlChar *href);
XMLPUBFUN xmlListPtr XMLCALL
          xmlCreateTreeChangesList       (void);
XMLPUBFUN unsigned int XMLCALL
          xmlSimilarity        (xmlNodePtr existing_node, 
                                xmlNodePtr new_node, 
                                xmlListPtr tree_changes,
                                const xmlNodeTriggerCallbackContextPtr xtrigger, 
                                const xmlNodeFilterCallbackContextPtr xfilter);
XMLPUBFUN xmlNodePtr XMLCALL
          xmlMergeHierarchy    (xmlNodePtr existing_node, 
                                xmlNodePtr new_node, 
                                const xmlNodeTriggerCallbackContextPtr xtrigger, 
                                const xmlNodeFilterCallbackContextPtr xfilter, 
                                int leave_existing);
XMLPUBFUN void XMLCALL
          xmlThrowNativeError   (const xmlChar *sParameter1, 
                                 const xmlChar *sParameter2, 
                                 const xmlChar *sParameter3);
#endif
