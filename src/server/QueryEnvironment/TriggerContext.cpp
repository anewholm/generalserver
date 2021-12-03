//platform agnostic
#include "TriggerContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslDoc.h"
#include "Database.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include <pthread.h>
#include <algorithm>

#include "Utilities/container.c"

namespace general_server {
 /*
  * Generic low-level direct triggering
  * Generally used for:
  * These are mostly designed to push design up to the next layer
  * rather than hardcoded or parameterised implementation here
  * hooks:
  *   the security hook fires before attemps to browse a node, for any reason. OP is passed in and through
  *   read triggers fire before and receive the context and xpath context
  *     useful for [re]populating dynamic content
  *     useful examining the xpath query and pre-populating the result only with required contents (and child count)
  *     sub-functions return xmlNodeListPtr not xmlDocPtr
  *     <repository:employers       database:on-before-read="database:replace-children(., postgres:get-query-results())" />    <-- xpath context input
  *     <repository:namespace-cache database:on-before-read="database:replace-children(., server:serialise('namespace-cache'))" />
  *     <repository:cron            database:on-before-read="database:replace-children(., repository:rx('/etc/crontab', ../rx:cron-tab))" />
  *     xpath: repository:employers/postgres:employer[@eid = 9]
  *   write triggers fire before and after write attempts
  *     the before trigger can cancel the write attempt
  *     useful for releasing watches on content
  *     @Database:on-after-write="database:transform(., )"
  *     @Database:on-after-write="database:move-node(., ../repository:archived)"
  *     @Database:on-after-write="database:set-attribute(@count, @count + 1)"
  */
  pthread_mutex_t TriggerContextDatabaseAttribute::m_mGlobalRunningTriggersManagementMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  XmlNodeMap<pthread_mutex_t> TriggerContextDatabaseAttribute::m_mGlobalRunningTriggers;

  TriggerContextDatabaseAttribute::TriggerContextDatabaseAttribute(Database *pDatabase, IXmlBaseDoc *pSingleWriteableDocumentContext):
    m_pDatabase(pDatabase),
    m_vCyclicRunningTriggers(pDatabase),
    m_pSingleWriteableDocumentContext(pSingleWriteableDocumentContext),
    m_bEnabled(true),
    m_pOwnerQE(0)
  {
    assert(m_pDatabase);
    assert(m_pSingleWriteableDocumentContext);
  }

  TriggerContextDatabaseAttribute::TriggerContextDatabaseAttribute(const TriggerContextDatabaseAttribute& trig):
    m_pDatabase(trig.m_pDatabase),
    m_pSingleWriteableDocumentContext(trig.m_pSingleWriteableDocumentContext),
    m_bEnabled(trig.m_bEnabled),
    m_pOwnerQE(0),
    m_vCyclicRunningTriggers(trig.m_vCyclicRunningTriggers)   //yay! copy constructor :)
  {}

