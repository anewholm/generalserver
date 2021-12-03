/*
 * imports.c: Implementation of the XSLT imports
 *
 * Reference:
 *   http://www.w3.org/TR/1999/REC-xslt-19991116
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXSLT
#include "libxslt.h"

#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#ifdef HAVE_NAN_H
#include <nan.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/hash.h>
#include <libxml/xmlerror.h>
#include <libxml/uri.h>
#include "xslt.h"
#include "xsltInternals.h"
#include "xsltutils.h"
#include "preproc.h"
#include "imports.h"
#include "documents.h"
#include "security.h"
#include "pattern.h"
#include "variables.h"
#include "xslt.h"


/************************************************************************
 *                                                                        *
 *                        Module interfaces                                *
 *                                                                        *
 ************************************************************************/
/**
 * xsltFixImportedCompSteps:
 * @master: the "master" stylesheet
 * @style: the stylesheet being imported by the master
 *
 * normalize the comp steps for the stylesheet being imported
 * by the master, together with any imports within that.
 *
 */
static void xsltFixImportedCompSteps(xsltStylesheetPtr master,
                        xsltStylesheetPtr style) {
    xsltStylesheetPtr res;
    xmlHashScan(style->templatesHash,
                    (xmlHashScanner) xsltNormalizeCompSteps, master);
    master->extrasNr += style->extrasNr;
    for (res = style->imports; res != NULL; res = res->next) {
        xsltFixImportedCompSteps(master, res);
    }
}

/**
 * xsltParseStylesheetImport:
 * @style:  the XSLT stylesheet
 * @cur:  the import element
 *
 * parse an XSLT stylesheet import element
 *
 * Returns 0 in case of success -1 in case of failure.
 */

int
xsltParseStylesheetImport(xsltStylesheetPtr style, xmlNodePtr cur) {
    int ret = -1;
    xmlDocPtr import = NULL;
    xmlChar *base = NULL;
    xmlChar *uriRef = NULL;
    xmlChar *URI = NULL;
    xsltStylesheetPtr res;
    xsltSecurityPrefsPtr sec;

    if ((cur == NULL) || (style == NULL))
        return (ret);

    uriRef = xmlGetNsProp(cur, (const xmlChar *)"href", NULL);
    if (uriRef == NULL) {
        xsltTransformError(NULL, style, cur,
            "xsl:import : missing href attribute\n");
        goto error;
    }

    base = xmlNodeGetBase(style->doc, cur);
    URI = xmlBuildURI(uriRef, base);
    if (URI == NULL) {
        xsltTransformError(NULL, style, cur,
            "xsl:import : invalid URI reference %s\n", uriRef);
        goto error;
    }

    res = style;
    while (res != NULL) {
        if (res->doc == NULL)
            break;
        if (xmlStrEqual(res->doc->URL, URI)) {
            xsltTransformError(NULL, style, cur,
               "xsl:import : recursion detected on imported URL %s\n", URI);
            goto error;
        }
        res = res->parent;
    }

    /*
     * Security framework check
     */
    sec = xsltGetDefaultSecurityPrefs();
    if (sec != NULL) {
        int secres;

        secres = xsltCheckRead(sec, NULL, URI);
        if (secres == 0) {
            xsltTransformError(NULL, NULL, NULL,
                 "xsl:import: read rights for %s denied\n",
                             URI);
            goto error;
        }
    }

    import = xsltDocDefaultLoader(URI, style->dict, XSLT_PARSE_OPTIONS,
                                  (void *) style, XSLT_LOAD_STYLESHEET);
    if (import == NULL) {
        xsltTransformError(NULL, style, cur,
            "xsl:import : unable to load %s\n", URI);
        goto error;
    }

    res = xsltParseStylesheetImportedDoc(import, style);
    if (res != NULL) {
        res->next = style->imports;
        style->imports = res;
        if (style->parent == NULL) {
            xsltFixImportedCompSteps(style, res);
        }
        ret = 0;
    } else {
        xmlFreeDoc(import);
        }

error:
    if (uriRef != NULL)
        xmlFree(uriRef);
    if (base != NULL)
        xmlFree(base);
    if (URI != NULL)
        xmlFree(URI);

    return (ret);
}

/**
 * xsltIncludeDocument:
 * @style:   the XSLT stylesheet
 * @cur:     xsl:include instruction
 * @include: the include document
 *
 * include an XSLT stylesheet
 *
 * Returns 0 in case of success -1 in case of failure
 */
