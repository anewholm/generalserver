//platform agnostic file
#ifndef _QUERYENVIRONMENT_H
#define _QUERYENVIRONMENT_H

#include "IXml/IXmlQueryEnvironment.h"   //direct inheritance
#include "IXml/IXmlSecurityContext.h"    //enums
#include "GrammarContext.h"         //direct member
#include "XPathProcessingContext.h" //direct member

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
  //-------------------------------------------------------------------------------- QueryEnvironment
  //---------------------------------------------------------------------------------------
  class QueryEnvironment: virtual public MemoryLifetimeOwner, implements_interface IXmlQueryEnvironment {
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

    //each has a param member which points to this pQueryEnvironment
    IXmlSecurityContext  *m_pSec;           //LibXML2: ctxt->xfilter  ->param = pQueryEnvironment
    IXmlTriggerContext   *m_pTrigger;       //LibXML2: ctxt->xtrigger ->param = pQueryEnvironment
    IXmlMaskContext      *m_pMask;
    IXmlGrammarContext   *m_pGrammar;       //LibXML2: ctxt->gp       ->param = pQueryEnvironment
    IXmlXPathProcessingContext *m_pXPathProcessingContext;

    //stateless, with no point back
    const IXmlBaseDoc    *m_pSingularDocumentContext; //this QE will only operate on this document
    IXslModuleManager    *m_pEMO;           //LibXML2: ctxt->emo = pQueryEnvironment
    IXmlDebugContext     *m_pDebug;
    IXmlProfiler         *m_pProfiler;      //owner for query profiling and memory management (is-a MemoryLifetimeOwner)

    //output components
    //lazy initialised (non-const), static, singleton, server freed
    //can be blank new ones, or set by a parent transform
    //may contain pointers to parent objects like transformContext()
    IXslTransformContext *m_pTransformContext; //set during transforms for use in child-routines (EMO) and XSLT $var resolution
    XmlNodeList<const IXmlBaseNode> *m_pNodes; //external pointer because it is non-const

    //ownership
    const bool m_bSecOurs;
    bool m_bInherited;
    //m_pTrigger NEVER_OURS
    //m_pEMO     NEVER_OURS

  protected:
    //--------------------------------------- construction
    friend class Database; //only the Database can create this QueryEnvironments
    //fresh QE from the Database
    QueryEnvironment(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      IXmlProfiler *pProfiler,
      const IXmlBaseDoc *pSingularDocumentContext,
      
      IXslModuleManager    *pEMO,
      IXmlSecurityContext  *pSec,
      IXmlTriggerContext   *pTrigger,
      IXmlMaskContext      *pMask,
      IXmlDebugContext     *pDebug,
      IXmlGrammarContext   *pGrammar,

      IXslTransformContext *pTransformContext = 0, //indicates inherited instanciation in fact...
      bool bInherited = false
    );
    ~QueryEnvironment();

  public:
    QueryEnvironment(const QueryEnvironment &qe2); //exception passing only
    IXmlQueryEnvironment *inherit(IXslDoc *pNewStylesheet = 0) const; //with new / inherited transform context
    IXmlQueryEnvironment *inherit(IXslModuleManager *pNewEMO) const; //REQUIRED: with new EMO
    bool inherited() const; //is this an inherited environment
    const char *toString() const;
    void addNode(const IXmlBaseNode *pNode) const {m_pNodes->push_back(pNode);}
    XmlNodeList<const IXmlBaseNode> *nodes() const {return m_pNodes;}
    void reportNodes(const XmlNodeList<const IXmlBaseNode> *pNodes) const;

    //--------------------------------------- environment components during construction and setup
    //not-const: these are used during setup of the QueryEnvironments
    IXmlSecurityContext  *securityContext()  const; //for logging in/out operations and masquerading
    IXmlTriggerContext   *triggerContext()   const; //for enable / disable triggers
    IXmlMaskContext      *maskContext()      const; //for independent control of node scope
    IXmlGrammarContext *grammarContext()   const; //suggest STATIC 
    IXmlXPathProcessingContext *xpathProcessingContext() const; //suggest STATIC 
    IXslModuleManager    *emoContext()       const; //extra XSL functions
    IXmlDebugContext     *debugContext()     const; //for stepping through the transforms, xpath, etc.
    IXmlDebugContext     *debugContext(IXmlDebugContext *pDebug);
    //set during transforms for use in child-routines (EMO) and XSLT $var resolution
    IXslTransformContext *transformContext() const; //only created via inheritance
    IXslXPathFunctionContext *newXPathContext(const IXmlBaseNode *pCurrentNode = 0, const char *sXPath = 0) const;

    //--------------------------------------- generic environment uses (conveinience)
    const IXmlLibrary *xmlLibrary()      const; //useful for IXmlBaseDoc creation in application layer

    //--------------------------------------- pre-execution requirements
    //singularDocumentContext() is the only document that this trigger will run on
    //create another trigger if triggers are to be run on another document
    const IXmlBaseDoc   *singularDocumentContext() const;
    bool                 isMultiDocumentEnvironment() const {return false;}
    IXmlProfiler *profiler()           const;

    //--------------------------------------- debug
    void setXSLTTraceFlags(IXmlLibrary::xsltTraceFlag iFlags) const;
    void clearXSLTTraceFlags() const;
    void setXMLTraceFlags(IXmlLibrary::xmlTraceFlag iFlags) const;
    void clearXMLTraceFlags() const;
    const char *currentParentRoute() const;
    const char *currentXSLCommandXPath() const;
    const char *currentSourceNodeXPath() const;
    
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
