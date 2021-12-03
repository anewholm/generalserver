/*
 * dynamic.c: Implementation of the EXSLT -- Dynamic module
 *
 * References:
 *   http://www.exslt.org/dyn/dyn.html
 *
 * See Copyright for the status of this software.
 *
 * Authors:
 *   Mark Vakoc <mark_vakoc@jdedwards.com>
 *   Thomas Broyer <tbroyer@ltgt.net>
 *
 * TODO:
 * elements:
 * functions:
 *    min
 *    max
 *    sum
 *    map
 *    closure
 */

#define IN_LIBEXSLT
#include "libexslt/libexslt.h"

#if defined(WIN32) && !defined (__CYGWIN__) && (!__MINGW32__)
#include <win32config.h>
#else
#include "config.h"
#endif

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/extensions.h>

#include "exslt.h"

/**
 * exsltDynEvaluateFunction:
 * @ctxt:  an XPath parser context
 * @nargs:  the number of arguments
 *
 * Evaluates the string as an XPath expression and returns the result
 * value, which may be a boolean, number, string, node set, result tree
 * fragment or external object.
 */

static void
exsltDynEvaluateFunction(xmlXPathParserContextPtr ctxt, int nargs) {
        xmlChar *str = NULL;
        xmlXPathObjectPtr ret = NULL;

        if (ctxt == NULL)
                return;
        if (nargs != 1) {
                /* Annesley: re-organise this code to throw ONE error with all data instead of 2 and the 2nd gets lost */
                xsltPrintErrorContext(xsltXPathGetTransformContext(ctxt), NULL, NULL, "dyn:evaluate() : invalid number of args");
                /* xsltGenericError(xsltGenericErrorContext,
                        "dyn:evaluate() : invalid number of args %d\n", nargs); */
                ctxt->error = XPATH_INVALID_ARITY;
                return;
        }
        str = xmlXPathPopString(ctxt);
        /* return an empty node-set if an empty string is passed in */
        if (!str||!xmlStrlen(str)) {
                if (str) xmlFree(str);
                valuePush(ctxt,xmlXPathNewNodeSet(NULL, NULL));
                return;
        }
        ret = xmlXPathEval(str,ctxt->context);
        if (ret)
                valuePush(ctxt,ret);
         else {
                /* Annesley: xpath module will have already created an error
                 * xsltGenericError(xsltGenericErrorContext,
                        "dyn:evaluate() : unable to evaluate expression '%s'\n",str);
                        */
                valuePush(ctxt,xmlXPathNewNodeSet(NULL, NULL));
        }
        xmlFree(str);
        return;
}

/**
 * exsltDynMapFunction:
 * @ctxt:  an XPath parser context
 * @nargs:  the number of arguments
 *
 * Evaluates the string as an XPath expression and returns the result
 * value, which may be a boolean, number, string, node set, result tree
 * fragment or external object.
 */