static int
xsltIncludeDocument(xsltStylesheetPtr style, xmlNodePtr cur, xsltDocumentPtr include) {
    int ret = -1;
    xmlDocPtr oldDoc;
    xsltStylesheetPtr result;
    int oldNopreproc;

    if (include == NULL) {
        xsltTransformError(NULL, style, cur, "xsl:include : unable to load");
        return 0;
    }
#ifdef XSLT_REFACTORED
    if (IS_XSLT_ELEM_FAST(cur) && (cur->psvi != NULL)) {
        xsltStyleItemIncludePtr psvi = (xsltStyleItemIncludePtr) cur->psvi;
        if (psvi->include == NULL) {
          psvi->include = include;
        } else {
          xsltTransformError(NULL, style, cur, "xsl:include : psvi->include already compiled");
        }
    } else {
        xsltTransformError(NULL, style, cur,
            "Internal error: (xsltParseStylesheetInclude) "
            "The xsl:include element was not compiled.\n");
        style->errors++;
    }
#endif
    oldDoc = style->doc;
    style->doc = include->doc;
    /* chain to stylesheet for recursion checking */
    include->includes = style->includes;
    style->includes = include;
    oldNopreproc = style->nopreproc;
    style->nopreproc = include->preproc;
    /*
    * TODO: This will change some values of the
    *  including stylesheet with every included module
    *  (e.g. excluded-result-prefixes)
    *  We need to strictly seperate such stylesheet-owned values.
    */
    result = xsltParseStylesheetProcess(style, include->doc);
    style->nopreproc = oldNopreproc;
    include->preproc = 1;
    style->includes = include->includes;
    style->doc = oldDoc;
    if (result != NULL) ret = 0;

    return (ret);
}

/**
 * xsltParseStylesheetInclude:
 * @style:  the XSLT stylesheet
 * @cur:  the include node
 *
 * parse an XSLT stylesheet include element
 * Introduced because <xsl:include @xpath="multiple documents at once"
 * which allows stylesheets to include all ~Class stylesheets
 *
 * Returns 0 in case of success -1 in case of failure
 */

