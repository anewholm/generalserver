#define LAST_LISTITEM -1

typedef void* (*xmlListWalkerSecure) (const void *data, const void *user1, const void *user2, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter);

XMLPUBFUN xmlListPtr XMLCALL
                xmlListWalkToSecure     (xmlListPtr lFrom, 
                                         xmlListPtr lTo, 
                                         xmlListWalkerSecure walker, 
                                         const void *user1, 
                                         const void *user2, 
                                         const xmlNodeTriggerCallbackContextPtr xtrigger, 
                                         const xmlNodeFilterCallbackContextPtr xfilter);

