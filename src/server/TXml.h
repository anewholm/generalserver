//platform agnostic file
#ifndef _TXML_H
#define _TXML_H

#include "define.h" //includes platform define also

//std library includes
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "MemoryLifetimeOwner.h"          //direct inheritance
#include "IXml/IXmlBaseNode.h"            //iDeviationType
#include "Xml/XmlAdminQueryEnvironment.h" //has-a

#define TXML_VERSION "1.0"

#define NO_TXML_BASENODE 0
#define NO_TXML_DESCRIPTION 0
#define SELECT_COPY true
#define REQUIRE_PLURAL true

namespace general_server {
   /* TXml (Transactional XML) based update
    * DOM (updates only) specification: http://www.w3.org/TR/DOM-Level-2-Core/core.html#i-Document
    * Document:
    *   Type create<Type>(in DOMString tagName);
    * Nodes:
    *   Node insertBefore(in Node newChild, in Node refChild);
    *   Node replaceChild(in Node newChild, in Node oldChild);
    *   Node removeChild(in Node oldChild);
    *   Node appendChild(in Node newChild);
    *   Node cloneNode(in boolean deep);
    *   (property) nodeValue(in DOMString value);
    * Attribute Named Node Maps:
    *   Node getNamedItem(in DOMString name);
    *   Node setNamedItem(in Node arg);
    *   Node removeNamedItem(in DOMString name);
    *   Node setNamedItemNS(in Node arg);
    *   // Introduced in DOM Level 2:
    *   Node removeNamedItemNS(in DOMString namespaceURI, in DOMString localName);
    *
    * BUT: We want a one-command, no persistent variables, with transforms based update capacity
    * XmlBaseDoc can understand it and supervise it with Repository without LibXmlBaseDoc needing to help
    * Request can communicate directly with the chosen IXmlBaseDoc with TXml objects through pure virtual ITXmlProcessor interface below
    *  or directly as it wishes
    *  using a setteable IXmlBaseNode *pBaseNode for following xmltransactions rather than an entire Database object + manager
    *  entire Database object + manager therefore moves in to the Request object
    * This is also what is used in the source XML now as well
    * <xmltransaction:move-child
    *   document-name="[name]"
    *   base-node or database-name="[unique-xpath, often id('db_test')]"
    *   [from="[unique-xpath]" to="[unique-xpath]" ...]
    *   [created="[UNIX timestamp]"]
    *   [xmlsecurity:username="[user]" xmlsecurity:password-hash="[pass]"]
    *   [record="false"]
    *   [immediacy="high"]
    *   [applied="yes"]
    * >
    *   [the single optional xml parameter]
    * </xmltransaction:move-child>
    *
    * move and copy, combined with transform give real power over the pre-defined XML to be added
    * outerXML allow arbitrary, user generated, XML in the system
    * @destination = appendChild, @before = insertBefore
    *
    * all actions require a @select node and can use a @base node
    */
  //normal ahead declerations
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXslDoc;
  interface_class IXslNode;
  interface_class ITXmlDirectFunctions;
  interface_class IXmlQueryEnvironment;
  interface_class IXslXPathFunctionContext;
  class TXmlProcessor;
  class Server;
  class XmlAdminQueryEnvironment;

  //--------------------------------------------------------------------------------------------------
  //----------------------------------------- TXml ---------------------------------------------------
  //--------------------------------------------------------------------------------------------------
  class TXml: virtual public MemoryLifetimeOwner {
    //TXml objects should be transportable, without pointers or dependencies on other external objects
    friend class TXmlProcessor;

  public:
    //enumerated types -----------------------------------------------------------
    enum transactionResult {
      transactionNotStarted = 0,
      transactionSuccess,
      transactionFailure,
      transactionFailureSecurity,
      transactionFailureDestinationInvalid,
      transactionFailureSourceInvalid,
      transactionFailureBeforeInvalid,
      transactionFailureToLazy,
      transactionDisabled
    };

    enum commitStyle {
      notRequired = 0,
      selective,
      full
    };

    enum commitStatus {
      commitStatusUnknown = 0,
      beginCommit,
      beginCommitCreatePlaceholders,
      beginCommitTransactionDelete,
      beginCommitSwapInPlaceholders,
      commitFinished
    };

