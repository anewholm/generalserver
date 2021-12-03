//platform specific file (UNIX)
#include "LibXmlLibrary.h"

#include "libxml/debugXML.h"
#include "libxml/HTMLtree.h"
#include "libxml/list.h"

#ifdef HAVE_LIBREADLINE
//apt-get install libreadline6-dev
#include <readline/readline.h>
#ifdef HAVE_LIBHISTORY
#include <readline/history.h>
#endif
#endif

#include "Debug.h"
#include "LibXmlBaseNode.h"
#include "LibXmlNode.h" //creation of LibXml*
#include "LibXslNode.h"
#include "LibXmlDoc.h"  //creation of LibXml*
#include "LibXslDoc.h"  //creation of LibXsl*
#include "LibXmlArea.h"
#include "LibXmlNodeMask.h"
#include "Server.h"

using namespace std;

namespace general_server {
  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- init -------------------------------------------------------
  //-----------------------------------------------------------------------------------------------------
  LibXmlLibrary::LibXmlLibrary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XmlLibrary(pMemoryLifetimeOwner)
  {
    //standard LibXML library init
    //http://www.xmlsoft.org/threads.html
    //xmlInitParser() calls:
    //  xmlInitThreads();
    //  xmlInitGlobals();
    //...
    //and is NOT_REENTRANT; uses a __xmlGlobalInitMutexLock()
    Debug::report("LibXmlLibrary: Initialising native [%s] parser", "libxml2");
    xmlInitParser();
    setDefaultXMLTraceFlags(XML_DEBUG_NONE); //lets monitor locking stuffs XML_DEBUG_TREE
    
    Debug::report("LibXmlLibrary: Initialising native [%s] engine", "libxslt");
    xsltInitGlobals();
    setDefaultXSLTTraceFlags(XSLT_TRACE_NONE); //we disagree with LibXML: our default trace is none
  }

  LibXmlLibrary::~LibXmlLibrary() {
    Debug::report("LibXmlLibrary: Cleaning up native [%s] engine", "libxslt");
    xsltCleanupGlobals(); //https://mail.gnome.org/archives/xslt/2008-October/msg00001.html
    Debug::report("LibXmlLibrary: Cleaning up native [%s] parser", "libxml2");
    xmlCleanupParser();   //xmlCleanupThreads()
    LibXslNode::freeParsedTemplates();
  }

  void LibXmlLibrary::threadInit() const {
    //TODO: xmlSetGenericErrorFunc doesn't seem to do the job always...
    setErrorFunctions();
    //setDebugFunctions();           //do this at application level
    exsltRegisterAll();              //exslt library
    xmlSubstituteEntitiesDefault(1); //substituting entity references in output
    xmlIndentTreeOutput = 1;         //indented output
    xmlKeepBlanksDefault(1);
    xmlDeregisterNodeDefault(LibXmlBaseNode::xmlDeregisterNodeFunc);
    //xmlSaveNoEmptyTags(1);
    //xmlSubstituteEntitiesDefaultValue(0);
    //TODO: LIBXML_NOCDATA??
    //xmlLoadExtDtdDefaultValue = 1; //load external entity references
  }

  void LibXmlLibrary::threadCleanup() const {
  }


  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- debug funcs ------------------------------------------------
  //-----------------------------------------------------------------------------------------------------
  void LibXmlLibrary::clearDebugFunctions() const {
    xsltSetGenericDebugFunc(NULL, NULL);
    xmlSetGenericDebugFunc(NULL, NULL);
  }
  void LibXmlLibrary::setDebugFunctions() const {
    xsltSetGenericDebugFunc((void*) this, genericDebugFunc);
    xmlSetGenericDebugFunc((void*) this, genericDebugFunc);
  }
  void LibXmlLibrary::genericDebugFunc(void * ctx, const char * msg, ...) {
    LIBXML_STACK_COMPILE_ERROR_MESSAGE; //not malloced
    Debug::report(sMessage);
  }

