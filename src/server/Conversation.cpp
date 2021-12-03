//platform agnostic file
#include "Conversation.h"

#include "RegularX.h"
#include "Service.h"
#include "IXml/IXslNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlSecurityContext.h"
#include "Response.h"
#include "Repository.h"
#include "MessageInterpretation.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "User.h"
#include "Session.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"
#include <unistd.h>
using namespace std;

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails> Conversation::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Conversation::m_XSLFunctions;

  Conversation::Conversation(Service *pService, DatabaseNode *pNode, SOCKET sc, DatabaseNode *pSessionsCollectionNode):
    MemoryLifetimeOwner(pService, pNode),
    ProfilerThread(pService, "conversation"), //profiling thread and MemoryLifetimeOwner
    GeneralServerDatabaseNodeServerObject(pNode),

    m_pService(pService),
    m_sc(sc),
    m_sTextStream(0),
    m_pCurrentRequest(0),
    m_pMI(0),
    m_bIsFirstRequestOnConnection(true),
    m_bWaitForFurtherRequests(true),
    m_bCloseConnection(true),
    m_bClientCanXSLT(false),
    m_bForceServerXSLT(false),

    m_pSessionsCollectionNode(pSessionsCollectionNode), //optional: if the conversation needs to look for or create a session
    m_pConversationNode_cached(m_pNode->node_transient()),
    m_pSession(0), //contains User

    m_pQE(0),      //initiated in thread, using this Profiler(...) as the profile
    m_ibqe_sessionManager(this, db()->document_const()),

    m_bLogRequests(false)
  {
    assert(m_pService);
  }

  Conversation::~Conversation() {
    m_pNode->node_admin(&m_ibqe_nodeControl)->removeAndDestroyNode(&m_ibqe_nodeControl); //destroy conversation
    if (m_sTextStream)      MMO_FREE(m_sTextStream);
    //if (m_pSessionsCollectionNode)       delete m_pSessionsCollectionNode    //belongs to the Service
    //if (m_pConversationNode_cached) delete m_pConversationNode_cached; //only a cache: managed by m_pNode
    //if (m_pSession)         delete m_pSession; //lasts beyond this conversation

    //ensure socket closed down
    if (m_sc != INVALID_SOCKET) try {CLOSE_SOCKET(m_sc); m_sc = INVALID_SOCKET;} catch (exception ex) {}
  }

  const char  *Conversation::xsltModuleNamespace()    const {return NAMESPACE_CONVERSATION;}
  const char  *Conversation::xsltModulePrefix()       const {return NAMESPACE_CONVERSATION_ALIAS;}
  const char  *Conversation::xslModuleManagerName()   const {return "Conversation";}

  //----------------- XslModuleManager interface environment accessors
  Session              *Conversation::session()          const {return m_pSession;}
  Server               *Conversation::server()           const {return m_pService->server();}
  Service              *Conversation::service()          const {return m_pService;}
  const IXmlLibrary    *Conversation::xmlLibrary()       const {return m_pService->xmlLibrary();}
  Database             *Conversation::db()               const {return m_pService->db();}
  IXmlQueryEnvironment *Conversation::queryEnvironment() const {return m_pQE;}

  void Conversation::threadInit() {
    //already in our own thread
    xmlLibrary()->threadInit(); //setGenericErrorFunc(), exsltRegisterAll() (maybe without Javascript and thus regexp support)

    //EMO: We are an XslModuleManager
    //select each XslModule we want and ask it to register its extensions
    //which effectively registers it on this thread only
    //but also records the details of the module to find them later
    //(IXslModuleManager*) this is sent through then to the primary transform mi->responseTransform(this, ....)
    registerXslModule(server());
    registerXslModule(m_pService);
    registerXslModule(this);
    registerXslModuleManager(db());

    m_pQE = db()->newQueryEnvironment(this, this); //Profiler, EMO
  }

  void Conversation::threadCleanup() {
    //still in our own thread
    xmlLibrary()->threadCleanup();
  }

  const StringMap<IXslModule::XslModuleCommandDetails> *Conversation::xslCommands() const {
    //Conversation is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("set-security-context",      XMC(Conversation::xslCommand_setSecurityContext));
      m_XSLCommands.insert("clear-security-context",    XMC(Conversation::xslCommand_clearSecurityContext));
      m_XSLCommands.insert("close-connection",          XMC(Conversation::xslCommand_closeConnection));
      m_XSLCommands.insert("wait-for-further-requests", XMC(Conversation::xslCommand_waitForFurtherConversations));
      m_XSLCommands.insert("clear-time-limit",          XMC(Conversation::xslCommand_clearTimeLimit));
      m_XSLCommands.insert("xpath-to-node",             XMC(Conversation::xslCommand_xpathToNode));
      //does not make any sense because the command node will always be the conversation:xpath-to-command-node
      //m_XSLCommands.insert("xpath-to-command-node",     XMC(Conversation::xslCommand_xpathToCommandNode));

    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Conversation::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("node",                     XMF(Conversation::xslFunction_node));
      m_XSLFunctions.insert("user-node",                XMF(Conversation::xslFunction_userNode));
      m_XSLFunctions.insert("is-logged-in",             XMF(Conversation::xslFunction_isLoggedIn));
      m_XSLFunctions.insert("client-can-XSLT",          XMF(Conversation::xslFunction_clientCanXSLT));
      m_XSLFunctions.insert("force-server-XSLT",        XMF(Conversation::xslFunction_forceServerXSLT));
      m_XSLFunctions.insert("xpath-to-node",            XMF(Conversation::xslFunction_xpathToNode));
      m_XSLFunctions.insert("xpath-to-command-node",    XMF(Conversation::xslFunction_xpathToCommandNode));
    }
    return &m_XSLFunctions;
  }

  const IXmlBaseNode *Conversation::sessionNode() const {return (m_pSession ? m_pSession->dbNode()->node_const() : 0);}
  const IXmlBaseNode *Conversation::userNode()    const {return (m_pSession && m_pSession->user() ? m_pSession->user()->dbNode()->node_const() : 0);}

  void Conversation::threadProcess() {
    //already in our own thread
    assert(m_pQE);

    //looping: init vars freed below
    IXmlBaseNode *pRequestNode       = 0;
    const char *sSessionID           = 0,
               *sConfigurationFlags  = 0,
               *sClientSoftwareIdentifier = 0,
               *sResourcesAreaIdentifier  = 0,
               *sHeader              = 0,
               *sML                 = 0;
    size_t iLenDataSize;

    //interpret commands (sending an IXmlBaseDoc and a text stream)
    do {
      //reset loop vars
      pRequestNode      = 0;
      m_pCurrentRequest = 0;
      sML              = 0;
      sResourcesAreaIdentifier = 0;

      try {
        //---------------------------------------------------- MI for the input text stream
        //create the Conversation directly into the Conversations repository (according to the Service) using RegularX
        //figure out which conversation snippet we are having from the incoming text stream
        //and, at the same time, how to end it!
        if (m_bIsFirstRequestOnConnection || m_bWaitForFurtherRequests || !m_pMI) { //always true on first loop
          //m_sTextStream, m_pMI fills out
          //deletes / frees them first if necessary
          //throws things
          getMIFromStream(); //m_sTextStream, m_pMI
        } //else use the previous MI

        if (m_pMI) {
          //---------------------------------------------------- sessionID: session location / creation
          //carried out here because regards security which cannot be compromised
          //can potentially change the security context
          if ( !sSessionID                  //we assume session remains equal for the conversation
            && m_pMI->hasAutoSession()      //this MI uses the auto-session function
            && !m_pSession                  //no session node is currently in use for this multi-request
            && m_pSessionsCollectionNode    //the service has a valid sessions repository
          ) {
            sSessionID = m_pMI->sessionID(m_sTextStream);
            //Session::factory() use the Multiton pattern of named instanses by session id
            //http://en.wikipedia.org/wiki/Multiton_pattern
            if (m_pSession = Session::factory(this, m_pSessionsCollectionNode, sSessionID)) {
              //not necessarily a new Session. may already have a valid logged in user
              if (m_pSession->user()) {
                //we have an existing valid, logged in user in the session
                //set up this QE
                if (!m_pQE->securityContext()->login_trusted(m_pSession->user()->name())) throw LoginFailed(this);
              }
            }
          }
          if (m_pSession) {
            registerXslModule(m_pSession);
            if (m_pSession->user()) {
              m_pQE->debugContext(m_pSession->user()); //IXmlDebugContext::informAndWait(...) etc.
              registerXslModule(m_pSession->user());   //user:steppingOn() user:stepInto() etc.
            }
          }

          //---------------------------------------------------- MI configuration flags (early)
          //this explicit configuration control is coded here in the MI, as well as in the XSL
          //  e.g. <debug:xslt-set-trace/>
          //so that early debug can be gleaned like global variable evaluation (happens before XSLT proper)
          //and for other pre-XSLT debug outputs / control
          //  like output to file
          setTimeLimit(20); //TODO: make MI configurable
          if (!sConfigurationFlags) { //we assume configuration remains equal for the conversation
            sConfigurationFlags = m_pMI->configurationFlags(m_sTextStream);
            if (sConfigurationFlags && *sConfigurationFlags) {
              m_pQE->setXSLTTraceFlags(xmlLibrary()->parseXSLTTraceFlags(sConfigurationFlags)); //Transform Context level
              m_pQE->setXMLTraceFlags( xmlLibrary()->parseXMLTraceFlags( sConfigurationFlags)); //actually XML Library level
              //m_pQE->xmlLibrary()->setDebugFunctions(); //implicit in set trace flags function

              if (strstr(sConfigurationFlags, "LOG_REQUEST")) { //strstr(LOG_REQUEST) includes LOG_REQUEST_DETAIL
                m_bLogRequests     = true;
                if (m_pSession) m_pSession->setTransientAttribute(m_pQE, "log-requests", m_bLogRequests);
                Debug::report("LOG_REQUEST raw text stream");
                Debug::report(m_sTextStream);
              }

              if (strstr(sConfigurationFlags, "FORCE_SERVER_XSLT")) {
                m_bForceServerXSLT = true;
                if (m_pSession) m_pSession->setTransientAttribute(m_pQE, "force-server-XSLT", m_bForceServerXSLT);
              }

              Debug::report("message based (first only) configuration flags [%s]", sConfigurationFlags);
            }
          }

          //---------------------------------------------------- client ID: XSLT possibilities and other capabilities
          if (!sClientSoftwareIdentifier) { //we assume software remains equal for the conversation
            sClientSoftwareIdentifier = m_pMI->clientSoftwareIdentifier(m_sTextStream);
            m_bClientCanXSLT = m_pService->clientCanXSLT(sClientSoftwareIdentifier);
            if (m_pSession) m_pSession->setTransientAttribute(m_pQE, "client-can-XSLT", m_bClientCanXSLT);
          }

          //---------------------------------------------------- resources: database, website, etc.
          sResourcesAreaIdentifier = m_pMI->resourcesAreaIdentifier(m_sTextStream);

          if (pRequestNode = m_pMI->messageTransform(m_sTextStream, m_pConversationNode_cached)) {
            //---------------------------------------------------- run
            //create the Request object to represent the object:Request node
            //TODO: dependency inject session and user nodes?
            m_pCurrentRequest = new Request(
              db()->factory_node(pRequestNode),
              this,
              m_pMI,
              m_bIsFirstRequestOnConnection,
              m_bClientCanXSLT,
              m_bForceServerXSLT,
              sResourcesAreaIdentifier
            );
            //if (m_bLogRequests) Debug::reportObject(m_pCurrentRequest); //done in the XSL now because of hardlinking the gs:query node id @attributes
            sML = m_pCurrentRequest->process();

            //debug request watching
            IFDEBUG(if (strstr(m_sTextStream, "javascript:code") && false) {
              Debug::report("########################## request\n%s", m_sTextStream);
              Debug::report("########################## response\n%s\n", sML);
            });

            //---------------------------------------------------- size checking and post-analysis
            if (sML) {
              //checks (may affect size)
              iLenDataSize = strlen(sML);
              if (_STREQUAL(m_pMI->name(), "GET request")) {
                if      (iLenDataSize > 100000)  Debug::report("Response more than 100Kb: this may cause class / object data analysis problems...", rtWarning);
                else if (iLenDataSize > 2000000) throw DataSizeAbort(this, iLenDataSize);
              }
            }

            //---------------------------------------------------- send it!
            //return the text stream to be piped down the SOCKET to the client
            iLenDataSize = strlen(sML); //altered iLenDataSize size with tag removals
            if (send(m_sc, sML, (int) iLenDataSize, MSG_NOSIGNAL) == SOCKET_ERROR) {
              //e.g. EPIPE Broken Pipe can happen when the client has closed down (AJAX listen)
              throw PipeFailure::factory(this, SOCKETS_ERRNO); //polymorphic pointer! TODO: should we switch (SOCKETS_ERRNO) on this instead?
            }
          } else throw ConversationFailedToRegularXInput(this, m_pMI->name(), MM_STRDUP(" (0x0 ConversationNode)"));

          //keep-alive? aka HTTP/1.1 etc.
          if (m_bCloseConnection && m_pMI->closeConnection()) blockingCancel();
        } else {
          //oops, input ended somehow and we dont know what they wanted...
          //leave these off in DEBUG mode
          IFNDEBUG(
            if      (!m_sTextStream)  throw ZeroInputStream(this);
            else if (!*m_sTextStream) throw BlankInputStream(this);
            else                      throw InvalidProtocol(this, m_sTextStream);
          );

          //these are here only to deal with the DEBUG situation
          //to prevent continuous cycles of blank input analysis
          //Debug::report("unhandled read failure (throws turned off)");
          blockingCancel();
        }
      } // /try
      //---------------------------------------------------- exception processing
      //Conversations do not throw exceptions because we have our own thread
      //we don't want to take the server out!
      //let's ignore zero and blank requests for the moment...
      catch (ZeroInputStream& eb)  {
        Debug::report("------ Exception Report");
        Debug::reportObject(&eb);
        blockingCancel(); //using explicit Thread:: for documentative purposes
      }
      catch (BlankInputStream& eb) {
        Debug::report("------ Exception Report");
        Debug::reportObject(&eb);
        blockingCancel(); //using explicit Thread:: for documentative purposes
      }
      catch (OutOfMemory& oom) {
        //eek. no memory left
        Debug::reportObject(&oom);
        blockingCancel(); //using explicit Thread:: for documentative purposes
        //TODO: throw this at the server
        throw oom;
      }
      catch (ExceptionBase& eb) {
        //includes MultipleResourceFound and ResourceNotFound
        Debug::report("------ Exception Report");
        Debug::reportObject(&eb);
        addErrorReportsToNodes(eb); //also releases any waits
        sML = generateXMLErrorReportForClient(eb);
        blockingCancel(); //using explicit Thread:: for documentative purposes
      }
      catch (exception &ex) {
        //base exception happened
        cout << ex.what() << "\n";
        blockingCancel(); //using explicit Thread:: for documentative purposes
      }

      m_bIsFirstRequestOnConnection = false;
      //cancelling any stepping on this conversation thread
      if (m_pSession && m_pSession->user()) m_pSession->user()->stepEnd();

      //---------------------------------------------------- loop free up
      if (sML)                      MMO_FREE(sML);
      if (m_pCurrentRequest)         delete m_pCurrentRequest;
      if (sResourcesAreaIdentifier)  MMO_FREE(sResourcesAreaIdentifier);
      //if (sHeader)                   MMO_FREE(sHeader); //pointer in to response
    } while (Thread::isContinue()); //wait for next bit of our chat: use blockingCancel() to stop isContinue()

    //conversation end free up
    if (sSessionID)                MMO_FREE(sSessionID);
    if (sConfigurationFlags)       MMO_FREE(sConfigurationFlags);
    if (sClientSoftwareIdentifier) MMO_FREE(sClientSoftwareIdentifier);
  }

  void Conversation::addErrorReportsToNodes(ExceptionBase& eb) {
    //on User and Session
    //this will trigger any waiting requests to report the errors to the other client sessions / places
    //session is all transient
    IXmlBaseNode *pErrorNode = 0;

    //none of this happens without session, and user cannot happen without session
    //const XmlAdminQueryEnvironment m_ibqe_sessionManager does not have triggers enabled
    if (m_pSession)         {
      pErrorNode = m_pSession->createErrorNode(eb);
      if (m_pCurrentRequest)  m_pCurrentRequest->hardlinkError(m_pQE, pErrorNode);
      if (m_pSession->user()) m_pSession->user()->hardlinkError(m_pQE, pErrorNode);
    }

    //free up
    if (pErrorNode) delete pErrorNode;
  }

  const char *Conversation::generateXMLErrorReportForClient(ExceptionBase& eb) {
    const char *sML = 0;
    const DatabaseNode *pErrorHandlerNode;

    if (pErrorHandlerNode = m_pMI->errorHandler(eb)) {
      sML = pErrorHandlerNode->xml(&m_ibqe_sessionManager);
      delete pErrorHandlerNode;
    } else {
      //no handler for this
      sML = MM_STRDUP("error thing happened");
    }

    return sML;
  }

  void Conversation::blockingCancel() {
    Thread::blockingCancel(); //just sets m_bContinue to false
    //force close the socket here to release any BLOCKING calls
    if (m_sc != INVALID_SOCKET) try {CLOSE_SOCKET(m_sc); m_sc = INVALID_SOCKET;} catch (exception ex) {}
  }

  void Conversation::getMIFromStream() {
    //fills out m_pMI and m_sTextStream
    //Exceptions will be caught by the Conversation loop
    //we combine reading with MI validity checking
    //because some MIs base their sentence end on content-length sent in headers
    //which causes us not to be able to complete reading until we understand what sort of request it is
    int iBytesRead, iRemainingBufferSpace, iBufferUsed;
    char *sRecvBuff, *sRecvBuffPointer;
    bool bSentenceFinished = false;

    //can be re-called so
    if (m_sTextStream) {MMO_FREE(m_sTextStream); m_sTextStream = 0;}
    if (m_pMI)         {delete m_pMI;           m_pMI         = 0;}

    //throws OutOfMemory() on fail
    sRecvBuff  = MMO_MALLOC(DEFAULT_BUFFLEN);

    if (sRecvBuff) {
      //setup and null terminate the buffer immediately
      m_sTextStream     = sRecvBuff;
      sRecvBuffPointer  = sRecvBuff;
      *sRecvBuffPointer = 0;

      //read from socket in to buffer
      do {
        //calculate the remaining buffer space we have (leaving space for the 0 terminating character) and read
        iBufferUsed = (int) (sRecvBuffPointer - sRecvBuff);
        //TODO: reallocate more buffer if needed (MMO_MALLOC_CHAR will throw OutOfMemory(this, ...) if necessary)
        iRemainingBufferSpace = (DEFAULT_BUFFLEN - iBufferUsed - 1);
        iBytesRead            = recv(m_sc, sRecvBuffPointer, iRemainingBufferSpace, 0);

        //decide what to do
        if (iBytesRead > 0) {
          //move stuff this way to allow print of the latest data
          sRecvBuffPointer[iBytesRead] = 0; //terminate the new request string
          sRecvBuffPointer += iBytesRead;   //increment pointer in recieve buffer

          //see if we can asertain which MI matches so we can check for sentence end
          //note that when manually telnet in, it reads 1 byte at a time (slow)
          if (!m_pMI) m_pMI = m_pService->mi(sRecvBuff);

          //if we got 1 then ask it if we have finished chatting yet
          if (m_pMI && m_pMI->isSenteceEnd(sRecvBuff))
            bSentenceFinished = true; //exit loop

          //buffer limit reached?
          if (iBytesRead == iRemainingBufferSpace) {
            iBytesRead = 0;
            throw OutOfMemory(this, MM_STRDUP("Conversation buffer space is fixed to only 64Kb"));
          }
        } else if (iBytesRead < 0) {
          //negative or null response indicating error
          iBytesRead = 0;
          throw PipeFailure::factory(this, SOCKETS_ERRNO);
        }
      } while (iBytesRead > 0 && !bSentenceFinished);
    }
  }


  IXmlBaseNode *Conversation::xslCommand_clearTimeLimit(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "conversation:clear-time-limit"; UNUSED(sCommandSignature);
    clearTimeLimit();
    return 0;
  }

  IXmlBaseNode *Conversation::xslCommand_closeConnection(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "conversation:close-connection"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    m_bCloseConnection = pCommandNodeType->attributeValueBoolDynamicString(pQE, "setting");
    return 0;
  }

  IXmlBaseNode *Conversation::xslCommand_waitForFurtherConversations(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "conversation:wait-for-further-conversations"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    m_bWaitForFurtherRequests = pCommandNodeType->attributeValueBoolDynamicString(pQE, "setting");
    return 0;
  }

  IXmlBaseNode *Conversation::xslCommand_setSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "conversation:set-security-context @username @password"; UNUSED(sCommandSignature);
    IXmlBaseNode *pUserNode;

    UNWIND_EXCEPTION_BEGIN {
      //may throw LoginFailed(this)
      db()->xslCommand_setSecurityContext(pQE, pCommandNode, pSourceNode, pOutputNode);
      if (pUserNode = pQE->securityContext()->userNode()) {
        if (m_pSession) m_pSession->user(User::factory(this, db()->factory_node(pUserNode)));
        else throw SessionRequired(this, MM_STRDUP("set security context cross session permanence"));
      }
    } UNWIND_EXCEPTION_END;

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Conversation::xslCommand_clearSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "conversation:clear-security-context"; UNUSED(sCommandSignature);

    UNWIND_EXCEPTION_BEGIN {
      db()->xslCommand_clearSecurityContext(pQE, pCommandNode, pSourceNode, pOutputNode);
      if (m_pSession) m_pSession->deleteUser();
    } UNWIND_EXCEPTION_END;

    UNWIND_EXCEPTION_THROW;

    return 0;
  }
  
  const char *Conversation::xslFunction_isLoggedIn(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //userNode() is a pointer, dont delete it
    static const char *sFunctionSignature = "conversation:is-logged-in()"; UNUSED(sFunctionSignature);
    return db()->xslFunction_isLoggedIn(pQE, pXCtxt, pNodes);
  }

  const char *Conversation::xslFunction_clientCanXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "conversation:client-can-XSLT()"; UNUSED(sFunctionSignature);
    return m_bClientCanXSLT ? MM_STRDUP("true") : 0;
  }

  const char *Conversation::xslFunction_forceServerXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "conversation:force-server-XSLT()"; UNUSED(sFunctionSignature);
    return m_bForceServerXSLT ? MM_STRDUP("true") : 0;
  }

  const char *Conversation::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "conversation:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *Conversation::xslFunction_userNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "conversation:user-node()"; UNUSED(sFunctionSignature);
    assert(pQE);
    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(pQE); //pQE as MM

    UNWIND_EXCEPTION_BEGIN {
      //userNode() is a pointer, dont delete it
      if (userNode()) pNewNodes->push_back(userNode()->clone_with_resources()); //node-set is deleted after
    } UNWIND_EXCEPTION_END;
    
    //free up
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);
    
    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Conversation::xslFunction_xpathToNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "conversation:xpath-to-node([node, base-node, enable-ids])"; UNUSED(sFunctionSignature);
    //  @node:
    //  @base-node:   start from this node (must be ancestor!)
    //  @enable-ids: path, rather than id link
    const char *sRet = 0;
    
