//platform agnostic
#include "Server.h"

//standard library includes
#include <map>
#include <algorithm>
#include "Utilities/strtools.h"
#include <ctime>
using namespace std;

//platform agnostic project headers
#include "define.h" //_WINDOWS, _LOGFILE, etc.

//include everything for compilationInformation()
#include "Conversation.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "DatabaseNodeServerObject.h"
#include "Debug.h"
#include "Exceptions.h"
#include "Main.h"
#include "MemoryLifetimeOwner.h"
#include "MessageInterpretation.h"
#include "RelationalDatabases.h"
#include "Repository.h"
#include "Request.h"
#include "Response.h"
#include "RegularX.h"
#include "Server.h"
#include "Service.h"
#include "Session.h"
#include "Thread.h"
#include "Tests.h"
#include "TXml.h"
#include "TXmlProcessor.h"
#include "User.h"

#include "FileSystem.h"

#include "QueryEnvironment/QueryEnvironment.h"
#include "QueryEnvironment/SecurityContext.h"
#include "QueryEnvironment/TriggerContext.h"
#include "QueryEnvironment/SecurityStrategy.h"
#include "LibXmlLibrary.h"

//interfaces
#include "IXml/IXslNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "Exceptions.h"

//utilities
#include "Utilities/container.c"
#include "Utilities/regexpr2.h"
using namespace regex;

namespace general_server {
  //-------------------------------------------------------------------------------------------
  //-------------------------------------- CommandLineProcessor ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  CommandLineProcessor::CommandLineProcessor(Server *pServer): 
    MemoryLifetimeOwner(pServer),
    m_pServer(pServer), 
    Thread(pServer, "CommandLineProcessor") 
  {}

  void CommandLineProcessor::threadProcess() {
    size_t iChars, iLen = 1024;
    char *sWait;

    //help
    sleep(1); //wait for startup sequence to complete before showing prompt
    cout << "\n\n------------------- general server command line\n";
    m_pServer->execute("help");

    //loop for commands
    do {
      sWait = 0; //getline() allocates the new buffer
      cout << "gs> ";
      iChars = getline(&sWait, &iLen, stdin);
      if (iChars && sWait) {
        if (*sWait) *(sWait + strlen(sWait) - 1) = 0; //remove end-of-line
        //can change m_bProcessCommandLine
        if (!m_pServer->execute(sWait)) blockingCancel();

        //clear up
        MMO_FREE(sWait);
      }
    } while (isContinue());

    //and bugger off
    cout << "exiting command line processor...\n";
  }

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- Server ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  WSADATA Server::w                = 0;
  WORD    Server::vVersionRequired = 0x0202;
  StringMap<IXslModule::XslModuleCommandDetails>  Server::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Server::m_XSLFunctions;
  pthread_mutex_t Server::m_mServiceManagement = PTHREAD_MUTEX_INITIALIZER;
  void Server::serviceManagementLock()   {pthread_mutex_lock(&m_mServiceManagement);}
  void Server::serviceManagementUnlock() {pthread_mutex_unlock(&m_mServiceManagement);}

