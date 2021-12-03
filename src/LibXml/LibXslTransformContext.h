//platform specific file (UNIX)
#ifndef _LIBXSLTRANSFORMCONTEXT_H
#define _LIBXSLTRANSFORMCONTEXT_H

//this file has been split out because of the LibXmlBase.cpp dependency on it
//  LibXsl.cpp -> LibXsl.h -> LibXml.h -> LibXmlBase.cpp which uses LibXslTransformContext directly in code
#include "Xml/XslDoc.h"       //full definition of XslTransformContext for inheritance
#include "LibXslModule.h" //various LibXsl libraries

using namespace std;

namespace general_server {
  /*
  struct _xsltTransformContext {
    xsltStylesheetPtr style;    // the stylesheet used
    xsltOutputType type;    // the type of output

    xsltTemplatePtr  templ;    // the current template
    int              templNr;    // Nb of templates in the stack
    int              templMax;    // Size of the templtes stack
    xsltTemplatePtr *templTab;    // the template stack

    xsltStackElemPtr  vars;    // the current variable list
    int               varsNr;    // Nb of variable list in the stack
    int               varsMax;    // Size of the variable list stack
    xsltStackElemPtr *varsTab;    // the variable list stack
    int               varsBase;    // the var base for current templ

    //
     * Extensions

    xmlHashTablePtr   extFunctions;  // the extension functions
    xmlHashTablePtr   extElements;  // the extension elements
    xmlHashTablePtr   extInfos;    // the extension data

    const xmlChar *mode;    // the current mode
    const xmlChar *modeURI;    // the current mode URI

    xsltDocumentPtr docList;    // the document list

    xsltDocumentPtr document;    // the current source document; can be NULL if an RTF
    xmlNodePtr node;      // the current node being processed
    xmlNodeSetPtr nodeList;    // the current node list
    // xmlNodePtr current;      the node

    xmlDocPtr output;      // the resulting document
    xmlNodePtr insert;      // the insertion node

    xmlXPathContextPtr xpathCtxt;  // the XPath context
    xsltTransformState state;    // the current state

    //
     * Global variables

    xmlHashTablePtr   globalVars;  // the global variables and params

    xmlNodePtr inst;      // the instruction in the stylesheet

    int xinclude;      // should XInclude be processed

    const char *      outputFile;  // the output URI if known

    int profile;                        // is this run profiled
    long             prof;    // the current profiled value
    int              profNr;    // Nb of templates in the stack
    int              profMax;    // Size of the templtaes stack
    long            *profTab;    // the profile template stack

    void            *_private;    // user defined data

    int              extrasNr;    // the number of extras used
    int              extrasMax;    // the number of extras allocated
    xsltRuntimeExtraPtr extras;    // extra per runtime informations

    xsltDocumentPtr  styleList;    // the stylesheet docs list
    void                 * sec;    // the security preferences if any

    xmlGenericErrorFunc  error;    // a specific error handler
    void              * errctx;    // context for the error handler

    xsltSortFunc      sortfunc;    // a ctxt specific sort routine

    //
     * handling of temporary Result Value Tree
     * (XSLT 1.0 term: "Result Tree Fragment")

    xmlDocPtr       tmpRVT;    // list of RVT without persistance
    xmlDocPtr       persistRVT;    // list of persistant RVTs
    int             ctxtflags;          // context processing flags

    //
     * Speed optimization when coalescing text nodes

    const xmlChar  *lasttext;    // last text node content
    unsigned int    lasttsize;    // last text node size
    unsigned int    lasttuse;    // last text node use
    //
     * Per Context Debugging

    int debugStatus;      // the context level debug status
    unsigned long* traceCode;    // pointer to the variable holding the mask

    int parserOptions;      // parser options xmlParserOption

    //
     * dictionary: shared between stylesheet, context and documents.

    xmlDictPtr dict;
    xmlDocPtr    tmpDoc; // Obsolete; not used in the library.
    //
     * all document text strings are internalized

    int internalized;
    int nbKeys;
    int hasTemplKeyPatterns;
    xsltTemplatePtr currentTemplateRule; // the Current Template Rule
    xmlNodePtr initialContextNode;
    xmlDocPtr initialContextDoc;
    xsltTransformCachePtr cache;
    void *contextVariable; // the current variable item
    xmlDocPtr localRVT; // list of local tree fragments; will be freed when
         the instruction which created the fragment
                           exits
    xmlDocPtr localRVTBase;
    int keyInitLevel;   // Needed to catch recursive keys issues
    int funcLevel;      // Needed to catch recursive functions issues
    xmlNodeFilterCallbackContextPtr xfilter;  //Annesley: additional security
    xmlNodeTriggerCallbackContextPtr xtrigger;
    void *emo; //Annesley: links through to emo C++ object for extensions
  };
  */

