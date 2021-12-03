//platform agnostic file
//Conversation is it's own thread that processes Requests synchronously on one socket connection
//Conversation 1-1 SOCKET connection
//Request - Response[, Request - Response, Request - Response, Request - Response]
//Conversation CAN process several requests and responses, e.g. HTTP/1.1
//it is really just a loop and some settings
#ifndef _CONVERSATION_H
#define _CONVERSATION_H

#include <vector>
using namespace std;

#define DEFAULT_BUFFLEN 64000

#include "Xml/XslModule.h"                //direct inheritance
#include "Thread.h"                   //direct inheritance
#include "ProfilerThread.h"           //direct inheritance
#include "DatabaseNodeServerObject.h" //direct inheritance
#include "Xml/XmlAdminQueryEnvironment.h" //full property

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class Repository;
  class Response;
  class MessageInterpretation;
  class Server;
  class Service;
  class SecurityContext;
  class DatabaseNode;
  class Database;
  class User;
  class Session;

  //each new request is on a new thread and accepts a text command stream
  //this gets converted into (multiple, hierarchical) command(s)
  //The server is responsible only for kicking off the thread, passing through the communications socket
  class Conversation: public XslModuleManager, public GeneralServerDatabaseNodeServerObject, public ProfilerThread, private SERVER_MAIN_XSL_MODULE {
    //----------------------------------------- properties
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    //socket communication settings
    Service *m_pService;                 //injected parent (for Database)
    SOCKET m_sc;
    char *m_sTextStream;

    //the m_sTextStream dictates the MI
    //the MI dctates the conversation settings and style
    const MessageInterpretation *m_pMI;  //current MI handler single threaded request
    bool m_bCloseConnection;             //overrides MI->closeConnection
    bool m_bWaitForFurtherRequests;      //re-processes initial request immediately (server events)
    bool m_bIsFirstRequestOnConnection;  //is it the first incoming question from the client (headers)
    bool m_bClientCanXSLT;
    bool m_bForceServerXSLT;

    //environment
    IXmlQueryEnvironment *m_pQE;                    //construction: db()->newQueryEnvironment(this): user state
    const XmlAdminQueryEnvironment m_ibqe_sessionManager; //administration functions

    //nodes representing the request, session and user
    //sent in to the transform as parameters
    DatabaseNode *m_pSessionsCollectionNode;  //dependency injected from the Service
    Request      *m_pCurrentRequest;          //the singular, synchronous current request
    IXmlBaseNode *m_pConversationNode_cached; //transient cached node pointer from m_pNode
    Session      *m_pSession;                 //transient DatabaseNode: singular per session across requests

    bool m_bLogRequests;

    //----------------------------------------- functions
    void threadInit();
    void threadCleanup();
    void getMIFromStream(); //conversion of input text stream into a CommandList ready for execution
    //error reporting
    const char *generateXMLErrorReportForClient(ExceptionBase& eb);
    void        addErrorReportsToNodes(ExceptionBase& eb);

  public:
    //----------------------------------------- construction
    Conversation(Service *pService, DatabaseNode *pNode, SOCKET s, DatabaseNode *pSessionsCollectionNode = 0);
    ~Conversation();

    void threadProcess();
    void blockingCancel();

    //accessors for all (synchronous) requests in this Conversation
    //current session and user
    const IXmlBaseNode *sessionNode() const; //transient
    const IXmlBaseNode *userNode()    const; //non-transient DatabaseNode stored in the User object
    IXmlQueryEnvironment *queryEnvironment() const;

    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;
    const char *toString() const;

    //----------------- XslModuleManager interface environment accessors
    Session           *session()    const;
    Server            *server()     const; //m_pService->server()
    Service           *service()    const;
    const IXmlLibrary *xmlLibrary() const; //m_pService->xmlLibrary();
    Database          *db()         const; //m_pService->db();

    //----------------- XslModule commands
    const StringMap<IXslModule::XslModuleCommandDetails> *xslCommands()   const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace()  const;
    const char *xsltModulePrefix()     const;
    const char *xslModuleManagerName() const;

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_setSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_clearSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_closeConnection(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_waitForFurtherConversations(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_clearTimeLimit(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xpathToNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_userNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isLoggedIn(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_clientCanXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_forceServerXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_xpathToNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_xpathToCommandNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    //----------------- interface navigation
    Conversation    *queryInterface(Conversation *p) {return this;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