  void LibXmlLibrary::setDefaultXSLTTraceFlags(xsltTraceFlag iFlags) const {
    setDebugFunctions();
    xsltDebugSetDefaultTrace((xsltDebugTraceCodes) iFlags);
  }

  void LibXmlLibrary::clearDefaultXSLTTraceFlags() const {
    xsltDebugSetDefaultTrace((xsltDebugTraceCodes) XSLT_TRACE_NONE);
  }

  void LibXmlLibrary::setDefaultXMLTraceFlags(xmlTraceFlag iFlags) const {
    setDebugFunctions();
    xmlDebugSetDefaultTrace((xmlDebugTraceCodes) iFlags);
  }

  void LibXmlLibrary::clearDefaultXMLTraceFlags() const {
    xmlDebugSetDefaultTrace((xmlDebugTraceCodes) XML_DEBUG_NONE);
  }

  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- error funcs ------------------------------------------------
  //-----------------------------------------------------------------------------------------------------
  void LibXmlLibrary::setErrorFunctions() const   {
    //TODO: xmlSetGenericErrorFunc doesn't seem to do the job always...
    xmlSetGenericErrorFunc(   NULL, genericErrorFunc);
    xmlSetStructuredErrorFunc(NULL, structuredErrorFunc);
    xsltSetGenericErrorFunc(  NULL, genericErrorFunc);    //xsltutils: search for "* xsltSet"
  }

  void LibXmlLibrary::clearErrorFunctions() const {
    //always have one set so that we can throw a SEGFAULT in LibXML2 if it tries to use stdout (error.c)
    xmlSetGenericErrorFunc(   NULL, genericErrorFuncNoThrow);
    xmlSetStructuredErrorFunc(NULL, structuredErrorFuncNoThrow);
    xsltSetGenericErrorFunc(  NULL, genericErrorFuncNoThrow);
  }

  void LibXmlLibrary::genericErrorFuncNoThrow(void * ctx, const char * msg, ...) {
    LIBXML_STACK_COMPILE_ERROR_MESSAGE; //not malloced
    Debug::report(sMessage, IReportable::rtError);
  }

  void LibXmlLibrary::structuredErrorFuncNoThrow(void *userData, xmlErrorPtr error) {
    return structuredErrorFuncNoThrow(userData, error, "%s: %s %s %s", error->message, error->str1, error->str2, error->str3);
  }

  void LibXmlLibrary::structuredErrorFuncNoThrow(void *userData, xmlErrorPtr error, const char *msg, ...) {
    LIBXML_STACK_COMPILE_ERROR_MESSAGE; //not malloced
    Debug::report(sMessage, IReportable::rtError);
  }
  
  void LibXmlLibrary::structuredErrorFunc(void *userData, xmlErrorPtr error) {
    return structuredErrorFunc(userData, error, "%s: %s %s %s", error->message, error->str1, error->str2, error->str3);
  }

  void LibXmlLibrary::throwNativeError(const char *sParameter1, const char *sParameter2, const char *sParameter3) const {
    xmlThrowNativeError(BAD_CAST sParameter1, BAD_CAST sParameter2, BAD_CAST sParameter3);
  }

