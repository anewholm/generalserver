//platform independent
#include "TXmlProcessor.h"

#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "Server.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Xml/XmlNodeList.h"
#include "Debug.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c" //vector_element_destroy(...)

#include <algorithm>

namespace general_server {
  TXmlProcessor::TXmlProcessor(const IMemoryLifetimeOwner *pMemoryLifetimeOwner):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_bEnabled(true)
  {}

  const char *TXmlProcessor::getCommitStatusName(TXml::commitStatus iCommitStatus) const {
    //returns a constant so dont free!
    const char *sCommitStatusName = 0;
    switch (iCommitStatus) {
      case TXml::commitStatusUnknown:           {sCommitStatusName = "commitStatusUnknown";           break;}
      case TXml::beginCommit:                   {sCommitStatusName = "beginCommit";                   break;}
      case TXml::beginCommitCreatePlaceholders: {sCommitStatusName = "beginCommitCreatePlaceholders"; break;}
      case TXml::beginCommitTransactionDelete:  {sCommitStatusName = "beginCommitTransactionDelete";  break;}
      case TXml::beginCommitSwapInPlaceholders: {sCommitStatusName = "beginCommitSwapInPlaceholders"; break;}
      case TXml::commitFinished:                {sCommitStatusName = "commitFinished";                break;}
      default: throw InvalidCommitStatus(this, itoa(iCommitStatus));
    }
    return sCommitStatusName;
  }
  
  const char *TXmlProcessor::getCommitStyleName(TXml::commitStyle ct) const {
    //returns a constant so dont free!
    const char *sCommitStyleName = 0;
    switch (ct) {
      case TXml::full:        {sCommitStyleName = "full";        break;}
      case TXml::selective:   {sCommitStyleName = "selective";   break;}
      case TXml::notRequired: {sCommitStyleName = "notRequired"; break;}
      default: throw InvalidCommitStyle(this, itoa(ct));
    }
    return sCommitStyleName;
  }
  
  TXml::commitStyle  TXmlProcessor::parseCommitStyle( const char *sCommitStyleName)  const {
    TXml::commitStyle iCommitStyle = TXml::notRequired;
    if (sCommitStyleName) {
      if      (!strcmp(sCommitStyleName, "notRequired")) iCommitStyle = TXml::notRequired;
      else if (!strcmp(sCommitStyleName, "selective"))   iCommitStyle = TXml::selective;
      else if (!strcmp(sCommitStyleName, "full"))        iCommitStyle = TXml::full;
      else throw InvalidCommitStyleName(this, sCommitStyleName);
    }
    return iCommitStyle;
  }

  TXml::commitStatus TXmlProcessor::parseCommitStatus(const char *sCommitStatusName)  const {
    TXml::commitStatus iCommitStatus = TXml::commitStatusUnknown;
    if (sCommitStatusName) {
      if      (!strcmp(sCommitStatusName, "beginCommit"))                   iCommitStatus = TXml::beginCommit;
      else if (!strcmp(sCommitStatusName, "beginCommitCreatePlaceholders")) iCommitStatus = TXml::beginCommitCreatePlaceholders;
      else if (!strcmp(sCommitStatusName, "beginCommitTransactionDelete"))  iCommitStatus = TXml::beginCommitTransactionDelete;
      else if (!strcmp(sCommitStatusName, "beginCommitSwapInPlaceholders")) iCommitStatus = TXml::beginCommitSwapInPlaceholders;
      else if (!strcmp(sCommitStatusName, "commitFinished"))                iCommitStatus = TXml::commitFinished;
      else throw InvalidCommitStatusName(this, sCommitStatusName);
    }
    return iCommitStatus;
  }