int
xsltParseStylesheetInclude(xsltStylesheetPtr style, xmlNodePtr cur) {
    int ret = -1;
    xmlChar *base = NULL;
    xmlChar *uriRef = NULL;
    xmlChar *URI = NULL;
    xsltDocumentPtr include;
    xsltDocumentPtr docptr;

    /* Annesley: loadType 0 = xpath, 1 = href */
    int loadType = 0;
    xmlXPathContextPtr oContext;
    xmlXPathObjectPtr  oResult;
    xmlNodeSetPtr      aNodeset;
    xmlDocPtr          oDoc;
    xmlNodePtr         oNode, oNewNode;
    xmlNodePtr         oldContextNode;
    xmlDocPtr          oldContextDoc;
    int i;

    if ((cur == NULL) || (style == NULL))
        return (ret);

    /* Annesley: xpath direct includes */
    uriRef = xmlGetNsProp(cur, (const xmlChar *)"xpath", NULL);
    if (uriRef == NULL) {
      uriRef = xmlGetNsProp(cur, (const xmlChar *)"href", NULL);
      if (uriRef == NULL) {
          xsltTransformError(NULL, style, cur,
              "xsl:include : missing href or xpath attribute\n");
          goto error;
      }
      loadType = 1;
    }

    if (loadType == 1) {
      base = xmlNodeGetBase(style->doc, cur);
      URI = xmlBuildURI(uriRef, base);
      if (URI == NULL) {
          xsltTransformError(NULL, style, cur,
              "xsl:include : invalid URI reference %s\n", uriRef);
          goto error;
      }
    } else URI = xmlStrdup(uriRef);

    /*
    * in order to detect recursion, we check all previously included
    * stylesheets.
    */
    docptr = style->includes;
    while (docptr != NULL) {
        if (xmlStrEqual(docptr->doc->URL, URI)) {
            xsltTransformError(NULL, style, cur,
                "xsl:include : recursion detected on included URL %s\n", URI);
            goto error;
        }
        docptr = docptr->includes;
    }

    if (loadType == 1) {
      /* ################################ old filesystem loader ################################
       * not used anymore. access to the filesystem is prohibited in this way
       * include = xsltLoadStyleDocument(style, URI);
       * include->doc->parent = style->doc->parent; later on: style->doc = include->doc; 
       * ret = xsltIncludeDocument(style, cur, include);
       */
      xsltTransformError(NULL, style, cur, "xsl:include : @href not supported anymore, use @xpath first load %s\n", uriRef);
      goto error;
    } else {
      /* ################################ xpath loader ################################
       * Annesley: xpath loader
       * MULTIPLE results are acceptable and all documents will be loaded in xpath result order
       * xpath results are from the parent of this doc indicated by doc->parent
       * doc->parent must be set before parse request
       */
      if (!style->doc->parent) {
        xsltTransformError(NULL, style, cur, "xsl:include : no parent xpath start point defined for xpath include [%s]", uriRef);
        goto error;
      }
      
      /* go 
       * style->doc->parent is the in main container for all stylesheets
       * node = the parent of the style
       * doc  = node->doc. important for id and ~Class lookups and root xpath
       * xpath context is sent through by the caller and can contain:
       *   GrammarProcessor
       *   XFilter security with node-mask
       *   Trigger strategy
       */
      oContext = XSLT_CCTXT(style)->xpathCtxt;
      if (oContext->nsHash->nbElems == 0) {
        xsltTransformError(NULL, style, cur, "xsl:include : no namespaces registered to handle [%s]", uriRef);
        goto error;
      }
      oldContextNode = oContext->node;
      oldContextDoc  = oContext->doc;
      oContext->node = style->doc->parent;
      oContext->doc  = oContext->node->doc; 
      oResult        = xmlXPathEvalExpression(uriRef, oContext);
      
      /* analyse results */
      if (oResult) {
        if ((aNodeset = oResult->nodesetval)) {
          if (aNodeset->nodeNr != 0) {
            for (i = 0; i < aNodeset->nodeNr; i++) {
              oNode        = aNodeset->nodeTab[i];
              
              /* Copied new doc attempts 
               * doc processing happens:
               *   whitespace and comments will be xmlFree()
               *   compiled psvi will be set
               * re-connect the xsl:stylesheet root node back to its original parent before processing
               *   for dynamic @match={}
               * re-connect the document to its including stylesheet for sub-xsl:includes
               *   for xsl:include @xpath
               */
              oDoc         = xmlNewDoc((xmlChar*) "1.0");
              oDoc->URL    = xmlStrdup(uriRef);     /* pass on the location for filesystem includes */
              oNewNode     = xmlCopyNode(oNode, 1); /* original oNode is NOT_OURS */
              xmlDocSetRootElement(oDoc, oNewNode); /* does an xmlUnlink() */
              oDoc->parent = oNode->parent;         /* pass on the location in the main document for further xpath includes */
              oNewNode->parent = oNode->parent;     /* link the stylesheet to its original class for ancestor calls */
              /* TODO: Shared doc attempts (xmlIsSharedDoc(doc)): 
               * xsltCompilerPopNode() throwing errors
               * changes happen to source
               */
              //oDoc->children = oNode;
              //oDoc->last   = oNode;
              
              include      = xsltNewStyleDocument(style, oDoc);
              
              /* only one include can be assigned to an xsl:include ->psvi (see xsltIncludeDocument)
               * so here we dynamically create a new prev xsl:include
               * to hold this new pre-comp include
               * TODO: move to a chained includes for the xsltStyleItemIncludePtr
               */
              if (i > 0) {
                oNewNode = xmlCopyNode(cur, 1);
                xmlAddPrevSibling(cur, oNewNode);
                xsltCompileXSLTIncludeElem(XSLT_CCTXT(style), oNewNode);
                cur = oNewNode;
              }
              
              /* set the single ->psvi->include of the node to point to the discovered include */
              ret          = xsltIncludeDocument(style, cur, include);
              
#ifdef LIBXML_DEBUG_ENABLED
              if (aNodeset->nodeNr > 1) {
                xmlChar *xmlID, *parentName, *styleName;
                if (i == 0) {
                  xmlID = xmlGetNsProp(cur, (const xmlChar*) "id", (const xmlChar*) XML_XML_NAMESPACE);
                  XSLT_TRACE_GENERIC3(XSLT_TRACE_PARSING, "xsltParseStylesheetInclude: [%s] multiple xpath include [%s] => [%i] stylesheets :)", xmlID, uriRef, aNodeset->nodeNr);
                  if (xmlID) xmlFree(xmlID);
                }
                xmlID      = xmlGetNsProp(oNode, (const xmlChar*) "id", (const xmlChar*) XML_XML_NAMESPACE);
                styleName  = xmlGetProp(oNode, (const xmlChar*) "name");
                parentName = xmlGetProp(oNode->parent, (const xmlChar*) "name");
                XSLT_TRACE_GENERIC4(XSLT_TRACE_PARSING, "xsltParseStylesheetInclude: multiple xpath include [%s]/[%s]/[%s] %s", 
                  parentName, xmlID, styleName,
                  (ret == 0 ? ":)" : "**** FAILED ****")
                );
                if (xmlID)      xmlFree(xmlID);
                if (styleName)  xmlFree(styleName);
                if (parentName) xmlFree(parentName);
              }
#endif
              if (ret != 0) goto error;
            }
          } else xsltTransformError(NULL, style, cur, "xsl:include : xpath [%s] returned empty set [%s]\n", uriRef, oContext->node->name);
        } else xsltTransformError(NULL, style, cur, "xsl:include : xpath [%s] returned invalid set [%s]\n", uriRef, oContext->node->name);
      } else xsltTransformError(NULL, style, cur, "xsl:include : xpath [%s] query failed [%s]\n", uriRef, oContext->node->name);
      
      /* reset */
      oContext->node = oldContextNode;
      oContext->doc  = oldContextDoc;
      
      /* free up */
      if (oResult) xmlXPathFreeObject(oResult);
    }

error:
    if (uriRef != NULL)
        xmlFree(uriRef);
    if (base != NULL)
        xmlFree(base);
    if (URI != NULL)
        xmlFree(URI);

    return (ret);
}