    enum updateType {
      //many of these actions can allow transforms and new nodes created
      //these return the IXmlBaseNode* (and thus xml:id) of the new created node
      unknownUpdateType = 0,
      moveChild,     //parameters: @select || child::*, @destination || @before [, @transform, @with-param, @interface-mode]
                           //xml:id stays the same, @destination is NOT the child
                           //xml:id IS in the select xpath, although the destination xpath won't help
                           //returns: @select
      copyChild,     //parameters: @select || child::*, @destination || @before [, @deep-clone, @transform, @with-param, @interface-mode]
                           //new xml:id, @destination is NOT the child
                           //return the IXmlBaseNode* (and thus xml:id) of the new created node
                           //returns: new node
      deviateNode,   //parameters: @hardlink, @destination, @deviant, @type
                           //existing deviant, created through a separate mechanism
                           //indicated in the xml markup by a <xml:deviation id="" /> but not a new node in the IXmlDoc
                           //returns the @deviant
      hardlinkChild, //parameters: @select, @destination || @before
                           //xml:id does not change, 1 more parent is created
                           //indicated in the xml markup by a <xml:hardlink id="" /> but not a new node in the IXmlDoc
                           //returns: @select
      softlinkChild, //parameters: @select, @destination
                           //additional node created with new id
                           //return the IXmlBaseNode* (and thus xml:id) of the new (softlink) node
                           //returns: new soft link node
      mergeNode,     //parameters: @select || child::* (@xml), @destination [, @transform, @with-param, @interface-mode]
                           //xml mode:  xml:id must be the same (throws up if not), select is *not* removed
                           //node mode: xml:id moves, select *is* removed
                           //@destination is the result (overwritten)
                           //return: @select (with new properties and children)
      replaceNodeCopy,   //parameters: @select || child::* (@xml), @destination [, @transform, @with-param, @interface-mode]
                           //xml mode:  xml:id must be the same (throws up if not), select is *not* removed
                           //node mode: xml:id moves, select *is* removed
                           //@destination is the result (overwritten)
                           //return: @select (with new properties and children)
      setAttribute,  //parameters: @select, @name, @value
                           //xml:id is the affected node
                           //return: @select
      setValue,      //parameters: @select, @value
                           //xml:id is the affected node
                           //return: @select
      removeNode,    //parameters: @select (can be an attribute: removeAttribute)
                           //xml:id is the affected node that needs to be deleted, or maybe it's parent repository saved
                           //it is available in the select xpath
                           //but we have no access to the node that was deleted, unless we save it in ChildDoc at execution moment
                           //return: zero
      changeName,    //parameters: @select, @name
                           //xml:id is the affected node
                           //return: @select
      createChildElement, //parameters: @select, @name, @content
                           //xml:id is the affected parent
                           //returns: new node
      touchNode      //parameters: @select
                           //only causes a saveTree with no actual changes to the node
                           //return: @select
    };

    //description of areas of a tree that need saving, rather than micro actions on that tree
    //for a Repository that does not support xpath, transforms or precise updates
    //  updateType     is a multi node verb describing complex transformative changes,
    //  affectedScope is a singular node noun describing a data save
    //so changes are communicated in both formats, depending on how the target Repository saves things
    //we can add new affectedScope as we get more Repository types that benefit from different types of info
    //  currently FileSystem benefits from knowing file name changes, tree saves, sym links and the like
    enum affectedScope {
      saveElementOnly,       //only the element or attributes have changed, only useful for directories
      syncImmediateChildren, //removed node cannot be added to after, so cannot exist or have descendants
      saveTree,              //saves top node also, so does a nodeDetailsChange
      linkChild              //create a hard / sym link
    };

    //the XMLDoc has the correct details
    //this procedure is to find the MINIMUM required Repository savings to sunchronise changes
    //as multiple changes happen to the structure often in the same places or sub-trees
    struct AffectedNodeDetails {
      affectedScope iAffectedScope;      //saveElementOnly, syncImmediateChildren, saveTree, linkChild
      const IXmlBaseNode *pRelatedNode;  //current, present, affected node or, in the case of remove-node, the parent thereof
      //mostly for removeNode situations:
      const char *sResultFQNodeName;     //used xpath (also now invalid xpath to the removed node)
      const char *sResultRepositoryName; //attribute / element name in the case of singular effect and / or removed
      const char *sResultXmlId;          //id() of the [removed] node if any
      const char *sResultParentXPath;
    };

