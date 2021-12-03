//platform agnostic file
#ifndef _SERVICE_H
#define _SERVICE_H

namespace general_server {class Service;}

//standard library includes
#include <vector>
#include <list>
#include <sstream>
using namespace std;

#include "Request.h"
#include "Thread.h"
#include "IReportable.h"              //direct inheritance
#include "MemoryLifetimeOwner.h"      //direct inheritance
#include "DatabaseNodeServerObject.h" //direct inheritance

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class Server;
  class MessageInterpretation;
  class DatabaseClass;

  class Service: public GeneralServerDatabaseNodeServerObject, public Thread, implements_interface IReportable, public SERVER_MAIN_XSL_MODULE {
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;
    
    DatabaseClass *m_pServiceClass;
    SOCKET m_scl;
    Server *m_pServer;
    const char *m_sServiceName; //name()
    int m_iPort;
    const char *m_sSentenceFinish;
    static const int m_sockopt_one;

    //associated DatabaseNodeServerObject and their management
    static pthread_mutex_t m_mConversationManagement;
    void conversationManagementLock();
    void conversationManagementUnlock();
    vector<const MessageInterpretation*> m_vMIs;
    DatabaseNode *m_pConversationsCollectionNode; //transient-area: so we can createChildElement()
    DatabaseNode *m_pSessionsCollectionNode;      //transient-area
    XmlAdminQueryEnvironment m_ibqe_conversationManager;
    list<Conversation*> m_lConversations;

    //Threaded
    void threadInit();
    void threadCleanup();
    void waitForConversationsToFinish();
    unsigned int primaryAcceptLoop();

  protected:
    //----------------- XslModule commands
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace()  const;
    const char *xsltModulePrefix()     const;
    const char *xslModuleManagerName() const;

  public:
    Service(Server *pServer, DatabaseNode *pNode);
    ~Service();

    //processing
    void threadProcess(); //listen on port
    void childThreadFinished(Thread *t);
    const MessageInterpretation *mi(const char *sTextStream) const;
    void blockingCancel();

    //functions
    bool clientCanXSLT(const char *sClientSoftwareIdentifier) const;

    //properties and serialisation
    const char *name() {return m_sServiceName;}
    const char *toString() const;
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;
    Server *server() const; //to register the XslModule
    static const char *xsd_name() {return "object:Service";}

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    //interface navigation
    Service    *queryInterface(Service *p) {return this;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
