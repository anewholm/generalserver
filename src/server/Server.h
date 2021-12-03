//platform agnostic file
#ifndef _SERVER_H
#define _SERVER_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "semaphore.h"

//platform generic includes
#include "define.h"
#include "Repository.h"          //Server has-a Repository
#include "Thread.h"              //direct inheritance
#include "DatabaseNodeServerObject.h"  //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance

//see server.cpp for the main comment block introduction
namespace general_server {
  class LibXmlLibrary;
  class Service;
  class PrimaryRepositoryManager;
  class Database;
  class SecurityContext;


  //-------------------------------------------------------------------------------------------
  //-------------------------------------- CommandLineProcessor -------------------------------
  //-------------------------------------------------------------------------------------------
  class CommandLineProcessor: public Thread {
    Server *m_pServer;

  public:
    CommandLineProcessor(Server *pServer);
    void threadProcess();     //loop main thread on the command line input
  };

  //-------------------------------------------------------------------------------------------
  //-------------------------------------- Server ---------------------------------------------
  //-------------------------------------------------------------------------------------------
  class Server: public GeneralServerDatabaseNodeServerObject, public ThreadCaller, public SERVER_MAIN_XSL_MODULE {
    static WSADATA w;
    static WORD vVersionRequired;

    static pthread_mutex_t m_mServiceManagement;
    StringMap<Service*> m_mServices;         //collection of services

    CommandLineProcessor m_clp;              //interactive
    const bool m_bListenForCommandLine;
    const bool m_bBlockThisThread;
    bool m_bReloading;
    sem_t m_semaphoreWait;                   //main thread blocker

    //XslModuleManager
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

  protected:
    void serviceManagementLock();
    void serviceManagementUnlock();

  public:
    //instanciation and destruction
    Server(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pNode, const bool bListenForCommandLine = true, const bool bBlockThisThread = true);
    ~Server();

    //startup and stop sequence
    void runInCurrentThread();                //all services->runThreaded()
    void blockingCancel();                    //stopAndWaitAllServices()
    void childThreadFinished(Thread *t);      //delete service
    void cancelSemaphoreWait();               //stops blocked main thread waiting for commands, leading to a ~Server()
    void stopAndWaitAllServices();            //called during ~Server() as part of unload

    static const char *xsd_name();
    bool execute(const char *sCommand); //string -> method() translation

    //state
    const string *currentHDD() const;
    const string *configFile() const;
    const char *serverProcessList() const;
    StringMap<Service*> *services() {return &m_mServices;}
    static const char *execServerProcess(const char *sCommand);
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;
    void addCompilationInformationNode(const IXmlQueryEnvironment *pQE, const char *sName, void (compFunc)(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput), IXmlBaseNode *pCompilationInformationNode) const;
    const char *toString() const;

    //XSL Module command setup
    const char *xsltModuleNamespace()  const;
    const char *xsltModulePrefix()     const;
    const char *xslModuleManagerName() const;
    const StringMap<IXslModule::XslModuleCommandDetails>   *xslCommands() const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_reload(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_execute(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_serialise(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_serverProcessList(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_date(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_dateCompare(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    static void compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput);
  };
}

#endif

