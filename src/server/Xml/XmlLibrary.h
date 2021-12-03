//platform agnostic file
//Abstract Factory Method: http://en.wikipedia.org/wiki/Abstract_factory_pattern
//  for creating all associated XML, XSL and XShema objects
//  parent class should instanciate the factory of its choice and then create through it
//  but can also create objects from other already instantiated objects
//interface that all XML libraries should implement
#ifndef _XMLLIBRARY_H
#define _XMLLIBRARY_H

#include "IXml/IXmlLibrary.h"  //full definition for inheritance

#include <vector>
using namespace std;

#define DATE_FORMATS_COUNT 6

namespace general_server {
  class Server;

  class XmlLibrary: implements_interface IXmlLibrary, virtual public MemoryLifetimeOwner {
    //no direct references to the base concrete classes
    //library allows creation of new concrete instances from existing ones to maintain type
    //and avoid the necessity for passing a reference to the library around
    const char *m_aDateFormats[DATE_FORMATS_COUNT];

  public:
    XmlLibrary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    ~XmlLibrary();

    virtual void threadInit() const     {}
    virtual void threadCleanup() const  {}

    void handleError(ExceptionBase& ex) const;

    //utilities
    //conversion and utility
    const char *fileSystemPathToXPath(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const char *sTargetType = "*", const char *sDirectoryNamespacePrefix = NULL, const bool bUseIndexes = true) const;
    bool maybeXPath(const IXmlQueryEnvironment *pQE, const char *sText, const size_t iLen = 0) const;
    bool maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE, const char *sText) const;
    bool isAttributeXPath(const char *sText, const size_t iLen = 0) const;
    bool maybeNamespacedXPath(const char *sText, const size_t iLen = 0) const;
    bool textEqualsTrue(const char *sText) const;           //yes, true, on, 1
    const char *xml_element_name(const char *sInput, const bool bAllowNamespace = false, const bool bMakeLowerCase = false) const;
    const char *normalise(const char *sString) const;       //replace \r and \n with characters
    const char *escapeForCDATA(const char *sString) const;
    struct tm parseDate(const char *sValue) const;
    const char *translateDateFormat(const char *sFormat) const;
    xsltTraceFlag parseXSLTTraceFlags(const char *sFlags) const;
    xmlTraceFlag  parseXMLTraceFlags( const char *sFlags) const;
    const char *nodeTypeXPath(const iDOMNodeTypeRequest) const;
    
    bool isNameChar(char c) const;
    bool isNameStartChar(char c) const;

    //required capabilities indicators
    virtual iSecurityImplementation whichSecurityImplementation() const {return SECURITY_IMPLEMENTATION_NONE;}
    virtual bool canHardLink()     const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool canSoftLink()     const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool canTransform()    const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool supportsXmlID()   const {return CAPABILITY_IMPLEMENTED;} //default to normal situation
    virtual bool supportsXmlRefs() const {return CAPABILITY_IMPLEMENTED;} //default to normal situation
    virtual bool hasJavascript()   const {return CAPABILITY_NOT_IMPLEMENTED;} //default to LibXSLT
    virtual bool hasEXSLTRegExp()  const {return hasJavascript();}
    virtual bool isNativeXML()     const {return CAPABILITY_XML_NATIVE;}
    virtual bool acceptsNodeParamsForTransform() const {return CAPABILITY_IMPLEMENTED;}
    virtual bool canUpdateGlobalVariables() const {return CAPABILITY_IMPLEMENTED;}
    virtual bool autoFixDuplicateXmlId() const {return CAPABILITY_IMPLEMENTED;}
    virtual bool hasSecurity()     const {return CAPABILITY_IMPLEMENTED;}
    virtual bool hasNameAxis()     const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool hasTopAxis()      const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool hasParentsAxis()  const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual bool hasAncestorsAxis() const {return CAPABILITY_NOT_IMPLEMENTED;}
    virtual const char *toString()       const;

    IXmlArea *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE) const;
    virtual IXmlArea *factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const XmlNodeList<const IXmlBaseNode> *pSourceNodes, const IXmlQueryEnvironment *pQE, const IXmlNodeMask *pNodeMask) const = 0;
  };
}

#endif