  void LibXmlLibrary::structuredErrorFunc(void *userData, xmlErrorPtr error, const char *msg, ...) {
    /* structuredErrorFunc error sources:
     *   dyn:evaluate(...)
     *   xpath
     * if we simply return from this function, processing continues
     *
     * note that the point here is to pass control up into the agnostic library wrappers
     * like:
     *   XslXPathFunctionContext
     *   XslTransformContext
     *   XslDoc
     * so that generic processing of the error can continue
     *

    struct _xmlError {
        int domain  : What part of the library raised this er
        int code  : The error code, e.g. an xmlParserError
        char *  message : human-readable informative error messag
        xmlErrorLevel level : how consequent is the error
        char *  file  : the filename
        int line  : the line number if available
        char *  str1  : extra string information
        char *  str2  : extra string information
        char *  str3  : extra string information
        int int1  : extra number information
        int int2  : column number of the error or 0 if N/A
        void *  ctxt  : the parser context if available
        void *  node  : the node in the tree
    }

    error->code:
      xmlerror.h for the xmlParser errors
    error->domain: {
      XML_FROM_NONE = 0,
      XML_FROM_PARSER,  // The XML parser
      XML_FROM_TREE,  // The tree module
      XML_FROM_NAMESPACE, // The XML Namespace module
      XML_FROM_DTD, // The XML DTD validation with parser context
      XML_FROM_HTML,  // The HTML parser
      XML_FROM_MEMORY,  // The memory allocator
      XML_FROM_OUTPUT,  // The serialization code
      XML_FROM_IO,  // The Input/Output stack
      XML_FROM_FTP, // The FTP module
      XML_FROM_HTTP (= 10),  // The HTTP module
      XML_FROM_XINCLUDE,  // The XInclude processing
      XML_FROM_XPATH, // The XPath module
      XML_FROM_XPOINTER,  // The XPointer module
      XML_FROM_REGEXP,  // The regular expressions module
      XML_FROM_DATATYPE,  // The W3C XML Schemas Datatype module
      XML_FROM_SCHEMASP,  // The W3C XML Schemas parser module
      XML_FROM_SCHEMASV,  // The W3C XML Schemas validation module
      XML_FROM_RELAXNGP,  // The Relax-NG parser module
      XML_FROM_RELAXNGV,  // The Relax-NG validator module
      XML_FROM_CATALOG (= 20), // The Catalog module
      XML_FROM_C14N,  // The Canonicalization module
      XML_FROM_XSLT,  // The XSLT engine from libxslt
      XML_FROM_VALID, // The XML DTD validation with valid context
      XML_FROM_CHECK, // The error checking module
      XML_FROM_WRITER,  // The xmlwriter module
      XML_FROM_MODULE,  // The dynamically loaded module module
      XML_FROM_I18N,  // The module handling character conversion
      XML_FROM_SCHEMATRONV,  // The Schematron validator module
      XML_FROM_SECURITY
    } xmlErrorDomain;
    */

    //native possible context pointers
    xmlParserCtxtPtr oPctxt;
    xmlXPathParserContextPtr oXPathParserContext;
    const IXmlQueryEnvironment *pQE  = 0;
    IXslXPathFunctionContext *pXCtxt = 0;
    const char *sDetail              = 0;
    xmlNodePtr        oNode          = 0;
    const LibXmlNode *pNode          = 0;

    if (error) {
      LIBXML_STACK_COMPILE_ERROR_MESSAGE;
      LIBXML_REMOVE_NEWLINES;

      switch (error->domain) {
        //------------------------------------------------------------------------------
        case XML_FROM_PARSER: {
          //during XML parsing operations, probably startup
          //we don't have a wrapper representation for this
          //so we just bubble up back to the DOC caller
          //hopefully with a useful xml:id
          xmlParserInputPtr oInput = 0;
          const char *sCurrent     = 0;
          const char *sXMLID       = 0;
          if (oPctxt = (xmlParserCtxtPtr) error->ctxt) {
            oInput = oPctxt->input;
            oNode  = oPctxt->node;
            if (oInput->cur) sCurrent = (const char*) oInput->cur;
            if (oNode)       sXMLID   = (const char*) xmlGetNsProp(oNode, (const xmlChar*) "id", (const xmlChar*) NAMESPACE_XML);
          }

          //throw up to the Database calling process
          //which can choose how to report the failed parse
          throw XMLParseFailedAt(NO_OWNER, _STRDDUP(sMessage, "<no message>"), _STRDDUP(sCurrent, "<no current>"), _STRDDUP(sXMLID, "<no @xml:id>"));
          break;
        }

        //------------------------------------------------------------------------------
        case XML_FROM_XPATH: {
          //we need to pass up to the generic XslXPathFunctionContext layer
          //because it may request ignoring the error
          //note that if the error happens during compilation, there will not be a outer context
          if ((oXPathParserContext = (xmlXPathParserContextPtr) error->ctxt)
            && oXPathParserContext->context
            && oXPathParserContext->context->xfilter
            && oXPathParserContext->context->xfilter->param
          ) {
            pQE = (const IXmlQueryEnvironment*) oXPathParserContext->context->xfilter->param;
          }
      
          if (pQE) {
            pXCtxt = new LibXslXPathFunctionContext(pQE, pQE, oXPathParserContext); //pQE as MM
            //using a new LibXslXPathFunctionContext, not local stack
            //because it will get UNWIND here if there is a throw AND deleted by the ~destructor
            //so we additionally delete it after the handleError() if there was no throw
            pXCtxt->handleError(sMessage);
            //delete pXCtxt; //NOT_OURS
          } else sDetail = (const char*) error->str1;
          break;
        }

        //------------------------------------------------------------------------------
        case XML_FROM_SECURITY: { 
          switch (error->code) {
            case XML_SECURITY_WRITE_ACCESS_DENIED: {throw NodeWriteableAccessDenied(NO_OWNER); break;}
            case XML_SECURITY_ADD_ACCESS_DENIED:   {throw NodeAddableAccessDenied(NO_OWNER);   break;}
            default: {exit(1);}
          }
          break;
        }

        //------------------------------------------------------------------------------
        case XML_FROM_LOCKING: {
          oNode = (const xmlNodePtr) error->node;
          pNode = new LibXmlNode(NO_OWNER, oNode, NULL, (const IXmlBaseDoc*) NULL); //TODO: MM?!
          
          switch (error->code) {
            case XML_LOCKING_LOCKED_NODE_CANNOT_BE_FREED: {
              Debug::report("XML_LOCKING_LOCKED_NODE_CANNOT_BE_FREED: [%s]\n  [%s]", 
                oNode->name ? (const char*) oNode->name : "<no name>",
                (sMessage ? sMessage : "")
              ); 
#ifdef LIBXML_DEBUG_ENABLED //xmlversion.h
              unsigned int uLock;
              const IXmlBaseNode *pLockingNode;
              const char *sReason;
              
              if (oNode->lock_externals) {
                for (uLock = 0; uLock < oNode->locks; uLock++) {
                  pLockingNode = (const IXmlBaseNode*) oNode->lock_externals[uLock];
                  sReason      = (const char*)         oNode->lock_reasons[uLock];
                  Debug::report("  <%s>: \"%s\"", pLockingNode ? pLockingNode->fullyQualifiedName() : "<no locking node>", sReason ? sReason : "<no reason>"); 
                }
              }
#endif
              throw NodeLockPreventedFree(NO_OWNER, pNode, MM_STRDUP(sMessage));
              break;
            }
            case XML_LOCKING_NODE_HAS_NO_LOCKS:
              throw NodeLockHasNoLocks(NO_OWNER, pNode, MM_STRDUP(sMessage));
              break;
            default: {
              throw XmlGenericException(NO_OWNER, MM_STRDUP(sMessage), MM_STRDUP("no specific handler for error message"));
              break;
            }
          }
          break;
        }

        //------------------------------------------------------------------------------
        case XML_FROM_VALID: {
          switch (error->code) {
            case XML_NAMESPACE_REQUIRED:           throw NodeWithoutNamespace(NO_OWNER, MM_STRDUP(error->str1)); break;
            case XML_NAMESPACE_WRONG_DOC:          assert(false); break;
            case XML_NAMESPACE_DEFAULT_NOT_FOUND:  throw CannotReconcileDefaultNamespace(NO_OWNER, MM_STRDUP(error->str1)); break;
            case XML_NAMESPACE_PREFIXED_NOT_FOUND: throw CannotReconcileNamespacePrefix(NO_OWNER, MM_STRDUP(error->str1)); break;
            case XML_DTD_ID_REDEFINED: {
              if (CAPABILITY_IMPLEMENTED) {
                //this is the Derived which knows if it is turned on or not
                //this is the initial parse process
                //allow the xml:id process after this to add a new xml:id here
                //change node from xml:id to xml:duplicate_id
                //processes after this can choose to remove them with //*[@xml:duplicate_id]
                oNode                  = (xmlNodePtr) error->node;
                xmlAttrPtr oAttr_xmlId = xmlHasNsProp(oNode, (const xmlChar*) "id", (const xmlChar*) NAMESPACE_XML);

                if (oAttr_xmlId) {
                  xmlUnsetNsProp(oNode, oAttr_xmlId->ns, oAttr_xmlId->name);
                  //xmlNodeSetName((xmlNodePtr) oAttr_xmlId, (const xmlChar*) "duplicate_id");
                }

                //contine without a throw ...
              } else {
                //TODO: send node through?
                throw XmlIdRedefined(NO_OWNER, MM_STRDUP(error->str1));
              }
              break;
            }
            default: {
              throw XmlGenericException(NO_OWNER, MM_STRDUP(sMessage), MM_STRDUP("no specific handler for error message"));
              break;
            }
          }
          break;
        }

        //------------------------------------------------------------------------------
        default: {
          //a specific context could not be found
          //TODO: hand out and up to the sevrer()->xmlLibrary()
          throw XmlGenericException(NO_OWNER, MM_STRDUP(sMessage), MM_STRDUP("no specific handler for error message"));
          break;
        }
      }
    } else {                   //------------------------------------------------------------------ generic
      //error details not sent through
      //should re-code LibXML2 to make sure it is
      //TODO: hand out and up to the sevrer()->xmlLibrary()
      throw XmlGenericException(NO_OWNER, MM_STRDUP("no error object"), MM_STRDUP(sDetail));
    }

    //note that this function CAN exit without doing anything
    //which indicates that the process is ok to continue
    //for example: if any @xsl:error-policy handling capacity is invoked
    
    //free up
    //oPctxt;              //direct pointer in to shared object
    //oXPathParserContext; //direct pointer in to shared object
    //pQE;                 //direct pointer in to shared object
    if (pXCtxt) delete pXCtxt;
    if (pNode)  delete pNode;
  }

