/**
 * xmlTranslateSubStructure:
 * xmlTranslateDocStructure:
 * xmlTranslateStructure: onlylegacy name for function
 * @node: the starting node sub-tree 
 * @hardlink_placeholders: OPTIONAL set of existing pre-discovered xml:hardlinks
 *   or example discovered during the stream parsing process
 *
 * This is a NEW, non-standard LibXml2 function
 * hardlink elements are translated to their associated targets:
 *   <xml:hardlink [@target=<xpath>|@target-id=@xml:id] />
 * All nodes self-or-descendent::@node are translated
 * Existing Hardlinks are not followed
 *
 * TODO: the errors are all recoverable so we need to goto xmlFree()
 * 
 * Returns the hardlinks translated
 */
static void xmlErrorOrAttribute(xmlNodePtr cur, int breakOnError, const char *sErrorFormat, ...) {
  va_list args;
  char sMessage[2048];

  va_start(args, sErrorFormat);
  vsnprintf(sMessage, 2048, sErrorFormat, args);
  va_end(args);

  if ((breakOnError == 1) || (cur == NULL)) xmlGenericError(xmlGenericErrorContext, "%s", sMessage);
  else {
    if      (cur->type == XML_ELEMENT_NODE)   xmlSetProp(cur, BAD_CAST "xml:error", BAD_CAST sMessage);
    else if (cur->type == XML_ATTRIBUTE_NODE) xmlSetProp(cur->parent, BAD_CAST "xml:error", BAD_CAST sMessage);
  }
}

