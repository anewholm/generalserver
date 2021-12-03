//platform agnostic file
#include "Session.h"

#include "IXml/IXslNode.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "User.h"

#include "Utilities/container.c"
using namespace std;

#ifdef WITHOUT_SESSION_XPATH
#define CHECK_WITH_SESSION_XPATH assert(false)
#else
#define CHECK_WITH_SESSION_XPATH assert(true)
#endif

namespace general_server {
  StringMap<Session*> Session::m_mSessions; //Session::m_mSessions is deleted by the ~Server()
  StringMap<IXslModule::XslModuleCommandDetails>  Session::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Session::m_XSLFunctions;

  Session *Session::factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pSessionsCollectionNode, const char *sSessionID = 0) {
    //http://en.wikipedia.org/wiki/Multiton_pattern
    Session *pSession = 0;
    StringMap<Session*>::iterator iSession;
    DatabaseNode *pSessionDatabaseNode;
    const char *sUUID = 0;

    //check for an existing Session object for this sSessionID
    //when there is a sSessionID this should ALWAYS be true because sSessionID is created always with a Session object
    if (sSessionID && *sSessionID) {
      iSession = m_mSessions.find(sSessionID);
      if (iSession != m_mSessions.end()) {
        //we have an existing stateful Session object
        pSession = iSession->second;
      }
    }

    if (!pSession) {
      if (pSessionsCollectionNode) {
        //no existing session: create a new one
        XmlAdminQueryEnvironment m_ibqe_sessionManager(pMemoryLifetimeOwner, pSessionsCollectionNode->db()->document_const());
        pSessionDatabaseNode = pSessionsCollectionNode->factory_node(pSessionsCollectionNode->createTransientChildElement(&m_ibqe_sessionManager, "Session", NULL, NAMESPACE_OBJECT));
        //there should NEVER be a sSessionID here because it should have been found in the map
        if (!sSessionID) sSessionID = sUUID = newUUID();

        //TODO: use xml:id, instead of UUID?
        //UUID is stored in m_mSessions now
        //this attribute is USED by the XSL layer to return the UUID to the client
        pSessionDatabaseNode->setTransientAttribute(&m_ibqe_sessionManager, "guid", sSessionID);
        Debug::report("created a new session [%s] for optional return to client for next time", sSessionID);

        pSession = new Session(pMemoryLifetimeOwner, pSessionDatabaseNode);
        m_mSessions.insert(MM_STRDUP(sSessionID), pSession);

        //TODO: drop sessions off the end if more than 1000
        //...
      } else throw SessionsCollectionRequired(pMemoryLifetimeOwner);
    }

    //free up
    if (sUUID) MM_FREE(sUUID);

    return pSession;
  }

  Session::Session(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pNode): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner, pNode),
    GeneralServerDatabaseNodeServerObject(pNode), 
    m_ibqe_sessionManager(this, pNode->db()->document_const()), 
    m_pUser(0) 
  {}

  Session::~Session() {
    vector_element_destroy(m_vSavedNodeLists); //local-variable
    //if (m_pNode) delete m_pNode; //will be deleted by GeneralServerDatabaseNodeServerObject
    m_pNode->removeAndDestroyNode(&m_ibqe_sessionManager, "~Session() being deleted with node");
    //if (m_pUser) delete m_pUser; //Multiton_pattern in use here. another session may be using this User object
  }

  const char  *Session::xsltModuleNamespace()    const {return NAMESPACE_SESSION;}
  const char  *Session::xsltModulePrefix()       const {return NAMESPACE_SESSION_ALIAS;}
  const char  *Session::xslModuleManagerName()   const {return "Session";}

  const StringMap<IXslModule::XslModuleCommandDetails> *Session::xslCommands() const {
    //Session is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("save-node-set",      XMC(Session::xslCommand_saveNodeSet));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Session::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("node",                    XMF(Session::xslFunction_node));
      m_XSLFunctions.insert("node-set",                XMF(Session::xslFunction_nodeSet));
      m_XSLFunctions.insert("save-node-set",           XMF(Session::xslFunction_saveNodeSet));
      m_XSLFunctions.insert("save-literal-node",       XMF(Session::xslFunction_saveLiteralNode));
    }
    return &m_XSLFunctions;
  }

  void Session::clearSessions() {
    Debug::report("clearing Multiton_pattern Session objects [%s]", m_mSessions.size());
    m_mSessions.elements_free_delete();
  }

  void Session::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace) const {
    return m_pNode->setTransientAttribute(pQE, sName, sValue, sNamespace);
  }

  void Session::setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue, const char *sNamespace) const {
    return m_pNode->setTransientAttribute(pQE, sName, bValue, sNamespace);
  }

  const IXmlBaseNode *Session::node_const() const {return m_pNode->node_const();}

  User *Session::user() const {
    return m_pUser;
  }
  
  const char *Session::UUID() const {
    const XmlAdminQueryEnvironment ibqe_session(this, document());
    return m_pNode->attributeValue(&ibqe_session, "guid");
  }

  User *Session::user(User *pUser) {
    assert(pUser);
    if (m_pUser && m_pUser != pUser) delete m_pUser;
    return m_pUser = pUser;
  }

  void Session::deleteUser() {
    if (m_pUser) delete m_pUser;
    m_pUser = 0;
  }

  void Session::compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) {
    
  }
  
  const char *Session::toString() const {
    return MM_STRDUP("Session");
  }
  
  IXmlBaseNode *Session::createErrorNode(ExceptionBase& eb) {
    //users and command nodes are NOT transient
    StringMap<const char *> *pmErrorDetails;
    StringMap<const char *>::iterator iDetail;
    DatabaseNode *pXSLCommandDatabaseNode              = 0;
    const char *sXMLID                     = 0;
    IXmlBaseNode *pErrorRepositoryNode     = 0,
                 *pErrorNode               = 0;

    pErrorRepositoryNode = m_pNode->getOrCreateTransientChildElement(&m_ibqe_sessionManager, "errors", NULL, NAMESPACE_REPOSITORY);
    pErrorNode           = pErrorRepositoryNode->createChildElement(&m_ibqe_sessionManager, "error", NULL, NAMESPACE_GS);
    delete pErrorRepositoryNode;

    //link XSL error node in to the error
    //and its xpath
    if (sXMLID = eb.contextXPath()) {
      pErrorNode->setAttribute(&m_ibqe_sessionManager, "xsl_xml_id", sXMLID);
      if (pXSLCommandDatabaseNode = db()->nodeFromID(&m_ibqe_sessionManager, sXMLID)) {
        pErrorNode->softlinkChild(&m_ibqe_sessionManager, pXSLCommandDatabaseNode->node_const());
      }
    }

    //get variables and populate attributes
    if (pmErrorDetails = eb.variables()) {
      for (iDetail = pmErrorDetails->begin(); iDetail != pmErrorDetails->end(); iDetail++) {
        pErrorNode->setAttribute(&m_ibqe_sessionManager, iDetail->first, iDetail->second);
        //TODO: update the actual XSL command node with the problem history?
        //if (pXSLCommandDatabaseNode) pXSLCommandDatabaseNode->setAttribute(&m_ibqe_sessionManager, iDetail->first, iDetail->second);
      }
    }

    //free up
    //if (sXMLID)                   MMO_FREE(sXMLID); //owner object freed
    if (pmErrorDetails)           map_element_free(pmErrorDetails); //first is constant
    //if (pErrorNode)               delete pErrorNode; //returned

    return pErrorNode;
  }

  const char *Session::xslFunction_nodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "session:node-set(num)";
    assert(pQE);
    assert(pXCtxt);
    CHECK_WITH_SESSION_XPATH; //if WITHOUT_SESSION_XPATH is 1 then we should not be asking this

    size_t iIndex = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 1: {
          iIndex = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack();
          break;
        }
        default: throw XPathTooFewArguments(this, sFunctionSignature);
      }

      if (iIndex < m_vSavedNodeLists.size()) {
        //GDB: (const LibXmlBaseNode*) (*pNodes)->first()
        *pNodes = m_vSavedNodeLists[iIndex]->clone_with_resources();
      } else throw SessionNodeSetOutOfRange(this);
    } UNWIND_EXCEPTION_END;

    UNWIND_EXCEPTION_THROW;

    return 0;
  }
  
  size_t Session::saveNodeSet(XmlNodeList<const IXmlBaseNode> *pvSelect) {
    size_t iIndex = 0;
    vector<XmlNodeList<const IXmlBaseNode>* >::const_iterator iSavedNodeSet;

    CHECK_WITH_SESSION_XPATH; //if WITHOUT_SESSION_XPATH is 1 then we should not be asking this
    
    iSavedNodeSet = m_vSavedNodeLists.begin();
    while (iSavedNodeSet != m_vSavedNodeLists.end() && !pvSelect->is(*iSavedNodeSet, EXACT_MATCH)) iSavedNodeSet++;
    //index (before insert)
    iIndex = iSavedNodeSet - m_vSavedNodeLists.begin();
    //save the set if not found
    if (iSavedNodeSet == m_vSavedNodeLists.end()) m_vSavedNodeLists.push_back(pvSelect->clone_with_resources());

    return iIndex;
  }

  const char *Session::xslFunction_saveLiteralNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //UNIQUE: this will scan the saved sets for a matching set first
    //NOTE: the literal node will be in a compiled copy of the original xsl:stylesheet
    //it will NOT be in the MAIN document
    //thus it will trigger a NULL document.ids error
    //so, we need to find this literal in the original document
    static const char *sFunctionSignature = "session:save-literal-node()";
    assert(pQE);
    assert(pXCtxt);
    CHECK_WITH_SESSION_XPATH; //if WITHOUT_SESSION_XPATH is 1 then we should not be asking this

    //TODO: replaced with a generic conversation:xpath-to-command-node
    NOT_CURRENTLY_USED("needs to be replaced with a generic conversation:xpath-to-command-node");
    
    const char *sXMLID = 0,
               *sIndex = 0;
    const IXmlBaseNode *pLiteralCommandNode              = 0,
                       *pLiteralCommandNodeInOriginalDoc = 0;
    XmlNodeList<const IXmlBaseNode> *pvSelect            = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pLiteralCommandNode = pXCtxt->literalCommandNode(pQE)) { //pQE as MM
        if (sXMLID = pLiteralCommandNode->xmlID(pQE)) {
          if (pLiteralCommandNodeInOriginalDoc = pQE->singularDocumentContext()->nodeFromID(pQE, sXMLID)) {
            pvSelect = new XmlNodeList<const IXmlBaseNode>(pLiteralCommandNodeInOriginalDoc); //pQE as MM
            sIndex   = itoa(saveNodeSet(pvSelect), "`"); //`45
          } else throw NodeNotFound(this, sFunctionSignature);
        } else throw ContextNodeMustHaveXMLID(this, sFunctionSignature);
      } else throw XPathContextNodeRequired(this, sFunctionSignature);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pLiteralCommandNode) delete pLiteralCommandNode;
    if (sXMLID)              MMO_FREE(sXMLID);
    if (pvSelect)            delete pvSelect; //NOT consumed by saveNodeSet
    //if (sIndex)              MMO_FREE(sIndex); //returned
    //if (pLiteralCommandNodeInOriginalDoc) delete pLiteralCommandNodeInOriginalDoc; //consumed by saveNodeSet
    
    UNWIND_EXCEPTION_THROW;

    return sIndex;
  }

  const char *Session::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "session:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *Session::xslFunction_saveNodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //UNIQUE: this will scan the saved sets for a matching set first
    static const char *sFunctionSignature = "session:save-node-set(node-set)";
    assert(pQE);
    assert(pXCtxt);
    CHECK_WITH_SESSION_XPATH; //if WITHOUT_SESSION_XPATH is 1 then we should not be asking this
    
    const char *sIndex = 0;
    XmlNodeList<const IXmlBaseNode> *pvSelect = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 1: {
          pvSelect = (XmlNodeList<const IXmlBaseNode>*) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          break;
        }
        case 0: {
          pvSelect = new XmlNodeList<const IXmlBaseNode>(pXCtxt->contextNode(pQE)); //pQE as MM
          break;
        }
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      if (pvSelect) {
        sIndex = itoa(saveNodeSet(pvSelect), "`"); //`45
      } else throw AttributeRequired(this, MM_STRDUP("select"), sFunctionSignature);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pvSelect) delete pvSelect;    //NOT consumed by saveNodeSet
    //if (sIndex) MMO_FREE(sIndex); //returned
    
    UNWIND_EXCEPTION_THROW;

    return sIndex;
  }
  
  IXmlBaseNode *Session::xslCommand_saveNodeSet(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //we could save these at the server level
    //that would be insecure if just using a numeric id though
    //because one would be able to see what others were doing
    static const char *sCommandSignature = "session:save-node-set [@select=node-set]"; UNUSED(sCommandSignature);
    char *sIndex = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    XmlNodeList<const IXmlBaseNode> *pvSelect = 0;
    vector<XmlNodeList<const IXmlBaseNode>* >::const_iterator iSavedNodeSet;
    
    CHECK_WITH_SESSION_XPATH; //if WITHOUT_SESSION_XPATH is 1 then we should not be asking this
    
    UNWIND_EXCEPTION_BEGIN {
      if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
        pvSelect = pCommandNodeType->attributeValueNodes(pQE, "select", NAMESPACE_NONE, pSourceNode);
        if (!pvSelect) pvSelect = new XmlNodeList<const IXmlBaseNode>(pSourceNode);

        //save the set
        if (pvSelect) {
          sIndex = itoa(saveNodeSet(pvSelect), "`"); //`45
          pOutputNode->createTextNode(pQE, sIndex);
        } else throw AttributeRequired(this, MM_STRDUP("select"), sCommandSignature);
      } else throw InterfaceNotSupported(this, MM_STRDUP("const IXmlBaseNode *"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sIndex)   MMO_FREE(sIndex);
    if (pvSelect) delete pvSelect; //NOT consumed by saveNodeSet
    
    UNWIND_EXCEPTION_THROW;
    
    return 0;
  }
}