static void
exsltDynMapFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    xmlChar *str = NULL;
    xmlNodeSetPtr nodeset = NULL;
    xsltTransformContextPtr tctxt;
    xmlXPathCompExprPtr comp = NULL;
    xmlXPathObjectPtr ret = NULL;
    xmlDocPtr oldDoc, container = NULL;
    xmlNodePtr oldNode;
    int oldContextSize;
    int oldProximityPosition;
    int i, j;
    xmlListPtr oldParentRoute;
    xmlXPathObjectPtr subResult;


    if (nargs != 2) {
        xmlXPathSetArityError(ctxt);
        return;
    }
    str = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) {
        xmlXPathSetTypeError(ctxt);
        return;
    }

    nodeset = xmlXPathPopNodeSet(ctxt);
    if (xmlXPathCheckError(ctxt)) {
        xmlXPathSetTypeError(ctxt);
        return;
    }
    /* Annesley: sending through the context to the xmlXPathCtxtCompile() here
     * so that it appears in any errors
     * TODO: put in xmlXPathCtxtCompile(ctxt->context, str)
     *
     * [xmlXPathCompile(str)] just sends NULL through to the xmlXPathCtxtCompile(NULL, str)
     * xmlXPathCtxtCompile(ctxt, str)
     *   NOT_CURRENTLY_USED:
     *   xmlXPathTryStreamCompile(ctxt, str) compile only if not present [] : () @ ::
     *       so not relevant for repository:shared/javascript:file
     *       would be relevant for admin/index or the invalid admin/ which are never used in GS
     *     xmlPatterncompile(...)
     *
     *   USED:
     *   xmlXPathCompileExpr(...)
     *     xmlXPathCompAndExpr(...)
     *       xmlXPathCompEqualityExpr(...)
     *         ...
     */
    if ( str == NULL
      || !xmlStrlen(str)
      || !(comp = xmlXPathCtxtCompile(ctxt->context, str))
    ) {
        if (nodeset != NULL)
            xmlXPathFreeNodeSet(nodeset);
        if (str != NULL)
            xmlFree(str);
        valuePush(ctxt, xmlXPathNewNodeSet(NULL, NULL));
        return;
    }

    ret = xmlXPathNewNodeSet(NULL, NULL);
    if (ret == NULL) {
        xsltGenericError(xsltGenericErrorContext,
                         "exsltDynMapFunction: ret == NULL\n");
        goto cleanup;
    }

    oldDoc = ctxt->context->doc;
    oldNode = ctxt->context->node;
    oldParentRoute = ctxt->context->parent_route;
    oldContextSize = ctxt->context->contextSize;
    oldProximityPosition = ctxt->context->proximityPosition;

        /**
         * since we really don't know we're going to be adding node(s)
         * down the road we create the RVT regardless
         */
    tctxt = xsltXPathGetTransformContext(ctxt);
    if (tctxt == NULL) {
        xsltTransformError(xsltXPathGetTransformContext(ctxt), NULL, NULL,
              "dyn:map : internal error tctxt == NULL\n");
        goto cleanup;
    }
    container = xsltCreateRVT(tctxt);
    if (container == NULL) {
        xsltTransformError(tctxt, NULL, NULL,
              "dyn:map : internal error container == NULL\n");
        goto cleanup;
    }
    xsltRegisterLocalRVT(tctxt, container);
    if (nodeset && nodeset->nodeNr > 0) {
        xmlXPathNodeSetSort(nodeset);
        ctxt->context->contextSize = nodeset->nodeNr;
        ctxt->context->proximityPosition = 0;
        for (i = 0; i < nodeset->nodeNr; i++) {
            ctxt->context->proximityPosition++;
            ctxt->context->node = nodeset->nodeTab[i];
            ctxt->context->parent_route = (nodeset->parent_routeTab != NULL ? xmlListDup(nodeset->parent_routeTab[i]) : NULL);
            ctxt->context->doc = nodeset->nodeTab[i]->doc;

            subResult = xmlXPathCompiledEval(comp, ctxt->context);
            if (subResult != NULL) {
                switch (subResult->type) {
                    case XPATH_NODESET:
                        if (subResult->nodesetval != NULL)
                            for (j = 0; j < subResult->nodesetval->nodeNr;
                                 j++)
                                xmlXPathNodeSetAdd(ret->nodesetval,
                                                   subResult->nodesetval->nodeTab[j],
                                                   (subResult->nodesetval->parent_routeTab == NULL ? NULL : subResult->nodesetval->parent_routeTab[j])
                                );
                        break;
                    case XPATH_BOOLEAN:
                        if (container != NULL) {
                            xmlNodePtr cur =
                                xmlNewChild((xmlNodePtr) container, NULL,
                                            BAD_CAST "boolean",
                                            BAD_CAST (subResult->
                                            boolval ? "true" : ""));
                            if (cur != NULL) {
                                cur->ns =
                                    xmlNewNs(cur,
                                             BAD_CAST
                                             "http://exslt.org/common",
                                             BAD_CAST "exsl");
                                xmlXPathNodeSetAddUnique(ret->nodesetval,
                                                         cur, NULL);
                            }
                            xsltExtensionInstructionResultRegister(tctxt, ret);
                        }
                        break;
                    case XPATH_NUMBER:
                        if (container != NULL) {
                            xmlChar *val =
                                xmlXPathCastNumberToString(subResult->
                                                           floatval);
                            xmlNodePtr cur =
                                xmlNewChild((xmlNodePtr) container, NULL,
                                            BAD_CAST "number", val);
                            if (val != NULL)
                                xmlFree(val);

                            if (cur != NULL) {
                                cur->ns =
                                    xmlNewNs(cur,
                                             BAD_CAST
                                             "http://exslt.org/common",
                                             BAD_CAST "exsl");
                                xmlXPathNodeSetAddUnique(ret->nodesetval,
                                                         cur, NULL);
                            }
                            xsltExtensionInstructionResultRegister(tctxt, ret);
                        }
                        break;
                    case XPATH_STRING:
                        if (container != NULL) {
                            xmlNodePtr cur =
                                xmlNewChild((xmlNodePtr) container, NULL,
                                            BAD_CAST "string",
                                            subResult->stringval);
                            if (cur != NULL) {
                                cur->ns =
                                    xmlNewNs(cur,
                                             BAD_CAST
                                             "http://exslt.org/common",
                                             BAD_CAST "exsl");
                                xmlXPathNodeSetAddUnique(ret->nodesetval,
                                                         cur, NULL);
                            }
                            xsltExtensionInstructionResultRegister(tctxt, ret);
                        }
                        break;
                    default:
                        break;
                }
                xmlXPathFreeObject(subResult);
            }
        }
    }
    ctxt->context->doc = oldDoc;
    ctxt->context->node = oldNode;
    ctxt->context->contextSize = oldContextSize;
    ctxt->context->proximityPosition = oldProximityPosition;
    ctxt->context->parent_route = oldParentRoute;



  cleanup:
    /* restore the xpath context */
    if (comp != NULL)
        xmlXPathFreeCompExpr(comp);
    if (nodeset != NULL)
        xmlXPathFreeNodeSet(nodeset);
    if (str != NULL)
        xmlFree(str);
    valuePush(ctxt, ret);
    return;
}


/**
 * exsltDynRegister:
 *
 * Registers the EXSLT - Dynamic module
 */

void
exsltDynRegister (void) {
    xsltRegisterExtModuleFunction ((const xmlChar *) "evaluate",
                                   EXSLT_DYNAMIC_NAMESPACE,
                                   exsltDynEvaluateFunction);
  xsltRegisterExtModuleFunction ((const xmlChar *) "map",
                                   EXSLT_DYNAMIC_NAMESPACE,
                                   exsltDynMapFunction);

}
