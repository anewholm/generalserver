//platform agnostic file
#ifndef _TXMLPROCESSOR_H
#define _TXMLPROCESSOR_H

#include "define.h" //includes platform define also

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "IReportable.h" //direct inheritance
#include "TXml.h"        //enums

#define NO_RESULTS_NODE NULL

namespace general_server {
  //--------------------------------------------------------------------------------------------------
  //----------------------------------------- TXmlProcessor -------------------------------------------
  //--------------------------------------------------------------------------------------------------
  class TXmlProcessor: virtual public MemoryLifetimeOwner {
    //inform the store that about transactions
    //called automatically by default applyTransaction(...)
    //but can to be implemented if overriden
    bool m_bEnabled;

  private:
    virtual bool transactionRequest(  TXml *t ATTRIBUTE_UNUSED) {return true;} //respond with false to prevent (server load, security, etc.)
    virtual void transactionSucceeded(TXml *t, IXmlBaseNode *pSerialisationNode = 0);
    virtual void transactionFailed(   TXml *t ATTRIBUTE_UNUSED, TXml::transactionResult tr ATTRIBUTE_UNUSED, IXmlBaseNode *pSerialisationNode ATTRIBUTE_UNUSED = 0) {} //usually security problems

    //this default virtual function runs standard transaction results on other linked repositories
    //typically this will be the Repository for the IXmlBaseDoc
    virtual const vector<TXmlProcessor*> *linkedTXmlProcessors(IXmlBaseNode *pSerialisationNode ATTRIBUTE_UNUSED = 0) const {return 0;} //typically returns the single repository

    //transaction display
    virtual const IXmlBaseNode *transactionDisplay(const TXml *t, TXml::transactionResult tr, IXmlBaseNode *pSerialisationNode = 0);
    virtual IXmlBaseNode       *transactionDisplayNode(IXmlBaseNode *pSerialisationNode = 0); //this MUST be implemented to enable default transactionDisplay()
    const vector<TXml*>        *transactions(const IXmlBaseDoc *pDoc);

    bool rationaliseAffectedAncestorsVector(vector<TXml::AffectedNodeDetails> *pvAncestors);

  protected:
    TXmlProcessor(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);

    //--------------------------------------------------- info
  public:
    virtual const char *name()   const {return "TXmlProcessor";}
    virtual const char *source() const {return name();}

  protected:
    //returning a valid in-memory-document will enable more TXmlProcessor features
    //this concrete function helps distinguish between:
    //  Repository TXmlProcessor and IXmlBaseDoc TXmlProcessor
    //  by having a valid in-memory doc
    virtual const IXmlBaseDoc *document_const() const {return 0;}

  public:
    //--------------------------------------------------- public commit(...)
    //overview commit process sequence
    virtual TXml::transactionResult commit(TXml::commitStatus csStartPoint = TXml::beginCommit, TXml::commitStyle ct = TXml::selective); //Database level, only works where the TXmlProcessor has an IXmlDoc associated
    virtual TXml::transactionResult commit(const IXmlBaseDoc *pDoc, const IXmlBaseNode *pTransactionDisplayNode, TXml::commitStatus csStartPoint = TXml::beginCommit, TXml::commitStyle ct = TXml::selective);
    virtual TXml::transactionResult commit(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes, TXml::commitStatus csStartPoint = TXml::beginCommit, TXml::commitStyle ct = TXml::selective);     //commit sequence for all types: full, selective, etc.
    //information and status
    virtual TXml::commitStyle defaultCommitStyle() const {return TXml::selective;}
    const char *getCommitStyleName(TXml::commitStyle ct) const;
    TXml::commitStatus parseCommitStatus(const char *sCommitStatusName) const;
    TXml::commitStyle  parseCommitStyle( const char *sCommitStyleName)  const;
    const char *getCommitStatusName(TXml::commitStatus iCommitStatus) const;
    virtual TXml::transactionResult commitGetPersistentStatus(TXml::commitStatus *cs, TXml::commitStyle *ct);
    virtual TXml::transactionResult commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct);

  protected:
    //derived Repository and IXmlBase sub-steps
    virtual TXml::transactionResult commitCreatePlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes);
    virtual TXml::transactionResult commitSwapInPlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes);
    virtual TXml::transactionResult commitDeleteTransactions(const vector<TXml*> *pvTXmls = 0); //All levels, e.g. delete the transactions file, or the in-memory nodes

  public:
    //--------------------------------------------------- transaction application
    //pass the TXml directly to the store so it can decide what to do with it
    //by default this function will use getSingleNode() and try to call ITXmlNode->function
    //Repositories override this because they often cannot directly apply the functions
    //  but do store the changes in a way for defered application, e.g. on load
    virtual TXml::transactionResult applyTransaction(  TXml *t, IXmlBaseNode *pSerialisationNode = 0, const IXmlQueryEnvironment *pDirectPassedQE = 0);
    virtual TXml::transactionResult reapplyTransaction(TXml *t, IXmlBaseNode *pSerialisationNode = 0, const IXmlQueryEnvironment *pDirectPassedQE = 0);
    void enableLinkedProcessors();
    void disableLinkedProcessors();
    bool enable();
    bool disable();
    bool enabled();
  protected:
    virtual int applyStartupTransactions(IXmlBaseDoc *pDoc); //Database calls this

  public:
    virtual ITXmlDirectFunctions *queryInterface(ITXmlDirectFunctions *p ATTRIBUTE_UNUSED) {return 0;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif

