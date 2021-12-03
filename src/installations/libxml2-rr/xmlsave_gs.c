/**
 * xmlDeviationsDumpOutput:
 * @deviation:  deviation list to output
 *
 *   <xml:deviation @original-id="<xml:id>" @deviant-id="<xml:id>" [@attribute-name="<FQN name>" @hardlink-id="<xml:id>"] />
 */
static void
xmlDeviationsDumpOutput(xmlSaveCtxtPtr ctxt, xmlDeviationPtr deviation) {
  xmlChar *xpath;
  xmlNodePtr original, deviant;

  while (deviation) {
    if ((ctxt->format == 1) && (xmlIndentTreeOutput))
        xmlOutputBufferWrite(ctxt->buf, ctxt->indent_size *
                              (ctxt->level > ctxt->indent_nr ?
                              ctxt->indent_nr : ctxt->level),
                              ctxt->indent);

    xmlOutputBufferWrite(ctxt->buf, 14, "<xml:deviation");

    original = deviation->original;
    deviant  = deviation->deviant;
    switch (deviation->original->type) {
      case XML_ATTRIBUTE_NODE:
        /* with attribute deviations we need to take the @xml:id's of the parents and the attribute name */
        original = deviation->original->parent;
        deviant  = deviation->deviant->parent;

        if (deviation->original->ns) {
          xmlOutputBufferWrite(ctxt->buf, 22, " attribute-namespace=\"");
          xmlOutputBufferWriteString(ctxt->buf, (const char*) deviation->original->ns->href);
          xmlOutputBufferWrite(ctxt->buf, 1, "\"");
        }
        
        xmlOutputBufferWrite(ctxt->buf, 17, " attribute-name=\"");
        xmlOutputBufferWriteString(ctxt->buf, (const char*) deviation->original->name);
        xmlOutputBufferWrite(ctxt->buf, 1, "\"");
        break;
      case XML_ELEMENT_NODE: break;
      default: xmlGenericError(xmlGenericErrorContext, "xmlDeviationsDumpOutput : deviant type not supported [%i] [%s]", deviation->original->type, deviation->original->name);
    }

    xmlOutputBufferWrite(ctxt->buf, 11, " original=\"");
    xpath = xmlGetNodePath(original);
    xmlOutputBufferWriteString(ctxt->buf, (const char*) xpath);
    xmlFree(xpath);
    xmlOutputBufferWrite(ctxt->buf, 1, "\"");

    xmlOutputBufferWrite(ctxt->buf, 10, " deviant=\"");
    xpath = xmlGetNodePath(deviant);
    xmlOutputBufferWriteString(ctxt->buf, (const char*) xpath);
    xmlFree(xpath);
    xmlOutputBufferWrite(ctxt->buf, 1, "\"");

    /* output is on THIS hardlink so no need to write the hardlink-id */

    xmlOutputBufferWrite(ctxt->buf, 3, " />");

    if (ctxt->format == 1) xmlOutputBufferWrite(ctxt->buf, 1, "\n");

    deviation = deviation->next;
  }
}

/**
 * xmlHardlinkInfoDumpOutput:
 * @hardlink_info:  hardlink_info to output
 *
 *   <xml:deviation>...s
 */
static void
xmlHardlinkInfoDumpOutput(xmlSaveCtxtPtr ctxt, xmlHardlinkInfoPtr hardlink_info) {
  if (hardlink_info == NULL) return;
  xmlDeviationsDumpOutput(ctxt, hardlink_info->deviants);
  xmlDeviationsDumpOutput(ctxt, hardlink_info->originals);
  xmlDeviationsDumpOutput(ctxt, hardlink_info->descendant_deviants);
}

/**
 * xmlHardLinkDumpOutput:
 * @cur:  node to output
 *
 * hardlink_info->next and hardlink_info->prev represent a double linked list of other hardlinks
 * serialised xml representation of a hardlink:
 *   <xml:hardlink @target-id="<xml:id>" or @target="<xpath to target>" />
 */
static void
xmlHardLinkDumpOutput(xmlSaveCtxtPtr ctxt, xmlNodePtr cur) {
  xmlChar *sNodeXPathIdentifier  = 0,
          *sNodeXPathIdentifier2 = 0;
  
  if ((cur == NULL) || (ctxt == NULL) || (ctxt->buf == NULL)) return;
  if (xmlIsOriginalHardLink(cur) == 1) xmlGenericError(xmlGenericErrorContext, "xmlHardLinkDumpOutput : original is not a hardlink [%s]", cur->name);
  if (xmlIsHardLink(cur) == 0)         xmlGenericError(xmlGenericErrorContext, "xmlHardLinkDumpOutput : not a hardlink [%s]", cur->name);
  
  xmlOutputBufferWrite(ctxt->buf, 13, "<xml:hardlink");
  /* @target=/object:Server/repository:groups */
  xmlOutputBufferWrite(ctxt->buf, 9, " target=\"");
  sNodeXPathIdentifier = xmlGetNodePath(xmlHardLinkOriginal(cur)); /* = xmlFirstHardLink() */
  xmlOutputBufferWriteString(ctxt->buf, (const char*) sNodeXPathIdentifier);
  xmlOutputBufferWrite(ctxt->buf, 1, "\"");
  
  if (cur->hardlink_info->vertical_parent_route_adopt != NULL) {
    /* @vertical-parent-route-adopt=/object:Server/repository:groups */
    if (xmlListSize(cur->hardlink_info->vertical_parent_route_adopt) > 1) {
      LIBXML_NOT_COMPLETE("@vertical-parent-route-adopt serialisation with parent_route");
    } else {
      /* no actual parent_route so we do not need to serialise it 
       * and the hardlink will be for the original
       */
      xmlOutputBufferWrite(ctxt->buf, 30, " vertical-parent-route-adopt=\"");
      sNodeXPathIdentifier2 = xmlGetNodePath(xmlHardLinkOriginal(cur));
      xmlOutputBufferWriteString(ctxt->buf, (const char*) sNodeXPathIdentifier2);
      xmlOutputBufferWrite(ctxt->buf, 1, "\"");
    }
  }

  if ( (cur->hardlink_info->deviants == NULL)
    && (cur->hardlink_info->originals == NULL)
    && (cur->hardlink_info->descendant_deviants == NULL)
  ) {
    xmlOutputBufferWrite(ctxt->buf, 3, " />");
  } else {
    xmlOutputBufferWrite(ctxt->buf, 1, ">");
    if (ctxt->format == 1) xmlOutputBufferWrite(ctxt->buf, 1, "\n");
    ctxt->level++;
    xmlHardlinkInfoDumpOutput(ctxt, cur->hardlink_info);
    ctxt->level--;
    xmlOutputBufferWrite(ctxt->buf, 15, "</xml:hardlink>");
  }

  if (ctxt->format == 1) xmlOutputBufferWrite(ctxt->buf, 1, "\n");

  //free up
  if (sNodeXPathIdentifier)  xmlFree(sNodeXPathIdentifier);
  if (sNodeXPathIdentifier2) xmlFree(sNodeXPathIdentifier2);
}

