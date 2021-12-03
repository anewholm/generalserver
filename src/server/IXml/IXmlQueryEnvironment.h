//platform agnostic file
#ifndef _IXMLQUERYENVIRONMENT_H
#define _IXMLQUERYENVIRONMENT_H

#include "define.h"
#include "IXml/IXmlLibrary.h"
#include "MemoryLifetimeOwner.h" //direct inheritance
#include "IReportable.h"         //direct inheritance

#include <vector>
using namespace std;

namespace general_server {
  interface_class IXslTransformContext;
  interface_class IXslXPathFunctionContext;
  interface_class IXslDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;
  interface_class IXmlArea;

  interface_class IXmlQueryEnvironment;
  interface_class IXmlSecurityContext;
  interface_class IXmlTriggerContext;
  interface_class IXmlMaskContext;
  interface_class IXmlGrammarContext;
  interface_class IXmlXPathProcessingContext;
  interface_class IXmlDebugContext;
  interface_class IXslModuleManager;
  interface_class IXmlProfiler;

  interface_class IXmlQueryEnvironment: implements_interface IMemoryLifetimeOwner, implements_interface IReportable {
    //--------------------------------------- queries
    //IXmlQueryEnvironment does not know about the Database layer
  public:
    //http://en.wikipedia.org/wiki/File_system_permissions
    //the underlying library must return the type of access it wants to check
    //these are the same as our xmltrigger.h defines:
    //  define TRIGGER_STAGE_*
    //  define TRIGGER_OP_*
    //  define TRIGGER_*

    enum accessOperation {
      OP_EXECUTE = 1,
      OP_WRITE   = 2,
      OP_READ    = 4,
      OP_ADD     = 8
    };

    enum accessStage {
      STAGE_BEFORE = 0,
      STAGE_AFTER = 1
    };

    enum accessResult {
      RESULT_INCLUDE = 1,
      RESULT_DENY = 0
    };

    enum grammarResult {
      GRAMMAR_OK = 0,
      GRAMMAR_UNHANDLED = 1
    };
    
    enum xprocessorPhase {
      NODE_TEST_NONE,
      NODE_TEST_TYPE,
      NODE_TEST_PI,
      NODE_TEST_ALL,
      NODE_TEST_NS,
      NODE_TEST_NAME
    };

    enum xprocessorCompilationPhase {
      TEMPLATE_GROUP_NAMES = 1 //compilation phase grouping
    };

    enum xprocessorResult {
      XP_TEST_HIT_NO,
      XP_TEST_HIT,
      XP_TEST_HIT_NS
    };

    virtual ~IXmlQueryEnvironment() {} //virtual destruction

    //--------------------------------------- transforms
    //transforms are update requests because of EMOs and write interaction
    //transform context CANNOT be added to an existing QE. they can only be inherited to a new QE
    //is added during transform() pQEInherited = pQE->inherit(IXslDoc)
    //an IXslDoc parameter indicates that a transform context is required for the given IXslDoc
    //wrapping of xmlDoc and xmlNode by LibXslModule for passing to application layer
    virtual IXmlQueryEnvironment *inherit(IXslDoc *pNewStylesheet = 0) const = 0; //REQUIRED: with new / inherited transform context
    virtual IXmlQueryEnvironment *inherit(IXslModuleManager *pNewEMO)  const = 0; //REQUIRED: with new EMO
    virtual bool inherited() const = 0; //is this an inherited environment
    virtual IXslTransformContext *transformContext() const = 0; //REQUIRED: new / inherited transform context, if any
    virtual void addNode(const IXmlBaseNode *pNode) const = 0;
    virtual const XmlNodeList<const IXmlBaseNode> *nodes() const = 0;
    virtual void reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes) const = 0;

    //--------------------------------------- environment components during construction and setup
    //not-const: these are used during setup of the QueryEnvironments
    //all MUST implements_interface IXmlQueryEnvironmentComponent(Volatile/Const)
    virtual IXmlSecurityContext  *securityContext()  const = 0; //REQUIRED: for logging in/out operations and masquerading
    virtual IXmlTriggerContext   *triggerContext()   const = 0; //OPTIONAL: for enable / disable triggers
    virtual IXmlMaskContext      *maskContext()      const = 0; //OPTIONAL: for enable / disable mask
    virtual IXmlGrammarContext   *grammarContext()   const = 0; //suggest STATIC
    virtual IXmlXPathProcessingContext *xpathProcessingContext() const = 0; //suggest STATIC 
    virtual IXslModuleManager    *emoContext()       const = 0; //OPTIONAL: module manager for extended functions / elements
    virtual IXmlDebugContext     *debugContext()     const = 0; //OPTIONAL; debugging happens every step, not just transforms
    virtual IXmlDebugContext     *debugContext(IXmlDebugContext *pDebug) = 0;
    //attach the TransactionXML object that cause the change to occur
    //TODO: virtual IXmlArea         *affectedArea() const = 0; //OPTIONAL: what area was affected
    //use the xmlLibrary() to create blank xpath contexts
    //or here to create one related to the potential transform context
    virtual IXslXPathFunctionContext *newXPathContext(const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const = 0;
    virtual const IXmlLibrary    *xmlLibrary()                 const = 0; //REQUIRED: useful for IXmlBaseDoc creation in application layer
    virtual const IXmlBaseDoc    *singularDocumentContext()    const = 0; //REQUIRED: copying namespace cache and future
    virtual bool                  isMultiDocumentEnvironment() const = 0;
    virtual IXmlProfiler         *profiler()    const = 0;

    //--------------------------------------- debug
    virtual void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const = 0;
    virtual void clearXSLTTraceFlags() const = 0;
    virtual void setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const = 0;
    virtual void clearXMLTraceFlags() const = 0;
    virtual const char *currentParentRoute() const = 0;
    virtual const char *currentXSLCommandXPath() const = 0;
    virtual const char *currentSourceNodeXPath() const = 0;
  };
}

#endif
