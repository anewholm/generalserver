//platform agnostic file
#ifndef _SESSION_H
#define _SESSION_H

#include "define.h"
#include "Utilities/strtools.h"
#include "Utilities/StringMap.h"          //member variable

#include "DatabaseNodeServerObject.h" //direct inheritance
#include "Xml/XslModule.h"                //direct inheritance

//platform specific project headers
#include "LibXslModule.h"

#include <vector>

using namespace std;

namespace general_server {
  interface_class IXmlQueryEnvironment;

  class User;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class Session: public GeneralServerDatabaseNodeServerObject, virtual public MemoryLifetimeOwner, public SERVER_MAIN_XSL_MODULE {
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    //http://en.wikipedia.org/wiki/Multiton_pattern
    const IXmlQueryEnvironment *m_pOwnerQE;
    XmlAdminQueryEnvironment    m_ibqe_sessionManager; //administration functions
    User *m_pUser;
    vector<XmlNodeList<const IXmlBaseNode>* > m_vSavedNodeLists;

  protected:
    static StringMap<Session*> m_mSessions;
    Session(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pSessionNode);

  public:
    //factory used to have one stateful Session per session across multiple conversations
    static Session *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pSessionsCollectionNode, const char *sSessionID);
    ~Session();

    //------------- accessors
    const IXmlBaseNode *node_const() const;
    User               *user() const;
    const char         *UUID() const;

    //------------- control
    static void clearSessions();
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const char *sValue, const char *sNamespace = NULL) const;
    void setTransientAttribute(const IXmlQueryEnvironment *pQE, const char *sName, const bool bValue,  const char *sNamespace = NULL) const;
    User *user(User *pUser);
    void deleteUser();
    IXmlBaseNode *createErrorNode(ExceptionBase& eb);
    size_t saveNodeSet(XmlNodeList<const IXmlBaseNode> *pvSelect);

    //----------------- XslModule commands
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace()  const;
    const char *xsltModulePrefix()     const;
    const char *xslModuleManagerName() const;

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_saveNodeSet(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_nodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_saveNodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_saveLiteralNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /* TODO: move these to the session namespace?
    const char   *xslFunction_userNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char   *xslFunction_isLoggedIn(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char   *xslFunction_clientCanXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char   *xslFunction_forceServerXSLT(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    */
    /** @} */

    static void compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput);
    const char *toString() const;
  };
}

#endif
