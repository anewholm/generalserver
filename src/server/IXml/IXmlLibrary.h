//platform agnostic file
//Abstract Factory Method: http://en.wikipedia.org/wiki/Abstract_factory_pattern
//  for creating all associated XML, XSL and XShema objects
//  parent class should instanciate the factory of its choice and then create through it
//  but can also create objects from other already instantiated objects
//interface that all XML libraries should implement
#ifndef _IXMLLIBRARY_H
#define _IXMLLIBRARY_H

#include "define.h"

#include "IReportable.h"     //IReportable direct inheritance
#include "Exceptions.h"      //passed on stack
#include "Xml/XmlNodeList.h" //template parameter

//XML specific capabilities
#define CAPABILITY_XML_NATIVE true
#define ALLOW_NAMESPACE true
#define MAKE_LOWERCASE  true

#include <vector>
using namespace std;

namespace general_server {
  class Repository;

  interface_class IXmlArea;
  interface_class IXmlNodeMask;
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXslDoc;
  interface_class IXslTransformContext;

  //used by XmlLibrary
  enum iSecurityImplementation
  {
    SECURITY_IMPLEMENTATION_NONE = 0,
    SECURITY_IMPLEMENTATION_UNIX = 1
  };

  interface_class IXmlLibrary: implements_interface IMemoryLifetimeOwner, implements_interface IReportable {
    //WRAPPER_ONLY;
    //no direct references to the base concrete classes
    //library allows creation of new concrete instances from existing ones to maintain type
    //and avoid the necessity for passing a reference to the library around
  public:
    //these virtual ones call selected static threadInit() functions
    //on other classes (not objects) to selectively initiate the thread
    //each class that inherits from Thread must initialise its XmlLibrary
    virtual void threadInit() const     = 0;
    virtual void threadCleanup() const  = 0;

    //surprise: these are the same as the LibXml codes
    enum xmlTraceFlag {
        XML_DEBUG_ALL =                 -1,
        XML_DEBUG_NONE =                0,
        XML_DEBUG_XPATH_STEP =          1<<0,
        XML_DEBUG_XPATH_EXPR =          1<<1,
        XML_DEBUG_XPATH_EVAL_COUNTS =   1<<2,
        XML_DEBUG_PARENT_ROUTE =        1<<3,
        XML_DEBUG_TREE =                1<<4
    };
    enum xsltTraceFlag {
        XSLT_TRACE_ALL =                -1,
        XSLT_TRACE_NONE =               0,
        XSLT_TRACE_COPY_TEXT =          1<<0,
        XSLT_TRACE_PROCESS_NODE =       1<<1,
        XSLT_TRACE_APPLY_TEMPLATE =     1<<2,
        XSLT_TRACE_COPY =               1<<3,
        XSLT_TRACE_COMMENT =            1<<4,
        XSLT_TRACE_PI =                 1<<5,
        XSLT_TRACE_COPY_OF =            1<<6,
        XSLT_TRACE_VALUE_OF =           1<<7,
        XSLT_TRACE_CALL_TEMPLATE =      1<<8,
        XSLT_TRACE_APPLY_TEMPLATES =    1<<9,
        XSLT_TRACE_CHOOSE =             1<<10,
        XSLT_TRACE_IF =                 1<<11,
        XSLT_TRACE_FOR_EACH =           1<<12,
        XSLT_TRACE_STRIP_SPACES =       1<<13,
        XSLT_TRACE_TEMPLATES =          1<<14,
        XSLT_TRACE_KEYS =               1<<15,
        XSLT_TRACE_VARIABLES =          1<<16,

        XSLT_TRACE_FUNCTION =           1<<17,
        XSLT_TRACE_PARSING =            1<<18,
        XSLT_TRACE_BLANKS =             1<<19,
        XSLT_TRACE_PROCESS =            1<<20,
        XSLT_TRACE_EXTENSIONS =         1<<21,
        XSLT_TRACE_ATTRIBUTES =         1<<22,
        XSLT_TRACE_EXTRA =              1<<23,
        XSLT_TRACE_AVT =                1<<24,
        XSLT_TRACE_PATTERN =            1<<25,
        XSLT_TRACE_VARIABLE =           1<<26  /* this is more global variables unlike XSLT_TRACE_VARIABLES */
    };
    virtual xsltTraceFlag parseXSLTTraceFlags(const char *sFlags) const = 0;
    virtual xmlTraceFlag  parseXMLTraceFlags( const char *sFlags) const = 0;

    virtual void setDebugFunctions()   const = 0;
    virtual void clearDebugFunctions() const = 0;
    virtual void setErrorFunctions()   const = 0;
    virtual void clearErrorFunctions() const = 0;
    virtual void setDefaultXSLTTraceFlags(xsltTraceFlag iFlags) const = 0;
    virtual void clearDefaultXSLTTraceFlags() const = 0;
    virtual void setDefaultXMLTraceFlags(xmlTraceFlag iFlags) const = 0;
    virtual void clearDefaultXMLTraceFlags() const = 0;

    //miscellaneous facilities
    //SECURITY: shellCommand(...) is mighty dangerous
    virtual bool shellCommand(const char *sCommand, IXmlBaseDoc *pDoc = 0, Repository *pRepository = 0) const = 0;
    virtual void handleError(ExceptionBase& ex) const = 0;
    virtual struct tm parseDate(const char *sValue) const = 0;
    virtual const char *translateDateFormat(const char *sFormat) const = 0;
    virtual const char *fileSystemPathToXPath(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const char *sTargetType = "*", const char *sDirectoryNamespacePrefix = NULL, const bool bUseIndexes = true) const = 0;
    virtual const char *convertHTMLToXML(const char *sContent) const = 0;
    virtual void throwNativeError(const char *sParameter1 = 0, const char *sParameter2 = 0, const char *sParameter3 = 0) const = 0;

    //required capabilities indicators
    virtual iSecurityImplementation whichSecurityImplementation() const = 0;
    virtual bool canHardLink()     const = 0;
    virtual bool canSoftLink()     const = 0;
    virtual bool canTransform()    const = 0;
    virtual bool supportsXmlID()   const = 0;
    virtual bool supportsXmlRefs() const = 0;
    virtual bool hasJavascript()   const = 0;
    virtual bool hasEXSLTRegExp()  const = 0;
    virtual bool isNativeXML()     const = 0;
    virtual bool acceptsNodeParamsForTransform() const = 0;
    virtual bool canUpdateGlobalVariables() const = 0;
    virtual bool hasSecurity()     const = 0;
    virtual bool autoFixDuplicateXmlId() const = 0;
    virtual const char *name()           const = 0;
    virtual const char *toString()       const = 0;

    //extra axies
    virtual bool hasNameAxis()     const = 0;
    virtual bool hasDefaultNameAxis() const = 0;
    virtual bool hasTopAxis()      const = 0;
    virtual bool hasParentsAxis()  const = 0;
    virtual bool hasAncestorsAxis() const = 0;

    //facilities
    //uses repository:<parts> + name::<last_part>
    //re-entrant conversion and utility
    virtual bool maybeXPath(const IXmlQueryEnvironment *pQE, const char *sText, const size_t iLen = 0) const = 0;
    virtual bool maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE, const char *sText) const = 0;
    virtual bool isAttributeXPath(const char *sText, const size_t iLen = 0) const = 0;
    virtual bool maybeNamespacedXPath(const char *sText, const size_t iLen = 0) const = 0;
    virtual bool textEqualsTrue(const char *sText) const = 0; //yes, true, on, 1
    virtual const char *xml_element_name(const char *sInput, const bool bAllowNamespace = false, const bool bMakeLowerCase = false) const = 0; //create an element name from a string
    virtual char *escape(  const char *sString) const = 0; //c escaping for string literals
    virtual char *unescape(const char *sString) const = 0; //c un-escaping for string literals
    virtual const char *escapeForCDATA(const char *sString) const = 0;
    virtual const char *normalise(const char *sString) const = 0; //replace \r and \n with characters
    virtual const char *nodeTypeXPath(const iDOMNodeTypeRequest iNodeTypes) const = 0;
    
    //parsing facilities
    //https://www.w3.org/TR/REC-xml/#NT-Name
    virtual bool isNameChar(char c) const = 0;
    virtual bool isNameStartChar(char c) const = 0;

    //factory functions
    //Factory Method: http://en.wikipedia.org/wiki/Factory_method_pattern
    virtual IXmlBaseDoc *factory_document(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const = 0;
    virtual IXslDoc     *factory_stylesheet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sAlias) const = 0;

    virtual IXslTransformContext     *factory_transformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseDoc *pSourceDoc, const IXslDoc *pStylesheet) const = 0;
    virtual IXslXPathFunctionContext *factory_xpathContext(    const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const = 0;

    virtual IXmlArea     *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE) const = 0;
    virtual IXmlArea     *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pSourceNodes, const IXmlQueryEnvironment *pQE, const IXmlNodeMask *pNodeMask) const = 0;
    virtual IXmlArea     *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, XmlNodeList<const IXmlBaseNode> *pDirectMaskNodes) const = 0;
    virtual IXmlNodeMask *factory_nodeMask(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeMask, const char *sNodeMaskType = 0) const = 0;
  };
}

#endif