    //members ------------------------------------------------------------------------------
  protected:
    updateType    m_iUpdateType;
    const bool    m_bExplicitPlural; //part of a set of TXml like database:softlink-children
    bool          m_bTriggers;       //some TXml likes to be silent

    //user database is implemented in the XSL layer now
    //this TXml can be run on any database programmatically by the Database / Server
    //all immediately serialisable because the IXmlDoc (not stored) may change
    const char   *m_sBaseXPath;
    const char   *m_sSelectXPath;
    const char   *m_sDescription;

    //used on some updateTypes that move / copy things around
    const char   *m_sDestinationXPath;    //combined with m_sBeforeXPath and m_iDestinationPosition
    const char   *m_sBeforeXPath;         //overridden by a non-0 m_iDestinationPosition
    const int     m_iDestinationPosition; //0 = not specified, 1 = first, -1 = last, -2 = penultimate
    //select copying, used when source is transient
    //entire input is stored and serialised in to the TXml
    bool          m_bSelectCopied; //indicates if the @select node was copied in to the child doc (m_pChildNode will also be completed)
    IXmlBaseDoc  *m_pChildDoc;     //must come from parsed source, e.g. RegularX
    IXmlBaseNode *m_pChildNode;
    bool          m_bFromXML;      //m_pChildDoc was generated from incoming XML
    //Transactions are storeable if they are repeatable after server re-start
    //this member bool refers to the situation at instanciation and is not dynamnic
    //  i.e. when the pDestinationNode was passed in, was it transient?
    //  this is calculated in every sub-class at instanciation against the target IXmlDoc
    bool          m_bLastDocIsRepeatableAfterReload; //valid against instanciation doc
    //don't bother copying the select if the destination is not reloadable anyway
    bool          m_bLastDocNeedsSelectCopy;         //valid against instanciation doc

    //transforms used on some updateTypes
    const char   *m_sTransformWithXPath;
    const char   *m_sWithParam;
    const char   *m_sWithMode;

    //security
    const char   *m_sReadOPUserUUIDForTransform;
    const char   *m_sUserName;
    const char   *m_sSpecialProgrammaticUser;
    const char   *m_sUserStoreLocation;

    //OUTPUT: results extra stored information
    //to identify the node that was acted upon
    //Repository aware
    bool          m_bAppliedOnce;
    bool          m_bHadNodesToProcess;    //only valid when m_bAppliedOnce is true
    const char   *m_sResultFQNodeName;     //not useful because not unique (selectXPath is better here)
    const char   *m_sResultXmlId;          //useful to find the copy-node, move-node[transformed], etc. result
    const char   *m_sResultRepositoryName; //useful for remove-node
    const char   *m_sResultParentXPath;    //needed to resolve parent when the target node has gone (remove-node)
    const char   *m_sExtensionElementPrefixes; //request Database server service etc.
    const char   *m_sXSLModuleManagerName;     //Request
    transactionResult m_iLastTransactionResult;

    //useful in replaceNodeCopy
    const char   *m_sDestinationXMLID;
    const char   *m_sSelectXMLID;

    //functionality ------------------------------------------------------------------------------
    //new protected factory base class uses
    TXml(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const updateType iUpdateType, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural, const IXmlBaseNode *pBeforeNode = NULL, const int iDesintationPosition = 0, const bool bSelectCopy = false);

    void addTransformPrivate(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);

    static const char *getUpdateTypeName(   const updateType iUpdateType);
    const char *getUpdateTypeName() const;
    static updateType parseUpdateType(const char *sUpdateTypeName, const bool bExplicitPlural = false);
    virtual TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  
      IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) = 0;

