//platform independent
#include "TXml.h"

#include "Exceptions.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlSecurityContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "Server.h"
#include "Debug.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c" //vector_element_destroy(...)

#include <algorithm>

namespace general_server {
  TXml *TXml::factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, TXmlProcessor *pStore, IXslXPathFunctionContext *pXCtxt) {
    //caller frees result and pXCtxt
    //factory pattern
    assert(pQE);
    assert(pStore);
    assert(pXCtxt);

    //base classes
    TXml *t = 0;
    //attributes of the TXml
    const char *sUpdateTypeName = 0;
    updateType iUpdateType;
    const IXmlBaseNode *pSelectNode    = 0,
                 *pDestinationNode     = 0,
                 *pBeforeNode          = 0;
    int           iDestinationPosition = 0;
    const char *sAffectedScope = 0,
               *sNamespace     = 0,
               *sName          = 0,
               *sValue         = 0;
    bool bDeepClone = true;
    unsigned int iNumParams    = 0;
    TXml::affectedScope as = TXml::saveTree; //default for touchNode

    //base update type that decides which sub-class
    //throws InvalidUpdateType()
    sUpdateTypeName = pXCtxt->functionName();
    iUpdateType     = parseUpdateType(sUpdateTypeName);
    iNumParams      = pXCtxt->valueCount();

    //derived class handles this instanciation
    //because we need to pop interpret off the xpath function parameter stack
    switch (iUpdateType) {
      case TXml::moveChild:       {
        switch (iNumParams) {
          case 4: iDestinationPosition = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 3: pBeforeNode          = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          default:
            pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        }
        t = new TXml_MoveChild(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition);
        break;
      }
      case TXml::copyChild:       {
        switch (iNumParams) {
          case 4: bDeepClone           = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 3: iDestinationPosition = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 2: pBeforeNode          = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          default:
            pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        }
        t = new TXml_CopyChild(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition, bDeepClone);
        break;
      }
      case TXml::hardlinkChild:   {
        switch (iNumParams) {
          case 4: iDestinationPosition = pXCtxt->popInterpretIntegerValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          case 3: pBeforeNode          = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          default:
            pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        }
        t = new TXml_HardlinkChild(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition);
        break;}
      case TXml::softlinkChild:   {
        switch (iNumParams) {
          case 3: pBeforeNode          = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
          default:
            pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        }
        t = new TXml_SoftlinkChild(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode, pBeforeNode);
        break;
      }
      case TXml::mergeNode: {
        pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_MergeNode(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode);
        break;
      }
      case TXml::replaceNodeCopy: {
        pDestinationNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_ReplaceNode(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, pDestinationNode);
        break;
      }
      case TXml::setAttribute:    {
        if (iNumParams > 3) sNamespace = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sValue           = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sName            = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_SetAttribute(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, sName, sValue, sNamespace);
        break;
      }
      case TXml::setValue:    {
        sValue           = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_SetValue(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, sValue);
        break;
      }
      case TXml::removeNode:      {
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_RemoveNode(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode);
        break;
      }
      case TXml::changeName:      {
        sName            = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_ChangeName(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, sName);
        break;
      }
      case TXml::touchNode:       {
        if (iNumParams == 2) {
          sAffectedScope   = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          as               = parseAffectedScopeName(sAffectedScope);
        }
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_TouchNode(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, as);
        break;
      }
      case TXml::createChildElement:
        if (iNumParams > 3) sNamespace = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sValue           = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sName            = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        pSelectNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        t = new TXml_CreateChildElement(pMemoryLifetimeOwner, pQE, NO_TXML_DESCRIPTION, NO_BASE_NODE, pSelectNode, sName, sValue, sNamespace);
        break;
      case TXml::deviateNode:
        NOT_COMPLETE("TXml::factory update type");
        break;
      case TXml::unknownUpdateType:
        throw TXmlUnknownTransactionType(pMemoryLifetimeOwner);
        break;
    }

    //free up
    //if (sUpdateTypeName)    MMO_FREE(sUpdateTypeName); //pointer in to LibXML2 structure, freed by pXCtxt
    if (pSelectNode)        delete pSelectNode;
    if (pDestinationNode)   delete pDestinationNode;
    if (pBeforeNode)        delete pBeforeNode;
    if (sName)              MM_FREE(sName);
    if (sValue)             MM_FREE(sValue);
    if (sAffectedScope)     MM_FREE(sAffectedScope);

    return t;
  }

  TXml *TXml::factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, TXmlProcessor *pStore, const IXmlBaseNode *pTXmlSpecNode, const IXmlBaseNode *pSelectNode, const bool bExplicitPluralParameter) {
    //caller frees result and pTXmlSpecNode
    //factory pattern
    //the factory pattern with separate asserts() in each sub-class ensures validation from the instanciation fromNode(...) option
    //used to alter the Database during an XSLT by generating TXml from a parameterised XML specification
    //first-application of
    //  e.g. <Database:move-child select="..." destination="dyn:evaluate(form/dest)" />
    //validation occurs in the pStore during applyTo(), not during creation
    //return a volatile TXml because we may want to addTransform(...) selectCopyIntoTXml() etc.
    //can be generated during a stylesheet transform with $parameters
    assert(pQE);
    assert(pStore);
    assert(pTXmlSpecNode);

    //base classes
    TXml *t = 0;
    //attributes of the TXml
    updateType iUpdateType;
    const char *sUpdateTypeName      = 0;
    const IXmlBaseNode *pBaseNode    = 0,
                 *pDestinationNode   = 0,
                 *pBeforeNode        = 0,
                 *pFirstChild        = 0,
                 *pUserStoreLocation = 0,
                 *pTransformWithXSLNode = 0,
                 *pFromNode          = 0;
    int           iDestinationPosition  = 0;
    const char *sWithParam = 0,
               *sWithMode  = 0,
               *sAffectedScope = 0,
               *sNamespace = 0,
               *sName      = 0,
               *sValue     = 0,
               *sXML       = 0,
               *sReadOPUserUUIDForTransform = 0,
               *sUserId    = 0,
               *sDescription = 0;
    TXml::affectedScope as = TXml::saveTree; //default for touchNode
    bool bDeepClone, bForceSelectCopy;

    //result details
    bool bHadNodesToProcess, bAppliedOnce, bExplicitPlural;
    const char *sResultFQNodeName     = 0,
               *sResultXmlId          = 0,
               *sResultRepositoryName = 0,
               *sResultParentXPath    = 0,
               *sExtensionElementPrefixes = 0,
               *sXSLModuleManagerName = 0;

    UNWIND_EXCEPTION_BEGIN_STATIC(pMemoryLifetimeOwner) {
      //base update type that decides which sub-class
      //throws InvalidUpdateType()
      bExplicitPlural = pTXmlSpecNode->attributeValueBoolDynamicString(pQE, "explicit-plural", NAMESPACE_NONE, bExplicitPluralParameter);
      sUpdateTypeName = pTXmlSpecNode->localName(NO_DUPLICATE);            //updateType
      iUpdateType     = parseUpdateType(sUpdateTypeName, bExplicitPlural); //may return unknownUpdateType (0)

      //common parameters like select
      //throws Security*()
      //pSelectNode = pTXmlSpecNode->attributeValueNode(pBaseNode, WARN_IF_TRANSIENT);
      //from node for @*=<xpath>:
      //absolute @select and @base: TXml can be executed in relation to itself
      //relative @select and @base: in relation to its current transform input node
      //e.g. @select=.
      if (pQE->transformContext()) pFromNode = pQE->transformContext()->sourceNode(pQE); //pQE as MM
      
      //we are not storing the source pTXmlSpecNode, instead:
      //  normalise all the xpath parameters now in relation to the doc
      //  using uniqueXPathToNode() -> fullyQualifiedName() that uses currentContextPrefix()
      //  reconcileNamespace() actually finds a valid local ancestor PREFIX that CAN be used instead of the blank one
      //generally parse all known xpath node types (there are no ambiguities)
      //@transform, @select, @destination, @before
      //TODO: this is based off the pCommandNode, needs to be based off the current output node in the stylesheet
      pBaseNode = pTXmlSpecNode->attributeValueNode(pQE, "base", NULL, pFromNode);
      if (pBaseNode) NEEDS_TEST("TXml::factory pBaseNode");

      //has the TXml been run before
      //it will have result attributes if it has
      //and security
      bAppliedOnce = pTXmlSpecNode->attributeValueBoolDynamicString(pQE, "applied-once");

      //"select" input: 3 possible sources:
      //  *) select:     from current document
      //  *) firstChild: in the TXml spec
      //  *) xml:        string input attribute - TXml will parse this immediately in to itself
      //validation happens in applyTo(), not during creation
      //all TXml sub-classes should need at least one of the following available:
      //  assert(pSelectNode || pFirstChild || sXML)
      if (!pSelectNode) 
        pSelectNode = pTXmlSpecNode->attributeValueNode(pQE, "select", NULL, pFromNode); //will be xpath pointer in to presistent doc
      pFirstChild   = pTXmlSpecNode->firstChild(pQE);                     //act on spec content, probably with a transform
      sXML          = pTXmlSpecNode->attributeValueDynamic(pQE, "xml");   //not member variable because parsed in to ChildDoc

      //security context: required at Derived construction always
      //used in read and write operations, and transforms: can fail to write!
      //there are 2 security contexts here:
      //  1) the current pQE that these pTXmlSpecNode are happening through (also necessary for further derived construction)
      //  2) the potential intended security context that the TXml should run through, if not pQE
      if (bAppliedOnce) {
        //ok, so the TXml has been applied before
        //and will thus have the details of the security context it last ran under
        sUserId = pTXmlSpecNode->attributeValue(    pQE, "user-id");
        //NOT_CURRENTLY_USED:
        //pUserStoreLocation            = pTXmlSpecNode->attributeValueNode(pQE, "user-store");
        //sReadOPUserUUIDForTransform   = pTXmlSpecNode->attributeValue(    pQE, "user-guid");

        //note that a zero iUserId indicates anonymous user
        pQE->securityContext()->masquerade(sUserId); //throws UserDoesNotHavePermission
      }

      //optional parameters, validated by TXml::TXml
      sDescription     = pTXmlSpecNode->attributeValueDynamic(pQE, "description");            //optional
      pDestinationNode = pTXmlSpecNode->attributeValueNode(   pQE, "destination", NULL, pFromNode); //optional: copyChild, moveChild, etc.
      pBeforeNode      = pTXmlSpecNode->attributeValueNode(   pQE, "before",      NULL, pFromNode);      //optional: copyChild, moveChild, etc.
      iDestinationPosition = pTXmlSpecNode->attributeValueInt(pQE, "destination-position");   //optional: copyChild, moveChild, etc.
      sNamespace       = pTXmlSpecNode->attributeValueDynamic(pQE, "namespace");              //optional: setAttribute, changeName
      sName            = pTXmlSpecNode->attributeValueDynamic(pQE, "name");                   //optional: setAttribute, changeName
      sValue           = pTXmlSpecNode->attributeValueDynamic(pQE, "value");                  //optional: setAttribute, setValue
      sAffectedScope   = pTXmlSpecNode->attributeValueDynamic(pQE, "affected-scope");         //optional: touch
      bForceSelectCopy = pTXmlSpecNode->attributeValueDynamic(pQE, "force-select-copy");      //optional: setAttribute
      bDeepClone       = pTXmlSpecNode->attributeValueBoolDynamicString(pQE, "deep-clone", NAMESPACE_NONE, true);  //yes, true or 1
      //m_bLastDocIsRepeatableAfterReload //dynamically calculated against this current [new] IXmlDoc nodes
      //m_bLastDocNeedsSelectCopy         //dynamically calculated against this current [new] IXmlDoc nodes
      if (sAffectedScope) as = parseAffectedScopeName(sAffectedScope);

      //create with selected parameters
      //we want to send in IXmlBaseNodes because we may instanciate these directly
      //most of the immediate serialisation work is done in TXml::TXml(...)
      switch (iUpdateType) {
        case TXml::moveChild:          {t = new TXml_MoveChild(         pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition, bExplicitPlural); break;}
        case TXml::copyChild:          {t = new TXml_CopyChild(         pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition, bDeepClone, bExplicitPlural); break;}
        case TXml::hardlinkChild:      {t = new TXml_HardlinkChild(     pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode, iDestinationPosition, bExplicitPlural); break;}
        case TXml::softlinkChild:      {t = new TXml_SoftlinkChild(     pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, pBeforeNode, bExplicitPlural); break;}
        case TXml::mergeNode:          {t = new TXml_MergeNode(         pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural); break;}
        case TXml::replaceNodeCopy:    {t = new TXml_ReplaceNode(       pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural); break;}
        case TXml::setAttribute:       {t = new TXml_SetAttribute(      pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, sName, sValue, sNamespace, bExplicitPlural); break;}
        case TXml::setValue:           {t = new TXml_SetValue(          pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, sValue, bExplicitPlural); break;}
        case TXml::removeNode:         {t = new TXml_RemoveNode(        pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, bExplicitPlural); break;}
        case TXml::changeName:         {t = new TXml_ChangeName(        pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, sName, bExplicitPlural); break;}
        case TXml::touchNode:          {t = new TXml_TouchNode(         pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, as, bExplicitPlural); break;}
        case TXml::createChildElement: {t = new TXml_CreateChildElement(pMemoryLifetimeOwner, pQE, sDescription, pBaseNode, pSelectNode, sName, sValue, sNamespace, bExplicitPlural); break;}
        
        case TXml::unknownUpdateType:
          throw TXmlUnknownTransactionType(pMemoryLifetimeOwner);
          break;
          
        case TXml::deviateNode:
          NOT_COMPLETE("TXml::factory update type");
      }

      //optional extra effects
      //some sub-classes will throw if not appropriate
      if (pTransformWithXSLNode = pTXmlSpecNode->attributeValueNode(pQE, "transform")) {
        sWithParam = pTXmlSpecNode->attributeValue(pQE, "with-param");
        sWithMode  = pTXmlSpecNode->attributeValueDynamic(pQE, "interface-mode");
        t->addTransform(pQE, pTransformWithXSLNode, sWithParam, sWithMode);
      }
      
      //some TXml likes to be silent
      //it is up to the TXmlProcessor if it wants to honour this
      t->m_bTriggers = pTXmlSpecNode->attributeValueBoolDynamicString(pQE, "triggers", NAMESPACE_NONE, true);

      //optional copy select in to the independent TXml child doc
      //work out if the select needs to be copied or referenced
      //this can be decided explicitly by @force-select-copy
      //or worked out by analysis of the transient nature of the source data
      //selectCopyIntoTXml() @return value in the child doc root node held in TXml, ~TXml deals with it
      if      (bForceSelectCopy && pSelectNode) t->selectCopyIntoTXml(pQE, pSelectNode);      //explicit request to select copy
      else if (pFirstChild)                     t->selectCopyIntoTXml(pQE, pFirstChild);      //first child takes precedence
      else if (t->m_bLastDocNeedsSelectCopy)    t->selectCopyIntoTXml(pQE, pSelectNode);      //transient source, so we need to copy it
      else if (sXML)                            t->selectCopyIntoTXml(pQE, sXML, pTXmlSpecNode->document()); //bring in the XML to the ChildDoc

      //result sync and checking
      //when reading from pTXmlSpecNode these attributes say what happened last time it was applied if it was
      //set these variables for analysing a second application of the TXml: applyStartupTransactions(...), commit(...)
      if (bAppliedOnce) {
        bHadNodesToProcess    = pTXmlSpecNode->attributeValueBoolDynamicString(pQE, "result-had-nodes-to-process"); //related only to m_bAppliedOnce
        sResultFQNodeName     = pTXmlSpecNode->attributeValue(    pQE, "result-qname");
        sResultXmlId          = pTXmlSpecNode->attributeValue(    pQE, "result-xmlid");
        sResultRepositoryName = pTXmlSpecNode->attributeValue(    pQE, "result-repository_name");
        sResultParentXPath    = pTXmlSpecNode->attributeValue(    pQE, "result-parent-xpath");
        sExtensionElementPrefixes = pTXmlSpecNode->attributeValue(pQE, "extension-element-prefixes");
        sXSLModuleManagerName = pTXmlSpecNode->attributeValue(    pQE, "extensions-module-name");
        //implicitly sets m_bAppliedOnce
        t->setResultsData(bHadNodesToProcess, sResultFQNodeName, sResultXmlId, sResultRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sXSLModuleManagerName);
      }
    } UNWIND_EXCEPTION_END_STATIC(pMemoryLifetimeOwner);

    //free up
    if (sXML)               MM_FREE(sXML);
    if (pBaseNode)          delete pBaseNode;
    if (pSelectNode)        delete pSelectNode;
    if (pFromNode)          MM_DELETE(pFromNode);
    if (pDestinationNode)   delete pDestinationNode;
    if (pFirstChild)        delete pFirstChild;
    if (pTransformWithXSLNode) delete pTransformWithXSLNode;
    if (pUserStoreLocation) delete pUserStoreLocation;
    if (sReadOPUserUUIDForTransform) MM_FREE(sReadOPUserUUIDForTransform);
    if (sUserId)            MM_FREE(sUserId);
    if (sDescription)       MM_FREE(sDescription);
    //TXml MM_STRDUP(...)s these if it wants them, and we release ours
    if (sName)              MM_FREE(sName);
    if (sValue)             MM_FREE(sValue);
    if (sAffectedScope)     MM_FREE(sAffectedScope);
    //if (sUpdateTypeName)    MM_FREE(sUpdateTypeName); //NO_DUPLICATE
    //handled by the TXml destructor, only created if set
    //if (sWithParam)         MM_FREE(sWithParam);
    //if (sWithMode)          MM_FREE(sWithMode);
    //if (sResultFQNodeName)  MM_FREE(sResultFQNodeName);
    //if (sResultXmlId)       MM_FREE(sResultXmlId);
    //if (sResultRepositoryName) MM_FREE(sResultRepositoryName);
    //if (sResultParentXPath) MM_FREE(sResultParentXPath);
    //if (sExtensionElementPrefixes) MM_FREE(sExtensionElementPrefixes);
    //if (sXSLModuleManagerName) MM_FREE(sXSLModuleManagerName);

    UNWIND_EXCEPTION_THROW;
    
    return t;
  }

  IXmlBaseNode *TXml::createChildDoc(const IXmlBaseDoc *pNamespaceDoc) {
    //returns new root node
    //sets default namespace to general_server
    //and auto-adds all the standard namespaces
    IXmlBaseNode *pChildDocRootNode = 0;

    m_bSelectCopied    = true;
    m_pChildDoc        = pNamespaceDoc->xmlLibrary()->factory_document(this, "ChildDoc");

    XmlAdminQueryEnvironment ibqe_childDocControl(this, m_pChildDoc);  //TXml control child document
    pChildDocRootNode  = m_pChildDoc->createRootChildElement(&ibqe_childDocControl, "root_namespace_holder", pNamespaceDoc);
    //ensure that any application does not now attempt to use the orignisecurity-strategyal source select node
    //this would mean that we cannot access the serialised select source anymore,
    //but we shouldn't because this is suppossed independent now
    if (m_sSelectXPath) MMO_FREE(m_sSelectXPath);
    m_sSelectXPath = 0;

    return pChildDocRootNode;
  }

  IXmlBaseNode *TXml::selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNodeToCopy, const IXmlBaseDoc *pNamespaceDoc) {
    //do not free result! because it is held in TXml space and released by ~TXml()
    //TODO: hardlinks and deviations needed to be converted to placeholders in the copy...
    assert(pChildNodeToCopy);

    IXmlBaseNode *pChildDocRootNode;

    if (!pNamespaceDoc) pNamespaceDoc = pChildNodeToCopy->document();
    pChildDocRootNode = createChildDoc(pNamespaceDoc); //LibXmlLibrary::factory_document(...)
    m_pChildNode      = pChildDocRootNode->copyChild(pQE, pChildNodeToCopy);

    //free up
    delete pChildDocRootNode;

    return m_pChildNode;
  }

  IXmlBaseNode *TXml::selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const char *sXML, const IXmlBaseDoc *pNamespaceDoc) {
    //do not free result! because it is held in TXml space and released by ~TXml()
    assert(sXML);

    IXmlBaseNode *pChildDocRootNode;
    if (pChildDocRootNode = createChildDoc(pNamespaceDoc)) {
      m_pChildNode = pChildDocRootNode->parseXMLInContextNoAddChild(pQE, sXML);
      pChildDocRootNode->moveChild(pQE, m_pChildNode);
      m_bFromXML   = true;
    } else throw ZeroDocument(this, MM_STRDUP("TXml child doc"));

    //free up
    if (pChildDocRootNode) delete pChildDocRootNode;

    return m_pChildNode;
  }
  
  void TXml::setResultsData(const bool bHadNodesToProcess, const char *sResultFQNodeName, const char *sResultXmlId, const char *sResultRepositoryName, const char *sResultParentXPath, const char *sExtensionElementPrefixes, const char *sXSLModuleManagerName) {
    //do not free these, ~TXml() will handle it
    m_bHadNodesToProcess    = bHadNodesToProcess;
    m_bAppliedOnce          = true;

    //results data can be changed / re-set
    if (m_sResultFQNodeName)     MMO_FREE(m_sResultFQNodeName);
    if (m_sResultXmlId)          MMO_FREE(m_sResultXmlId);
    if (m_sResultRepositoryName) MMO_FREE(m_sResultRepositoryName);
    if (m_sResultParentXPath)    MMO_FREE(m_sResultParentXPath);
    if (m_sExtensionElementPrefixes) MMO_FREE(m_sExtensionElementPrefixes);
    if (m_sXSLModuleManagerName) MMO_FREE(m_sXSLModuleManagerName);

    m_sResultFQNodeName     = sResultFQNodeName;
    m_sResultXmlId          = sResultXmlId;
    m_sResultRepositoryName = sResultRepositoryName;
    m_sResultParentXPath    = sResultParentXPath;
    m_sExtensionElementPrefixes = sExtensionElementPrefixes;
    m_sXSLModuleManagerName = sXSLModuleManagerName;
  }

  TXml::TXml(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const updateType iUpdateType, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural, const IXmlBaseNode *pBeforeNode, const int iDesintationPosition, const bool bSelectCopy):
    //manual memzero is best with C++ as the first 4 bytes are the vptr
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_iUpdateType(unknownUpdateType),
    m_bExplicitPlural(bExplicitPlural),
    m_bTriggers(true),
    m_sDescription(sDescription ? MM_STRDUP(sDescription) : 0),
    m_sBaseXPath(0),
    m_sSelectXPath(0),
    m_sDestinationXPath(0),
    m_sBeforeXPath(0),
    m_iDestinationPosition(iDesintationPosition),

    m_sTransformWithXPath(0),
    m_sWithParam(0),
    m_sWithMode(0),

    m_sReadOPUserUUIDForTransform(0),
    m_sUserName(0),
    m_sUserStoreLocation(0),

    m_bSelectCopied(bSelectCopy),
    m_pChildDoc(0),
    m_pChildNode(0),
    m_bFromXML(false),
    m_bLastDocNeedsSelectCopy(false),
    m_bLastDocIsRepeatableAfterReload(true),

    m_bAppliedOnce(false),
    m_bHadNodesToProcess(false),
    m_sResultFQNodeName(0),
    m_sResultXmlId(0),
    m_sResultRepositoryName(0),
    m_sResultParentXPath(0),
    m_sExtensionElementPrefixes(0),
    m_sXSLModuleManagerName(0),
    m_iLastTransactionResult(TXml::transactionNotStarted),

    m_sDestinationXMLID(0),
    m_sSelectXMLID(0)
  {
    //private base class instanciation
    assert(iUpdateType);
    assert(pQE);

    m_iUpdateType = iUpdateType;
    
    //serialise / copy inputs
    if (!pSelectNode) throw TXmlSelectNodeRequired(mmParent(), MM_STRDUP("TXml::select-node"));
    if (bSelectCopy) selectCopyIntoTXml(pQE, pSelectNode);
    else {
      m_sSelectXPath = pSelectNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT);
      m_sSelectXMLID = pSelectNode->xmlID(pQE);
    }

    //TXml in to transient destination area, possibly @attribute-xpath softlinking, may fail until node locking is implemented
    if (pDestinationNode) {
      m_sDestinationXPath = pDestinationNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT); //throw TransientNode(this, ...)
      m_sDestinationXMLID = pDestinationNode->xmlID(pQE);
    }
    if (pBeforeNode) m_sBeforeXPath = pBeforeNode->uniqueXPathToNode(pQE, pBaseNode);

    //serialise the security settings
    //required security
    assert(pQE->securityContext());
    m_sReadOPUserUUIDForTransform = pQE->securityContext()->userUUID();  //MM_STRDUP
    m_sUserName                   = pQE->securityContext()->username();  //MM_STRDUP
    m_sUserStoreLocation          = pQE->securityContext()->userStore(); //MM_STRDUP unique xpath
  }

  TXml::TXml(const TXml& t):
    MemoryLifetimeOwner(t.mmParent()), //addChild()
    m_iUpdateType(t.m_iUpdateType),
    m_bExplicitPlural(t.m_bExplicitPlural),
    m_bTriggers(t.m_bTriggers),
    m_bSelectCopied(t.m_bSelectCopied),
    m_bFromXML(t.m_bFromXML),
    m_bLastDocIsRepeatableAfterReload(t.m_bLastDocIsRepeatableAfterReload),
    m_bLastDocNeedsSelectCopy(t.m_bLastDocNeedsSelectCopy),
    m_bAppliedOnce(t.m_bAppliedOnce),
    m_bHadNodesToProcess(t.m_bHadNodesToProcess),
    m_iLastTransactionResult(t.m_iLastTransactionResult),

    m_sDescription(t.m_sDescription ?  MM_STRDUP(t.m_sDescription) : 0),
    m_sBaseXPath(t.m_sBaseXPath ?      MM_STRDUP(t.m_sBaseXPath) : 0),
    m_sSelectXPath(t.m_sSelectXPath ?  MM_STRDUP(t.m_sSelectXPath) : 0),
    m_sDestinationXPath(t.m_sDestinationXPath ? MM_STRDUP(t.m_sDestinationXPath) : 0),
    m_sBeforeXPath(t.m_sBeforeXPath ?  MM_STRDUP(t.m_sBeforeXPath) : 0),
    m_iDestinationPosition(t.m_iDestinationPosition),

    m_sTransformWithXPath(t.m_sTransformWithXPath ? MM_STRDUP(t.m_sTransformWithXPath) : 0),
    m_sWithParam(t.m_sWithParam ?   MM_STRDUP(t.m_sWithParam) : 0),
    m_sWithMode(t.m_sWithMode ?     MM_STRDUP(t.m_sWithMode) : 0),

    //NOT_CURRENTLY_USED: m_sSpecialProgrammaticUser(t.m_sSpecialProgrammaticUser ? MM_STRDUP(t.m_sSpecialProgrammaticUser) : 0),
    m_sReadOPUserUUIDForTransform(t.m_sReadOPUserUUIDForTransform ? MM_STRDUP(t.m_sReadOPUserUUIDForTransform) : 0),
    m_sUserStoreLocation(t.m_sUserStoreLocation ? MM_STRDUP(t.m_sUserStoreLocation) : 0),
    m_sUserName(t.m_sUserName ?      MM_STRDUP(t.m_sUserName) : 0),

    m_sResultFQNodeName(t.m_sResultFQNodeName ?      MM_STRDUP(t.m_sResultFQNodeName) : 0),
    m_sResultXmlId(t.m_sResultXmlId ?                MM_STRDUP(t.m_sResultXmlId) : 0),
    m_sResultRepositoryName(t.m_sResultRepositoryName ? MM_STRDUP(t.m_sResultRepositoryName) : 0),
    m_sResultParentXPath(t.m_sResultParentXPath ?    MM_STRDUP(t.m_sResultParentXPath) : 0),
    m_sExtensionElementPrefixes(t.m_sExtensionElementPrefixes ? MM_STRDUP(t.m_sExtensionElementPrefixes) : 0),
    m_sXSLModuleManagerName(t.m_sXSLModuleManagerName ? MM_STRDUP(t.m_sXSLModuleManagerName) : 0),

    m_sDestinationXMLID(t.m_sDestinationXMLID ? MM_STRDUP(t.m_sDestinationXMLID) : 0),
    m_sSelectXMLID(t.m_sSelectXMLID ?           MM_STRDUP(t.m_sSelectXMLID) : 0)
  {
    IXmlBaseNode *pRootNode = 0;

    //m_pChildDoc is always OURS because the constructor said so
    //when we created it from a pSourceNode
    m_pChildDoc  = 0;
    m_pChildNode = 0;
    if (t.m_pChildDoc) {
      //copy doc
      m_pChildDoc = t.m_pChildDoc->clone_with_resources(); //OURS

      //set first child (if any)
      XmlAdminQueryEnvironment ibqe_childDocControl(this, m_pChildDoc);  //TXml control child document
      if (pRootNode = m_pChildDoc->rootNode(&ibqe_childDocControl)) {
        m_pChildNode = pRootNode->firstChild(&ibqe_childDocControl); //NEVER_OURS
      }
    }
    
    //free up
    if (pRootNode) delete pRootNode;
  }

  TXml::~TXml() {
    if (m_sDescription)      MMO_FREE(m_sDescription);
    if (m_sBaseXPath)        MMO_FREE(m_sBaseXPath);
    if (m_sSelectXPath)      MMO_FREE(m_sSelectXPath);
    if (m_sDestinationXPath) MMO_FREE(m_sDestinationXPath);
    if (m_sBeforeXPath)      MMO_FREE(m_sBeforeXPath);

    if (m_sTransformWithXPath) MMO_FREE(m_sTransformWithXPath);
    if (m_sWithParam)        MMO_FREE(m_sWithParam);
    if (m_sWithMode)         MMO_FREE(m_sWithMode);

    if (m_sReadOPUserUUIDForTransform) MMO_FREE(m_sReadOPUserUUIDForTransform);
    //if (m_sSpecialProgrammaticUser) MMO_FREE(m_sSpecialProgrammaticUser); //const
    if (m_sUserStoreLocation)  MMO_FREE(m_sUserStoreLocation);
    if (m_sUserName)           MMO_FREE(m_sUserName);

    if (m_pChildNode)        delete m_pChildNode; //child documents only
    if (m_pChildDoc)         delete m_pChildDoc;  //child documents only

    if (m_sResultFQNodeName) MMO_FREE(m_sResultFQNodeName);
    if (m_sResultXmlId)      MMO_FREE(m_sResultXmlId);
    if (m_sResultRepositoryName) MMO_FREE(m_sResultRepositoryName);
    if (m_sResultParentXPath)    MMO_FREE(m_sResultParentXPath);
    if (m_sExtensionElementPrefixes) MMO_FREE(m_sExtensionElementPrefixes);
    //if (m_sXSLModuleManagerName) MMO_FREE(m_sXSLModuleManagerName); //static string

    if (m_sDestinationXMLID) MMO_FREE(m_sDestinationXMLID);
    if (m_sSelectXMLID)      MMO_FREE(m_sSelectXMLID);
  }

  void TXml::addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    throw TXmlTransformNotSupportedFor(this, getUpdateTypeName());
  }
  void TXml::addTransformPrivate(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    assert(pTransformWithStylesheet);
    m_sTransformWithXPath = pTransformWithStylesheet->uniqueXPathToNode(pQE);
    m_sWithParam          = sWithParam;
    m_sWithMode           = sWithMode;
  }

  const char *TXml::getAffectedScopeName(const affectedScope iAffectedScope) {
    //returns a constant so dont free!
    const char *sAffectedScope = 0;
    switch (iAffectedScope) {
      case saveElementOnly:       {sAffectedScope = "saveElementOnly";       break;}
      case syncImmediateChildren: {sAffectedScope = "syncImmediateChildren"; break;}
      case saveTree:              {sAffectedScope = "saveTree";              break;}
      case linkChild:             {sAffectedScope = "linkChild";             break;}
    }
    return sAffectedScope;
  }
  TXml::affectedScope TXml::parseAffectedScopeName(const char *sAffectedScopeName) {
    TXml::affectedScope iAffectedScope = TXml::saveTree;
    if (sAffectedScopeName) {
      if      (!strcmp(sAffectedScopeName, "saveElementOnly"))       iAffectedScope = saveElementOnly;
      else if (!strcmp(sAffectedScopeName, "syncImmediateChildren")) iAffectedScope = syncImmediateChildren;
      else if (!strcmp(sAffectedScopeName, "saveTree"))              iAffectedScope = saveTree;
      else if (!strcmp(sAffectedScopeName, "linkChild"))             iAffectedScope = linkChild;
      //if not recognised, returns saveTree
    }
    return iAffectedScope;
  }
  const char *TXml::getUpdateTypeName() const {
    return getUpdateTypeName(m_iUpdateType);
  }

  const char *TXml::getUpdateTypeName(const updateType iUpdateType) {
    //returns a constant so dont free!
    const char *sUpdateTypeName = 0;
    switch (iUpdateType) {
      case moveChild:     {sUpdateTypeName = "move-child";     break;}
      case copyChild:     {sUpdateTypeName = "copy-child";     break;}
      case hardlinkChild: {sUpdateTypeName = "hardlink-child"; break;}
      case softlinkChild: {sUpdateTypeName = "softlink-child"; break;}
      case mergeNode:     {sUpdateTypeName = "merge-node";     break;}
      case replaceNodeCopy:   {sUpdateTypeName = "replace-node";   break;}
      case setAttribute:  {sUpdateTypeName = "set-attribute";  break;}
      case setValue:      {sUpdateTypeName = "set-value";  break;}
      case removeNode:    {sUpdateTypeName = "remove-node";    break;}
      case changeName:    {sUpdateTypeName = "change-name";    break;}
      case createChildElement: {sUpdateTypeName = "create-child-element"; break;}
      case touchNode:     {sUpdateTypeName = "touch-node";     break;}
      case deviateNode:   {sUpdateTypeName = "deviate-node";   break;}
      case unknownUpdateType: ATTRIBUTE_FALLTHROUGH;
    }
    return sUpdateTypeName;
  }

  TXml::updateType TXml::parseUpdateType(const char *sUpdateTypeName, const bool bExplicitPlural ) {
    updateType iUpdateType         = unknownUpdateType;
    bool bUpdateTypeNameDuplicated = false;
    bool bExplicitPluralTypeNotFound       = false;
    size_t iLen;
    
    if (sUpdateTypeName) {
      //------------------------------------ plural update types
      //e.g. database:softlink-children => multiple softlinkChild
      //the TXml will be sent through the bExplicitPlural indicator
      //if it cares. TXml_SoftlinkChild cares
      if (bExplicitPlural) {
        iLen = strlen(sUpdateTypeName);
        if        (iLen > 3 && _STREQUAL(sUpdateTypeName + iLen - 3, "ren")) {
          sUpdateTypeName = MM_STRNDUP(sUpdateTypeName, iLen - 3);
          bUpdateTypeNameDuplicated = true;
        } else if (iLen > 1 && sUpdateTypeName[iLen-1] == 's') {
          sUpdateTypeName = MM_STRNDUP(sUpdateTypeName, iLen - 1);
          bUpdateTypeNameDuplicated = true;
        } else {
          //the update type is NOT plural
          //this was required, so we return an unknownUpdateType
          bExplicitPluralTypeNotFound = true;
        }
      }
      
      //------------------------------------ standard singular update types
      if (!bExplicitPluralTypeNotFound) {
        if      (!strcmp(sUpdateTypeName, "move-child"))     iUpdateType = moveChild;
        else if (!strcmp(sUpdateTypeName, "copy-child"))     iUpdateType = copyChild;
        else if (!strcmp(sUpdateTypeName, "hardlink-child")) iUpdateType = hardlinkChild;
        else if (!strcmp(sUpdateTypeName, "softlink-child")) iUpdateType = softlinkChild;
        else if (!strcmp(sUpdateTypeName, "merge-node"))     iUpdateType = mergeNode;
        else if (!strcmp(sUpdateTypeName, "replace-node"))   iUpdateType = replaceNodeCopy;
        else if (!strcmp(sUpdateTypeName, "set-attribute"))  iUpdateType = setAttribute;
        else if (!strcmp(sUpdateTypeName, "set-value"))      iUpdateType = setValue;
        else if (!strcmp(sUpdateTypeName, "remove-node"))    iUpdateType = removeNode;
        else if (!strcmp(sUpdateTypeName, "change-name"))    iUpdateType = changeName;
        else if (!strcmp(sUpdateTypeName, "create-child-element")) iUpdateType = createChildElement;
        else if (!strcmp(sUpdateTypeName, "touch-node"))     iUpdateType = touchNode;
      }
    }
    
    //free up
    if (bUpdateTypeNameDuplicated) MM_FREE(sUpdateTypeName);
    
    return iUpdateType;
  }
  
  const char *TXml::toString() const {
    stringstream sOut;
    const char *sXML, *sXMLStart;

    sOut << "TXml:" << getUpdateTypeName(m_iUpdateType);
    if (!m_bLastDocIsRepeatableAfterReload) sOut << " (notRepeatable)";
    if (m_bLastDocNeedsSelectCopy)          sOut << " (selectCopied)";
    if (m_bExplicitPlural)                  sOut << " (explicitPlural)";
    if (m_sDescription)                     sOut << "\n description:" << m_sDescription;
    if (m_sSelectXPath)                     sOut << "\n select:"      << m_sSelectXPath;
    if (m_sDestinationXPath)                sOut << "\n destination:" << m_sDestinationXPath;
    if (m_iDestinationPosition)             sOut << " *[" << m_iDestinationPosition << "]";
    if (m_pChildNode) {
      const XmlAdminQueryEnvironment ibqe_internal_serialisation(this, m_pChildDoc);
      if (sXML = m_pChildNode->xml(&ibqe_internal_serialisation)) {
        sXMLStart = strndup(sXML, 500);
        sOut << "\n XML: " << sXMLStart << (strlen(sXML) > 500 ? "..." : "");
        MMO_FREE(sXMLStart);
        MMO_FREE(sXML);
      }
    }
    if (m_sTransformWithXPath) {
      sOut << "\n Transform";
      if (m_sWithParam || m_sWithMode) {
        sOut << " [";
        if (m_sWithParam) sOut << m_sWithParam << ",";
        if (m_sWithMode)  sOut << m_sWithMode;
        sOut << "]";
      }
      sOut << ": " << m_sTransformWithXPath;
    }

    return MM_STRDUP(sOut.str().c_str());
  }

  const char *TXml::serialise(IXmlBaseNode *pTempParent) const {
    //used by FileSystem functions when storing string versions of Transactions
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pTempParent->document());
    IXmlBaseNode *pTXmlSpecNode = toNode(pTempParent);
    const char *sTXmlSpecString = pTXmlSpecNode->xml(&ibqe_internal_serialisation);
    pTXmlSpecNode->removeAndDestroyNode(&ibqe_internal_serialisation);
    delete pTXmlSpecNode;

    return sTXmlSpecString;
  }

  IXmlBaseNode *TXml::toNode(IXmlBaseNode *pParent) const {
    //caller ->removes() and frees result
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    const char *sLocalName = getUpdateTypeName(m_iUpdateType);
    size_t iLen            = 14 + 1 + strlen(sLocalName) + 1;
    char *sQualifiedName   = MMO_MALLOC(iLen);
    _SNPRINTF2(sQualifiedName, iLen, "%s:%s", "xmltransaction", sLocalName);
    IXmlBaseNode *pTXmlSpecNode = pParent->createChildElement(&ibqe_internal_serialisation, sQualifiedName);

    if (pTXmlSpecNode) {
      //location
      if (m_sDescription)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "description",          m_sDescription);
      if (m_sBaseXPath)            pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "base",                 m_sBaseXPath);
      if (m_sSelectXPath)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "select",               m_sSelectXPath);
      if (m_sDestinationXPath)     pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "destination",          m_sDestinationXPath);
      if (m_sBeforeXPath)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "before",               m_sBeforeXPath);
      if (m_iDestinationPosition)  pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "destination-position", m_iDestinationPosition);

      //transforms
      if (m_sTransformWithXPath) {
        pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "transform", m_sTransformWithXPath);
        if (m_sWithParam)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "with-param",      m_sWithParam);
        if (m_sWithMode)           pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "interface-mode",       m_sWithMode);
      }

      //security context
      if (m_sUserName)               pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "user-id", m_sUserName);
      if (m_sReadOPUserUUIDForTransform) pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "user-guid", m_sReadOPUserUUIDForTransform);
      if (m_sUserStoreLocation)    pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "user-store", m_sUserStoreLocation);

      //select copying and subsequent child doc
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "select-copied",                                m_bSelectCopied);
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "explicit-plural",                              m_bExplicitPlural);
      if (m_pChildNode)          delete pTXmlSpecNode->copyChild(&ibqe_internal_serialisation, m_pChildNode);
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "from-xml",                                     m_bFromXML);
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "is-repeatable-after-reload",                   m_bLastDocIsRepeatableAfterReload);
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "needs-select-copy",                            m_bLastDocNeedsSelectCopy);

      //result sync and checking
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "applied-once",                                 m_bAppliedOnce);       //store just for documentation purposes, will not be un-serialised though
      pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "had-nodes-to-process",                         m_bHadNodesToProcess); //store just for documentation purposes, will not be un-serialised though
      if (m_sResultFQNodeName)     pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "result-qname",      m_sResultFQNodeName);
      if (m_sResultXmlId)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "result-xmlid",      m_sResultXmlId);
      if (m_sResultRepositoryName) pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "result-repository_name", m_sResultRepositoryName);
      if (m_sResultParentXPath)    pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "result-parent-xpath", m_sResultParentXPath);
      if (m_sExtensionElementPrefixes) pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "extension-element-prefixes", m_sExtensionElementPrefixes);
      if (m_sXSLModuleManagerName) pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "extensions-module-name", m_sXSLModuleManagerName);

      //base xml:ids for input and destination if available
      //useful for checking basic identity changes during transofrms
      if (m_sDestinationXMLID)     pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "xmlid-destination", m_sDestinationXMLID);
      if (m_sSelectXMLID)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "xmlid-input",       m_sSelectXMLID);
    }

    //free up
    //MMO_FREE(sLocalName); //constants!
    MMO_FREE(sQualifiedName);

    return pTXmlSpecNode;
  }

  bool TXml::hasNodesToProcess() const {
    //the transform result or select path has yielded nodes
    //still *valid* if not
    return m_sSelectXPath || m_pChildNode;
  }

  TXml::transactionResult TXml::applyTo(ITXmlDirectFunctions *pStore, IXmlBaseNode **ppResultNode, const IXmlQueryEnvironment *pDirectPassedQE) {
    TXml::transactionResult tr  = TXml::transactionSuccess;

    //result sync and checking
    //  these functions receive text data back to serialise
    //  to help the application of the transaction in associated repositories
    //  and the checking that they are re-apply properly
    const char *sNewResultFQNodeName           = 0,
               *sNewResultXmlId                = 0,
               *sNewResultRepositoryName       = 0,
               *sNewResultParentXPath          = 0,
               *sExtensionElementPrefixes      = 0,
               *sNewResultXslModuleManagerName = 0;

    //virtual
    tr = runFunctionOnStore(pStore, m_sUserStoreLocation, pDirectPassedQE, ppResultNode,
           &sNewResultFQNodeName, 
           &sNewResultXmlId, 
           &sNewResultRepositoryName, 
           &sNewResultParentXPath, 
           &sExtensionElementPrefixes, 
           &sNewResultXslModuleManagerName
    );

    //TODO: throws OutOfSync if the new results don't match
    //and free()s appropriately
    //TODO: bHadNodesToProcess
    setResultsData(
      true,
      sNewResultFQNodeName,
      sNewResultXmlId,
      sNewResultRepositoryName,
      sNewResultParentXPath,
      sExtensionElementPrefixes,
      sNewResultXslModuleManagerName
    );
    m_bAppliedOnce = true;

    return tr;
  }


  //----------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------- TXml sub-classes
  //----------------------------------------------------------------------------------------
  TXml_MoveChild::TXml_MoveChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDesintationPosition, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
     TXml(pMemoryLifetimeOwner, pQE,TXml::moveChild, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, pBeforeNode, iDesintationPosition) //protected
  {
    m_bLastDocNeedsSelectCopy         = pSelectNode && pSelectNode->isTransient();
    m_bLastDocIsRepeatableAfterReload = pDestinationNode && !pDestinationNode->isTransient();
  }
  
  TXml::transactionResult TXml_MoveChild::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->moveChild(m_sUserName, m_sUserStoreLocation, pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                             m_sBaseXPath, m_sSelectXPath, m_pChildNode, m_sDestinationXPath, m_sBeforeXPath, m_iDestinationPosition, 
                             m_sTransformWithXPath, m_sWithParam, m_sWithMode,
                             pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                            );
  }
  
  void TXml_MoveChild::addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    addTransformPrivate(pQE, pTransformWithStylesheet, sWithParam, sWithMode);
  }
  
  void TXml_MoveChild::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //moveChild -> 1 x replaceNodeCopy + 1 x removeNode:  descendant-or-self of the accompanying nodes have changed
    //    source-node gone: always the case, register parent
    //    dest-node gone:   ignore dest-node   (something else has rmeoved it and will save it)
    //    dest-node is ancestor: (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultParentXPath)) {
      TXml::AffectedNodeDetails andetails = {syncImmediateChildren, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultXmlId)) {
      TXml::AffectedNodeDetails andetails = {saveTree, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }

  
  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_CopyChild ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_CopyChild::TXml_CopyChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDesintationPosition, const bool bDeepClone, const bool bSelectCopy, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::copyChild, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, pBeforeNode, iDesintationPosition, bSelectCopy), //protected
    m_bDeepClone(bDeepClone)
  {
    m_bLastDocNeedsSelectCopy         = pSelectNode && pSelectNode->isTransient();
    m_bLastDocIsRepeatableAfterReload = pDestinationNode && !pDestinationNode->isTransient();
  }
  
  TXml::transactionResult TXml_CopyChild::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    if (!m_bDeepClone) NOT_COMPLETE("TXml_CopyChild: shallow clone"); //TXml_CopyChild::runFunctionOnStore()
    return pStore->copyChild(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                             m_sBaseXPath, m_sSelectXPath, m_pChildNode, m_sDestinationXPath, m_sBeforeXPath, m_iDestinationPosition, m_bDeepClone,
                             m_sTransformWithXPath, m_sWithParam, m_sWithMode,
                             pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                            );
  }
  
  void TXml_CopyChild::addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    addTransformPrivate(pQE, pTransformWithStylesheet, sWithParam, sWithMode);
  }
  
  IXmlBaseNode *TXml_CopyChild::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    //rare ones: @xml, @name, @value, @deep-clone
    pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "deep-clone", m_bDeepClone);
    return pTXmlSpecNode;
  }
  
  IXmlBaseNode *TXml_CopyChild::selectCopyIntoTXml(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pChildNodeToCopy, const IXmlBaseDoc *pNamespaceDoc) {
    //do not free result! because it is held in TXml space and released by ~TXml()
    assert(pChildNodeToCopy);

    IXmlBaseNode *pChildDocRootNode = createChildDoc(pNamespaceDoc ? pNamespaceDoc : pChildNodeToCopy->document());
    m_pChildNode = pChildDocRootNode->copyChild(pQE, pChildNodeToCopy, m_bDeepClone);
    delete pChildDocRootNode;

    return m_pChildNode;
  }
  
  void TXml_CopyChild::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //copyChild -> replace-node: only 1 new node
    //    FileSystem -> save File
    //    node gone:         ignore TXml (another subsequent or previous TXml has removed it)
    //    node is ancestor: remove descendant TXmls (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->document()->nodeFromID(&ibqe_commit, m_sResultXmlId)) {
      TXml::AffectedNodeDetails andetails = {saveTree, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_DeviateNode ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_DeviateNode::TXml_DeviateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pHardlink, const IXmlBaseNode *pNewNode, const iDeviationType iType, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::deviateNode, sDescription, pBaseNode, pSelectNode, pHardlink, bExplicitPlural, pNewNode) //protected
  {
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && pHardlink && pNewNode
                                     && !pSelectNode->isTransient() && !pHardlink->isTransient() && !pNewNode->isTransient();
  }
  TXml::transactionResult TXml_DeviateNode::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->deviateNode(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                 m_sBaseXPath, m_sSelectXPath, m_sDestinationXPath, m_sBeforeXPath, 
                                 pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                );
  }
  void TXml_DeviateNode::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //hardlinkChild, softlinkChild -> same:  only 1 new node?
    //    FileSystem -> possibly sym links and hard links
    //    node gone:        ignore TXml
    //    node is ancestor: (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultParentXPath)) {
      TXml::AffectedNodeDetails andetails = {linkChild, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_HardlinkChild ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_HardlinkChild::TXml_HardlinkChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const int iDesintationPosition, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::hardlinkChild, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, pBeforeNode, iDesintationPosition) //protected
  {
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && pDestinationNode && !pSelectNode->isTransient() && !pDestinationNode->isTransient();
  }
  TXml::transactionResult TXml_HardlinkChild::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->hardlinkChild(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                 m_sBaseXPath, m_sSelectXPath, m_sDestinationXPath, m_sBeforeXPath, m_iDestinationPosition,
                                 pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                );
  }
  void TXml_HardlinkChild::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //hardlinkChild, softlinkChild -> same:  only 1 new node?
    //    FileSystem -> possibly sym links and hard links
    //    node gone:        ignore TXml
    //    node is ancestor: (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultParentXPath)) {
      TXml::AffectedNodeDetails andetails = {linkChild, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_SoftlinkChild ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_SoftlinkChild::TXml_SoftlinkChild(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const IXmlBaseNode *pBeforeNode, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::softlinkChild, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, pBeforeNode) //protected
  {
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && pDestinationNode && !pSelectNode->isTransient() && !pDestinationNode->isTransient();
  }
  
  TXml::transactionResult TXml_SoftlinkChild::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->softlinkChild(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                 m_sBaseXPath, m_sSelectXPath, m_sDestinationXPath, m_sBeforeXPath,
                                 pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                );
  }
  
  void TXml_SoftlinkChild::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //hardlinkChild, softlinkChild -> same:  only 1 new node?
    //    FileSystem -> possibly sym links and hard links
    //    node gone:        ignore TXml
    //    node is ancestor: (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultParentXPath)) {
      TXml::AffectedNodeDetails andetails = {linkChild, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_MergeNode ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_MergeNode::TXml_MergeNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::mergeNode, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, 0) //protected
  {
    m_bLastDocNeedsSelectCopy         = pSelectNode && pSelectNode->isTransient();
    m_bLastDocIsRepeatableAfterReload = pDestinationNode && !pDestinationNode->isTransient();
  }
  TXml::transactionResult TXml_MergeNode::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->mergeNode(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                   m_sBaseXPath, m_sSelectXPath, m_pChildNode, m_sDestinationXPath,
                                   m_sTransformWithXPath, m_sWithParam, m_sWithMode,
                                   pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                  );
  }
  void TXml_MergeNode::addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    addTransformPrivate(pQE, pTransformWithStylesheet, sWithParam, sWithMode);
  }
  void TXml_MergeNode::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode, *pParentNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //mergeNode from XML or copied from select -> mergeNode:  descendant-or-self of the accompanying node have changed
    //however, TXml requires that the xml:id of the primary node remains the same
    //so m_sResultXmlId = @xml:id of the original node
    //    FileSystem -> saving several files for each TXml
    //    node gone:        ignore TXml (another subsequent or previous TXml has removed it)
    //    node is ancestor: remove descendant TXmls (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sDestinationXPath)) {
      TXml::AffectedNodeDetails andetails1 = {saveTree, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails1); //copy
      if (pParentNode = pNewAffectedNode->parentNode(&ibqe_commit)) {
        TXml::AffectedNodeDetails andetails2 = {syncImmediateChildren, pParentNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
        pvAffectedNodes->push_back(andetails2); //copy
      }
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }
  
  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_ReplaceNode ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_ReplaceNode::TXml_ReplaceNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const IXmlBaseNode *pDestinationNode, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::replaceNodeCopy, sDescription, pBaseNode, pSelectNode, pDestinationNode, bExplicitPlural, 0) //protected
  {
    m_bLastDocNeedsSelectCopy         = pSelectNode && pSelectNode->isTransient();
    m_bLastDocIsRepeatableAfterReload = pDestinationNode && !pDestinationNode->isTransient();
  }
  TXml::transactionResult TXml_ReplaceNode::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->replaceNodeCopy(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                   m_sBaseXPath, m_sSelectXPath, m_pChildNode, m_sDestinationXPath,
                                   m_sTransformWithXPath, m_sWithParam, m_sWithMode,
                                   pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                  );
  }
  void TXml_ReplaceNode::addTransform(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTransformWithStylesheet, const char *sWithParam, const char *sWithMode) {
    addTransformPrivate(pQE, pTransformWithStylesheet, sWithParam, sWithMode);
  }
  void TXml_ReplaceNode::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode, *pParentNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //replaceNodeCopy from XML or copied from select -> replaceNodeCopy:  descendant-or-self of the accompanying node have changed
    //however, TXml requires that the xml:id of the primary node remains the same
    //so m_sResultXmlId = @xml:id of the original node
    //    FileSystem -> saving several files for each TXml
    //    node gone:        ignore TXml (another subsequent or previous TXml has removed it)
    //    node is ancestor: remove descendant TXmls (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sDestinationXPath)) {
      TXml::AffectedNodeDetails andetails1 = {saveTree, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails1); //copy
      if (pParentNode = pNewAffectedNode->parentNode(&ibqe_commit)) {
        TXml::AffectedNodeDetails andetails2 = {syncImmediateChildren, pParentNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
        pvAffectedNodes->push_back(andetails2); //copy
      }
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_SetAttribute ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_SetAttribute::TXml_SetAttribute(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const char *sValue, const char *sNamespace, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::setAttribute, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0), //protected
    m_sNamespace(0),
    m_sName(0),
    m_sValue(0)
  {
    if (pSelectNode->isTransient())
      throw TransientNode(mmParent(), MM_STRDUP("TXml_SetAttribute::select-node"));
    
    if (sNamespace) m_sNamespace = MM_STRDUP(sNamespace);
    if (sName)      m_sName      = MM_STRDUP(sName);
    if (sValue)     m_sValue     = MM_STRDUP(sValue);

    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }
  
  TXml_SetAttribute::TXml_SetAttribute(const TXml_SetAttribute& t): 
    MemoryLifetimeOwner(t.mmParent()),
    TXml(t),
    m_sNamespace(t.m_sNamespace  ? MM_STRDUP(t.m_sNamespace)  : NULL),
    m_sName(     t.m_sName       ? MM_STRDUP(t.m_sName)  : NULL),
    m_sValue(    t.m_sValue      ? MM_STRDUP(t.m_sValue) : NULL)
  {}
  
  TXml_SetAttribute::~TXml_SetAttribute() {
    if (m_sNamespace)  MMO_FREE(m_sNamespace);
    if (m_sName)       MMO_FREE(m_sName);
    if (m_sValue)      MMO_FREE(m_sValue);
  }

  const char *TXml_SetAttribute::toString() const {
    stringstream sOut;

    sOut << TXml::toString();
    if (m_sNamespace)        sOut << "\n @namespace:"  << m_sNamespace;
    if (m_sName)             sOut << "\n @name:"       << m_sName;
    if (m_sValue)            sOut << "\n @value:"      << m_sValue;

    return MM_STRDUP(sOut.str().c_str());
  }

  IXmlBaseNode *TXml_SetAttribute::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    if (m_sName)               pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "name",              m_sName);
    if (m_sNamespace)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "namespace",         m_sNamespace);
    if (m_sValue)              pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "value",             m_sValue);
    return pTXmlSpecNode;
  }

  TXml::transactionResult TXml_SetAttribute::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->setAttribute(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                m_sBaseXPath, m_sSelectXPath, m_sName, m_sValue, m_sNamespace,
                                pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName);
  }

  void TXml_SetAttribute::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pBaseNode            = 0;
    XmlNodeList<const IXmlBaseNode> *pvSelectNodes = 0;
    XmlNodeList<const IXmlBaseNode>::iterator iSelectNode;

    //base for xpath resolutions
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //setAttribute -> setAttribute: attribute(s) have been affected on the accompanying node
    //    FileSystem -> relates to changes in directory.metaxml, or a File save
    //    node gone:        ignore TXml (another subsequent or previous TXml has moved / removed it and will save it)
    //    node is ancestor: additional
    if (pvSelectNodes = pBaseNode->getMultipleNodes(&ibqe_commit, m_sSelectXPath)) {
      for (iSelectNode = pvSelectNodes->begin(); iSelectNode != pvSelectNodes->end(); iSelectNode++) {
        TXml::AffectedNodeDetails andetails = {saveElementOnly, *iSelectNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
        pvAffectedNodes->push_back(andetails); //copy
      }
    }

    //clear up
    if (pBaseNode)     delete pBaseNode;
    if (pvSelectNodes) delete pvSelectNodes;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_SetValue ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_SetValue::TXml_SetValue(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sValue, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::setValue, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0), //protected
    m_sValue(0)
  {
    if (sValue) m_sValue = MM_STRDUP(sValue);

    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }

  TXml_SetValue::TXml_SetValue(const TXml_SetValue& t): 
    MemoryLifetimeOwner(t.mmParent()),
    TXml(t),
    m_sValue(t.m_sValue ? MM_STRDUP(t.m_sValue) : NULL)
  {}
  
  TXml_SetValue::~TXml_SetValue() {
    if (m_sValue) MMO_FREE(m_sValue);
  }
  
  const char *TXml_SetValue::toString() const {
    stringstream sOut;

    sOut << TXml::toString();
    if (m_sValue)            sOut << "\n @value:"      << m_sValue;

    return MM_STRDUP(sOut.str().c_str());
  }

  IXmlBaseNode *TXml_SetValue::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    if (m_sValue)              pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "value",             m_sValue);
    return pTXmlSpecNode;
  }
  
  TXml::transactionResult TXml_SetValue::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->setValue(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                m_sBaseXPath, m_sSelectXPath, m_sValue,
                                pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName);
  }
  void TXml_SetValue::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //setAttribute -> setAttribute: attribute(s) have been affected on the accompanying node
    //    FileSystem -> relates to changes in directory.metaxml, or a File save
    //    node gone:        ignore TXml (another subsequent or previous TXml has moved / removed it and will save it)
    //    node is ancestor: additional
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sSelectXPath)) {
      TXml::AffectedNodeDetails andetails = {saveElementOnly, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }


  //----------------------------------------------------------------------------------------
  //------------------------------------ TXml_RemoveNode ------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_RemoveNode::TXml_RemoveNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::removeNode, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0) //protected
  {
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }
  
  TXml::transactionResult TXml_RemoveNode::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *sUserStoreXPath, const IXmlQueryEnvironment *pDirectPassedQE,  IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
   return pStore->removeNode(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                             m_sBaseXPath, m_sSelectXPath,
                             pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                            );
  }
  
  void TXml_RemoveNode::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //removeNode -> removeNode:  this is the *parent* of the missing node. accompanying data identifies which node
    //    FileSystem -> realtes to delete a file or save a file
    //    node gone:        always the case, register parent
    //    parent gone:      ignore TXml (another subsequent or previous TXml has removed it)
    //    node is ancestor: remove descendant TXmls
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sSelectXPath)) {
      //the removedNode node still there so we don't need to delete it
      //this can only be caused by an undo of some kind
      //at the moment it would be an error
      //the removedNode xml:id SHOULD NOT have been re-used
      delete pNewAffectedNode;
    } else {
      if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sResultParentXPath)) {
        TXml::AffectedNodeDetails andetails = {syncImmediateChildren, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
        pvAffectedNodes->push_back(andetails); //copy
      }
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }

  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_ChangeName::TXml_ChangeName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::changeName, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0), //protected
    m_sName(0)
  {
    if (sName) m_sName  = MM_STRDUP(sName);

    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }

  TXml_ChangeName::TXml_ChangeName(const TXml_ChangeName& t): 
    MemoryLifetimeOwner(t.mmParent()),
    TXml(t),
    m_sName(t.m_sName ? MM_STRDUP(t.m_sName) : NULL)
  {}

  TXml_ChangeName::~TXml_ChangeName() {
    if (m_sName)  MMO_FREE(m_sName);
  }
  
  const char *TXml_ChangeName::toString() const {
    stringstream sOut;

    sOut << TXml::toString();
    if (m_sName)             sOut << "\n @name:"       << m_sName;

    return MM_STRDUP(sOut.str().c_str());
  }
  
  TXml::transactionResult TXml_ChangeName::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *pUserStore, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->changeName(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                              m_sBaseXPath, m_sSelectXPath, m_sName,
                              pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                             );
  }

  IXmlBaseNode *TXml_ChangeName::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    if (m_sName) pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "name",             m_sName);
    return pTXmlSpecNode;
  }
  
  void TXml_ChangeName::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //changeName -> changeName:   local-name change on the accompanying node
    //    FileSystem -> relates to changes in directory.metaxml, or a File save
    //    node gone:        ignore TXml (another subsequent or previous TXml has moved / removed it and will save it)
    //    node is ancestor: additional
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sSelectXPath)) {
      TXml::AffectedNodeDetails andetails = {saveElementOnly, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }

  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_CreateChildElement::TXml_CreateChildElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const char *sName, const char *sValue, const char *sNamespace, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::createChildElement, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0), //protected
    m_sName(     sName      ? MM_STRDUP(sName)       : 0),
    m_sValue(    sValue     ? MM_STRDUP(sValue)      : 0),
    m_sNamespace(sNamespace ? MM_STRDUP(sNamespace)  : 0)
  {
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }
  
  TXml_CreateChildElement::TXml_CreateChildElement(const TXml_CreateChildElement& t): 
    MemoryLifetimeOwner(t.mmParent()),
    TXml(t),
    m_sName(     t.m_sName      ? MM_STRDUP(t.m_sName)      : NULL),
    m_sValue(    t.m_sValue     ? MM_STRDUP(t.m_sValue)     : NULL),
    m_sNamespace(t.m_sNamespace ? MM_STRDUP(t.m_sNamespace) : NULL)
  {}

  TXml_CreateChildElement::~TXml_CreateChildElement() {
    if (m_sName)      MMO_FREE(m_sName);
    if (m_sValue)     MMO_FREE(m_sValue);
    if (m_sNamespace) MMO_FREE(m_sNamespace);
  }
  const char *TXml_CreateChildElement::toString() const {
    stringstream sOut;

    sOut << TXml::toString();
    if (m_sName)             sOut << "\n @name:"       << m_sName;
    if (m_sValue)            sOut << "\n @value:"      << m_sValue;
    if (m_sNamespace)        sOut << "\n @namespace:"  << m_sNamespace;

    return MM_STRDUP(sOut.str().c_str());
  }
  IXmlBaseNode *TXml_CreateChildElement::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    if (m_sName)               pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "name",              m_sName);
    if (m_sValue)              pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "value",             m_sValue);
    if (m_sNamespace)          pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "namespace",         m_sNamespace);
    return pTXmlSpecNode;
  }
  
  TXml::transactionResult TXml_CreateChildElement::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *pUserStore, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->createChildElement(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                      m_sBaseXPath, m_sSelectXPath, m_sName, m_sValue, m_sNamespace,
                                      pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                     );
  }
  
  void TXml_CreateChildElement::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //createChildElement -> createChildElement:   new sub-element on the accompanying node
    //    FileSystem -> save of whole sub-tree and anything under it also created after
    //    node gone:        ignore TXml (another subsequent or previous TXml has moved / removed it and will save it)
    //    node is ancestor: additional
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sSelectXPath)) {
      TXml::AffectedNodeDetails andetails = {saveTree, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }

  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------
  TXml_TouchNode::TXml_TouchNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE, const char *sDescription, const IXmlBaseNode *pBaseNode, const IXmlBaseNode *pSelectNode, const TXml::affectedScope as, const bool bExplicitPlural):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXml(pMemoryLifetimeOwner, pQE,TXml::touchNode, sDescription, pBaseNode, pSelectNode, NULL, bExplicitPlural, 0) //protected
  {
    m_as                              = as;
    m_bLastDocNeedsSelectCopy         = false;
    m_bLastDocIsRepeatableAfterReload = pSelectNode && !pSelectNode->isTransient();
  }
  TXml::transactionResult TXml_TouchNode::runFunctionOnStore(ITXmlDirectFunctions *pStore, const char *pUserStore, const IXmlQueryEnvironment *pDirectPassedQE, IXmlBaseNode **pResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sResultParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName) {
    return pStore->touch(m_sUserName, m_sUserStoreLocation,  pDirectPassedQE, m_sReadOPUserUUIDForTransform,
                                      m_sBaseXPath, m_sSelectXPath,
                                      pResultNode, sSelectFQNodeName, sSelectXmlId, sSelectRepositoryName, sResultParentXPath, sExtensionElementPrefixes, sNewResultXslModuleManagerName
                                     );
  }
  
  IXmlBaseNode *TXml_TouchNode::toNode(IXmlBaseNode *pParent) const {
    XmlAdminQueryEnvironment ibqe_internal_serialisation(this, pParent->document());
    IXmlBaseNode *pTXmlSpecNode = TXml::toNode(pParent);
    pTXmlSpecNode->setAttribute(&ibqe_internal_serialisation, "affected-scope", getAffectedScopeName(m_as)); //constants
    return pTXmlSpecNode;
  }
  
  void TXml_TouchNode::affectedNodes(vector<TXml::AffectedNodeDetails> *pvAffectedNodes, const IXmlBaseDoc *pInThisDoc) const {
    const XmlAdminQueryEnvironment ibqe_commit(this, pInThisDoc);
    const IXmlBaseNode *pNewAffectedNode;

    //base for xpath resolutions
    const IXmlBaseNode *pBaseNode = 0;
    if (m_sBaseXPath) pBaseNode = pInThisDoc->getSingleNode(&ibqe_commit, m_sBaseXPath);
    else              pBaseNode = pInThisDoc->documentNode(); //all xpath will be absolute anyway

    //touchNode -> touchNode:   save the tree
    //    FileSystem -> relates to changes in directory.metaxml, or a File save
    //    node gone:        ignore TXml (another subsequent or previous TXml has moved / removed it and will save it)
    //    node is ancestor: remove descendant TXmls (resultant changes will get saved)
    if (pNewAffectedNode = pBaseNode->getSingleNode(&ibqe_commit, m_sSelectXPath)) {
      TXml::AffectedNodeDetails andetails = {m_as, pNewAffectedNode, m_sResultFQNodeName, m_sResultRepositoryName, m_sResultXmlId, m_sResultParentXPath};
      pvAffectedNodes->push_back(andetails); //copy
    }

    //clear up
    if (pBaseNode) delete pBaseNode;
  }
}