#ifdef WITHOUT_SESSION_XPATH
    sRet = db()->xslFunction_xpathToNode(pQE, pXCtxt, pNodes);
#else
    if (m_pSession) sRet = m_pSession->xslFunction_saveNodeSet(pQE, pXCtxt, pNodes);
    else            sRet = db()->xslFunction_xpathToNode(pQE, pXCtxt, pNodes);
#endif
    
    return sRet;
  }
  
  const char *Conversation::xslFunction_xpathToCommandNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //conversation:xpath-to-command-node([base-node, enable-ids])
    //  @disable-ids: path, rather than id link
    //  @base-node:   start from this node (must be ancestor!)
    static const char *sFunctionSignature = "conversation:xpath-to-command-node([base-node, enable-ids])"; UNUSED(sFunctionSignature);
    const char *sRet         = 0;
    IXmlBaseNode *pBaseNode  = 0;
    bool bEnableIds          = false,
         bForceBaseNode;
    const IXmlBaseNode *pLiteralCommandNode   = 0;
    XmlNodeList<const IXmlBaseNode> *pvSelect = 0;
    assert(pXCtxt);

    UNWIND_EXCEPTION_BEGIN {
      pLiteralCommandNode = pXCtxt->literalCommandNode(pQE); //pQE as MM
      if (pLiteralCommandNode) {
#ifdef WITHOUT_SESSION_XPATH
        //--------------------------------------- database:xpath-to-node
        //all optional
        switch (pXCtxt->valueCount()) {
          case 2: bEnableIds             = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 1: pBaseNode              = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 0: break;
          default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
        }
        bForceBaseNode = (pBaseNode != NULL);
        sRet           = pLiteralCommandNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT, bForceBaseNode, bEnableIds);
#else
        if (m_pSession) {
          //--------------------------------------- session:save-node-set
          //all optional
          //we need to pop these off the stack anyway
          switch (pXCtxt->valueCount()) {
            case 2: bEnableIds     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
            case 1: pBaseNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
            case 0: break;
            default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
          }
          pvSelect = new XmlNodeList<const IXmlBaseNode>(pLiteralCommandNode); //pQE as MM
          sRet     = itoa(m_pSession->saveNodeSet(pvSelect), "`"); //`45
        } else {
          //--------------------------------------- database:xpath-to-node
          //all optional
          switch (pXCtxt->valueCount()) {
            case 2: bEnableIds     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
            case 1: pBaseNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
            case 0: break;
            default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
          }
          bForceBaseNode = (pBaseNode != NULL);
          sRet           = pLiteralCommandNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT, bForceBaseNode, bEnableIds);
        }