static xmlNodePtr 
xmlTranslateSubStructureHardlinkRecursive(xmlXPathContextPtr xpath_ctxt, xmlNodePtr xml_hardlink, int breakOnError, xmlNodeSetPtr new_hardlinks) {
  xmlNodePtr new_hardlink_node = NULL;
  
  /* target discovery */
  xmlNodePtr target_node              = 0;
  xmlXPathObjectPtr  targets_result   = 0;
  xmlAttrPtr id_attr                  = 0,
             target_xpath             = 0,
             target_xmlid             = 0;

  /* xml:deviation */
  xmlNodePtr deviation_spec, 
             deviation_original, 
             deviation_deviant, 
             deviation_hardlink;
  int deviation_type;
  const xmlChar *sAttributeName = 0,
                *sAttributeNamespace = 0,
                *sVerticalParentRouteAdopt = 0,
                *sDeviationType = 0,
                *sHardlinkID    = 0, 
                *sHardlink      = 0,
                *sOriginalID    = 0, 
                *sOriginal      = 0,
                *sDeviantID     = 0,
                *sDeviant       = 0;
  
  target_xpath = xmlHasProp(xml_hardlink, BAD_CAST "target");
  target_xmlid = xmlHasProp(xml_hardlink, BAD_CAST "target-id"); //Deprecated

  /* checks */
  if ((target_xpath == NULL) && (target_xmlid == NULL)) xmlErrorOrAttribute(xml_hardlink, breakOnError, "xmlTranslateSubStructure: xml:hardlink has no @target or @target-id");
  if ((target_xpath != NULL) && (strncmp((const char*) target_xpath->children->content, "id(", 3) == 0)) xmlErrorOrAttribute(xml_hardlink, breakOnError, "xmlTranslateSubStructure: xml:hardlink @target uses id() function only [%s] consider using @target-id", target_xpath->children->content);

  /* --------------------------------------- find the target
    * its possible that this xml:hardlink points to another xml:hardlink 
    * that has not been resolved yet
    * using /object:Server/ *[2] for example
    * below we will substitute later hardlinks by checking back pointers
    * before removal of them
    */
  target_node = NULL;
  if (target_xpath != NULL) {
    xpath_ctxt->node = xml_hardlink; /* allow relative xpath to work */
    targets_result   = xmlXPathEvalExpression(target_xpath->children->content, xpath_ctxt);
    if (targets_result != NULL) {
      if ((targets_result->nodesetval != NULL) && (targets_result->nodesetval->nodeNr == 1))
        target_node = targets_result->nodesetval->nodeTab[0];
      xmlXPathFreeObject(targets_result);
    }
  } else if (target_xmlid != NULL) {
    id_attr = xmlGetID(xml_hardlink->doc, target_xmlid->children->content);
    if (id_attr != NULL) target_node = id_attr->parent;
  }
  
  /* check if attempting to link to an unresolved xml:hardlink 
    * recursively resolve xml:hardlink
    * descendant traversal below will not descend on hardlinks that are ersolved in advance
    * TODO: cyclic check?
    */
  if ( (target_node     != NULL) 
    && (target_node->ns != NULL) 
    && (xmlStrEqual(target_node->ns->href, XML_XML_NAMESPACE) == 1) 
    && (xmlStrEqual(target_node->name, BAD_CAST "hardlink"))
  ) {
    target_node = xmlTranslateSubStructureHardlinkRecursive(xpath_ctxt, target_node, breakOnError, new_hardlinks);
  }

  if (target_node == NULL) {
    xmlErrorOrAttribute(xml_hardlink, breakOnError, "xmlTranslateSubStructure: xml:hardlink @target[-id] [%s%s] not found\n", 
      (target_xpath != NULL ? target_xpath->children->content : BAD_CAST ""),
      (target_xmlid != NULL ? target_xmlid->children->content : BAD_CAST "")
    );
  } 
  
  /* --------------------------------------- hardlinking */
  else {                
    new_hardlink_node = xmlHardlinkChild(xml_hardlink->parent, NULL, target_node, xml_hardlink, NULL, NULL);
    if (new_hardlink_node == NULL) {
      xmlErrorOrAttribute(xml_hardlink, breakOnError, "xmlTranslateSubStructure: xml:hardlink : failed to create hardlink");
    } else {
      if (new_hardlinks) xmlXPathNodeSetAddUnique(new_hardlinks, new_hardlink_node, NULL);

      /* vertical_parent_route_adopt */
      sVerticalParentRouteAdopt = xmlGetProp(target_node, BAD_CAST "vertical-parent-route-adopt");
      if (sVerticalParentRouteAdopt) LIBXML_NOT_COMPLETE("@vertical-parent-route-adopt");
      
      /* deviations 
      * allow relative xpath to work based on hardlink
      */
      deviation_spec   = xml_hardlink->children;
      xpath_ctxt->node = new_hardlink_node; 
      while (deviation_spec != NULL) {
        if (deviation_spec->type == XML_ELEMENT_NODE) {
          /* optional */
          sHardlinkID    = xmlGetProp(deviation_spec, BAD_CAST "hardlink-id");    /* deviation sub-specs can point to somewhere different! */
          sHardlink      = xmlGetProp(deviation_spec, BAD_CAST "hardlink");       /* deviation sub-specs can point to somewhere different! */
          sAttributeName = xmlGetProp(deviation_spec, BAD_CAST "attribute-name"); /* @attribute deviations only */
          sAttributeNamespace = xmlGetProp(deviation_spec, BAD_CAST "attribute-namespace"); /* @attribute deviations only */
          sDeviationType = xmlGetProp(deviation_spec, BAD_CAST "type");           /* type defaults to 0 */
          /* required */
          sOriginalID    = xmlGetProp(deviation_spec, BAD_CAST "original-id");
          sOriginal      = xmlGetProp(deviation_spec, BAD_CAST "original");
          sDeviantID     = xmlGetProp(deviation_spec, BAD_CAST "deviant-id");
          sDeviant       = xmlGetProp(deviation_spec, BAD_CAST "deviant");
          deviation_type = (sDeviationType == NULL ? 0 : 1); /* Only 0 supported currently */

          /* find nodes by ID / xpath
            * @hardlink[-id] is optional and defaults to THIS hardlink
            */
          deviation_hardlink = NULL;
          if (sHardlinkID) {
            id_attr = xmlGetID(xml_hardlink->doc, sHardlinkID);
            if (id_attr) deviation_hardlink = id_attr->parent;
          } else if (sHardlink) {
            targets_result = xmlXPathEvalExpression(sHardlink, xpath_ctxt);
            if (targets_result != NULL) {
              if ((targets_result->nodesetval != NULL) && (targets_result->nodesetval->nodeNr == 1))
                deviation_hardlink = targets_result->nodesetval->nodeTab[0];
              xmlXPathFreeObject(targets_result);
            }
          } else {
            deviation_hardlink = new_hardlink_node;
          }
          
          deviation_original = NULL;
          if (sOriginalID) {
            id_attr = xmlGetID(xml_hardlink->doc, sOriginalID);
            if (id_attr) deviation_original = id_attr->parent;
          } else if (sOriginal) {
            targets_result = xmlXPathEvalExpression(sOriginal, xpath_ctxt);
            if (targets_result != NULL) {
              if ((targets_result->nodesetval != NULL) && (targets_result->nodesetval->nodeNr == 1))
                deviation_original = targets_result->nodesetval->nodeTab[0];
              xmlXPathFreeObject(targets_result);
            }
          }
          
          deviation_deviant = NULL;
          if (sDeviantID) {
            id_attr = xmlGetID(xml_hardlink->doc, sDeviantID);
            if (id_attr) deviation_deviant = id_attr->parent;
          } else if (sDeviant) {
            targets_result = xmlXPathEvalExpression(sDeviant, xpath_ctxt);
            if (targets_result != NULL) {
              if ((targets_result->nodesetval != NULL) && (targets_result->nodesetval->nodeNr == 1))
                deviation_deviant = targets_result->nodesetval->nodeTab[0];
              xmlXPathFreeObject(targets_result);
            }
          }

          if (sAttributeName != NULL) {
            deviation_original = (xmlNodePtr) xmlHasNsProp(deviation_original, sAttributeName, sAttributeNamespace);
            deviation_deviant  = (xmlNodePtr) xmlHasNsProp(deviation_deviant,  sAttributeName, sAttributeNamespace);
          }

          /* deviate! */
          if ((deviation_original != NULL) && (deviation_deviant != NULL)) {
            if (deviation_original == deviation_deviant) {
              xmlErrorOrAttribute(xml_hardlink, 0, "xmlTranslateSubStructure: xml:hardlink deviant [%s:%s|%s|%s|%s] same as original", 
                (sAttributeNamespace != NULL ? sAttributeNamespace : BAD_CAST "no @attribute-namespace"),
                (sAttributeName != NULL ? sAttributeName : BAD_CAST "no @attribute-name"),
                (sHardlink != NULL ? sHardlink : BAD_CAST "no @hardlink"),
                (sOriginal != NULL ? sOriginal : BAD_CAST "no @original"),
                (sDeviant  != NULL ? sDeviant  : BAD_CAST "no @deviant")
              );
            } else {
              xmlDeviateNode(deviation_original, 
                deviation_hardlink,
                deviation_deviant, 
                deviation_type
              );
            }
          } else {
            xmlErrorOrAttribute(xml_hardlink, breakOnError, "xmlTranslateSubStructure: xml:hardlink deviant [%s|%s|%s|%s] invalid [%s] not found", 
              (sAttributeNamespace != NULL ? sAttributeNamespace : BAD_CAST "no @attribute-namespace"),
              (sAttributeName != NULL ? sAttributeName : BAD_CAST "no @attribute-name"),
              (sHardlink != NULL ? sHardlink : BAD_CAST "no @hardlink"),
              (sOriginal != NULL ? sOriginal : BAD_CAST "no @original"),
              (sDeviant  != NULL ? sDeviant  : BAD_CAST "no @deviant"),
              (deviation_original == NULL ? "original" : "deviant")
            );
          }
        }
        deviation_spec = deviation_spec->next;
      }

      /* ------------------------------------ destruction
        * now destroy the xml:hardlink placeholder, child deviations, and its @xml:id 
        * an already resolved hardlinked node may be pointing to this xml:hardlink already
        * check its hardlink_info and adjust if necessary
        */
      xmlUnlinkHardLink(xml_hardlink);
      xmlUnlinkNode(xml_hardlink);
      xmlFreeNode(xml_hardlink);
    }
  }
  
  /* free up */
  if (sAttributeName != NULL) xmlFree((void*) sAttributeName);
  if (sAttributeNamespace != NULL) xmlFree((void*) sAttributeNamespace);
  if (sVerticalParentRouteAdopt != NULL) xmlFree((void*) sVerticalParentRouteAdopt);
  if (sDeviationType != NULL) xmlFree((void*) sDeviationType);
  if (sHardlinkID != NULL)    xmlFree((void*) sHardlinkID);
  if (sOriginalID != NULL)    xmlFree((void*) sOriginalID);
  if (sDeviantID != NULL)     xmlFree((void*) sDeviantID);
          
  return (new_hardlink_node ? new_hardlink_node : xml_hardlink);
}

