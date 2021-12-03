//platform agnostic file
#ifndef _USER_H
#define _USER_H

#include "define.h"
#include "semaphore.h"

#include "IXml/IXmlDebugContext.h"         //direct inheritance
#include "LibXslModule.h"             //direct inheritance SERVER_MAIN_XSL_MODULE
#include "DatabaseNodeServerObject.h" //direct inheritance

namespace general_server {
  interface_class IXmlQueryEnvironment;

  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  class User: implements_interface IXmlDebugContext, virtual public MemoryLifetimeOwner, public GeneralServerDatabaseNodeServerObject, public SERVER_MAIN_XSL_MODULE {
    //http://en.wikipedia.org/wiki/Multiton_pattern
    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    bool m_bStepping;
    bool m_bEnabled;
    DatabaseNode *m_pNextStop;
    pthread_t m_steppingThread; //User stepping is single threaded
    sem_t m_semaphoreStep;
    const IXmlQueryEnvironment *m_pOwnerQE;
    pthread_mutex_t m_mStepThreadManagement;
    XmlNodeList<IXmlBaseNode> *m_pvUserBreakpoints;

  protected:
    static StringMap<User*> m_mUsers;
    User(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pUserNode);

  public:
    static User *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pUserNode);
    ~User();
    static void clearUsers();

    //------------- accessors
    const char *name() const; //@name
    const IXmlBaseNode *node_const() const;

    //------------- IXmlQueryEnvironmentComponent interface
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}
    
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}

    //------------- IXmlDebugContext interface
    const char *type()     const;
    const char *toString() const;

    //server events
    IXmlQueryEnvironment::accessResult informAndWait(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pSourceXmlNode, const IXmlQueryEnvironment::accessOperation iFilterOperation, IXmlQueryEnvironment::accessResult iIncludeNode);

    //client debug flow controls
    IXmlBaseNode *setBreakpoint(IXmlBaseNode *pBreakpointNode);
    void clearBreakpoint(IXmlBaseNode *pBreakpointNode);
    void stepInit();
    bool isInitiated() const;
    bool isSteppingThread() const;
      void stepOver();
      void stepInto();
      void stepOut();
      void stepContinue();
    void stepEnd();

    //----------------- XslModule commands
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_stepOver(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_stepInto(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_stepOut(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_stepContinue(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_breakpoint(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_stepOver(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_stepInto(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_stepOut(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_stepContinue(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_setBreakpoint(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_clearBreakpoint(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