  void LibXmlLibrary::transformErrorFunc(void * ctx, const char * msg, ...) {
    //also xsltSetTransformErrorFunc(m_ctxt, m_ctxt, NULL); 
    const IXmlQueryEnvironment *pQE;
    IXslTransformContext *pTCtxt;
    xsltTransformContextPtr oTCtxt = (xsltTransformContextPtr) ctx;
    
    LIBXML_STACK_COMPILE_ERROR_MESSAGE; //not malloced
    
    if ((oTCtxt = (xsltTransformContextPtr) ctx)
      && oTCtxt->xfilter
      && oTCtxt->xfilter->param
      && (pQE    = (const IXmlQueryEnvironment*) oTCtxt->xfilter->param)
      && (pTCtxt = pQE->transformContext())
    ) {
      //using a new LibXslTransformContext, not local stack
      //because it will get UNWIND here if there is a throw AND deleted by the ~destructor
      //so we additionally delete it after the handleError() if there was no throw
      pTCtxt->handleError(sMessage);
      //delete pTCtxt; //NOT_OURS
    } else throw XmlGenericException(NO_OWNER, MM_STRDUP(sMessage));
  }
  
  void LibXmlLibrary::genericErrorFunc(void * ctx, const char * msg, ...) {
    LIBXML_STACK_COMPILE_ERROR_MESSAGE; //not malloced

    //throw up but allow to continue
    //base library can decide what to do
    //cerr << sMessage << "\n";
    if (!strncmp(sMessage, "namespace", 9)) throw XmlParserNamespaceNotDeclared(NO_OWNER, MM_STRDUP(sMessage));
    else                                    throw XmlGenericException(NO_OWNER, MM_STRDUP(sMessage));
    //throw XmlGenericException(this, MM_STRDUP(sMessage));
  }


  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- shell interaction ------------------------------------------
  //-----------------------------------------------------------------------------------------------------
  char *LibXmlLibrary::xmlGSShellReadline(char *sPrompt) {
    char *sLine_read = 0;

#ifdef HAVE_LIBREADLINE
    sLine_read = readline(sPrompt);
    if (sLine_read && *sLine_read) add_history (sLine_read);
    return sLine_read;
#else
    size_t iLen = 1024;
    cout << sPrompt;
    getline(&sLine_read, &iLen, stdin);
#endif

    return sLine_read;
  }