  void TriggerContextDatabaseAttribute::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}

  const char *TriggerContextDatabaseAttribute::toString() const {
    stringstream sOut;
    XmlNodeList<const IXmlBaseNode>::const_iterator iRunningTrigger;
    const IXmlBaseNode *pTriggerNode;
    const char *sNodeName;

    sOut << "TriggerContextDatabaseAttribute";
    if (m_vCyclicRunningTriggers.size()) {
      sOut << ":\n  running triggers [";
      for (iRunningTrigger = m_vCyclicRunningTriggers.begin(); iRunningTrigger != m_vCyclicRunningTriggers.end(); iRunningTrigger++) {
        pTriggerNode = *iRunningTrigger;
        sNodeName = pTriggerNode->fullyQualifiedName();
        if (iRunningTrigger != m_vCyclicRunningTriggers.begin()) sOut << ", ";
        sOut << sNodeName;
      }
      sOut << "]";
    }

    return MM_STRDUP(sOut.str().c_str());
  }
  const char *TriggerContextDatabaseAttribute::type() const {return "TriggerContextDatabaseAttribute";}

  TriggerContextDatabaseAttribute *TriggerContextDatabaseAttribute::clone_with_resources() const {
    return new TriggerContextDatabaseAttribute(*this);
  }

  IXmlTriggerContext *TriggerContextDatabaseAttribute::inherit() const {
    return clone_with_resources();
  }

  bool TriggerContextDatabaseAttribute::cyclicTrigger(IXmlBaseNode *pCur) const {
    return pCur->in(&m_vCyclicRunningTriggers, EXACT_MATCH);
  }

  void TriggerContextDatabaseAttribute::addRunningTrigger(IXmlBaseNode *pCur) {
    m_vCyclicRunningTriggers.push_back(pCur);
  }

  bool TriggerContextDatabaseAttribute::removeRunningTrigger(IXmlBaseNode *pCur) {
    XmlNodeList<const IXmlBaseNode>::iterator iEntry = find(m_vCyclicRunningTriggers.begin(), m_vCyclicRunningTriggers.end(), pCur);
    bool bFound = (iEntry != m_vCyclicRunningTriggers.end());
    if (bFound) m_vCyclicRunningTriggers.erase(iEntry);
    return bFound;
  }

  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::runAttributeTrigger(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur, const char *sAttributeLocalName) {
    //we default to 5 seconds just to trap multiple calls
    //use @Database:on-before-read-cachetime to alter this
    assert(sAttributeLocalName);
    assert(pQE == m_pOwnerQE);
    assert(pQE->triggerContext() == this);

    IXmlQueryEnvironment::accessResult iContinue = IXmlQueryEnvironment::RESULT_INCLUDE;
    const XmlAdminQueryEnvironment ibqe_trigger_manager(m_pDatabase, m_pSingleWriteableDocumentContext); //trigger manager (not trigger runner)
    IXmlQueryEnvironment *pQETriggerEnvironment = 0; //trigger running environment
    IXmlTriggerContext   *pTrigger              = 0; //QE will inherit the trigger
    const char *sXPathCommand                   = 0,
               *sCacheTimeSeconds               = 0,
               *sLastRun                        = 0;
    XmlNodeList<IXmlBaseNode> *pNodeList        = 0;
    XmlNodeMap<pthread_mutex_t>::const_iterator iGlobalTrigger;
    pthread_mutex_t mGlobalTriggerMutex;
    bool bRunAgain = true;
    int iLastRun, iNow;
    string stAttributeNameLastRun;
    int iCacheTimeSeconds = 5;
    IFDEBUG(iCacheTimeSeconds = 30);

    //trigger running concepts:
    //--------- Enabled triggers: triggers are sometimes disabled for admin and startup tasks
    //  TXml should not re-run triggers
    //  changes created by triggers should create additional TXml manually thus recording the results, not the process
    //  m_bEnabled
    //--------- Cyclic triggers: this thread may run a trigger which then runs the *same* trigger
    //  this TriggerContextDatabaseAttribute() instance contains the ancestor triggers list that are in process
    //  if a cyclic condition occurs the child trigger is not run and the ancestor trigger will then complete
    //  m_vCyclicRunningTriggers
    //--------- Cached triggers: triggers default to being run only once every 5 seconds
    //  only because triggers *usually* populate their children from foriegn sources
    //  configurable, can be turned off if required
    //  stAttributeNameLastRun
    //--------- Trigger waiting: if 2 processes try to run the same trigger at the same time then the second will wait
    //  mutex on each IXmlBaseNode trigger
    //  waiting threads will try to re-run the trigger, but get caught by the cache if active
    //  TODO: mutex map is NOT cleaned up on the fly because it is always useful currently, however IXmlBaseNodes might be changed in the long run
    //  m_mGlobalRunningTriggers

    //TODO: reduce the number of // descendant reads at startup
#ifndef WITHOUT_DATABASE_TRIGGER
    if (m_bEnabled) {
      if (sXPathCommand = pCur->attributeValueDirect(&ibqe_trigger_manager, sAttributeLocalName, NAMESPACE_DATABASE)) {
        //ok, we have a trigger decleration
        //check that this trigger instance is not running this as the result of the same trigger (immediate exit, no waiting)
        if (cyclicTrigger(pCur)) {
          //Debug::report("cyclic trigger blocked [%s]", pCur->uniqueXPathToNode(&ibqe_trigger_manager, NO_BASE_NODE, INCLUDE_TRANSIENT), IXmlBaseNode::rtWarning);
        } else {
          //---------------------------------- global trigger run waiting
          //get / lazy create a mutex for this node to use to block with
          //TODO: make an IXmlBaseNode::lock(bWriteOnly = false, bRecursive = true) function
          pthread_mutex_lock(&m_mGlobalRunningTriggersManagementMutex);
          iGlobalTrigger = m_mGlobalRunningTriggers.find(pCur);
          if (iGlobalTrigger != m_mGlobalRunningTriggers.end()) {
            //found an existing mutex
            //someone else is already running this trigger
            //so wait
            mGlobalTriggerMutex = iGlobalTrigger->second;
          } else {
            //no existing mutex
            //no one globally is running this trigger
            //lazy initialisztion of mutexes
            //lock and run
            //TODO: make the mGlobalTriggerMutex recursive just in case cyclicTrigger() fails
            pthread_mutex_init(&mGlobalTriggerMutex, NULL);
            iGlobalTrigger = m_mGlobalRunningTriggers.insert(pCur, mGlobalTriggerMutex);
          }
          pthread_mutex_unlock(&m_mGlobalRunningTriggersManagementMutex);

          pthread_mutex_lock(&mGlobalTriggerMutex); {
            //---------------------------------- check cacheing
            //one at a time: cacheing might be disabled / configured differently
            stAttributeNameLastRun += sAttributeLocalName;
            stAttributeNameLastRun += "-last-run";
            iNow = time(0);
            //using attributeValueDirect() for efficiency: don't need security or deviations
            if (sCacheTimeSeconds = pCur->attributeValueDirect(&ibqe_trigger_manager, "cache-timeout", NAMESPACE_DATABASE)) {
              if (*sCacheTimeSeconds) iCacheTimeSeconds = atoi(sCacheTimeSeconds);
            }
            if (sLastRun = pCur->attributeValueDirect(&ibqe_trigger_manager, stAttributeNameLastRun.c_str(), NAMESPACE_DATABASE)) {
              iLastRun  = atoi(sLastRun);
              bRunAgain = ((iNow - iLastRun) > iCacheTimeSeconds);
            }

            if (bRunAgain) {
              //---------------------------------- running
              //new QE: shouldnt have anything to do with the parent one anyways... apart from:
              //  triggerContext: with runningTriggers to prevent cyclic behaviour
              //  EMO: triggers can use the Server, Request and Response EMO!
              //but not:
              //  security context for trigger
              pQETriggerEnvironment = m_pDatabase->newQueryEnvironment(pQE->profiler(), pQE->emoContext(), this);
              pTrigger              = pQETriggerEnvironment->triggerContext();
              //TODO: log in as the trigger creator
              //pQETriggerEnvironment->securityContext()->login_trusted(ROOT_USERNAME);

              //update the last run time here: this is an additional prevention of cyclic behaviour
              pCur->setAttribute(&ibqe_trigger_manager, stAttributeNameLastRun.c_str(), iNow, NAMESPACE_DATABASE);

              //this getMultipleNodes() which *is* the trigger can now run more triggers
              IFDEBUG(Debug::report("[%s] triggered at [%u]", pCur->fullyQualifiedName(), iNow));
              pTrigger->addRunningTrigger(pCur); {
                //TODO: allow relative paths to trigger node inputs, include cur in the xmlTriggerCall()?
                pNodeList = pCur->getMultipleNodes(pQETriggerEnvironment, sXPathCommand);
              } pTrigger->removeRunningTrigger(pCur);
            } //IFDEBUG(else cout << "cached trigger\n");
          } pthread_mutex_unlock(&mGlobalTriggerMutex);
        }
      }
    }
#endif

    //free up
    //if (sXPathCommand)           MM_FREE(sXPathCommand); //attributeValueDirect()
    //if (sLastRun)                MM_FREE(sLastRun); //attributeValueDirect()
    //if (sCacheTimeSeconds)       MM_FREE(sCacheTimeSeconds); //attributeValueDirect()
    if (pNodeList)               vector_element_destroy(pNodeList);
    if (pQETriggerEnvironment)   delete pQETriggerEnvironment;

    return iContinue;
  }
  
  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::runAttributeTriggerTree(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED, const char *sAttributeLocalName ATTRIBUTE_UNUSED) {
    //TODO: search up the tree for tree scope triggers
    //NOT_COMPLETE(""); //TriggerContextDatabaseAttribute::runAttributeTriggerTree()
    IXmlQueryEnvironment::accessResult iContinue = IXmlQueryEnvironment::RESULT_INCLUDE;

    if (m_bEnabled) {
      //...
    }

    return iContinue;
  }

  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::beforeReadTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) {
    return runAttributeTrigger(pQE, pCur, "on-before-read");
  }
  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::afterReadTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) {
    return runAttributeTrigger(pQE, pCur, "on-after-read");
  }
  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::beforeWriteTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) {
    runAttributeTrigger(    pQE, pCur, "on-before-write");
    runAttributeTriggerTree(pQE, pCur, "on-before-write-tree");
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }
  
  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::beforeAddTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) {
    runAttributeTrigger(    pQE, pCur, "on-before-add");
    runAttributeTriggerTree(pQE, pCur, "on-before-write-add");
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }

  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::afterWriteTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pCur) {
    IFDEBUG(
      if (pCur->isTransient()) {
        //Debug::report("IFDEBUG mode ignoring transient changes on [%s]", pCur->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT));
        return IXmlQueryEnvironment::RESULT_INCLUDE;
      }
    )
    runAttributeTrigger(    pQE, pCur, "on-after-write");
    runAttributeTriggerTree(pQE, pCur, "on-after-write-tree");
    m_pDatabase->registerWriteEvent(pQE, pCur); //will releaseWriteListeners(); and clone pCur
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }
  
  IXmlQueryEnvironment::accessResult TriggerContextDatabaseAttribute::afterAddTriggerCallback(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pParent) {
    IFDEBUG(
      if (pParent->isTransientAreaParent()) {
        //Debug::report("IFDEBUG mode ignoring transient changes on children of [%s]", pParent->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT));
        return IXmlQueryEnvironment::RESULT_INCLUDE;
      }
    )
    runAttributeTrigger(pQE, pParent, "on-after-add");
    m_pDatabase->registerWriteEvent(pQE, pParent); //will releaseWriteListeners(); and clone pCur
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }
}
