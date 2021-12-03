//platform agnostic file
#include "User.h"

#include "Utilities/StringMap.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXslNode.h"

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails>  User::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> User::m_XSLFunctions;

  StringMap<User*> User::m_mUsers; //User::m_mUsers is deleted by the ~Server()

  User *User::factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pUserNode) {
    //http://en.wikipedia.org/wiki/Multiton_pattern
    assert(pUserNode);

    User *pUser = 0;
    StringMap<User*>::iterator iUser;
    const char *sUserName;
    const XmlAdminQueryEnvironment m_ibqe_staticNodeControl(pMemoryLifetimeOwner, pUserNode->db()->document_const());

    if (sUserName = pUserNode->attributeValue(&m_ibqe_staticNodeControl, "name")) {
      iUser = m_mUsers.find(sUserName);
      if (iUser != m_mUsers.end()) {
        //we have an existing stateful User object
        pUser = iUser->second;
      } else {
        //create a new User object for this node
        pUser = new User(pMemoryLifetimeOwner, pUserNode);
        m_mUsers.insert(sUserName, pUser);
      }
    }

    //free up
    if (sUserName) MM_FREE(sUserName);

    return pUser;
  }

  User::User(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, DatabaseNode *pNode):
    MemoryLifetimeOwner(pMemoryLifetimeOwner, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    m_bStepping(false),
    m_pNextStop(0),
    m_steppingThread(0),
    m_pvUserBreakpoints(0)
  {
    sem_init(&m_semaphoreStep, NOT_FORK_SHARED, 0);
    pthread_mutex_init(&m_mStepThreadManagement, NULL);
  }

  User::~User() {
    const char *sUserName;
    StringMap<User*>::iterator iUser;
    const XmlAdminQueryEnvironment m_ibqe_staticNodeControl(this, m_pNode->db()->document_const());

    if (sUserName = m_pNode->attributeValue(&m_ibqe_staticNodeControl, "name")) {
      //delete iUser->second; //we are in the process of doing that already!
      m_mUsers.erase(sUserName);
    }

    //free up
    if (sUserName) MM_FREE(sUserName);
    //if (m_pUserNode) delete m_pUserNode; //will be deleted by GeneralServerDatabaseNodeServerObject
    
    stepEnd();
    sem_destroy(&m_semaphoreStep);
    pthread_mutex_destroy(&m_mStepThreadManagement);
  }

  void User::clearUsers() {
    Debug::report("clearing Multiton_pattern User objects [%s]", m_mUsers.size());
    m_mUsers.elements_free_delete();
  }


  const StringMap<IXslModule::XslModuleCommandDetails> *User::xslCommands() const {
    //User is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("step-over",     XMC(User::xslCommand_stepOver));
      m_XSLCommands.insert("step-into",     XMC(User::xslCommand_stepInto));
      m_XSLCommands.insert("step-out",      XMC(User::xslCommand_stepOut));
      m_XSLCommands.insert("step-continue", XMC(User::xslCommand_stepContinue));
      m_XSLCommands.insert("breakpoint",    XMC(User::xslCommand_breakpoint));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *User::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("step-over",        XMF(User::xslFunction_stepOver));
      m_XSLFunctions.insert("step-into",        XMF(User::xslFunction_stepInto));
      m_XSLFunctions.insert("step-out",         XMF(User::xslFunction_stepOut));
      m_XSLFunctions.insert("step-continue",    XMF(User::xslFunction_stepContinue));

      m_XSLFunctions.insert("set-breakpoint",   XMF(User::xslFunction_setBreakpoint));
      m_XSLFunctions.insert("clear-breakpoint", XMF(User::xslFunction_clearBreakpoint));
      m_XSLFunctions.insert("node",             XMF(User::xslFunction_node));
    }
    return &m_XSLFunctions;
  }

  const char  *User::xsltModuleNamespace()    const {return NAMESPACE_USER;}
  const char  *User::xsltModulePrefix()       const {return NAMESPACE_USER_ALIAS;}

  const char *User::name() const {return m_pNode->attributeValue(&m_ibqe_nodeControl, "name");}
  const IXmlBaseNode *User::node_const() const {return m_pNode->node_const();}

  //------------- IXmlQueryEnvironmentComponent interface
  void User::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}

  //------------- IXmlDebugContext interface
  const char *User::type()     const {return "User";}
  const char *User::toString() const {return MM_STRDUP(type());}

  //server events
  IXmlQueryEnvironment::accessResult User::informAndWait(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pSourceXmlNode, const IXmlQueryEnvironment::accessOperation iFilterOperation, IXmlQueryEnvironment::accessResult iIncludeNode) {
    //PERFORMANCE: this is a HIGH volume area. every node security check goes through here
    //  only do anything here if we are sure we need to
    //stepping initialisation:
    //  we want the first thread that hits a breakpoint to take over with stepInit()
    //    not the first thread to come here, all threads should check until the first one hits
    //  after stepInit(), other threads should be ignored
    //    until the primary request stepping finishes with stepEnd()
    IXslTransformContext *pTCtxt;                 //server controlled
    IXmlBaseNode *pDebugNode            = 0,      //object:User/gs:debug
                 *pBreakpointCollection = 0;      //object:User/gs:debug/gs:breakpoints/(hardlinks!)
    bool bOnBreakpoint                  = false;  //just hit breakpoint
    const char *sCommandXmlID           = 0,
               *sSourceXmlID            = 0,
               *sSteppingThread         = 0,
               *sOutputXmlID            = 0,
               *sBreakpoint             = 0,
               *sBreakpointValue        = 0;
    const IXmlBaseNode *pInstructionNode = 0; //server controlled

    //PERFORMANCE: isInitiated() is super fast and optimised and the normal mode
    if (!isInitiated() || isSteppingThread()) {
      //PERFORMANCE: transformContext() is just an accessor to a server managed pointer
      //TODO: PERFORMANCE: commandNode() creates a new IXmlBaseNode()
      if ( (pTCtxt           = pQE->transformContext())
        && (pInstructionNode = pTCtxt->instructionNode(this))
      ) {
        //check if we are on a breakpoint:
        //  if we need to START stepping for a User breakpoint
        //  i.e. are we at a breakpoint, or the next node to stop at
        //stepContinue() may have happened, which needs to stop at the next breakpoint
        //we still do this if we are m_bStepping because we want to know if bOnBreakpoint
        //we cache the breakpoint existence
        //  because normal non-debug threads come here so we don't want to slow them down
        //  and to prevent the request that creates the breakpoint hitting it
        //  stepEnd() will m_bLookedForUserBreakpoints = false

        //attribute breakpoint
        //PERFORMANCE: attributeValueBoolDynamicString() is slow
        //so we use the Direct string first which gives us a direct pointer in to the xmlNode
        if ( (sBreakpoint = pInstructionNode->attributeValueDirect(&m_ibqe_nodeControl, "breakpoint", NAMESPACE_GS))
          && (sBreakpointValue = pInstructionNode->valueDynamic(pQE, sBreakpoint))
          && (xmlLibrary()->textEqualsTrue(sBreakpointValue))
        ) {
          m_bStepping   = true;
          bOnBreakpoint = true;
        }

        //command breakpoint
        //PERFORMANCE: isNamespace() is just a string comparison with no malloc
        if (pInstructionNode->isNamespace(NAMESPACE_USER)) {
          if (_STREQUAL(pInstructionNode->localName(NO_DUPLICATE), "breakpoint")) {
            m_bStepping   = true;
            bOnBreakpoint = true;
          }
        }

        //next stop
        //PERFORMANCE: m_pNextStop is zero unless stepping, so ok
        if (m_pNextStop && m_pNextStop->is(pInstructionNode, EXACT_MATCH)) {
          m_bStepping = true;
        }

        //XSL User breakpoints (pre-imptively populated by setBreakpoint())
        //PERFORMANCE: m_pvUserBreakpoints is only non-zero when there are some
        //set only with user:set-breakpoint()
        if (m_pvUserBreakpoints && pInstructionNode->in(m_pvUserBreakpoints, EXACT_MATCH)) {
          m_bStepping      = true;
          bOnBreakpoint    = true;
        }

        //Source tree User breakpoints (pre-imptively populated by setBreakpoint())
        //PERFORMANCE: m_pvUserBreakpoints is only non-zero when there are some
        //set only with user:set-breakpoint()
        if (m_pvUserBreakpoints && pSourceXmlNode->in(m_pvUserBreakpoints, EXACT_MATCH)) {
          m_bStepping      = true;
          bOnBreakpoint    = true;
        }

        //stepInit() internally uses a mutex so that only one thread can isSteppingThread()
        if (m_bStepping) stepInit();

        //Debug::report("pSourceXmlNode: %s", IReportable::rtInformation, IReportable::rlExpected, pSourceXmlNode->uniqueXPathToNode(&m_ibqe_nodeControl, 0, false, false, false, true));
        //Debug::report("  pCommandNode: %s", IReportable::rtInformation, IReportable::rlExpected, pInstructionNode->uniqueXPathToNode(&m_ibqe_nodeControl, 0, false, false, false, false));

        //if we are stepping then stop at every statement, update and wait
        //this may be caused by any breakpoint system, e.g. <user:breakpoint/>
        if (isSteppingThread() && m_bStepping) {
          //update the User nodes with the situation
          //m_ibqe_nodeControl has triggers disabled
          NOT_COMPLETE("needs changing to xpath, not @xml:id");
          
          if (!pDebugNode) pDebugNode = m_pNode->getOrCreateTransientChildElement(&m_ibqe_nodeControl, "debug", NULL, NAMESPACE_GS);
          sCommandXmlID = pInstructionNode->xmlID(&m_ibqe_nodeControl);
          sSourceXmlID  = pTCtxt->sourceNode(pQE) ? pTCtxt->sourceNode(pQE)->xmlID(&m_ibqe_nodeControl) : 0; //pQE as MM
          sOutputXmlID  = pTCtxt->outputNode(pQE) ? pTCtxt->outputNode(pQE)->xmlID(&m_ibqe_nodeControl) : 0; //pQE as MM
          if (sCommandXmlID) pDebugNode->setAttribute(&m_ibqe_nodeControl, "current-xsl-command", sCommandXmlID);
          if (sSourceXmlID)  pDebugNode->setAttribute(&m_ibqe_nodeControl, "current-xml-source",  sSourceXmlID);
          if (sOutputXmlID)  pDebugNode->setAttribute(&m_ibqe_nodeControl, "current-xml-output",  sOutputXmlID);
          pDebugNode->setAttribute(&m_ibqe_nodeControl, "breakpoint", bOnBreakpoint);

          //inform listeners to this User of the update
          //this will release listener threads on this User and ajax update the User with the new details
          sSteppingThread = itoa(m_steppingThread);
          Debug::report("m_semaphoreStep:\n  thread [%s]\n  command [%s]\n  source [%s]\n  output [%s]",
            sSteppingThread,
            sCommandXmlID,
            sSourceXmlID,
            sOutputXmlID);
          m_pNode->touch(pQE, "transient debug stepping information updated on the User node");

          //wait for next stepping command
          sem_wait(&m_semaphoreStep);
        }
      }
    }

    //free up
    //if (pTCtxt)                delete pTCtxt; //server controlled
    if (pInstructionNode)      delete pInstructionNode; //TODO: make this a local var eventually
    if (sCommandXmlID)         MMO_FREE(sCommandXmlID);
    if (sSourceXmlID)          MMO_FREE(sSourceXmlID);
    if (sOutputXmlID)          MMO_FREE(sOutputXmlID);
    if (sSteppingThread)       MMO_FREE(sSteppingThread);
    if (sBreakpointValue && sBreakpointValue != sBreakpoint) MMO_FREE(sBreakpointValue);
    if (pDebugNode)            delete pDebugNode;
    if (pBreakpointCollection) delete pBreakpointCollection;
    //if (sBreakpoint)           MMO_FREE(sBreakpoint); //pointer in to xmlNode

    return iIncludeNode;
  }

  IXmlBaseNode *User::setBreakpoint(IXmlBaseNode *pBreakpointNode) {
    //breakpoints are hardlinked under the gs:breakpoints collection in User
    //these breakpoints natural singular mothers are left un-affected so traversal will be natural and un-changed
    //the only knowledge of their precense is in the additional harlinked_next linked list in _xmlNode
    IXmlBaseNode *pDebugNode            = 0,       //object:User/gs:debug
                 *pBreakpointCollection = 0,       //object:User/gs:debug/gs:breakpoints/(hardlinks!)
                 *pResultNode           = 0;

    UNWIND_EXCEPTION_BEGIN {
      pDebugNode            = m_pNode->getOrCreateTransientChildElement(&m_ibqe_nodeControl, "debug", NULL, NAMESPACE_GS);
      pBreakpointCollection = pDebugNode->getOrCreateChildElement(&m_ibqe_nodeControl, "breakpoints", NULL, NAMESPACE_GS);
      pResultNode           = pBreakpointCollection->hardlinkChild(&m_ibqe_nodeControl, pBreakpointNode);

      if (pResultNode) Debug::report("new breakpoint set, re-setting breakpoint vector");
      else             throw FailedToSetBreakpoint(this);

      //pre-imptive cache
      if (m_pvUserBreakpoints) delete m_pvUserBreakpoints->element_destroy();
      m_pvUserBreakpoints   = pBreakpointCollection->children(&m_ibqe_nodeControl);;
    } UNWIND_EXCEPTION_END;

    //free up
    if (pDebugNode)            delete pDebugNode;
    if (pBreakpointCollection) delete pBreakpointCollection;

    UNWIND_EXCEPTION_THROW;
    
    return pResultNode;
  }

  void User::clearBreakpoint(IXmlBaseNode *pBreakpointNode) {
    NOT_COMPLETE("clearBreakpoint");
  }

  void User::stepInit() {
    //stepping is limited only to 1 single thread per Session
    //the first thread to hit any of the Users hardlinked breakpoints
    //this MUST be un-done at the end of each Request::responseTransform() with stepEnd()
    pthread_mutex_lock(&m_mStepThreadManagement); {
      if (!m_steppingThread) {
        m_steppingThread = pthread_self();
        Debug::report("stepInit [%s]", m_steppingThread);
      }
    } pthread_mutex_unlock(&m_mStepThreadManagement);
  }

  bool User::isInitiated() const {
    return m_steppingThread != 0;
  }

  bool User::isSteppingThread() const {
    return m_steppingThread == pthread_self();
  }

  //client debug flow controls
  void User::stepOver() {
    NOT_COMPLETE("stepOver");
    //m_pNextStop = ...
  }

  void User::stepInto() {
    sem_post(&m_semaphoreStep);
  }

  void User::stepOut()  {
    NOT_COMPLETE("stepOut");
    //m_pNextStop = ...
  }

  void User::stepContinue()  {
    m_bStepping = false;
    sem_post(&m_semaphoreStep);
  }

  void User::stepEnd()  {
    //cancels this debug session completely
    //breakpoints are still kept
    //but a new thread can now hit them
    //only the stepping thread should be able to do this
    if (m_steppingThread == pthread_self()) {
      //PERFORMANCE: DO NOT un-cache breakpoint knowledge
      //if (m_pvUserBreakpoints) delete m_pvUserBreakpoints->element_destroy();
      //m_pvUserBreakpoints  = 0;
      m_bStepping      = false;
      m_steppingThread = 0;
      sem_post(&m_semaphoreStep);
    }
  }

  IXmlBaseNode *User::xslCommand_breakpoint(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //we do nothing here because the normal informAndWait() will spot this
    return 0;
  }

  IXmlBaseNode *User::xslCommand_stepOver(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    stepOver();
    return 0;
  }

  IXmlBaseNode *User::xslCommand_stepInto(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    stepInto();
    return 0;
  }

  IXmlBaseNode *User::xslCommand_stepOut(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    stepOut();
    return 0;
  }

  IXmlBaseNode *User::xslCommand_stepContinue(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    stepContinue();
    return 0;
  }

  const char *User::xslFunction_stepOver(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    stepOver();
    return 0;
  }

  const char *User::xslFunction_stepInto(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    stepInto();
    return 0;
  }

  const char *User::xslFunction_stepOut(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    stepOut();
    return 0;
  }

  const char *User::xslFunction_stepContinue(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    stepContinue();
    return 0;
  }

  const char *User::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "user:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *User::xslFunction_setBreakpoint(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    IXmlBaseNode *pBreakpoint = 0,
                 *pResultNode = 0;

    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);

    UNWIND_EXCEPTION_BEGIN {
      if (pBreakpoint = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        if (pResultNode = setBreakpoint(pBreakpoint)) {
          pNewNodes->push_back(pResultNode);
        }
      } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("No place"), MM_STRDUP("user:set-breakpoint(node)"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (pBreakpoint) delete pBreakpoint;
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *User::xslFunction_clearBreakpoint(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    NOT_COMPLETE("xslFunction_clearBreakpoint");
    //clearBreakpoint();
    return 0;
  }
}