xmlNodeSetPtr 
xmlTranslateDocStructure(xmlDocPtr doc, xmlNodeSetPtr hardlink_placeholders, int breakOnError) {
  return xmlTranslateSubStructure(xmlDocGetRootElement(doc), hardlink_placeholders, breakOnError);
}

xmlNodeSetPtr 
xmlTranslateSubStructure(xmlNodePtr start, xmlNodeSetPtr hardlink_placeholders, int breakOnError) {
  xmlNodePtr         cur; /* can be xml:hardlink */
  xmlNodeSetPtr      new_hardlinks    = 0;
  int i_placeholder = 0;
  xmlNsPtr nsDef;
  xmlNodePtr root;
  xmlXPathContextPtr xpath_ctxt;

  /* input checks */
  if (start == NULL) return 0;
  if ((start->type != XML_ELEMENT_NODE) && (start->type != XML_DOCUMENT_NODE)) return 0;

  /* initiate OPTIONAL hardlink_placeholders traversal */
  if (hardlink_placeholders != NULL) {
    if (hardlink_placeholders->nodeNr != 0) cur = hardlink_placeholders->nodeTab[i_placeholder++];
    else cur = NULL;
  } else {
    /* hierarchical analysis */
    cur = start;
  }

  /* objects
   * @target xpath potentially has namespace prefixes in it: register all top level namespaces 
   */
  new_hardlinks = xmlXPathNodeSetCreateSize(hardlink_placeholders != NULL ? hardlink_placeholders->nodeNr : 10);
  xpath_ctxt    = xmlXPathNewContext(cur->doc); /* Annesley: xmlTranslateSubStructure() RARE */
  root          = xmlDocGetRootElement(cur->doc);
  if (root != NULL) {
    for (nsDef = root->nsDef; nsDef != 0; nsDef = nsDef->next) 
      xmlXPathRegisterNs(xpath_ctxt, nsDef->prefix, nsDef->href);
  }
  
  /* ------------------------------------------- begin traverse */
  while (cur != NULL) {
    if ((cur->type == XML_ELEMENT_NODE) 
      && (cur->ns != NULL) 
      && (xmlStrEqual(cur->ns->href, XML_XML_NAMESPACE) == 1)
      && (xmlStrEqual(cur->name, BAD_CAST "hardlink") == 1)
    ) {
      /* <xml:hardlink @target[-id]=<xpath or xml:id> />
       * pParent gets pTargetNode hardlinked under it next to pHardLinkNode (== pBeforeNode in this case) 
       * assigning the new hardlinked cur will prevent descent to children
       * recursive function:
       */
      cur = xmlTranslateSubStructureHardlinkRecursive(xpath_ctxt, cur, breakOnError, new_hardlinks);
    }
    
    /* traverse next step */
    if (hardlink_placeholders != NULL) {
      if (i_placeholder < hardlink_placeholders->nodeNr) 
        cur = hardlink_placeholders->nodeTab[i_placeholder++];
      else                                   
        cur = NULL; /* end */
    } else {
      /* descendant axis 
       * descend children on only original nodes 
       * the structure is being hardlinked as we traverse it
       * so some nodes will be part of hardlinked chains but still the original
       * ALSO: we can also be processing documents that already contain hardlinks
       * 
       * if ((cur->type == XML_ELEMENT_NODE)) 
       * printf("<%s:%s>\n", (cur->ns && cur->ns->prefix ? (const char*) cur->ns->prefix : ""), (const char*) cur->name);
       */
      if ((cur->type == XML_ELEMENT_NODE)
        && (xmlIsHardLink(cur) == 0) 
        && (cur->children != NULL)
      )                              cur = cur->children;
      else if (cur->next != NULL)    cur = cur->next;
      else {
        /* climb back up the tree 
         * and along any more siblings
         * noting that the parent moves may be to the last sibling
         * finally to start or XML_DOCUMENT_NODE
         */
        do {                         
                                     cur = cur->parent;
        } while ((cur != NULL) 
          && (cur->type == XML_ELEMENT_NODE)
          && (cur != start) 
          && (cur->next == NULL));
        if ((cur != NULL) && (cur->next != NULL))
                                     cur = cur->next;
        else                         cur = NULL; /* end */
      }
    }
  } //while (cur != NULL) traverse

  /* free up */
  if (xpath_ctxt != NULL) xmlXPathFreeContext(xpath_ctxt);
  
  return new_hardlinks;
}