  class LibXslTransformContext: public XslTransformContext {
    xsltTransformContextPtr m_ctxt;
    unsigned long int m_iTraceFlags;

    const bool m_bOurs;
    const bool m_bInherited;
    bool       m_bEnabled;

    //inputs
    //recorded because they have extra stuff on them like namespace caches
    const IXmlBaseDoc *m_pSourceDoc;
    const IXslDoc     *m_pStylesheet;
    //IXmlBaseDoc       *m_pOutputDoc; //we get this from the native, without a cache setup

    //params: these are sent directly to the xsltApplyStylesheetUser()
    StringMap<const size_t>        m_paramsInt;
    StringMap<const char*>         m_paramsChar;
    StringMap<const XmlNodeList<const IXmlBaseNode>*> m_paramsNodeSet;

    //construction
    friend class LibXmlLibrary; //only LibXmlLibrary can create these
    LibXslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, const IXmlBaseDoc *pSourceDoc, const IXslDoc *pStylesheet, xsltTransformContextPtr ctxt = 0);
    LibXslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE, xsltTransformContextPtr ctxt);

    const char **assembleStoredBasicParams() const;
    const char **assembleStoredNodeSetParams() const;
    bool updateOutputNode(const IXmlBaseNode *pOutputNode = 0) const;
    
  public:
    //platform agnostic
    LibXslTransformContext(const LibXslTransformContext& lxtc); //exception copy constructor
    ~LibXslTransformContext();

    //accessors
    xsltTransformContextPtr libXmlContext() const {return m_ctxt;}
    IXslTransformContext *clone_with_resources() const {return new LibXslTransformContext(*this);}

    //control: has no meaning currently in TransformContext
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}

    //environment access
    IXslTransformContext *inherit(const IXslDoc *pChangeStylesheet = 0) const;
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}
    const char *type() const {return toString();}

    //documents and nodes
    //commandNode() will throw InterfaceNotSupported if the current instruction is not an XSL OR extension-prefix
    const IXmlBaseNode    *sourceNode(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner)    const; //input XML document
    const IXslCommandNode *commandNode( const IMemoryLifetimeOwner *pMemoryLifetimeOwner)    const; //stylesheet and current xsl command or extension-prefix command
    const IXmlBaseNode    *literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const; //xsl command lists include literals
    const IXslCommandNode *templateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner)    const; //stylesheet and current xsl template
    const IXmlBaseNode    *instructionNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const; //stylesheet and current instruction, INCLUDING direct output nodes, e.g. html:div
    IXmlBaseNode          *outputNode(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner)    const; //output XML doc
    const IXmlBaseDoc     *sourceDoc()  const;
    const IXslDoc         *commandDoc() const;

    //setup for a transform
    void setMode(const char *sWithMode = 0);
    bool clearOutputNode();
    void setStartNode(const IXmlBaseNode *pStartNode = 0);
    void addParams(const StringMap<const size_t> *pParamsInt);
    void addParams(const StringMap<const char*> *pParamsChar);
    void addParams(const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet);
    IXmlBaseDoc *transform() const;
    void         transform(const IXmlBaseNode *pInitialOutputNode) const;

    //error reporting
    const char *currentParentRoute() const;
    StringMap<const char*> *globalVars() const;
    StringMap<const char*> *localVars()  const;
    XmlNodeList<const IXmlBaseNode> *templateStack(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const;
    const char *currentModeName() const;
    const char *toString() const;

    //platform agnostic
    void globalVariableUpdate(const char *sName, const char *sValue);
    void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags);
    IXmlLibrary::xsltTraceFlag traceFlags() const;
    void clearXSLTTraceFlags();
    IXslTransformContext::iHardlinkPolicy hardlinkPolicy(const char *sHardlinkPolicy);
    IXslTransformContext::iHardlinkPolicy hardlinkPolicy(const iHardlinkPolicy iHardlinkPolicy);
    void continueTransformation(const IXmlBaseNode *pOutputNode = 0) const;

    //access to underlying XSLT commands
    void xsltCommand_element() const;
  };
}

#endif