  Server::Server(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pNode, const bool bListenForCommandLine, const bool bBlockThisThread):
    MemoryLifetimeOwner(pMemoryLifetimeOwner, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    m_bReloading(false),
    m_bListenForCommandLine(bListenForCommandLine),
    m_bBlockThisThread(bBlockThisThread),
    ThreadCaller(pNode->db()),
    m_clp(this)
  {
    Service *pService;
    XmlNodeList<DatabaseNode> *pServiceNodes = 0;
    XmlNodeList<DatabaseNode>::iterator iServiceNode;

    //socket communication
    if (SOCKETS_INIT(vVersionRequired, &w)) throw ServiceCannotListen(this, MM_STRDUP("SOCKETS_INIT()"), SOCKETS_ERRNO, 0);

    //initialise the thread wait semaphore
    sem_init(&m_semaphoreWait, NOT_FORK_SHARED, 0);

    //components below report as server loads
    Debug::reportObject(this);

    //------------------------------------------------ services GeneralServerDatabaseNodeServerObjects
    pServiceNodes = m_pNode->getMultipleNodes(m_pIBQE_serverStartup, "repository:services/*");
    if (!pServiceNodes->size()) Debug::report("no services found");
    for (iServiceNode = pServiceNodes->begin(); iServiceNode != pServiceNodes->end(); iServiceNode++) {
      pService = new Service(this, *iServiceNode);
      m_mServices.insert(pService->name(), pService);
    }

    //clear up
    if (pServiceNodes) delete pServiceNodes;
  }

  Server::~Server() {
    if (m_mServices.size()) Debug::report("server has outstanding services running", IReportable::rtWarning, IReportable::rlWorrying);
    Session::clearSessions();
    User::clearUsers();
    sem_destroy(&m_semaphoreWait);
    SOCKETS_CLEANUP();
  }

  void Server::runInCurrentThread() {
    StringMap<Service*>::iterator iService;
    
    serviceManagementLock(); {
      for (iService = m_mServices.begin(); iService != m_mServices.end(); iService++)
        iService->second->runThreaded();
    } serviceManagementUnlock();

    Tests ts_networking(m_pNode->db(), NULL, "HTTP-tests"); //BLOCKING

    //block this thread if requested
    if (m_bListenForCommandLine) m_clp.runThreaded();          //default behaviour: THREADED listen for input
    if (m_bBlockThisThread)      sem_wait(&m_semaphoreWait);   //default behaviour: BLOCKING, "exit" causes it to stop waiting
    else                         Debug::report("server will immediately finish as in NO_BLOCK mode");
    if (m_bListenForCommandLine) m_clp.immediateKillObjectThread();        //we can't stop it listening for input nicely...
  }

  void Server::childThreadFinished(Thread *t) {
    //Service OR CommandLineProcessor
    Service *s;
    if (t == &m_clp) {
      //no clean up needed for CommandLineProcessor
      //because it is local
    } else if (s = t->queryInterface((Service*) 0)) {
      serviceManagementLock(); {
        m_mServices.erase(s->name());
        delete s; //free up memory immediately
      } serviceManagementUnlock();
    } else throw ThreadTypeUnknown(this, MM_STRDUP("Server"));
  }

  void Server::blockingCancel() {
    stopAndWaitAllServices();
  }

  void Server::stopAndWaitAllServices() {
    //TODO: stopAndWaitAllServices() should we include this intelligence in Thread?
    //asks Services to stop, exit, threadCleanup() calling Server::childThreadFinished() to delete them
    //blocks until all Services have exited nicely
    StringMap<Service*>::iterator iService;
    if (m_mServices.size()) {
      //TODO: stopAndWaitAllServices() mutex of service management around this
      serviceManagementLock(); {
        Debug::report("Server: asking [%s] Services to finish / cancel...", m_mServices.size());
        for (iService = m_mServices.begin(); iService != m_mServices.end(); iService++) iService->second->blockingCancel();
      } serviceManagementUnlock();
    }

    //wait 1 second before polling the services to see if they have exited
    sleep(1);
    if (m_mServices.size()) {
      Debug::report("Server: waiting 5 seconds for [%s] Services to finish / cancel...", m_mServices.size());
      //progress report wait for services to finish
      int i = 5;
      Debug::startProgressReport();
      while (m_mServices.size() && i--) {
        Debug::progressReport();
        sleep(1);
      }
      if (m_mServices.size()) Debug::report("failed to shutdown all services. forcing exit");
      //TODO: mutex delete services
      Debug::completeProgressReport();
    }
  }

  const char *Server::xsltModuleNamespace()  const {return NAMESPACE_SERVER;}
  const char *Server::xsltModulePrefix()     const {return NAMESPACE_SERVER_ALIAS;}
  const char *Server::xslModuleManagerName() const {return "Server";}
  const char *Server::xsd_name() {return "object:Server";}

  const StringMap<IXslModule::XslModuleCommandDetails> *Server::xslCommands() const {
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("reload",         XMC(Server::xslCommand_reload));
      m_XSLCommands.insert("execute",        XMC(Server::xslCommand_execute));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Server::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("serialise",           XMF(Server::xslFunction_serialise));
      m_XSLFunctions.insert("server-process-list", XMF(Server::xslFunction_serverProcessList));
      m_XSLFunctions.insert("date",                XMF(Server::xslFunction_date));
      m_XSLFunctions.insert("date-compare",        XMF(Server::xslFunction_dateCompare));
    }
    return &m_XSLFunctions;
  }

  const string *Server::currentHDD() const {
    return new string;
  }
  const string *Server::configFile() const {
    return new string;
  }
  const char *Server::serverProcessList() const {
    return execServerProcess("ps -A --no-heading -o comm,pid,%mem,%cpu");
  }
  const char *Server::execServerProcess(const char *sCommand) {
    FILE *pFile;
    char sBuffer[2048];
    string stOutput;

    if (pFile = popen(sCommand, "r")) {
      while (fgets(sBuffer, sizeof(sBuffer), pFile)) stOutput += sBuffer;
      pclose(pFile);
    }

    return MM_STRDUP(stOutput.c_str());
  }
  
