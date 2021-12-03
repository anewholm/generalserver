//platform agnostic file
#include "Service.h"

#include "define.h"
#include <netinet/tcp.h>

//standard library includes
#include <algorithm>
#include <sstream>
using namespace std;

#include "Server.h"
#include "Utilities/StringMap.h"
#include "MessageInterpretation.h"
#include "Conversation.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "DatabaseClass.h"

#include "Utilities/container.c"

namespace general_server {
  //Service represents a TSR listening on a port and has actions associated with it
  //It is not the windows service interface: see general_server.cpp for that

  const int Service::m_sockopt_one = 1;
  pthread_mutex_t Service::m_mConversationManagement = PTHREAD_MUTEX_INITIALIZER;
  void Service::conversationManagementLock()   {pthread_mutex_lock(&m_mConversationManagement);}
  void Service::conversationManagementUnlock() {pthread_mutex_unlock(&m_mConversationManagement);}
  StringMap<IXslModule::XslModuleCommandDetails>  Service::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Service::m_XSLFunctions;

  Service::Service(Server *pServer, DatabaseNode *pNode):
    MemoryLifetimeOwner(pServer, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    m_pServer(pServer),
    m_ibqe_conversationManager(this, pNode->db()->document_const()),
    Thread(pServer, "service")
  {
    //assert(m_pNode); //GeneralServerDatabaseNodeServerObject will do this
    assert(m_pServer);

    vector<DatabaseClass*> *pvServiceClasses;
    DatabaseNode *pServiceClassNode;
    MessageInterpretation *pMI;
    XmlNodeList<DatabaseNode> *pMessageInterpretationNodes;
    XmlNodeList<DatabaseNode>::iterator iMINode;

    //Service class (singular)
    pvServiceClasses  = DatabaseClass::classesFromElement(m_pNode);
    if (pvServiceClasses->size() != 1) throw ServiceRequiresASingleClass(this);
    m_pServiceClass   = pvServiceClasses->at(0);
    pServiceClassNode = m_pServiceClass->dbNode();

    //Service properties
    //instance can override the Class settings
    m_sServiceName    =  m_pNode->attributeValue(m_pIBQE_serverStartup, "name");    //e.g. HTTP
    if (!m_sServiceName) m_sServiceName = pServiceClassNode->attributeValue(m_pIBQE_serverStartup, "name");
    m_iPort           =  m_pNode->attributeValueInt(m_pIBQE_serverStartup, "port"); //e.g. 80
    if (!m_iPort)        m_iPort = pServiceClassNode->attributeValueInt(m_pIBQE_serverStartup, "port");
    m_pConversationsCollectionNode = m_pNode->getOrCreateChildElement(m_pIBQE_serverStartup, "requests", NULL, NAMESPACE_REPOSITORY);
    m_pConversationsCollectionNode->setTransientArea(m_pIBQE_serverStartup);
    m_pSessionsCollectionNode   = m_pNode->getOrCreateChildElement(m_pIBQE_serverStartup, "sessions", NULL, NAMESPACE_REPOSITORY);
    m_pSessionsCollectionNode->setTransientArea(m_pIBQE_serverStartup);
    m_sSentenceFinish = MM_STRDUP("\n"); //TODO: not currently used (default MI sentenceFinish?)

    //conversations MIs
    pMessageInterpretationNodes = pServiceClassNode->getMultipleNodes(m_pIBQE_serverStartup, "repository:controllers/%s", MessageInterpretation::xsd_name());
    for (iMINode = pMessageInterpretationNodes->begin(); iMINode != pMessageInterpretationNodes->end(); iMINode++) {
      pMI = new MessageInterpretation(*iMINode, this);
      m_vMIs.push_back(pMI);
    }

    //sort them by priority
    sort(m_vMIs.begin(), m_vMIs.end(), MessageInterpretation::Comp());

    //defaults and checks
    UNWIND_EXCEPTION_BEGIN {
      if (!m_sServiceName) m_sServiceName = MM_STRDUP("no name");
      if (!m_iPort)        throw ServicePortRequired(this, m_sServiceName);
      if (!pMessageInterpretationNodes->size()) throw ServiceMIsRequired(this, m_sServiceName);
    } UNWIND_EXCEPTION_END;

    //clear up
    //UNWINDING note that the ~destructor() NOT run in the case of an exception
    UNWIND_DELETE_IF_EXCEPTION(m_pNode);
    UNWIND_DELETE_IF_EXCEPTION(m_pConversationsCollectionNode);
    UNWIND_DELETE_IF_EXCEPTION(m_pSessionsCollectionNode);
    UNWIND_FREE_IF_EXCEPTION(m_sServiceName);
    UNWIND_FREE_IF_EXCEPTION(m_sSentenceFinish);
    UNWIND_IF_NOEXCEPTION(delete pMessageInterpretationNodes);
    UNWIND_IF_NOEXCEPTION(delete pvServiceClasses);
    UNWIND_IF_EXCEPTION(vector_element_destroy(pMessageInterpretationNodes));

    UNWIND_EXCEPTION_THROW;
  }

  Service::~Service() {
    if (m_lConversations.size()) {
      Debug::report("service [%s] has outstanding conversations, requesting a wait", m_sServiceName, rtWarning, rlWorrying);
      //will force an immediateKillObjectThread() if they don't nicely shutdown
      waitForConversationsToFinish();
    }

    vector_element_destroy(m_vMIs);
    if (m_sServiceName)    MMO_FREE(m_sServiceName);
    if (m_sSentenceFinish) MMO_FREE(m_sSentenceFinish);
    //delete m_vMIs; //local
    //m_lConversations;   //local
    if (m_pConversationsCollectionNode)   delete m_pConversationsCollectionNode;
    if (m_pSessionsCollectionNode)   delete m_pSessionsCollectionNode;
  }

  void Service::childThreadFinished(Thread *t) {
    //our Conversation Threads can't clean themselves up
    //the Thread class calls it's parent childThreadFinished()
    Conversation *pConversation = t->queryInterface((Conversation*) 0);
    if (pConversation) {
      conversationManagementLock(); {
        m_lConversations.remove(pConversation);
      } conversationManagementUnlock();
      delete pConversation;  //free up memory immediately
    } else throw InterfaceNotSupported(this, MM_STRDUP("Conversation*"));
  }

  const char *Service::xsltModuleNamespace()  const {return NAMESPACE_SERVICE;}
  const char *Service::xsltModulePrefix()     const {return NAMESPACE_SERVICE_ALIAS;}
  const char *Service::xslModuleManagerName() const {return "Service";}

  const StringMap<IXslModule::XslModuleCommandDetails> *Service::xslCommands() const {
    if (!m_XSLCommands.size()) {}
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Service::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("node",                XMF(Service::xslFunction_node));
    }
    return &m_XSLFunctions;
  }
  