#endif
      } else throw CommandNodeRequired(this, sFunctionSignature);
    } UNWIND_EXCEPTION_END;
    
    //free up
    if (pBaseNode)           delete pBaseNode;
    if (pvSelect)            delete pvSelect;    //NOT consumed by saveNodeSet
    if (pLiteralCommandNode) delete pLiteralCommandNode;
    
    UNWIND_EXCEPTION_THROW;
    
    return sRet;
  }

  IXmlBaseNode *Conversation::xslCommand_xpathToNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    static const char *sCommandSignature = "conversation:xpath-to-node [@node, @base-node, @enable-ids]"; UNUSED(sCommandSignature);
    //  @node:
    //  @base-node:  start from this node (must be ancestor!)
    //  @enable-ids: path, rather than id link
    IXmlBaseNode *pOutput = 0;
    
#ifdef WITHOUT_SESSION_XPATH
    pOutput = db()->xslCommand_xpathToNode(pQE, pCommandNode, pSourceNode, pOutputNode);
#else
    if (m_pSession) pOutput = m_pSession->xslCommand_saveNodeSet(pQE, pCommandNode, pSourceNode, pOutputNode);
    else                             pOutput = db()->xslCommand_xpathToNode(pQE, pCommandNode, pSourceNode, pOutputNode);
#endif
    
    return pOutput;
  }
  
  void Conversation::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const {
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

  const char *Conversation::toString() const {
    //caller free
    const char *sThreadToString = Thread::toString();
    size_t iLen = 10 + strlen(sThreadToString);
    char *s = (char*) MM_MALLOC_FOR_RETURN(iLen + 1);
    _SNPRINTF2(s, iLen+1, "%s%s", "----------", sThreadToString);
    MM_FREE(sThreadToString);
    return s;
  }
}