  public:
    //factory pattern
    //the factory pattern with separate asserts() in each sub-class ensures validation from the instanciation fromNode(...) option
    //directly instanciate sub-classes programmatically also
    //  for example TXml_TouchNode to save xml:id changes
    //uses protected TXml(...)
    //2 types of instanciation:
    //  1) instanciation from a node without XSLT: security and EMO needed to read the node
    //     typical in Database applyStartupTransactions()
    //     there will not be triggers on static serialised TXmls of course
    //     however, that doesn't stop someone creating read trigger based TXml
    //  2) instanciation from a node during an XSLT: the transforming stylesheet contains the security and EMO
    //     triggers could fire
    static TXml *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, TXmlProcessor *pStore, const IXmlBaseNode *pTXmlSpecNode, const IXmlBaseNode *pSelectNode = NULL, const bool bExplicitPlural = false);
    static TXml *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, TXmlProcessor *pStore, IXslXPathFunctionContext *pXCtxt);

    TXml(const TXml& t); //copy constructor
    virtual ~TXml();     //virtual public destructors allow proper destruction of derived object
    virtual TXml *clone_with_resources() const = 0;

    //optional extras after instanciation
    //override them to access functionality in base class
    virtual void addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);
    IXmlBaseNode *createChildDoc(const IXmlBaseDoc *pNamespaceDoc = 0);
    //selectCopy() is also useful in situations where the source may not be available
    //e.g. serialised transport of additions to another server
    virtual IXmlBaseNode *selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNodeToCopy, const IXmlBaseDoc *pNamespaceDoc = 0);
    IXmlBaseNode         *selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const char *sXML, const IXmlBaseDoc *pNamespaceDoc = 0);
    void setResultsData(const bool bHadNodesToProcess, const char *sResultFQNodeName, const char *sResultXmlId, const char *sResultRepositoryName, const char *sResultParentXPath, const char *sExtensionElementPrefixes, const char *sXSLModuleManagerName);

    //caller instanciates pvAffectedNodes and manages it
    //like this so that many TXml->affectedNodes(...) can be called directly on the same vector without mergeing

    //architecture: -------------------------------------------------------------------------
    //we have the current correct state of play *pInThisDoc
    //  so we are not trying to chronologically create it in the Repository by ourselves
    //  although we could, we aren't
    //  applyStartupTransactions() ensures that the memory returns to the current correct state on restart
    //  instead we are just seeing which bits we have to save (an attribute, a node name, a sub-tree or a deletion)

    //DISCUSSION: ---------------------------------------------------------------------------
    //affectedNodes(...) only gets the actual nodes BEFORE the TXml is run
    //  otherwise the nodes have gone (in the case of remove-node)! or a different TXml has removed its parent
    //  but we are using it during the commit() on the changed structure!
    //  for example remove-node(repository) -> commit() will not find the selectNode and thus not work
    //  but maybe not a problem if we can get the nearest ancestor::[@repository:name] cause that's all that's needed...
    //    LibXML2->getClosestNode(...) which, rather than returning zero, returns the bottom most leaf of the xpath
    //  could be good also because actually, if many changes affect the same area then it gets saved once.
    //    could also use isNodeInSubTreeOfAny(pvNodes)
    //  also: this is only an issue if the node is removed or changed and removed, not if it is just changed
    //    so we mark the change as a remove and then the sub-system just runs deletes for stuff not there...
    //    it only needs to delete a file if the parent() is a directory
    //    in which case directory remove-node needs just to check it's children and delete any non-existing ones
    //  if a changed node is not found, then it has been removed and thus is not relevant anymore
    //    it is not affected pInThisDoc if pInThisDoc doesn't have it
    //  node name changes, e.g. repository:* folders should be allowed
    //    or should they be <repository:folder displayname="my folder" /> types
    //if we have id()s then we can work purely on those...
    //  id()s DO NOT allow us to find the parent if the node has gone... using xpath string handling
    //    but xpath string handling is risky because the previous directive is not necessarily on the parent -> child axis
    //    so: we store the parent xpath also. if that has gone then we don't give a shit
    //  and thus the original xpaths would not indicate the resultant node
    //  unless the TXml was updated while being run
    //  maybe ALL TXml transactions should return the node id() affected
    //  select does this already, but destination doesn't
    //  and copy or transform will not either

    //used for: -------------------------------------------------------------------------
    //  working out which areas of the document need saving
    //    after successful IXmlBaseDoc changes (live or startup)
    //    during subsequent Database::commit(...) of IXmlBaseDoc <transaction> nodes -> Repository directories and files
    //  working out if any of the transaction comes from a transient area
    //    after successful IXmlBaseDoc changes (immediately after transaction)
    //    during subsequent Repository::applyTransaction() -> transaction file

    //Repository saving: -------------------------------------------------------------------------
    //TODO: what if it is a new repository? under a directory, not a file obviously
    //      if the parent is a DIRECTORY
    //        then the FileSystem has a choice on how it saves contents
    //        if there are @repository:type indicators already then follow them but reset the @name
    //        if there are new @repository:type indicators then follow them and complete the @name
    //    then it could just be saved itself rather than saving the whole directory
    //    a new (directory|file) repository is only possible if destination is a directory
    //    a new file is the only save type otherwise
    //    what is the new filename / directoryname? : look at the @repository:name indicator
    //      need to link this with thinking about stylesheet name referencing
    virtual void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pDoc) const = 0;

    const char *version() {return TXML_VERSION;}
    bool hasNodesToProcess() const; //the transform result or select path has vielded nodes (still *valid* if not)

    //updates: appication and recording
    //keep the responsibility for understanding the interpretation of TXML encapsualted here
    transactionResult applyTo(ITXmlDirectFunctions *pStore, IXmlBaseNode **pResultNode = 0, const IXmlQueryEnvironment *pQE = NULL); //TXml calls the correct function
    //const transactionResult applyTo(TXmlProcessor              *pDoc)  const; //the acceptor takes responsibility
    //const transactionResult applyTo(vector<TXmlProcessor*>     *pDocs) const; //the acceptors take responsibility

    //status
    void succeed();                        //status="0"
    void fail(const transactionResult tr); //status="[tr]"

    //transaction details storage for transport and display
    virtual const char   *serialise(IXmlBaseNode *pSerialisationNode) const;
    virtual const char   *toString() const;
    virtual IXmlBaseNode *toNode(   IXmlBaseNode *pSerialisationNode) const;
    static const char *getAffectedScopeName(const affectedScope iAffectedScope);
    static TXml::affectedScope parseAffectedScopeName(const char *sAffectedScopeName);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  //--------------------------------------------------------------------------------------------------
  //----------------------------------------- TXml updateType sub-classes ----------------------------
  //--------------------------------------------------------------------------------------------------
  //sub-classes used because we want to maintain *compile-time* validity
  //instanciated optionally by factory pattern from a TXmlSpecNode
  class TXml_MoveChild: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_MoveChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition = 0, const bool bExplicitPlural = false);
    TXml_MoveChild *clone_with_resources() const {return new TXml_MoveChild(*this);}
    //optional extras that affect result
    void addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_CopyChild: public TXml {
    bool           m_bDeepClone;
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_CopyChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition = 0, const bool bDeepClone = DEEP_CLONE, const bool bSelectCopy = false, const bool bExplicitPlural = false);
    TXml_CopyChild *clone_with_resources() const {return new TXml_CopyChild(*this);}
    //optional extras that affect result
    void addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    IXmlBaseNode *selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNodeToCopy, const IXmlBaseDoc *pNamespaceDoc = 0);
  };

  class TXml_HardlinkChild: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_HardlinkChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDestinationPosition = 0, const bool bExplicitPlural = false);
    TXml_HardlinkChild *clone_with_resources() const {return new TXml_HardlinkChild(*this);}
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_DeviateNode: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_DeviateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pHardlink, const IXmlBaseNode *pNewNode, const iDeviationType iType, const bool bExplicitPlural = false);
    TXml_DeviateNode *clone_with_resources() const {return new TXml_DeviateNode(*this);}
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_SoftlinkChild: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_SoftlinkChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const bool bExplicitPlural = false);
    TXml_SoftlinkChild *clone_with_resources() const {return new TXml_SoftlinkChild(*this);}
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_MergeNode: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_MergeNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural = false);
    TXml_MergeNode *clone_with_resources() const {return new TXml_MergeNode(*this);}
    //optional extras that affect result
    void addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_ReplaceNode: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_ReplaceNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural = false);
    TXml_ReplaceNode *clone_with_resources() const {return new TXml_ReplaceNode(*this);}
    //optional extras that affect result
    void addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam = 0, const char *sWithMode = 0);
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_SetAttribute: public TXml {
    const char *m_sNamespace;
    const char *m_sName;
    const char *m_sValue;

  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_SetAttribute(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const char *sValue, const char *sNamespace = NULL, const bool bExplicitPlural = false);
    TXml_SetAttribute(const TXml_SetAttribute& t); //copy constructor
    TXml_SetAttribute *clone_with_resources() const {return new TXml_SetAttribute(*this);}
    ~TXml_SetAttribute();

    const char *toString() const;
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_SetValue: public TXml {
    const char *m_sValue;

  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_SetValue(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sValue, const bool bExplicitPlural = false);
    TXml_SetValue(const TXml_SetValue &t);
    TXml_SetValue *clone_with_resources() const {return new TXml_SetValue(*this);}
    ~TXml_SetValue();

    const char *toString() const;
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_RemoveNode: public TXml {
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_RemoveNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const bool bExplicitPlural = false);
    TXml_RemoveNode *clone_with_resources() const {return new TXml_RemoveNode(*this);}
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_ChangeName: public TXml {
    const char *m_sName;

  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_ChangeName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const bool bExplicitPlural = false);
    TXml_ChangeName(const TXml_ChangeName &t);
    TXml_ChangeName *clone_with_resources() const {return new TXml_ChangeName(*this);}
    ~TXml_ChangeName();
    const char *toString() const;
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_CreateChildElement: public TXml {
    const char *m_sNamespace;
    const char *m_sName;
    const char *m_sValue;

  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_CreateChildElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const char *sValue, const char *sNamespace = 0, const bool bExplicitPlural = false);
    TXml_CreateChildElement(const TXml_CreateChildElement &t);
    TXml_CreateChildElement *clone_with_resources() const {return new TXml_CreateChildElement(*this);}
    ~TXml_CreateChildElement();
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    const char *toString() const;
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };

  class TXml_TouchNode: public TXml {
    TXml::affectedScope m_as;
  protected:
    TXml::transactionResult runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName);
  public:
    TXml_TouchNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const TXml::affectedScope as = TXml::saveTree, const bool bExplicitPlural = false);
    TXml_TouchNode *clone_with_resources() const {return new TXml_TouchNode(*this);}
    IXmlBaseNode *toNode(IXmlBaseNode *pParent) const;
    void affectedNodes(vector<AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const;
  };


  //--------------------------------------------------------------------------------------------------
  //----------------------------------------- TXmlCollection -----------------------------------------
  //--------------------------------------------------------------------------------------------------
  //TODO: TXmlCollection
  class TXmlCollection: public vector<TXml*> {
  public:
    //updates: appication and recording
    //keep the responsibility for understanding the interpretation of TXML encapsualted here
    /*
    //const TXmlProcessor::transactionResult applyTo(ITXmlDirectFunctions *pStore)  const; //TXml takes responsibility for calling the correct function
    //const TXmlProcessor::transactionResult applyTo(TXmlProcessor         *pStore)  const; //the acceptor takes responsibility for carrying out the transaction
    //const TXmlProcessor::transactionResult applyTo(vector<TXmlProcessor*> pStores) const; //the acceptors take responsibility for carrying out the transaction

    //transaction details storage for transport and display
    //const char *serialise(IXmlBaseDoc *pDoc) const;
    */
  };

  //--------------------------------------------------------------------------------------------------
  //----------------------------------------- ITXmlDirectFunctions -----------------------------------
  //--------------------------------------------------------------------------------------------------