/**
 * xsltNextImport:
 * @cur:  the current XSLT stylesheet
 *
 * Find the next stylesheet in import precedence.
 *
 * Returns the next stylesheet or NULL if it was the last one
 */

xsltStylesheetPtr
xsltNextImport(xsltStylesheetPtr cur) {
    if (cur == NULL)
        return(NULL);
    if (cur->imports != NULL)
        return(cur->imports);
    if (cur->next != NULL)
        return(cur->next) ;
    do {
        cur = cur->parent;
        if (cur == NULL) break;
        if (cur->next != NULL) return(cur->next);
    } while (cur != NULL);
    return(cur);
}

/**
 * xsltNeedElemSpaceHandling:
 * @ctxt:  an XSLT transformation context
 *
 * Checks whether that stylesheet requires white-space stripping
 *
 * Returns 1 if space should be stripped, 0 if not
 */

int
xsltNeedElemSpaceHandling(xsltTransformContextPtr ctxt) {
    xsltStylesheetPtr style;

    if (ctxt == NULL)
        return(0);
    style = ctxt->style;
    while (style != NULL) {
        if (style->stripSpaces != NULL)
            return(1);
        style = xsltNextImport(style);
    }
    return(0);
}

/**
 * xsltFindElemSpaceHandling:
 * @ctxt:  an XSLT transformation context
 * @node:  an XML node
 *
 * Find strip-space or preserve-space informations for an element
 * respect the import precedence or the wildcards
 *
 * Returns 1 if space should be stripped, 0 if not, and 2 if everything
 *         should be CDTATA wrapped.
 */

int
xsltFindElemSpaceHandling(xsltTransformContextPtr ctxt, xmlNodePtr node) {
    xsltStylesheetPtr style;
    const xmlChar *val;

    if ((ctxt == NULL) || (node == NULL))
        return(0);
    style = ctxt->style;
    while (style != NULL) {
        if (node->ns != NULL) {
            val = (const xmlChar *)
              xmlHashLookup2(style->stripSpaces, node->name, node->ns->href);
            if (val == NULL) {
                val = (const xmlChar *)
                    xmlHashLookup2(style->stripSpaces, BAD_CAST "*",
                                   node->ns->href);
            }
        } else {
            val = (const xmlChar *)
                  xmlHashLookup2(style->stripSpaces, node->name, NULL);
        }
        if (val != NULL) {
            if (xmlStrEqual(val, (xmlChar *) "strip"))
                return(1);
            if (xmlStrEqual(val, (xmlChar *) "preserve"))
                return(0);
        }
        if (style->stripAll == 1)
            return(1);
        if (style->stripAll == -1)
            return(0);

        style = xsltNextImport(style);
    }
    return(0);
}

/**
 * xsltFindTemplate:
 * @ctxt:  an XSLT transformation context
 * @name: the template name
 * @nameURI: the template name URI
 *
 * Finds the named template, apply import precedence rule.
 * REVISIT TODO: We'll change the nameURI fields of
 *  templates to be in the string dict, so if the
 *  specified @nameURI is in the same dict, then use pointer
 *  comparison. Check if this can be done in a sane way.
 *  Maybe this function is not needed internally at
 *  transformation-time if we hard-wire the called templates
 *  to the caller.
 *
 * Returns the xsltTemplatePtr or NULL if not found
 */
xsltTemplatePtr
xsltFindTemplate(xsltTransformContextPtr ctxt, const xmlChar *name,
                 const xmlChar *nameURI) {
    xsltTemplatePtr cur;
    xsltStylesheetPtr style;

    if ((ctxt == NULL) || (name == NULL))
        return(NULL);
    style = ctxt->style;
    while (style != NULL) {
        cur = style->templates;
        while (cur != NULL) {
            if (xmlStrEqual(name, cur->name)) {
                if (((nameURI == NULL) && (cur->nameURI == NULL)) ||
                    ((nameURI != NULL) && (cur->nameURI != NULL) &&
                     (xmlStrEqual(nameURI, cur->nameURI)))) {
                    return(cur);
                }
            }
            cur = cur->next;
        }

        style = xsltNextImport(style);
    }
    return(NULL);
}

