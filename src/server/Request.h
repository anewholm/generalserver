//platform agnostic file
//Request (GeneralServerDatabaseNodeServerObject) represents a <object:Request>
//Conversation synchronously processes requests
//Request is NOT threaded, Conversation is it's own thread
//Conversation 1-1 SOCKET connection
#ifndef _REQUEST_H
#define _REQUEST_H

//TODO: remove
namespace general_server {class Request;class Response;}

using namespace std;

#include "Xml/XslModule.h"     //direct inheritance
#include "Thread.h"        //direct inheritance
#include "MemoryLifetimeOwner.h" //direct inheritance
#include "DatabaseNodeServerObject.h" //direct inheritance
#include "Xml/XmlAdminQueryEnvironment.h" //has-a

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
  class Conversation;
  class Tests;

  //each new request is on a new thread and accepts a text command stream
  //this gets converted into (multiple, hierarchical) command(s)
  //The server is responsible only for kicking off the thread, passing through the communications socket
  class Request: public XslModuleManager, public GeneralServerDatabaseNodeServerObject, private SERVER_MAIN_XSL_MODULE, virtual public MemoryLifetimeOwner {
    //----------------------------------------- properties
    static StringMap<IXslModule::XslModuleCommandDetails> m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    //environment
    Conversation            *m_pConversation;
    IXmlQueryEnvironment    *m_pQE;
    const MessageInterpretation *m_pMI;
    Response                *m_pResponse;           //construction: new Response(this)
    XmlAdminQueryEnvironment m_ibqe_requestManager; //needs to set the id on the request

    //for unique identification of request nodes in the requests XML collection
    static size_t m_iNextRequestId;
    const  size_t m_iThisRequestId;
    const bool  m_bIsFirstRequestOnConnection;
    const bool  m_bClientCanXSLT;
    const bool  m_bForceServerXSLT;
    const char *m_sResourcesAreaIdentifier;
    
    friend class Tests; //allows also access to the IXslModule
    Request(Tests *pTest, DatabaseNode *m_pNode); //fake request setup
    
  public:
    //----------------------------------------- construction
    Request(
      DatabaseNode *m_pNode, 
      Conversation *pConversation, 
      const MessageInterpretation *pMI, 
      const bool bIsFirstRequestOnConnection = false, 
      const bool bClientCanXSLT = false, 
      const bool bForceServerXSLT = false,
      const char *sResourcesAreaIdentifier = 0
    );
    ~Request();

    const bool shouldServerSideXSLT() const;
    const char *process();
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;
    const char *toString() const;

    //----------------- XslModuleManager interface environment accessors
    //everything comes from the service
    const Conversation *conversation() const;
    const Response     *response()     const;
    Server             *server()       const; //m_pConversation->server()
    const IXmlLibrary  *xmlLibrary()   const; //m_pConversation->xmlLibrary();
    Database           *db()           const; //m_pConversation->db();
    const MessageInterpretation *mi()  const;
    const char         *sessionUUID()  const; //m_pConversation->session()->UUID()

    //----------------- XslModule commands
    const StringMap<IXslModule::XslModuleCommandDetails> *xslCommands() const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;
    const char *xslModuleManagerName() const;

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_resourceMatch(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_resourceID(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_globalVariableUpdate(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_throw(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