  char *LibXmlLibrary::xmlGSShellStartupCommands(char *sPrompt) {
    NOT_CURRENTLY_USED("");
    return MM_STRDUP("setrootns\n");
  }

  bool LibXmlLibrary::shellCommand(const char *sCommand, IXmlBaseDoc *pDoc, Repository *pRepository) const {
    LibXmlBaseDoc *pDocType = 0;
    xmlDocPtr oDoc          = 0;
    if (pDoc && (pDocType = dynamic_cast<LibXmlBaseDoc*>(pDoc))) oDoc = pDocType->libXmlDocPtr();

    cout << "entering LibXML2 shell...";
    if (!oDoc) cout << " (without document!)";
    cout << "\n";
    cout << "  documentation: http://www.xmlsoft.org/html/libxml-debugXML.html#xmlShell\n";
    cout << "  type \"help\" for commands...\n";
    cout << "  ******* IMPORTANT: setrootns first to register the namespaces\n";

    //startup shell commands
    //xmlShell(oDoc, (char*) "/media/Data/general_server/out.txt", xmlGSShellStartupCommands, stdout);

#ifdef LIBXML_DEBUG_ENABLED //xmlversion.h
    //honest request
    xmlShell(oDoc, (char*) "/media/Data/general_server/out.txt", xmlGSShellReadline, stdout);
#else
    cout << "need to enable LIBXML_DEBUG_ENABLED\n"; //xmlversion.h
#endif
    return true;
  }

