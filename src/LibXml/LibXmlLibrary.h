//platform specific file (UNIX)
#ifndef _LIBXMLLIBRARY_H
#define _LIBXMLLIBRARY_H

/* --------------------------------------- A word about LibXML2
http://xmlsoft.org/threads.html
http://xmlsoft.org/
http://xmlsoft.org/python.html C++ bindings that we dont use
LibXML++ is in scratchpad and most of this is based on that
LibXML2 is a procedural C library.
It does run on Windows but we are not currently using the Windows version.
*/

using namespace std;

//platform agnostic includes
#include "Xml/XmlLibrary.h" //full definition for inheritance

#include "libxml/xmlerror.h"

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>

#include <map>

//need to do this because we are reading the variable args of the function each time
//TODO: not really understand that line now...
#define LIBXML_REMOVE_NEWLINES \
    char c; \
    char *sChar = sMessage; \
    if (sChar) while (c = *sChar) {if (c == '\n' || c == '\r') *sChar = ' '; sChar++;}
#define LIBXML_STACK_COMPILE_ERROR_MESSAGE \
    char sMessage[2048]; \
    va_list arg_ptr; \
    va_start(arg_ptr, msg); {\
      vsnprintf(sMessage, 2048, msg, arg_ptr); \
    } va_end(arg_ptr);

#define CREATE_IFNULL true

namespace general_server {
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  class Repository;
  class LibXmlBaseNode;
  class LibXmlBaseDoc;

  class LibXmlLibrary: public XmlLibrary {
    friend class LibXslTransformContext;
    
    static void genericDebugFunc(void *ctx, const char *msg, ...);
    static void genericErrorFunc(void *ctx, const char *msg, ...);
    static void genericErrorFuncNoThrow(void *ctx, const char *msg, ...);
    static void structuredErrorFunc(void *userData, xmlErrorPtr error);
    static void structuredErrorFunc(void *userData, xmlErrorPtr error, const char *msg, ...);
    static void structuredErrorFuncNoThrow(void *userData, xmlErrorPtr error);
    static void structuredErrorFuncNoThrow(void *userData, xmlErrorPtr error, const char *msg, ...);
    static void transformErrorFunc(void *ctx, const char * msg, ...);
    static char *xmlGSShellReadline(char *sPrompt);
    static char *xmlGSShellStartupCommands(char *sPrompt);

  public:
    LibXmlLibrary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    ~LibXmlLibrary();

    void threadInit() const;
    void threadCleanup() const;

    void setDebugFunctions()   const;
    void clearDebugFunctions() const;
    void setErrorFunctions()   const;
    void clearErrorFunctions() const;
    void setDefaultXSLTTraceFlags(xsltTraceFlag iFlags) const;
    void clearDefaultXSLTTraceFlags() const;
    void setDefaultXMLTraceFlags(xmlTraceFlag iFlags) const;
    void clearDefaultXMLTraceFlags() const;
    bool shellCommand(const char *sCommand, IXmlBaseDoc *pDoc = 0, Repository *pRepository = 0) const;

    bool canHardLink()      const {return CAPABILITY_IMPLEMENTED;}
    bool canSoftLink()      const {return CAPABILITY_IMPLEMENTED;}
    bool canTransform()     const {return CAPABILITY_IMPLEMENTED;}
    bool hasNameAxis()      const {return CAPABILITY_IMPLEMENTED;}
    bool hasDefaultNameAxis() const {return CAPABILITY_IMPLEMENTED;}
    bool hasTopAxis()       const {return CAPABILITY_IMPLEMENTED;}
    bool hasParentsAxis()   const {return CAPABILITY_IMPLEMENTED;}
    bool hasAncestorsAxis() const {return CAPABILITY_IMPLEMENTED;}
    const char *name()            const {return "LibXML2++";}

    char *escape(  const char *sString)   const; //c escaping for string literals
    char *unescape(const char *sString)   const; //c un-escaping for string literals
    const char *convertHTMLToXML(const char *sContent) const;
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
    void throwNativeError(const char *sParameter1 = 0, const char *sParameter2 = 0, const char *sParameter3 = 0) const;

    //------------------------------------------- platform agnostic
    //returns IXmlBaseDocs only
    IXmlBaseDoc  *factory_document(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const;

    IXslDoc      *factory_stylesheet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const;

    IXslTransformContext     *factory_transformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pSourceDoc, const IXslDoc *pStylesheet) const;
    IXslXPathFunctionContext *factory_xpathContext(    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const;

    IXmlArea     *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pSourceNodes, const IXmlQueryEnvironment *pQE = 0, const IXmlNodeMask *pNodeMask = 0) const;
    IXmlArea     *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, XmlNodeList<const IXmlBaseNode> *pDirectMaskNodes) const;
    IXmlNodeMask *factory_nodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const char *sNodeMaskType = 0) const;

    //------------------------------------------- platform specific
    static LibXmlBaseNode       *factory_node(    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodePtr oNode, IXmlBaseDoc *pDoc,       const xmlListPtr oParentRoute = NO_PARENTROUTE, const char *sReason = 0, const bool bDocOurs = NOT_OURS, const bool bMorphSoftlinks = false);
    static const LibXmlBaseNode *factory_node(    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodePtr oNode, const IXmlBaseDoc *pDoc, const xmlListPtr oParentRoute = NO_PARENTROUTE, const char *sReason = 0, const bool bDocOurs = NOT_OURS, const bool bMorphSoftlinks = false);
    static LibXmlBaseDoc        *factory_document(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, const xmlDocPtr oDoc, const bool bOurs = true);
    static xmlNodeSetPtr xPathNodeSetCreate(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pNodeSet, const bool bCreateIfNull = false);
    static XmlNodeList<const IXmlBaseNode> *factory_const_nodeset(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodeSetPtr nodeset, const IXmlBaseDoc *pDoc, const bool bCreateIfNull = false, const char *sLockingReason = 0, const bool bMorphSoftlinks = false);
    static XmlNodeList<IXmlBaseNode> *factory_nodeset(            const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNodeSetPtr nodeset,       IXmlBaseDoc *pDoc, const bool bCreateIfNull = false, const char *sLockingReason = 0, const bool bMorphSoftlinks = false);
  };
}

#endif