  const char *Server::toString() const {
    //caller frees result
    stringstream sOut;
    sOut << "Server:";
    IFDEBUG(sOut << " (WITH_DEBUG mode)");
    IFDEBUG_EXCEPTIONS(sOut << " (WITH_DEBUG_EXCEPTIONS mode)");
    IFDEBUG_RX(sOut << " (WITH_DEBUG_RX mode)");
    if (m_bListenForCommandLine) sOut << " (interactive)"; else sOut << " (non-interactive)";
    if (m_bBlockThisThread) sOut << " (primary thread blocking)";
    sOut << "\n  MemoryLifetimeOwner (OS):        [" << (memUsageOperatingSystem() / 1024) << "] Mb";
    sOut << "\n  MemoryLifetimeOwner (allocated): [" << (memAllocated() / (1024*1024)) << "] Mb";
    return MM_STRDUP(sOut.str().c_str());
  }

  IXmlBaseNode *Server::xslCommand_execute(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sCommand;

    //required destination parameter
    pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    if (sCommand = pCommandNodeType->attributeValueDynamic(pQE, "command")) {
      Debug::report("server command over http port [%s]", sCommand);
      execute(sCommand);
      MMO_FREE(sCommand);
    }

    return 0;
  }

  void Server::cancelSemaphoreWait() {
    sem_post(&m_semaphoreWait);
  }

  bool Server::execute(const char *sCommand) {
    bool bContinue = true;

    if (sCommand && *sCommand) {
      //------------------------------- help
      if        (!strcmp(sCommand, "help") || !strcmp(sCommand, "?")) {
        cout << "  reload, commit, commit full, rollback, exit\n";
        cout << "  p [server, config, library]\n";
        cout << "  xshell -> xml library command shell\n";

      //------------------------------- actions
      } else if (!strcmp(sCommand, "reload")) {
        cout << "not supported yet\n";
      } else if (!strcmp(sCommand, "commit")) {
        db()->commit();
      } else if (!strcmp(sCommand, "commit full")) {
        db()->commit(TXml::beginCommit, TXml::full);
      } else if (!strcmp(sCommand, "rollback")) {
        db()->rollback();
      } else if (!strcmp(sCommand, "exit")
              || !strcmp(sCommand, "quit")
              || !strcmp(sCommand, "q")
      ) {
        cancelSemaphoreWait();
        bContinue = false;

      //------------------------------- information
      } else if (!strcmp(sCommand, "p server")) {
        Debug::reportObject(this);
      } else if (!strcmp(sCommand, "p config")
              || !strcmp(sCommand, "p doc")
              || !strcmp(sCommand, "p main")
              || !strcmp(sCommand, "c")     //check
              || !strcmp(sCommand, "check") //check
      ) {
        Debug::reportObject(db());
      } else if (!strcmp(sCommand, "p library")) {
        Debug::reportObject(db()->xmlLibrary());

      //------------------------------- LibXML2 shell
      } else if (!strcmp(sCommand, "xshell")) {
        if (!db()->shellCommand(sCommand))
          cout << "shell failure\n";
      } else {
        cout << "unknown command [" << sCommand << "]\n";
      }
    }

    return bContinue;
  }

  IXmlBaseNode *Server::xslCommand_reload(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //inputs to this debug() call
    //pCommandNode: e.g. <config:appendChild>
    //pSourceNode:  e.g. <repository:index_xsl> if the transform is currently traversing the process_main_stylesheet
    //pOutputNode:  e.g. <root>

    //no attributes for this debug() call

    //this is really silly: it's kindof pulling the rug out from under the feet
    Debug::report("\n\n--------------------------------------- server reload\n");
    m_bReloading = true;
    stopAndWaitAllServices();

    return 0;
  }

  const char *Server::xslFunction_serialise(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    assert(pQE);
    IXmlBaseNode *pOutputNode = 0;
    static const char *sFunctionSignature = "server:serialise(node)";

    if (pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
      serialise(pQE, pOutputNode);
    } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("No Output node"), sFunctionSignature);

    //free up
    if (pOutputNode) delete pOutputNode;