  const char *LibXmlLibrary::convertHTMLToXML(const char *sContent) const {
    //caller frees result

    /*
    Enum htmlParserOption {
      HTML_PARSE_RECOVER = 1 : Relaxed parsing
      HTML_PARSE_NODEFDTD = 4 : do not default a doctype if not found
      HTML_PARSE_NOERROR = 32 : suppress error reports
      HTML_PARSE_NOWARNING = 64 : suppress warning reports
      HTML_PARSE_PEDANTIC = 128 : pedantic error reporting
      HTML_PARSE_NOBLANKS = 256 : remove blank nodes
      HTML_PARSE_NONET = 2048 : Forbid network access
      HTML_PARSE_NOIMPLIED = 8192 : Do not add implied html/body... elements
      HTML_PARSE_COMPACT = 65536 : compact small text nodes
      HTML_PARSE_IGNORE_ENC = 2097152 : ignore internal document encoding hint
    }
    */

    htmlDocPtr oHTMLDoc     = 0;
    const char *sXML        = 0;
    xmlBufferPtr nodeBuffer = 0;
    xmlNodePtr oNode        = 0;

    clearErrorFunctions(); {
      oHTMLDoc = htmlReadDoc((xmlChar*) sContent, NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NONET);
    } setErrorFunctions();

    if ( (oHTMLDoc)
      && (nodeBuffer = xmlBufferCreate())
      && (oNode = xmlDocGetRootElement(oHTMLDoc))
      && (xmlNodeDump(nodeBuffer, oHTMLDoc, oNode, ZERO_INITIAL_INDENT, FORMAT_INDENT, 0))
    ) {
      sXML = MM_STRDUP((char*) nodeBuffer->content);
    }

    //free up
    if (oHTMLDoc)   xmlFreeDoc(oHTMLDoc);
    if (nodeBuffer) xmlBufferFree(nodeBuffer);

    return sXML;
  }

  char *LibXmlLibrary::escape(const char *sString) const {
    //caller frees result ONLY if not equal
    //c escaping for string literals
    return (char*) xmlXPathEscapeString((const xmlChar*) sString);
  }