  Server *Service::server() const {return m_pServer;}

  void Service::threadInit() {
     xmlLibrary()->threadInit();
  }
  void Service::threadCleanup() {
     xmlLibrary()->threadCleanup();
  }

  bool Service::clientCanXSLT(const char *sClientSoftwareIdentifier) const {
    //TODO: look this up in the DB somehow
    //NOT_COMPLETE("clientCanXSLT");
    return true;
  }

  void Service::threadProcess() {
    //working code goes here to actually listen and stuff
    //http://msdn.microsoft.com/en-us/library/ms740668(v=vs.85).aspx
    //socket communication
    m_scl               = INVALID_SOCKET;
    SOCKADDR_IN addr; // The address structure for a TCP socket
    const char *sErrorProcessList   = 0;

    //everything okey, okey so far
    memset((char *) &addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;           //Address family
    addr.sin_port        = htons(m_iPort);    //Assign port to this socket (80 = web)
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //Accept a connection from any IP using INADDR_ANY
    m_scl                = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket

    //trying to allow reuse of the socket
    if (setsockopt(m_scl, SOL_SOCKET, SO_REUSEADDR, (const char*) &m_sockopt_one, sizeof(m_sockopt_one)) == SOCKET_ERROR) {
      CLOSE_SOCKET(m_scl);
      m_scl = INVALID_SOCKET;
      throw ServiceCannotListen(this, MM_STRDUP("setsockopt(SO_REUSEADDR)"), SOCKETS_ERRNO, m_iPort);
    }

    //use SOCKETS_ERRNO to examine problems (errno.h)
    //IF there are other general_server processes bound to this port then the failure will occur HERE
    if (bind(m_scl, (LPSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR) {
      CLOSE_SOCKET(m_scl);
      Debug::report(ServiceCannotListen::netStat(m_iPort));
      Debug::report(sErrorProcessList = ServiceCannotListen::processes(_EXECUTABLE));
      //cannot auto kill others cause we dont know which one we are!
      throw ServiceCannotListen(this, MM_STRDUP("bind()"), SOCKETS_ERRNO, m_iPort);
    }

    //on Ubuntu laptop SOMAXCONN = 128
    //this does not block, accept() does
    if (listen(m_scl, SOMAXCONN) == SOCKET_ERROR) { //NON-BLOCKING
      blockingCancel();
      throw ServiceCannotListen(this, MM_STRDUP("listen()"), SOCKETS_ERRNO, m_iPort);
    }

    Debug::reportObject(this);
    primaryAcceptLoop();
  }

  unsigned int Service::primaryAcceptLoop() {
    //Conversation objects to handle the comms on a Thread
    DatabaseNode *pConversationNode   = 0;
    Conversation *pConversation = 0;
    unsigned int iSocketErrors  = 0;
    struct timeval timeout;
#ifdef GS_SO_RCVTIMEO
    unsigned int iGS_SO_RCVTIMEO = GS_SO_RCVTIMEO;
#else
    unsigned int iGS_SO_RCVTIMEO = 3;
#endif
    //socket accept vars
    SOCKET scAccepted = INVALID_SOCKET;
    sockaddr  remote_addr;
    socklen_t remote_addrlen = sizeof(sockaddr);
    //int iAcceptFlags            = 0; //accept4() only

    //-------------------------------------------------------- primary loop
    while (isContinue()) {
      //blocking accept on socket, wait for incoming reqeuest
      //pending network errors will be reported here immediately
      //http://manpages.ubuntu.com/manpages/lucid/man2/accept.2.html
      //TODO: the second 2 accept() arguments are the peer IP address which we should get used and passed through...
      //TODO: need to place a MACRO around accept and accept4 to test for it during compile
      //#ifdef _GNU_SOURCE sc = accept4(m_scl, &remote_addr, &remote_addrlen, iAcceptFlags); NOT_CURRENTLY_USED("");
      //can error with ENETDOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH, EOPNOTSUPP, and ENETUNREACH.
      //EGEAIN loop implementation
      iSocketErrors = 0;
      do {
        scAccepted = accept(m_scl, &remote_addr, &remote_addrlen); //BLOCKING
        if (scAccepted == 0) throw ServiceAssignedSocketZero(this, "was close(0) run?");
        if (scAccepted == (SOCKET) SOCKET_ERROR) Debug::report("accept() error on socket [%s]", strerror(SOCKETS_ERRNO)); //strerror = static C error buffer
      } while (scAccepted == (SOCKET) SOCKET_ERROR && ++iSocketErrors && iSocketErrors <= 10);

      if (scAccepted == (SOCKET) SOCKET_ERROR) {
        blockingCancel();
        throw ServiceCannotListen(this, MM_STRDUP("accept()"), SOCKETS_ERRNO, m_iPort);
      } else Debug::report("Incoming connection accepted on [%s]", m_iPort);

      //we calculate the entire package all the time so no need for Nagle
      //except when doing AJAX listening, where we small packages
      if (setsockopt(scAccepted, IPPROTO_TCP, TCP_NODELAY, (char *) &m_sockopt_one, sizeof(m_sockopt_one)) == SOCKET_ERROR) {
        blockingCancel();
        throw ServiceCannotListen(this, MM_STRDUP("setsockopt(TCP_NODELAY)"), SOCKETS_ERRNO, m_iPort);
      }

      //set socket timeouts
      memset(&timeout, 0, sizeof(timeout)); // zero timeout struct before use
      timeout.tv_sec  = iGS_SO_RCVTIMEO;
      timeout.tv_usec = 0;
      //setsockopt(scAccepted, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)); //send timeout
      //setsockopt(scAccepted, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); //receive timeout

      //the thread end will delete this Service
      //TODO: is this all safe? conversationManagementLock() is it necessary?
      if (isContinue()) {
        //this is a transient-area so the TXml will not repeatable
        //TODO (PERFORMANCE): we could have a way of creating transient children without temporary non-repeatable TXml
        pConversationNode = m_pNode->factory_node(m_pConversationsCollectionNode->createTransientChildElement(&m_ibqe_conversationManager, "conversation", NULL, NAMESPACE_GS, "new incoming conversation"));
        pConversation     = new Conversation(this, pConversationNode, scAccepted, m_pSessionsCollectionNode);
        conversationManagementLock(); {
          m_lConversations.push_back(pConversation);
        } conversationManagementUnlock();
        pConversation->runThreaded();
      }
    } //while (...)

    return iSocketErrors;
  }

  void Service::waitForConversationsToFinish() {
    //TODO: waitForConversationsToFinish() should we include this intelligence in Thread?
    //the finishing Conversation will trigger the childThreadFinished() event which manages the m_lConversations list and deletes the Request object
    list<Conversation*>::iterator iConversation;
    if (m_lConversations.size()) {
      //ask conversations to stop
      //there will be 1 in the queue if this conversation comes from a HTTP command
      Debug::report("Service [%s]: waiting 5 seconds for [%s] conversations to finish / cancel...", m_sServiceName, m_lConversations.size());
      conversationManagementLock(); {
        for (iConversation = m_lConversations.begin(); iConversation != m_lConversations.end(); iConversation++)
          (*iConversation)->blockingCancel();
      } conversationManagementUnlock();

      //progress report wait for conversations to finish
      int i = 5;
      Debug::startProgressReport();
      while (m_lConversations.size() && i--) {
        Debug::progressReport();
        sleep(1);
      }
      if (m_lConversations.size()) Debug::report("failed to shutdown all conversations. forcing exit");
      conversationManagementLock(); {
        for (iConversation = m_lConversations.begin(); iConversation != m_lConversations.end(); iConversation++) {
          (*iConversation)->immediateKillObjectThread();
          delete (*iConversation);
        }
      } conversationManagementUnlock();
      Debug::completeProgressReport();
    }
  }

  const char *Service::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "service:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  void Service::blockingCancel() {
    //this will be called from a different thread
    Thread::blockingCancel(); //just sets m_bContinue to false: no new conversations
    //TODO: make this block until the Service stops
    //TODO: make it finish its conversations first
    waitForConversationsToFinish();
    immediateKillObjectThread();   //no resources lost here

    //stop listening
    CLOSE_SOCKET(m_scl);
    m_scl = INVALID_SOCKET;
  }

  const MessageInterpretation *Service::mi(const char *sTextStream) const {
    //no one frees anything!
    vector<const MessageInterpretation*>::const_iterator iMI;
    iMI = find_if(m_vMIs.begin(),  m_vMIs.end(), MessageInterpretation::GimmePred(sTextStream));
    return (iMI == m_vMIs.end() ? 0 : *iMI);
  }

  const char *Service::toString() const {
    //caller frees result
    //server notification output
    vector<const MessageInterpretation*>::const_iterator iMI;
    stringstream sOut;
    const char *sMI;
    sOut << "Service ready [" << m_sServiceName << ":" << m_iPort << "] (" << threadingModel() << ")\n";
    sOut << "  conversations (in order of gimme() evaluation):";
    for (iMI = m_vMIs.begin(); iMI != m_vMIs.end(); iMI++) {
      sMI = (*iMI)->toString();
      sOut << "\n" << "  " << sMI;
      MMO_FREE(sMI);
    }
    if (!m_vMIs.size()) sOut << "\n";
    return MM_STRDUP(sOut.str().c_str());
  }

  void Service::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    assert(pOutputNode);

    IXmlBaseNode *pCurrent;

    pCurrent = pOutputNode->createChildElement(pQE, "service");
    pCurrent->setAttribute(pQE, "name", m_sServiceName);
    pCurrent->setAttribute(pQE, "port", m_iPort);
    pCurrent->setAttribute(pQE, "sentenceFinish", m_sSentenceFinish);

    //messageInterpretation
    /*
    psSerialise->append("<messageInterpretation>");
    char *sMessageInterpretation=m_pMessageInterpretation->xml();
    psSerialise->append(sMessageInterpretation);
    if (sMessageInterpretation) free(sMessageInterpretation);
    psSerialise->append("</messageInterpretation>");
    //messageInterpretation
    psSerialise->append("<commandsXSL>");
    char *sCommandsXSL=m_pCommandsXSL->xml();
    psSerialise->append(sCommandsXSL);
    if (sCommandsXSL) free(sCommandsXSL);
    psSerialise->append("</commandsXSL>");
    */
    //conversations
    pCurrent = pCurrent->createChildElement(pQE, "conversations");
    list<Conversation*>::const_iterator iConversation;
    for (iConversation = m_lConversations.begin(); iConversation != m_lConversations.end(); iConversation++) {
      (*iConversation)->serialise(pQE, pCurrent);
    }
  }
}