    return 0;
  }

  const char *Server::xslFunction_serverProcessList(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "server:server-process-list()"; UNUSED(sFunctionSignature);
    return serverProcessList();
  }

  const char *Server::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "server:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *Server::xslFunction_date(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //parameter:
    //  @date: can equal "now"
    //  @format: see below
    //http://www.cplusplus.com/reference/ctime/strftime/
    //%s
    //  The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
    //supports the translated formats also:
    //%XML, %ISO8601
    //  2015-09-08T12:30:03+00:00
    //%RFC822
    //  RFC 822, updated by RFC 1123
    //  Sun, 06 Nov 1994 08:49:37 GMT
    //%RFC850
    //  RFC 850, obsoleted by RFC 1036
    //  Sunday, 06-Nov-94 08:49:37 GMT
    //%ANSI
    //  ANSI C's asctime() format
    //  Sun Nov  6 08:49:37 1994
    static const char *sFunctionSignature = "server:date([date, format])"; UNUSED(sFunctionSignature);
    size_t iMaxChars = RFC822_LEN + 10;

    time_t rawtime;
    struct tm tGMT;
    char *sDateTime;
    const char *sFormat, *sFreeFormat = 0;
    size_t iCharsOut;

    UNWIND_EXCEPTION_BEGIN {
      sDateTime    = MM_MALLOC_FOR_RETURN(iMaxChars);

      if (pXCtxt->valueCount() == 2) {
        sFormat = sFreeFormat = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sFormat = xmlLibrary()->translateDateFormat(sFormat);
      } else sFormat = DATE_FORMAT_XML; //sortable XML Format 2015-09-28T23:00:01Z

      if (pXCtxt->valueCount()) {
        tGMT = pXCtxt->popInterpretDateFromXPathFunctionCallStack();
      } else {
        time(&rawtime);
        gmtime_r(&rawtime, &tGMT); //reentrant
      }

      iCharsOut = strftime(sDateTime, iMaxChars, sFormat, &tGMT);

      //since we cannot predict the output size
      do {
        iCharsOut = strftime(sDateTime, iMaxChars, sFormat, &tGMT);
      } while (
           !iCharsOut
        && (iMaxChars = iMaxChars * 2) < 2048
        && (sDateTime = (char*) realloc(sDateTime, iMaxChars))
      );
      if (!iCharsOut) throw DateFormatResultTooLong(this, sFormat);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sFreeFormat) MMO_FREE(sFreeFormat);
    UNWIND_FREE_IF_EXCEPTION(sDateTime);

    UNWIND_EXCEPTION_THROW;

    return sDateTime;
  }

  const char *Server::xslFunction_dateCompare(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //throw FailedToParseDate(this, sValue);
    //compare the 2 dates
    //  @date1:
    //  @date2: optional, default to now
    //returns:
    //  ">" @date1 > @date2
    //  "=" @date1 = @date2
    //  "<" @date1 < @date2
    //date formats:
    //  ...
    static const char *sFunctionSignature = "server:date-compare(date1 [, date2])"; UNUSED(sFunctionSignature);
    const char *sRet = 0;
    time_t iDate1, iDate2;
    double dSeconds;

    //inputs
    if (pXCtxt->valueCount() == 2) {
      //throw FailedToParseDate(this, sValue);
      struct tm tDate2 = pXCtxt->popInterpretDateFromXPathFunctionCallStack();
      iDate2 = mktime(&tDate2);
    } else {
      time(&iDate2);
    }
    if (pXCtxt->valueCount()) {
      //throw FailedToParseDate(this, sValue);
      struct tm tDate1 = pXCtxt->popInterpretDateFromXPathFunctionCallStack();
      iDate1 = mktime(&tDate1);
    }
    else throw XPathStringArgumentRequired(this, MM_STRDUP("date"));

    //compare
    dSeconds = difftime(iDate1, iDate2);
    if      (dSeconds > 0) sRet = ">";
    else if (dSeconds < 0) sRet = "<";
    else                   sRet = "=";

    return MM_STRDUP(sRet);
  }

  void Server::compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) {
    IFDEBUG( pOutput->setAttribute(pQE, "DEBUG", "on"));
    IFNDEBUG(pOutput->setAttribute(pQE, "DEBUG", "off"));
#ifdef VERSION_MAJOR
    pOutput->setAttribute(pQE, "VERSION_MAJOR", VERSION_MAJOR);
#endif
#ifdef VERSION_MINOR
    pOutput->setAttribute(pQE, "VERSION_MINOR", VERSION_MINOR);
#endif
#ifdef VERSION_BUILD
    pOutput->setAttribute(pQE, "VERSION_BUILD", VERSION_BUILD);
#endif
#ifdef VERSION_TYPE
    pOutput->setAttribute(pQE, "VERSION_TYPE",  VERSION_TYPE);
#endif
#ifdef CFLAGS
    pOutput->setAttribute(pQE, "CFLAGS",  CFLAGS);
#endif
#ifdef CFLAGS
    pOutput->setAttribute(pQE, "CPPFLAGS",  CPPFLAGS);
#endif
  }

  void Server::addCompilationInformationNode(const IXmlQueryEnvironment *pQE, const char *sName, void (compFunc)(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput), IXmlBaseNode *pCompilationInformations) const {
    const char *sAttribute;
    IXmlBaseNode *pThisCompilationInformationNode = pCompilationInformations->createChildElement(pQE, sName);

    //structure
    delete pThisCompilationInformationNode->createChildElement(pQE, "patches");
    delete pThisCompilationInformationNode->createChildElement(pQE, "components");
    delete pThisCompilationInformationNode->createChildElement(pQE, "release-notes");
    delete pThisCompilationInformationNode->createChildElement(pQE, "history");

    compFunc(pQE, pThisCompilationInformationNode);

    //versioning
    sAttribute = pThisCompilationInformationNode->attributeValue(pQE, "version-major");
    if (!sAttribute) {
      pThisCompilationInformationNode->setAttribute(pQE, "version-major", "1");
      pThisCompilationInformationNode->setAttribute(pQE, "version-minor", "0");
    } else MMO_FREE(sAttribute);

    //reporting
    sAttribute = pThisCompilationInformationNode->attributeValue(pQE, "reporting");
    if (!sAttribute) {
      pThisCompilationInformationNode->setAttribute(pQE, "reporting", "annesley_newholm@yahoo.it");
    } else MMO_FREE(sAttribute);

    //free up
    if (pThisCompilationInformationNode) delete pThisCompilationInformationNode;
  }

  void Server::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    assert(pOutputNode);
    pOutputNode->removeAndDestroyChildren(pQE);
    IXmlBaseNode *pServer = pOutputNode->createChildElement(pQE, "server", NULL, NAMESPACE_GS);

    //-------------------------------------------------- static Server compilation information
    //C++ Class versions, patches and components
    IXmlBaseNode *pCompilationInformation = pServer->createChildElement(pQE, "compilation-information", NULL, NAMESPACE_GS);

    //General
    addCompilationInformationNode(pQE, "Conversation", Conversation::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Database", Database::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "DatabaseNodeServerObject", DatabaseNodeServerObject::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Debug", Debug::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "ExceptionBase", ExceptionBase::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Main", Main::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "MemoryLifetimeOwner", MemoryLifetimeOwner::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "MessageInterpretation", MessageInterpretation::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "RelationalDatabase", RelationalDatabase::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Repository", Repository::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Request", Request::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Response", Response::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "RegularX", RegularX::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Server", Server::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Service", Service::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Session", Session::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "Thread", Thread::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "TXml", TXml::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "TXmlProcessor", TXmlProcessor::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "User", User::compilationInformation, pCompilationInformation);

    //FileSystem.cpp
    addCompilationInformationNode(pQE, "Directory", Directory::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "File", File::compilationInformation, pCompilationInformation);

    //QueryEnvironment folder
    addCompilationInformationNode(pQE, "QueryEnvironment", QueryEnvironment::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "SecurityContext", SecurityContext::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "TriggerContextDatabaseAttribute", TriggerContextDatabaseAttribute::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "AccessControlList", AccessControlList::compilationInformation, pCompilationInformation);
    addCompilationInformationNode(pQE, "UnixSecurityStrategy", UnixSecurityStrategy::compilationInformation, pCompilationInformation);

    //Xml Library
    addCompilationInformationNode(pQE, "SERVER_MAIN_XML_LIBRARY", SERVER_MAIN_XML_LIBRARY::compilationInformation, pCompilationInformation);

    delete pCompilationInformation;

    //-------------------------------------------------- dynamic object instance information
    //------------------- Database
    db()->serialise(pQE, pServer);

    //------------------- Service's
    StringMap<Service*>::const_iterator iService; //due to the const nature of the function
    IXmlBaseNode *pServices = pServer->createChildElement(pQE, "services");
    for (iService = m_mServices.begin(); iService != m_mServices.end(); iService++)
      iService->second->serialise(pQE, pServices);
    delete pServices;

    //free up
    delete pServer;
  }
}