  int TXmlProcessor::applyStartupTransactions(IXmlBaseDoc *pDoc) {
    //apply outstanding standardised startup transactions from transactions node
    //after load from a repository
    const vector<TXml*> *pvTXmls = 0;
    vector<TXml*>::const_iterator iTXml;
    TXml *pTXml = 0;
    TXml::transactionResult tr;
    const char *sTotal  = 0, 
               *sFailed = 0;
    int iTotal          = 0, 
        iFailed         = 0;

    if (pvTXmls = transactions(pDoc)) {
      iTotal = pvTXmls->size();
      sTotal = itoa(iTotal);
      if (iTotal) {
        Debug::report("Applying [%s] startup transactions", sTotal);
        Debug::startProgressReport();

        for (iTXml = pvTXmls->begin(); iTXml != pvTXmls->end(); iTXml++) {
          pTXml = (*iTXml);
          //Debug::reportObject(pTXml);

          try {
            //only apply the transaction to the IXmlDoc, not the associated Repositorys
            tr = reapplyTransaction(pTXml);
            if (tr != TXml::transactionSuccess) {
              //TODO: TXml::transactionFailed
            }
          } catch (TXmlRequiredNodeNotFound &ex) {
            iFailed++;
          }
          Debug::progressReport();
        }
        Debug::completeProgressReport();

        //report
        if (iFailed) {
          sFailed = itoa(iFailed);
          Debug::report("completed with [%s] failures\n", sFailed);
        }
      }
    }

    //free up
    if (sTotal)  MMO_FREE(sTotal);
    if (sFailed) MMO_FREE(sFailed);
    if (pvTXmls) vector_const_element_destroy(pvTXmls);

    return iTotal;
  }

  bool TXmlProcessor::rationaliseAffectedAncestorsVector(vector<TXml::AffectedNodeDetails> *pvAncestors) {
    //create a distinct list of areas of an IXmlBaseDoc from a vector of AffectedNodeDetails
    //this means that if a area effect node is an ancestor of other nodes in the array then they should be removed
    vector<TXml::AffectedNodeDetails>::iterator iAncestorNode, iDecendentNode;
    TXml::AffectedNodeDetails andAncestorNode, andDecendentNode;
    const IXmlBaseNode *pAncestorNode, *pDecendentNode;
    size_t iInitialSize = pvAncestors->size();

    for (iAncestorNode = pvAncestors->begin(); iAncestorNode != pvAncestors->end(); iAncestorNode++) {
      andAncestorNode = *iAncestorNode;
      pAncestorNode   = andAncestorNode.pRelatedNode;
      const XmlAdminQueryEnvironment ibqe_commit(this, pAncestorNode->document());

      switch (andAncestorNode.iAffectedScope) {
        //these cause sym-links, they don't change the area underneath
        //so don't replace descendants
        case TXml::linkChild:

        //remove node will only delete the indicated child it will not save the entire parent node pRelatedNode tree
        //so don't remove the descendants of the pRelatedNode because it is just the parent and won't save all its children
        //these are caused by moveChild and removeNode
        //removeChild cannot remove a saveTree
        case TXml::syncImmediateChildren:

        //these are additional if ancestors so keep their descendant AffectedNodeDetails
        //because they affect a different axis, e.g. a Directory name, not the Files in it
        case TXml::saveElementOnly: {
          iDecendentNode = pvAncestors->end();
          while (iDecendentNode != pvAncestors->begin()) {
            iDecendentNode--;
            if (iAncestorNode != iDecendentNode) {
              andDecendentNode = *iDecendentNode;
              pDecendentNode   = andDecendentNode.pRelatedNode;

              //erase identical requests
              if (andDecendentNode.iAffectedScope == andAncestorNode.iAffectedScope && pAncestorNode->is(pDecendentNode, HARDLINK_AWARE)) {
                pvAncestors->erase(iDecendentNode);
                //erased one the before iAncestorNode?
                if (iDecendentNode < iAncestorNode) iAncestorNode--;
              }
            }
          }
          break;
        }

        //this re-writes the node and area underneath so we need to remove ALL descendant AffectedNodeDetails
        //and also the current node, with name and attributes and all removeNode's
        //these are caused by moveChild, copyChild and saveTree also
        case TXml::saveTree: {
          //traverse backwards so that erasing after elements does not cause navigation issues
          iDecendentNode = pvAncestors->end();
          while (iDecendentNode != pvAncestors->begin()) {
            iDecendentNode--;
            if (iAncestorNode != iDecendentNode) {
              andDecendentNode = *iDecendentNode;
              pDecendentNode   = andDecendentNode.pRelatedNode;

              //erase all other requests on this node, or its descendants
              if (pAncestorNode->is(pDecendentNode, HARDLINK_AWARE) || pAncestorNode->isAncestorOf(&ibqe_commit, pDecendentNode)) {
                //the ancestor saveTree node is equal to or above this descendant node, so remove it
                pvAncestors->erase(iDecendentNode);
                //we are erasing vector elements whilst traversing in the outside loop
                //iterators are simple pointers that increment by sizeof(whatever)
                //so comparison and movement are simple
                //erased one the before iAncestorNode?
                if (iDecendentNode < iAncestorNode) iAncestorNode--;
              }
            }
          }
        }
      }
    }

    return iInitialSize != pvAncestors->size();
  }

  TXml::transactionResult TXmlProcessor::commit(TXml::commitStatus cs, TXml::commitStyle ct) {throw CapabilityNotSupported(this, MM_STRDUP("top level commit"));}
  TXml::transactionResult TXmlProcessor::commitCreatePlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) {throw CapabilityNotSupported(this, MM_STRDUP("commit"));}
  TXml::transactionResult TXmlProcessor::commitSwapInPlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) {throw CapabilityNotSupported(this, MM_STRDUP("commit"));}
  TXml::transactionResult TXmlProcessor::commitGetPersistentStatus(TXml::commitStatus *cs, TXml::commitStyle *ct) {throw CapabilityNotSupported(this, MM_STRDUP("commitGetPersistentStatus"));}
  TXml::transactionResult TXmlProcessor::commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct) {throw CapabilityNotSupported(this, MM_STRDUP("setPersistentCommitStatus"));}

  TXml::transactionResult TXmlProcessor::commit(const IXmlBaseDoc *pDoc, const IXmlBaseNode *pTransactionDisplayNode, TXml::commitStatus cs, TXml::commitStyle ct) {
    //called mostly from Database TXmlProcessor level
    //for each transaction save the permanent repository part(s)
    //referenced from the passed IXmlBaseDoc
    //  Repository: A permanent, slow, *non-queryable* structure XML store
    //  IXmlDoc:    A temporary, high-speed, queryable           XML store
    //Database: Repositories need an associated IXmlDoc in order to sync changes
    assert(pDoc);
    assert(pTransactionDisplayNode);

    TXml::transactionResult tr = TXml::transactionSuccess;

    const vector<TXml*> *pvTXmls = 0;
    vector<TXml*>::const_iterator iTXml;
    TXml *pTXml = 0;
    vector<TXml::AffectedNodeDetails> vAffectedNodes;
    vector<TXml::AffectedNodeDetails>::iterator iAffectedNode;

    TXmlProcessor *pTXmlProcessor = 0;
    vector<TXmlProcessor*>::const_iterator iTXmlProcessor;
    const vector<TXmlProcessor*> *pvLinkedTXmlProcessors = 0;

    int iTotal;

    //get current transaction list
    //and compile affected nodes list for saving
    Debug::report("begin commit on [%s]", name()); //TXmlProcessor owns name() so no free
    if (pvTXmls = transactions(pDoc)) {
      switch (ct) {
        case TXml::selective: {
          iTotal = pvTXmls->size();
          if (iTotal) {
            Debug::report("  compile affected nodes for selective commit from %s transactions", iTotal);
            for (iTXml = pvTXmls->begin(); iTXml != pvTXmls->end(); iTXml++) {
              pTXml = (*iTXml);
              //applies unique
              pTXml->affectedNodes(&vAffectedNodes, pDoc);
            }
            rationaliseAffectedAncestorsVector(&vAffectedNodes);
          } else Debug::report("  no transactions");
          break;
        }
        case TXml::full: {
          //affected node is just the root and we saveTree on the whole thing
          //commit(...) sub calls are the same as a selective
          //rootNode() deleted below
          const XmlAdminQueryEnvironment ibqe_commit(this, pDoc);
          Debug::report("  setting root node saveTree for full commit");
          iTotal = 1;
          TXml::AffectedNodeDetails andetails = {TXml::saveTree, pDoc->rootNode(&ibqe_commit), 0, 0, 0, 0};
          vAffectedNodes.push_back(andetails);
          break;
        }
        case TXml::notRequired: break;
      }

      if (iTotal) {
        //get repository(s)
        pvLinkedTXmlProcessors = linkedTXmlProcessors();
        commitSetPersistentStatus(TXml::beginCommit, ct);
        for (iTXmlProcessor = pvLinkedTXmlProcessors->begin(); iTXmlProcessor != pvLinkedTXmlProcessors->end(); iTXmlProcessor++) {
          //Repository being asked to commit from an IXmlBaseDoc
          //pass decision for commiting the doc changes to the derived Repository object
          pTXmlProcessor = (*iTXmlProcessor);
          tr = pTXmlProcessor->commit(pDoc, pvTXmls, &vAffectedNodes, cs, ct);
          //report
          if (tr == TXml::transactionFailure) {
            //TODO: ...
          }
        }

        //delete in-memory transactions also
        Debug::report("  deleteing in-memory IXmlDoc transaction list");
        commitSetPersistentStatus(TXml::beginCommitTransactionDelete, ct);
        commitDeleteTransactions(pvTXmls);

        //complete
        commitSetPersistentStatus(TXml::commitFinished, ct);
        Debug::report("  :)");
      }
    } else Debug::report("  transaction list not found, ignore not applicable");

    //clear up
    if (pvLinkedTXmlProcessors) delete pvLinkedTXmlProcessors;
    if (pvTXmls)               vector_const_element_destroy(pvTXmls);
    for (iAffectedNode = vAffectedNodes.begin(); iAffectedNode != vAffectedNodes.end(); iAffectedNode++) delete (*iAffectedNode).pRelatedNode;

    return tr;
  }

  TXml::transactionResult TXmlProcessor::commit(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes, TXml::commitStatus csStartPoint, TXml::commitStyle ct) {
    TXml::transactionResult tr = TXml::transactionNotStarted;
    Debug::report("  processing linked Repository [%s] of type [%s] with commitStyle [%s]", name(), source(), getCommitStyleName(ct)); //Repository owns name()
    if (csStartPoint > TXml::beginCommit) Debug::report("  starting from [%s] saved point", getCommitStatusName(csStartPoint));

    //create the new stores next to the existing ones
    //status is set here for all child repository types. its not that we dont trust them of course
    //basically this means creating new files and directories with an 0_commit_<filename> at the start
    //starts with a number because XML elements cannot
    if (csStartPoint <= TXml::beginCommitCreatePlaceholders) {
      Debug::report("    beginCommitCreatePlaceholders");
      commitSetPersistentStatus(TXml::beginCommitCreatePlaceholders, ct);
      tr = commitCreatePlaceholders(pDoc, pvTXmls, pvAffectedNodes);
    }

    //swap in the placeholders to live
    //move live files to 0_commit_<filename>
    //starts with a number because XML elements cannot
    if (csStartPoint <= TXml::beginCommitSwapInPlaceholders) {
      Debug::report("    beginCommitSwapInPlaceholders");
      commitSetPersistentStatus(TXml::beginCommitSwapInPlaceholders, ct);
      tr = commitSwapInPlaceholders(pDoc, pvTXmls, pvAffectedNodes);
    }

    //remove all the repositories transactions
    //e.g. the transactions file(s) at transactions/transactions.xml
    if (csStartPoint <= TXml::beginCommitTransactionDelete) {
      Debug::report("    beginCommitTransactionDelete");
      commitSetPersistentStatus(TXml::beginCommitTransactionDelete, ct);
      tr = commitDeleteTransactions(pvTXmls);
    }

    //finish
    commitSetPersistentStatus(TXml::commitFinished, ct); //deletes the status file

    return tr;
  }

  TXml::transactionResult TXmlProcessor::commitDeleteTransactions(const vector<TXml*> *pvTXmls) {
    TXml::transactionResult tr = TXml::transactionSuccess;
    return tr;
  }

  TXml::transactionResult TXmlProcessor::reapplyTransaction(TXml *t, IXmlBaseNode *pSerialisationNode, const IXmlQueryEnvironment *pDirectPassedQE) {
    //only use this default function if ITXmlDirectFunctions is supported
    //otherwise it will throw up
    TXml::transactionResult tr = TXml::transactionFailure;
    ITXmlDirectFunctions *pThisType = queryInterface((ITXmlDirectFunctions*) 0);
    if (pThisType) {
      tr = t->applyTo(pThisType, NO_RESULTS_NODE, pDirectPassedQE);
    } else throw InterfaceNotSupported(this, MM_STRDUP("ITXmlDirectFunctions*"));

    return tr;
  }
  
  TXml::transactionResult TXmlProcessor::applyTransaction(TXml *t, IXmlBaseNode *pSerialisationNode, const IXmlQueryEnvironment *pDirectPassedQE) {
    TXml::transactionResult tr = TXml::transactionFailure;

    //apply
    //note that TXml is updated with results (created node FQName, xmlId, repository:name)
    //  these are then serialised in to the linkedTXmlProcessors (Repositories)
    ITXmlDirectFunctions *pThisType = queryInterface((ITXmlDirectFunctions*) 0);

    UNWIND_EXCEPTION_BEGIN {
      if (pThisType) {
        try {
          //IFDEBUG(dynamic_cast<const Database*>(this)->validityCheck("check Database before TXml application"));
          //this application may involve a transform, and thus a new cached stylesheet
          tr = t->applyTo(pThisType, NO_RESULTS_NODE, pDirectPassedQE);
          //IFDEBUG(dynamic_cast<const Database*>(this)->validityCheck("check Database after TXml application"));
        } catch (ExceptionBase &eb) {
          throw TXmlApplicationFailed(eb, t);
        } catch (exception &ex) {
          throw TXmlApplicationFailed(StandardException(this, ex), t);
        } catch (...) {throw;}
      } else throw InterfaceNotSupported(this, MM_STRDUP("ITXmlDirectFunctions*"));

      //reporting and further actions
      switch (tr) {
        case TXml::transactionSuccess: transactionSucceeded(t,  pSerialisationNode); break;
        case TXml::transactionNotStarted:           ATTRIBUTE_FALLTHROUGH;
        //TODO: applyTo() is throwing exceptions for transactionFailureSecurity instead...
        case TXml::transactionFailureSecurity:      ATTRIBUTE_FALLTHROUGH;
        case TXml::transactionFailureDestinationInvalid: ATTRIBUTE_FALLTHROUGH;
        case TXml::transactionFailureSourceInvalid: ATTRIBUTE_FALLTHROUGH;
        case TXml::transactionFailureBeforeInvalid: ATTRIBUTE_FALLTHROUGH;
        case TXml::transactionFailureToLazy:        ATTRIBUTE_FALLTHROUGH;
        case TXml::transactionFailure: 
          transactionFailed(t, tr, pSerialisationNode); break;
        case TXml::transactionDisabled: {
          //intentionally disabled
        }
      }
    } UNWIND_EXCEPTION_END;

    UNWIND_EXCEPTION_THROW;

    return tr;
  }

  void TXmlProcessor::transactionSucceeded(TXml *t, IXmlBaseNode *pSerialisationNode) {
    const IXmlBaseNode *pTransactionNode = 0;
    TXml::transactionResult tr           = TXml::transactionNotStarted;
    const IXmlBaseDoc *pInThisDoc        = 0;
    vector<TXmlProcessor*>::const_iterator ilinkedTXmlProcessor;
    const vector<TXmlProcessor*> *pvLinkedTXmlProcessors = linkedTXmlProcessors();

    //apply same transaction to all associated linked transaction capable stores
    //  unless it is transient in the context of the associated in-memory-document
    //if we have a valid in-memory-document
    if (pInThisDoc = document_const()) {
      if (t->m_bLastDocIsRepeatableAfterReload) {
        if (pvLinkedTXmlProcessors) {
          for (ilinkedTXmlProcessor = pvLinkedTXmlProcessors->begin(); ilinkedTXmlProcessor != pvLinkedTXmlProcessors->end(); ilinkedTXmlProcessor++) {
            tr = (*ilinkedTXmlProcessor)->applyTransaction(t, pSerialisationNode);
          }
        }
        //display the TXml in the IXml*
        pTransactionNode = transactionDisplay(t, tr, pSerialisationNode);
      }
    }

    //free up
    if (pTransactionNode)       delete pTransactionNode;
    if (pvLinkedTXmlProcessors) delete pvLinkedTXmlProcessors;
    //vector_const_element_destroy(pvLinkedTXmlProcessors); //don't want to delete the Repository!
  }

  void TXmlProcessor::disableLinkedProcessors() {
    vector<TXmlProcessor*>::const_iterator ilinkedTXmlProcessor;
    const vector<TXmlProcessor*> *pvLinkedTXmlProcessors = linkedTXmlProcessors();

    if (pvLinkedTXmlProcessors) {
      for (ilinkedTXmlProcessor = pvLinkedTXmlProcessors->begin(); ilinkedTXmlProcessor != pvLinkedTXmlProcessors->end(); ilinkedTXmlProcessor++)
        (*ilinkedTXmlProcessor)->disable();
    }

    //free up
    if (pvLinkedTXmlProcessors) delete pvLinkedTXmlProcessors;
    //vector_const_element_destroy(pvLinkedTXmlProcessors); //don't want to delete the Repository!
  }

  void TXmlProcessor::enableLinkedProcessors() {
    vector<TXmlProcessor*>::const_iterator ilinkedTXmlProcessor;
    const vector<TXmlProcessor*> *pvLinkedTXmlProcessors = linkedTXmlProcessors();

    if (pvLinkedTXmlProcessors) {
      for (ilinkedTXmlProcessor = pvLinkedTXmlProcessors->begin(); ilinkedTXmlProcessor != pvLinkedTXmlProcessors->end(); ilinkedTXmlProcessor++)
        (*ilinkedTXmlProcessor)->enable();
    }

    //free up
    if (pvLinkedTXmlProcessors) delete pvLinkedTXmlProcessors;
    //vector_const_element_destroy(pvLinkedTXmlProcessors); //don't want to delete the Repository!
  }

  bool TXmlProcessor::enabled() {
    return m_bEnabled;
  }
  
  bool TXmlProcessor::enable() {
    bool bEnabled = m_bEnabled;
    m_bEnabled = true;
    return bEnabled;
  }
  
  bool TXmlProcessor::disable() {
    bool bEnabled = m_bEnabled;
    m_bEnabled = false;
    return bEnabled;
  }
  
  IXmlBaseNode *TXmlProcessor::transactionDisplayNode(IXmlBaseNode *pSerialisationNode) {
    return 0;
  }

  const vector<TXml*>  *TXmlProcessor::transactions(const IXmlBaseDoc *pDoc) {
    //caller frees non-zero result
    const XmlAdminQueryEnvironment ibqe_commit(this, pDoc);
    const IXmlBaseNode *pTransactionDisplayNode;
    const XmlNodeList<const IXmlBaseNode> *pvTXmlNodes = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iTXmlNode;
    vector<TXml*> *pvTXmls = 0;

    if (pTransactionDisplayNode = transactionDisplayNode(0)) {
      pvTXmlNodes = pTransactionDisplayNode->children(&ibqe_commit);
      pvTXmls     = new vector<TXml*>();
      for (iTXmlNode = pvTXmlNodes->begin(); iTXmlNode != pvTXmlNodes->end(); iTXmlNode++)
        pvTXmls->push_back(TXml::factory(this, &ibqe_commit, this, *iTXmlNode));
    }

    //free up
    if (pvTXmlNodes) vector_const_element_destroy(pvTXmlNodes);

    return pvTXmls;
  }

  const IXmlBaseNode *TXmlProcessor::transactionDisplay(const TXml *t, TXml::transactionResult tr, IXmlBaseNode *pSerialisationNode) {
    //caller frees result
    //add the node version directly in to the document
    IXmlBaseNode *pNewTransactionNode     = 0;
    IXmlBaseNode *pTransactionDisplayNode = transactionDisplayNode();

    if (pTransactionDisplayNode)
      pNewTransactionNode = t->toNode(pTransactionDisplayNode); //adds the node directly under the parent anyway

    return pNewTransactionNode;
  }
}
