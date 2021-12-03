/**
 * xmlListWalkTo:
 * @lFrom:  input list
 * @lTo:    output list
 * @walker: a processing function
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
xmlListPtr
xmlListWalkTo(xmlListPtr lTo, xmlListPtr lFrom, xmlListWalker walker, const void *user1, const void *user2) {
    xmlLinkPtr lk;
    void *data = NULL;

    if (lTo == NULL)    lTo = xmlListCreate(NULL, NULL);
    if (lFrom == NULL)  return lTo;
    
    if (walker == NULL) xmlListMerge(lTo, lFrom);
    else {
      for(lk = lFrom->sentinel->next; (lk != lFrom->sentinel) && (data == NULL); lk = lk->next) {
          data = walker(lk->data, user1, user2);
          if (data != NULL) {
            if (lTo->unordered == 1) xmlListPushBack(lTo, data);
            else                     xmlListInsert(  lTo, data);
          }
      }
    }
    
    return lTo;
}

/**
 * xmlListWalkToSecure:
 * @lFrom:  input list
 * @lTo:    output list
 * @walker: a processing function
 * @user1:  a user parameter passed to the walker function
 * @user2:  a user parameter passed to the walker function
 * @xtrigger: trigger strategy
 * @xfilter:  security strategy
 *
 * Walk all the element of the first from first to last and
 * apply the walker function to it
 * 
 * xmlListWalker() returns:
 *   NULL    - continue walking
 *   1       - terminate walking and return NULL (error)
 *   poniter - terminate walking, update the _xmlLink value and return the pointer
 * 
 * Returns the xmlListWalkerSecure() returned pointer or NULL
 */
xmlListPtr
xmlListWalkToSecure(xmlListPtr lTo, xmlListPtr lFrom, xmlListWalkerSecure walker, const void *user1, const void *user2, const xmlNodeTriggerCallbackContextPtr xtrigger, const xmlNodeFilterCallbackContextPtr xfilter) {
    xmlLinkPtr lk;
    void *data = NULL;

    if (lTo == NULL)    lTo = xmlListCreate(NULL, NULL);
    if (lFrom == NULL)  return lTo;
    
    if (walker == NULL) xmlListMerge(lTo, lFrom);
    else {
      for(lk = lFrom->sentinel->next; (lk != lFrom->sentinel) && (data == NULL); lk = lk->next) {
          data = walker(lk->data, user1, user2, xtrigger, xfilter);
          if (data != NULL) {
            if (lTo->unordered == 1) xmlListPushBack(lTo, data);
            else                     xmlListInsert(  lTo, data);
          }
      }
    }
    
    return lTo;
}