#define ITDF_RETURN           TXml::transactionResult
#define ITDF_SECURITY_PARAMS  const char *sUserId, const char *sUserStoreLocation, const IXmlQueryEnvironment *pDirectPassedQE, const char *sReadOPUserUUIDForTransform
#define ITDF_BASE_PARAMS      const char *sBaseXPath, const char *sSelectXPath
#define ITDF_TRANSFORM_PARAMS const char *sTransformWithXPath, const char *sWithParam, const char *sWithMode
#define ITDF_RESULT_PARAMS    IXmlBaseNode **ppResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sSelectParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName
  interface_class ITXmlDirectFunctions {
  public:
    //required functions on a class that can be directly processed by TXml->applyTo(...) TXML function
    //usually in memory IXmlBaseDocs that support direct immediate manipulation
    //TXML v1.0 function prototypes (built form updateType spec above):
    //these return string based details of the affected nodes
    //which are then placed back in to the TXml
    virtual ITDF_RETURN moveChild(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN copyChild(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, const bool bDeepClone, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN hardlinkChild(     ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN softlinkChild(     ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN deviateNode(       ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sHardlinkXPath, const char *sNewNodeXPath, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN mergeNode(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN replaceNodeCopy(   ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN setAttribute(      ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN setValue(          ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sValue, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN removeNode(        ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN createChildElement(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN changeName(        ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, ITDF_RESULT_PARAMS) = 0;
    virtual ITDF_RETURN touch(             ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS) = 0;
  };
}

#endif

