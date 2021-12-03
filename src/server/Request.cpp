//platform agnostic file
#include "Request.h"

#include "RegularX.h"
#include "Service.h"
#include "Session.h"
#include "IXml/IXslNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlSecurityContext.h"
#include "Conversation.h"
#include "Response.h"
#include "Repository.h"
#include "MessageInterpretation.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "User.h"
#include "Tests.h"
#include "FileSystem.h"

#include <vector> //sendResource()
#include "Utilities/strtools.h"
#include "Utilities/container.c"
using namespace std;

namespace general_server {
  size_t Request::m_iNextRequestId = 0;
  StringMap<IXslModule::XslModuleCommandDetails> Request::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Request::m_XSLFunctions;

  Request::Request(Tests *pTest, DatabaseNode *pNode):
    MemoryLifetimeOwner(pTest, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    
    m_pConversation(0),
    m_pMI(0),

    m_iThisRequestId(0),
    m_bIsFirstRequestOnConnection(true),
    m_bClientCanXSLT(true),
    m_bForceServerXSLT(false),
    m_sResourcesAreaIdentifier(""),

    m_pQE(0),
    m_pResponse(new Response(this)),
    m_ibqe_requestManager(this, pNode->db()->document_const())
  {    
    registerXslModule(this);
    registerXslModule(m_pResponse);
  }
  
  Request::Request(DatabaseNode *pNode, Conversation *pConversation, const MessageInterpretation *pMI, const bool bIsFirstRequestOnConnection, const bool bClientCanXSLT, const bool bForceServerXSLT, const char *sResourcesAreaIdentifier):
    MemoryLifetimeOwner(pConversation, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    
    m_pConversation(pConversation),
    m_pMI(pMI),

    m_iThisRequestId(m_iNextRequestId++),
    m_bIsFirstRequestOnConnection(bIsFirstRequestOnConnection),
    m_bClientCanXSLT(bClientCanXSLT),
    m_bForceServerXSLT(bForceServerXSLT),
    m_sResourcesAreaIdentifier(sResourcesAreaIdentifier), //freed by the Conversation

    m_pQE(pConversation->queryEnvironment()->inherit(this)),
    m_pResponse(new Response(this)),
    m_ibqe_requestManager(this, db()->document_const())
  {
    assert(m_pConversation);
    assert(m_pMI);
  }

  Request::~Request() {
    //delete the request when the conversation finishes
    //so that requests can access previous requests during the conversation
    //very long conversations are the management task of the conversation XSL
    //m_pNode->removeAndDestroyNode(&m_ibqe_nodeControl);
    if (m_pResponse)   delete m_pResponse;
    if (m_pQE)         delete m_pQE;
  }

  const char  *Request::xsltModuleNamespace()    const {return NAMESPACE_REQUEST;}
  const char  *Request::xsltModulePrefix()       const {return NAMESPACE_REQUEST_ALIAS;}
  const char  *Request::xslModuleManagerName()   const {return "Request";}

  //----------------- XslModuleManager interface environment accessors
  const Conversation *Request::conversation()  const {return m_pConversation;}
  const Response     *Request::response()      const {return m_pResponse;}
  Server             *Request::server()        const {return conversation()->server();}
  const IXmlLibrary  *Request::xmlLibrary()    const {return conversation()->xmlLibrary();}
  Database           *Request::db()            const {return conversation()->db();}
  const char         *Request::sessionUUID()   const {return (conversation()->session() ? conversation()->session()->UUID() : NULL);}
  const MessageInterpretation *Request::mi()   const {return m_pMI;}

  const StringMap<IXslModule::XslModuleCommandDetails> *Request::xslCommands() const {
    //Request is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLCommands.size()) {
      //m_XSLCommands.insert("global-variable-update",    XMC(Request::xslCommand_globalVariableUpdate)); //NOT_CURRENTLY_USED
      m_XSLCommands.insert("throw",                     XMC(Request::xslCommand_throw));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Request::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("resource-ID",              XMF(Request::xslFunction_resourceID));
      m_XSLFunctions.insert("resource-match",           XMF(Request::xslFunction_resourceMatch));
      m_XSLFunctions.insert("node",                     XMF(Request::xslFunction_node));
    }
    return &m_XSLFunctions;
  }

  const bool Request::shouldServerSideXSLT() const {
    return m_bForceServerXSLT || !m_bClientCanXSLT;
  }
  
  const char *Request::process() {
    //already in our own thread
    IXmlBaseDoc  *pDataDoc          = 0,
                 *pAnalysedDataDoc  = 0;
    const char   *sML               = 0;
    StringMap<const XmlNodeList<const IXmlBaseNode>*> mParamsNodeSet;
    StringMap<const size_t> mParamsInt;
    StringMap<const char*>::const_iterator iPath;
    XmlNodeList<const IXmlBaseNode> *pvPathNodes;

    //EMO: We are an XslModuleManager
    //select each XslModule we want and ask it to register its extensions
    //which effectively registers it on this thread only
    //but also records the details of the module to find them later
    //(IXslModuleManager*) this is sent through then to the primary transform mi->responseTransform(this, ....)
    registerXslModuleManager(m_pConversation);
    registerXslModule(m_pResponse);
    registerXslModule(this);

    //set the id on the request (NOT_CURRENTLY_USED for anything);
    m_pNode->setTransientAttribute(&m_ibqe_requestManager, "id", m_iThisRequestId);

    //---------------------------------------------------- transform!
    //then transform it into an response (according to the Service)
    //this is the PRIMARY transform and subsequent output
    //it triggers other transforms during the main transform
    //this is the PRIMARY transform and subsequent output
    //node parameters
    //we always want them to exist so that the XSL does not conditionally fail
    XmlNodeList<const IXmlBaseNode> vServer(    this, server()->dbNode()->node_const());
    XmlNodeList<const IXmlBaseNode> vRequest(   this, m_pNode->node_const());
    XmlNodeList<const IXmlBaseNode> vSession(   this, m_pConversation->sessionNode());
    XmlNodeList<const IXmlBaseNode> vUser(      this, m_pConversation->userNode(), IFNOTNULL);
    XmlNodeList<const IXmlBaseNode> vService(   this, m_pConversation->service()->dbNode()->node_const());
    XmlNodeList<const IXmlBaseNode> vPrimaryStylesheet(this, m_pMI->primaryOutputTransformation()->node_const());
    XmlNodeList<const IXmlBaseNode> vMI(        this, m_pMI->dbNode()->node_const());
    
    mParamsNodeSet.insert("gs_server",      &vServer);  // /object:Server
    mParamsNodeSet.insert("gs_service",     &vService);
    mParamsNodeSet.insert("gs_session",     &vSession);
    mParamsNodeSet.insert("gs_user",        &vUser);
    mParamsNodeSet.insert("gs_request",     &vRequest);
    mParamsNodeSet.insert("gs_message_interpretation", &vMI);
    mParamsNodeSet.insert("gs_primary_stylesheet",     &vPrimaryStylesheet);
    mParamsInt.insert("gs_is_first_request_on_connection", (m_bIsFirstRequestOnConnection ? 1U : 0U));

    //MI paths from paths.xml in the MessageInterpretation
    for (iPath = m_pMI->paths()->begin(); iPath != m_pMI->paths()->end(); iPath++) {
      pvPathNodes = m_pNode->node_const()->getMultipleNodes(m_pQE, iPath->second);
      if (pvPathNodes) {
        if (pvPathNodes->size() == 0) Debug::report("path [%s] empty node set", iPath->first, rtWarning);
        mParamsNodeSet.insert(iPath->first, pvPathNodes);
      } else Debug::report("path [%s] failed to resolve", iPath->first, rtWarning);
    }

    //---------------------------------------------------- primary transform
    //DatabaseNode pRequestNode->transform()
    //primaryOutputTransformation is required by MI
    //will add trigger context for the Database
    //TODO: output in to the temp area for efficiency? and also the xpath namespace cache
    //SECURITY: would need to ensure that inter-document security is maintained. i.e. no access
    pDataDoc = m_pNode->transform(
      this, m_pMI->primaryOutputTransformation()->node_const((IXslStylesheetNode*) 0), m_pQE,
      &mParamsInt, NO_PARAMS_CHAR, &mParamsNodeSet
    );

    //---------------------------------------------------- post-document analysis
    //GDB: dynamic_cast<const LibXmlBaseDoc*>( pAnalysedDataDoc)->m_oDoc
    if (pDataDoc) {
      if (pDataDoc->loadedAndParsed()) {
        //second transform
        //required for final document-output analysis
        //input document will be small, params the same
        //EMO, security and triggers all included
        //NO_TRIGGERS because this is a separate doc
        //xmlStaticCopyNode() has already triggered everythingin the main documentcduring the transform
        //we don't want stuff happening as a result of copied attributes
        if (m_pMI->analyserOutputTransformation()) {
          pAnalysedDataDoc = pDataDoc->transform(
            this, m_pMI->analyserOutputTransformation()->node_const((IXslStylesheetNode*) 0), m_pQE,
            &mParamsInt, NO_PARAMS_CHAR, &mParamsNodeSet
          );
        } else pAnalysedDataDoc = pDataDoc;
        
        //will prepend the Response::headers
        //may include the server-side-xslt
        //by parsing the <?xml-stylesheet ...
        sML = m_pResponse->process(this, m_pQE, m_pMI, pAnalysedDataDoc, shouldServerSideXSLT());
      } else throw DocumentParseError(this, MM_STRDUP("Request output"));
    } else throw TransformFailed(this, "DataDoc 0x0");

    if (!sML) sML = MM_STRDUP("blank document");
      //throw BlankDocument(this, MM_STRDUP("Request output"));
    
    //---------------------------------------------------- checks
    //NOTE: HTTP specific for debug only
    IFDEBUG(
      if      (_STREQUAL(  sML, "blank document"))        Debug::report("[%s] response blank", m_pMI->name(), rtWarning);
      else if (_STRNEQUAL( sML, "<!DOCTYPE", 9))          Debug::report("[%s] response starts with a <!DOCTYPE", m_pMI->name(), rtWarning);
      else if (_STRNEQUAL( sML, "<?xml ", 6))             Debug::report("[%s] response starts with an <?xml decleration", m_pMI->name(), rtWarning);
      else if (_STRNEQUAL( sML, "<?xml-stylesheet ", 17)) Debug::report("[%s] response starts with an <?xml decleration", m_pMI->name(), rtWarning);
      else if (_STRNEQUAL( sML, "<html", 5))              Debug::report("[%s] response starts with <html>", m_pMI->name(), rtWarning);
      else if (_STRNEQUAL( sML, "<object:Response", 16))  Debug::report("[%s] response starts with <object:Response>", m_pMI->name(), rtWarning);
      else if (!_STRNEQUAL(sML, "HTTP", 4))               Debug::report("[%s] response does not start with HTTP protocol headers", m_pMI->name(), rtWarning);

      if (strstr(sML, " xmlns:ns_1=")) Debug::report("xmlns:ns_1 namespacing issues in output", rtWarning);
    );
    
    //---------------------------------------------------- free up
    if (pDataDoc)                       delete pDataDoc;
    if (pAnalysedDataDoc != pDataDoc)   delete pAnalysedDataDoc;

    return sML;
  }

  const char *Request::xslFunction_resourceID(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    return m_sResourcesAreaIdentifier ? MM_STRDUP(m_sResourcesAreaIdentifier) : 0;
  }

  const char *Request::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "request:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *Request::xslFunction_resourceMatch(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "request:resource-match([@attribute])";
    assert(pQE);
    assert(pXCtxt);

    const char *sResourcesAreaMatch = 0;
    const IXmlBaseNode  *pInputNode = 0;
    bool bMatch = false;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      //optional
      switch (pXCtxt->valueCount()) {
        case 1: {
          sResourcesAreaMatch = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        }
        case 0: {
          if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
            sResourcesAreaMatch = pInputNode->attributeValue(pQE, "resource-match");
          } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("attribute context node"), MM_STRDUP("no context node"), sFunctionSignature);
          break;
        }
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      bMatch = sResourcesAreaMatch
        && m_sResourcesAreaIdentifier
        && RegularX::match(sResourcesAreaMatch, m_sResourcesAreaIdentifier);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sResourcesAreaMatch) MMO_FREE(sResourcesAreaMatch);
    if (pInputNode)          delete pInputNode;

    UNWIND_EXCEPTION_THROW;

    return bMatch ? MM_STRDUP("true") : 0;
  }

  IXmlBaseNode *Request::xslCommand_throw(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //TODO: change this to allowing registration of
    // <request:throw-<class-name> parameter="" />
    // <request:throw-Up parameter="" />
    static const char *sCommandSignature = "request:throw [@native @class @parameter]"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sClass     = 0;
    const char *sParameter = 0;
    bool bNative;

    UNWIND_EXCEPTION_BEGIN {
      if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
        //optional
        sClass     = pCommandNodeType->attributeValue(pQE, "class");
        sParameter = pCommandNodeType->attributeValue(pQE, "parameter");
        bNative    = pCommandNodeType->attributeValueBoolInterpret(pQE, "native", NULL, false);

        if (bNative) pQE->xmlLibrary()->throwNativeError(sParameter);
        else if (!sClass || !*sClass) throw XSLTThrow(this, sParameter);
        else ExceptionBase::factory_throw(this, sClass, sParameter); 
        
        //code whould never arrive here 
        //because ExceptionBase::factory() will also throw an ExceptionClassRequired() if not found
        throw ExceptionClassRequired(this);
      } else throw InterfaceNotSupported(this, MM_STRDUP("const IXmlBaseNode *"));
    } UNWIND_EXCEPTION_END;

    //if we are not unwinding an exception 
    //then the UNWIND system is broken
    //it relies on the operator const bool() const {return m_pOuterEB;}
    assert(unwinder);
    
    //free up
    //if (sParameter) MMO_FREE(sParameter); //managed/freed by the Exception
    if (sClass) MMO_FREE(sClass);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Request::xslCommand_globalVariableUpdate(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result

    //inputs to this globalVariableUpdate() call
    //pCommandNode: e.g. <request:set-blah>
    //pSourceNode:  e.g. <repository:index_xsl> if the transform is currently traversing the process_main_stylesheet
    //pOutputNode:  e.g. <root>

    //attributes for this globalVariableUpdate() call
    //@name:  global variable name
    //@value: new value

    static const char *sCommandSignature = "request:global-variable-update @name @select";
    const char *sName                    = 0;
    const char *sSelect                  = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    IXslTransformContext *pCtxt          = 0;

    NOT_CURRENTLY_USED("");

    UNWIND_EXCEPTION_BEGIN {
      pCtxt = pQE->transformContext();
      assert(pCtxt);

      pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
      if (sName = pCommandNodeType->attributeValue(pQE, "name")) {
        if (sSelect = pCommandNodeType->attributeValue(pQE, "select")) {
          pCtxt->globalVariableUpdate(sName, sSelect);
        } else throw AttributeRequired(this, MM_STRDUP("select"), sCommandSignature);
      } else throw AttributeRequired(this, MM_STRDUP("name"), sCommandSignature);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sName)          MMO_FREE(sName);
    if (sSelect)        MMO_FREE(sSelect);
    //if (pCtxt) delete pCtxt;             //freed by calling method

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  void Request::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const {
    //caller freed (needs to be efficient)
    /*
    psSerialise=new string("<request>");
    //input stream
    psSerialise->append("<inputStream>");
    psSerialise->append(m_sTextStream);
    psSerialise->append("</inputStream>");
    //output analysis

    psSerialise->append("</request>");
    */
  }

  const char *Request::toString() const {
    //caller free
    const char *sXML = 0,
               *sCSI = 0;
    stringstream sOut;

    sOut << "\n------------------------------------------ new [" << m_pMI->name() << "] request:\n";
    sOut << (sXML = m_pNode->xml(&m_ibqe_requestManager)) << "\n";

    //free up
    if (sXML) MMO_FREE(sXML);
    if (sCSI) MMO_FREE(sCSI);

    return MM_STRDUP(sOut.str().c_str());
  }
}

