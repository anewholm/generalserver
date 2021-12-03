//platform agnostic file
#ifndef _COMPILATIONENVIRONMENT_H
#define _COMPILATIONENVIRONMENT_H

#include "IXml/IXmlQueryEnvironment.h" //direct inheritance
#include "IXml/IXmlSecurityContext.h"  //enums
#include "GrammarContext.h"       //direct member

#include <vector>
using namespace std;

#define INHERITED true

namespace general_server {
  class Database;
  class DatabaseNode;
  class Profiler;

  //these are included in the Database.cpp
  interface_class IXmlDoc;
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXslCommandNode;     //XSL extensions
  interface_class IXmlTriggerContext;
  interface_class IXmlMaskContext;

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class CompilationEnvironment: virtual public MemoryLifetimeOwner, implements_interface IXmlQueryEnvironment {
    //this is an IXml* level object. Database uses it to make requests to IXml
    //contains settings and objects for a specific query to the IXmlBaseDoc
    //rationalises all calls to the IXmlBaseDoc
    //includes:
    //  security and its status (object also overrideable)
    //  trigger context and instructions (override or disable the Database trigger system)
    //  extended modules and functions during call (override the Database)
    //can create:
    //  xpath execution context
    //  transform context
    //the various functions require compiled privileges (RootSecurityContext) passed in

    //components sent in to XML library
    vector<IXmlQueryEnvironmentComponent*> m_vComponents;
    const IXmlQueryEnvironment *m_pSourceQE;
    IXslDoc *m_pMainXSLDoc;

    //each has a param member which points to this pQueryEnvironment
    IXmlSecurityContext  *m_pSec;           //LibXML2: ctxt->xfilter  ->param = pQueryEnvironment
    IXmlTriggerContext   *m_pTrigger;       //LibXML2: ctxt->xtrigger ->param = pQueryEnvironment
    IXmlGrammarContext   *m_pGrammar;       //LibXML2: ctxt->gp       ->param = pQueryEnvironment

    //stateless, with no point back
    IXmlDebugContext     *m_pDebug;

    //output components
    //lazy initialised (non-const), static, singleton, server freed
    //can be blank new ones, or set by a parent transform
    //may contain pointers to parent objects like transformContext()
    IXslTransformContext *m_pTransformContext; //set during transforms for use in child-routines (EMO) and XSLT $var resolution

  public:
    //--------------------------------------- construction
    friend class Database; //only the Database can create this QueryEnvironments
    //fresh QE from the Database
    CompilationEnvironment(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, IXslDoc *pMainXSLDoc);
    ~CompilationEnvironment();
    CompilationEnvironment(const CompilationEnvironment &qe2); //exception passing only

    IXmlQueryEnvironment *inherit(IXslDoc *pNewStylesheet ATTRIBUTE_UNUSED = 0) const {return 0;} //with new / inherited transform context
    IXmlQueryEnvironment *inherit(IXslModuleManager *pNewEMO ATTRIBUTE_UNUSED)  const {return 0;} //REQUIRED: with new EMO
    bool inherited() const {return false;}
    const char *toString() const;
    void addNode(const IXmlBaseNode *pNode ATTRIBUTE_UNUSED) const {}
    XmlNodeList<const IXmlBaseNode> *nodes() const {return 0;}
    void reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes ATTRIBUTE_UNUSED) const {}

    //--------------------------------------- environment components during construction and setup
    //not-const: these are used during setup of the QueryEnvironments
    //PERFORMANCE: borrowed, attached and then given back to the original source QE
    IXmlSecurityContext  *securityContext()  const {return m_pSec;} //for logging in/out operations and masquerading
    IXmlTriggerContext   *triggerContext()   const {return m_pTrigger;} //for enable / disable triggers
    IXmlMaskContext      *maskContext()      const {return 0;} //for independent control of node scope
    IXmlGrammarContext   *grammarContext()   const {return m_pGrammar;}
    IXmlXPathProcessingContext *xpathProcessingContext() const {return 0;}
    IXslModuleManager    *emoContext()       const {return m_pSourceQE->emoContext();} //extra XSL functions
    IXmlDebugContext     *debugContext()     const {return m_pDebug;} //for stepping through the transforms, xpath, etc.
    IXmlDebugContext     *debugContext(IXmlDebugContext *pDebug ATTRIBUTE_UNUSED) {return 0;}
    //set during transforms for use in child-routines (EMO) and XSLT $var resolution
    IXslTransformContext *transformContext() const {return m_pTransformContext;} //only created via inheritance
    IXslXPathFunctionContext *newXPathContext(const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const;

    //--------------------------------------- generic environment uses (conveinience)
    const IXmlLibrary *xmlLibrary()      const {return m_pSourceQE->xmlLibrary();} //useful for IXmlBaseDoc creation in application layer

    //--------------------------------------- pre-execution requirements
    //singularDocumentContext() is the only document that this trigger will run on
    //create another trigger if triggers are to be run on another document
    const IXmlBaseDoc   *singularDocumentContext() const;
    bool                 isMultiDocumentEnvironment() const {return true;}
    IXmlProfiler *profiler()           const {return 0;}

    //--------------------------------------- debug
    void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const;
    void clearXSLTTraceFlags() const;
    void setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const;
    void clearXMLTraceFlags() const;
    const char *currentParentRoute()     const {return 0;}
    const char *currentXSLCommandXPath() const {return 0;}
    const char *currentSourceNodeXPath() const {return 0;}
    
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