  char *LibXmlLibrary::unescape(const char *sString) const {
    //caller frees result always
    //c un-escaping for string literals
    char *sCopy = MM_STRDUP_FOR_RETURN(sString);
    xmlXPathUnEscapeString((xmlChar*) sCopy);
    return sCopy;
  }

  //-----------------------------------------------------------------------------------------------------
  //---------------------------------------- object factory ---------------------------------------------
  //-----------------------------------------------------------------------------------------------------
  //-------------------------------------------------- platform agnostic
  IXmlBaseDoc *LibXmlLibrary::factory_document(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const {
    return new LibXmlDoc(pMemoryLifetimeOwner, this, sAlias);
  }

  IXslDoc *LibXmlLibrary::factory_stylesheet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const {return new LibXslDoc(pMemoryLifetimeOwner, this, sAlias);}

  IXslTransformContext     *LibXmlLibrary::factory_transformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pSourceDoc, const IXslDoc *pStylesheet) const {
    return new LibXslTransformContext(pMemoryLifetimeOwner, pQE, pSourceDoc, pStylesheet);
  }
  
  IXslXPathFunctionContext *LibXmlLibrary::factory_xpathContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode, const char *sXPath) const {
    return new LibXslXPathFunctionContext(pMemoryLifetimeOwner, pQE, pCurrentNode, sXPath);
  }

  IXmlArea     *LibXmlLibrary::factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pSourceNodes, const IXmlQueryEnvironment *pQE, const IXmlNodeMask *pNodeMask) const {
    return new LibXmlArea(pMemoryLifetimeOwner, pSourceNodes, pQE, pNodeMask);
  }

  IXmlArea     *LibXmlLibrary::factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, XmlNodeList<const IXmlBaseNode> *pDirectMaskNodes) const {
    return new LibXmlArea(pMemoryLifetimeOwner, pDirectMaskNodes);
  }

  IXmlNodeMask *LibXmlLibrary::factory_nodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const char *sNodeMaskType) const {
    return new LibXmlNodeMask(pMemoryLifetimeOwner, sNodeMask, sNodeMaskType);
  }

  //-------------------------------------------------- platform specific
  const LibXmlBaseNode *LibXmlLibrary::factory_node(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodePtr oNode, const IXmlBaseDoc *pDoc, const xmlListPtr oParentRoute, const char *sReason, const bool bDocOurs, const bool bMorphSoftlinks) {
    return factory_node(pMemoryLifetimeOwner, oNode, (IXmlBaseDoc*) pDoc, oParentRoute, sReason, bDocOurs, bMorphSoftlinks);
  }
  
  LibXmlBaseNode *LibXmlLibrary::factory_node(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodePtr oNode, IXmlBaseDoc *pDoc, const xmlListPtr oParentRoute, const char *sReason, const bool bDocOurs, const bool bMorphSoftlinks) {
    //platform specific
    //create a volatile node with a volatile doc: all write operations available
    LibXmlBaseNode *pNode        = 0;
    xmlListPtr oParentRouteToUse = oParentRoute;
    xmlNodePtr oNodeToUse        = oNode;

    assert(oNode);
    assert(!xmlIsSoftLink(oNode) || xmlListSize(oNode->hardlink_info->vertical_parent_route_adopt));

    //auto-morph in to the softlink target
    //xmlListDup(oParentRoute) in LibXmlBaseNode constructor
    if (bMorphSoftlinks && xmlIsSoftLink(oNode)) {
      oParentRouteToUse = xmlListDup(oNode->hardlink_info->vertical_parent_route_adopt);
      oNodeToUse        = (const xmlNodePtr) xmlListPopBack(oParentRouteToUse);
    }

    //factory
    if      (LibXslStylesheetNode::gimme(oNode)) pNode = new LibXslStylesheetNode(pMemoryLifetimeOwner, oNodeToUse, oParentRouteToUse, pDoc, NO_REGISTRATION, sReason, bDocOurs);
    else if (LibXslTemplateNode::gimme(oNode))   pNode = new LibXslTemplateNode(  pMemoryLifetimeOwner, oNodeToUse, oParentRouteToUse, pDoc, NO_REGISTRATION, sReason, bDocOurs);
    else if (LibXslNode::gimme(oNode))           pNode = new LibXslNode(          pMemoryLifetimeOwner, oNodeToUse, oParentRouteToUse, pDoc, NO_REGISTRATION, sReason, bDocOurs);
    else if (LibXslCommandNode::gimme(oNode))    pNode = new LibXslCommandNode(   pMemoryLifetimeOwner, oNodeToUse, oParentRouteToUse, pDoc, NO_REGISTRATION, sReason, bDocOurs);
    else                                         pNode = new LibXmlNode(          pMemoryLifetimeOwner, oNodeToUse, oParentRouteToUse, pDoc, NO_REGISTRATION, sReason, bDocOurs);
    
    //free up
    //xmlListDup(oParentRoute) in LibXmlBaseNode constructor
    if (oParentRouteToUse && oParentRouteToUse != oParentRoute) xmlListDelete(oParentRouteToUse);
    
    return pNode;
  }

  LibXmlBaseDoc *LibXmlLibrary::factory_document(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, const xmlDocPtr oDoc, const bool bOurs) {
    return new LibXmlDoc(pMemoryLifetimeOwner, pLib, sAlias, oDoc, bOurs);
  }

  xmlNodeSetPtr LibXmlLibrary::xPathNodeSetCreate(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pNodeSet, const bool bCreateIfNull) {
    //platform specific
    //you get out what you put in: pNodeSet can be zero: return will be zero
    //empty XmlNodeList = empty xmlNodeSet
    xmlNodeSetPtr nodeset = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    const LibXmlBaseNode* pNodeType;

    if (pNodeSet || bCreateIfNull) {
      nodeset = xmlXPathNodeSetCreate(NULL, NULL);
      for (iNode = pNodeSet->begin(); iNode != pNodeSet->end(); iNode++) {
        pNodeType = dynamic_cast<const LibXmlBaseNode*>(*iNode);
        //xmlXPathNodeSetAddUnique() will NOT check for duplicates
        //xmlXPathNodeSetAddUnique() will xmlXPathCopyNodeSet(parent_route)
        if (pNodeType) xmlXPathNodeSetAddUnique(nodeset, pNodeType->libXmlNodePtr(), pNodeType->libXmlParentRoutePtr());
      }
    }

    return nodeset;
  }

  XmlNodeList<const IXmlBaseNode> *LibXmlLibrary::factory_const_nodeset(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodeSetPtr aNodeset, const IXmlBaseDoc *pDoc, const bool bCreateIfNull, const char *sLockingReason, const bool bMorphSoftlinks) {
    return (XmlNodeList<const IXmlBaseNode>*) factory_nodeset(pMemoryLifetimeOwner, aNodeset, (IXmlBaseDoc*) pDoc, bCreateIfNull, sLockingReason, bMorphSoftlinks);
  }

  XmlNodeList<IXmlBaseNode> *LibXmlLibrary::factory_nodeset(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodeSetPtr aNodeset, IXmlBaseDoc *pDoc, const bool bCreateIfNull, const char *sLockingReason, const bool bMorphSoftlinks) {
    //platform specific
    //if aNodeset is zero, return will be zero
    XmlNodeList<IXmlBaseNode> *pNodeSet = 0;
    xmlListPtr oParentRoute;

    if (aNodeset || bCreateIfNull) pNodeSet = new XmlNodeList<IXmlBaseNode>(pMemoryLifetimeOwner);
    if (aNodeset) {
      for (int i = 0; i < aNodeset->nodeNr; i++) {
        oParentRoute = (aNodeset->parent_routeTab ? aNodeset->parent_routeTab[i] : NULL);
        pNodeSet->push_back(LibXmlLibrary::factory_node(pMemoryLifetimeOwner, aNodeset->nodeTab[i], pDoc, oParentRoute, sLockingReason, NOT_OURS, bMorphSoftlinks));
      }
    }

    return pNodeSet;
  }
}
