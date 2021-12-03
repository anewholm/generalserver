//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXSLTRANSFORMCONTEXT_H
#define _IXSLTRANSFORMCONTEXT_H

#include "define.h"
#include "IXml/IXmlQueryEnvironment.h"          //enums
#include "IXml/IXmlQueryEnvironmentComponent.h" //inheritance
#include "MemoryLifetimeOwner.h"                //direct inheritance

namespace general_server {
  interface_class IXmlLibrary;
  interface_class IXslDoc;
  interface_class IXslCommandNode;
  interface_class IXslModuleManager;
  class Server;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- IXslTransformContext ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  interface_class IXslTransformContext: implements_interface IXmlQueryEnvironmentComponent, implements_interface IMemoryLifetimeOwner {
    //construction: only IXmlLibrary should create these
  public:
    enum iHardlinkPolicy {
      includeAllHardlinks = 0,
      suppressRecursiveHardlinks,
      suppressRepeatHardlinks,
      suppressExactRepeatHardlinks,
      suppressAllHardlinks,
      disableHardlinkRecording
    };

    virtual ~IXslTransformContext() {} //virtual public destructors allow proper destruction of derived object

    //environment access
    virtual const IXmlQueryEnvironment *ownerQE() const = 0;
    virtual IXslTransformContext *inherit(const IXslDoc *pStylesheet = 0) const = 0;
    virtual IXslTransformContext *clone_with_resources() const = 0;

    //documents and nodes
    //commandNode() will throw InterfaceNotSupported if the current instruction is not an XSL OR extension-prefix
    virtual const IXmlBaseNode    *sourceNode(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner)  const = 0; //input XML document
    virtual const IXslCommandNode *commandNode( const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //stylesheet and current xsl command OR other extension-prefix namespace
    virtual const IXmlBaseNode    *literalCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //xsl command lists include literals
    virtual const IXmlBaseNode    *instructionNode(   const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //stylesheet and current instruction, INCLUDING direct output nodes, e.g. html:div
    virtual IXmlBaseNode          *outputNode(  const IMemoryLifetimeOwner *pMemoryLifetimeOwner)  const = 0; //output XML doc
    virtual const IXslCommandNode *templateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0; //stylesheet and current xsl template
    virtual const IXmlBaseDoc     *sourceDoc()  const = 0; //XML being transformed
    virtual const IXslDoc         *commandDoc() const = 0; //compiled read-only stylesheet XSL copy

    //error reporting
    virtual const char *currentParentRoute() const = 0;
    virtual const char *currentSourceNodeXPath() const = 0;
    virtual const char *currentXSLCommandXPath() const = 0;
    virtual const char *currentXSLTemplateXPath() const = 0;
    virtual StringMap<const char*> *globalVars() const = 0;
    virtual StringMap<const char*> *localVars()  const = 0;
    virtual XmlNodeList<const IXmlBaseNode> *templateStack(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) const = 0;
    virtual const char *currentModeName() const = 0;

    //setup for a transform
    virtual void setMode(const char *sWithMode = 0) = 0;
    virtual void setStartNode( const IXmlBaseNode *pStartNode = 0)  = 0;
    virtual void addParams(const StringMap<const size_t> *pParamsInt) = 0;
    virtual void addParams(const StringMap<const char*> *pParamsChar) = 0;
    virtual void addParams(const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet) = 0;
    virtual IXmlBaseDoc *transform() const = 0;
    virtual void transform(const IXmlBaseNode *pInitialOutputNode) const = 0;
    virtual void handleError(const char *sErrorMessage) = 0;

    //xsl environment functionality
    virtual void globalVariableUpdate(const char *sName, const char *sValue) = 0;
    virtual void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) = 0;
    virtual IXmlLibrary::xsltTraceFlag traceFlags() const = 0;
    virtual void clearXSLTTraceFlags() = 0;
    virtual iHardlinkPolicy hardlinkPolicy(const char *sHardlinkPolicy) = 0;
    virtual iHardlinkPolicy hardlinkPolicy(const iHardlinkPolicy iHardlinkPolicy) = 0;
    virtual void continueTransformation(const IXmlBaseNode *pOutputNode = NULL) const = 0;
  };
}

#endif
