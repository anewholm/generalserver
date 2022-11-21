#include "Response.h"

#include "IXml/IXslNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlNamespace.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXslTransformContext.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "MessageInterpretation.h"
#include "Exceptions.h"
#include "QueryEnvironment/BrowserQueryEnvironment.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"
#include <cmath>

//DEBUG output
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails> Response::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Response::m_XSLFunctions;

  Response::Response(Request *pRequest): 
    MemoryLifetimeOwner(pRequest),
    m_pRequest(pRequest), 
    m_pPostTransformStylesheetDatabaseNode(0), 
    m_sPostTransformResultStartNodeXPath(0) 
  {
    assert(m_pRequest);
  }

  Response::~Response() {
    if (m_pPostTransformStylesheetDatabaseNode) delete m_pPostTransformStylesheetDatabaseNode;
    if (m_sPostTransformResultStartNodeXPath)   MMO_FREE(m_sPostTransformResultStartNodeXPath);
  }

  const char *Response::xsltModuleNamespace() const {return NAMESPACE_RESPONSE;}
  const char *Response::xsltModulePrefix()    const {return NAMESPACE_RESPONSE_ALIAS;}

  const IXmlLibrary *Response::xmlLibrary() const    {return m_pRequest->xmlLibrary();}

  const StringMap<IXslModule::XslModuleFunctionDetails> *Response::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("can-have-attribute",    XMF(Response::xslFunction_canHaveAttribute));
      m_XSLFunctions.insert("cannot-have-attribute", XMF(Response::xslFunction_cannotHaveAttribute));
      m_XSLFunctions.insert("set-post-transform",    XMF(Response::xslFunction_setPostTransform));
    }
    return &m_XSLFunctions;
  }

  const StringMap<IXslModule::XslModuleCommandDetails> *Response::xslCommands() const {
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("set-header", XMC(Response::xslCommand_setHeader));
    }
    return &m_XSLCommands;
  }
  
  Database *Response::db() const {return m_pRequest->db();}

  const char *Response::header() const {return m_stHeader.c_str();}

  const char *Response::xslFunction_cannotHaveAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "response:cannot-have-attribute([attribute-name or @attribute-node, xpath])"; UNUSED(sFunctionSignature);
    const char *sResult = xslFunction_canHaveAttribute(pQE, pXCtxt, pNodes);
    bool bCanHaveAttribute = sResult;
    if (sResult) MMO_FREE(sResult);
    return (bCanHaveAttribute ? 0 : MM_STRDUP("true"));
  }

  IXmlBaseNode *Response::xslCommand_setHeader(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //TODO: allow dynamic sub-content
    //if the value is blank:
    //  we quietly fail here assuming that the header should not be added
    static const char *sCommandSignature = "response:set-header @value [@header @header-delimiter @line-return]"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sHeaderDelimiter = 0,
               *sHeader          = 0,
               *sToken           = 0,
               *sValue           = 0,
               *sLineReturn      = 0;

    UNWIND_EXCEPTION_BEGIN {
      if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
        //required
        sValue           = pCommandNodeType->attributeValueDynamic(pQE, "value");
        //optional
        sHeader          = pCommandNodeType->attributeValueDynamic(pQE, "header");
        sToken           = pCommandNodeType->attributeValueDynamic(pQE, "token");
        sHeaderDelimiter = pCommandNodeType->attributeValueDynamic(pQE, "header-delimiter", NAMESPACE_NONE, ": ");
        sLineReturn      = pCommandNodeType->attributeValueDynamic(pQE, "line-return",      NAMESPACE_NONE, "\r\n");
        
        if ((sValue && *sValue) || (sToken && *sToken)) {
          if (sHeader && *sHeader) {
            m_stHeader += sHeader;
            m_stHeader += sHeaderDelimiter;
          }
          
          if (sValue && *sValue) m_stHeader += sValue;
          
          if (sToken && *sToken) {
            m_stHeader += "[response:";
            m_stHeader += sToken;
            m_stHeader += ']';
          }
          if (sLineReturn && *sLineReturn) m_stHeader += sLineReturn;
        } //we quietly fail here assuming that the header should not be added. maybe a $variable that did not resolve
      } else throw InterfaceNotSupported(this, MM_STRDUP("const IXmlBaseNode *"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sHeaderDelimiter) MMO_FREE(sHeaderDelimiter);
    if (sHeader)          MMO_FREE(sHeader);
    if (sToken)           MMO_FREE(sToken);
    if (sValue)           MMO_FREE(sValue);
    if (sLineReturn)      MMO_FREE(sLineReturn);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  const char *Response::xslFunction_canHaveAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //usage:
    //  <xsl:copy-of select="@*[response:can-have-attribute()]"/>
    //  <xsl:if test="response:can-have-attribute('zeta:class')"><xsl:copy-of select="@zeta:class"/>
    static const char *sFunctionSignature = "response:can-have-attribute([attribute-name or @attribute-node, xpath])";
    assert(pQE);
    assert(pXCtxt);

    bool bCanHaveAttribute = false;
    IXslTransformContext *pTransformContext = 0;
    const char *sXPath         = 0,
               *sAttributeName = 0,
               *sNamespace     = 0;
    IXmlBaseNode *pInputNode   = 0,
                 *pOutputNode  = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      //optional
      switch (pXCtxt->valueCount()) {
        case 2: {
          //sXPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          throw XPathTooManyArguments(this, sFunctionSignature);
          NOT_COMPLETE("response:can-have-attribute 2 params");
        }
        case 1: {
          switch (pXCtxt->xtype()) {
            case XPATH_NODESET: {
              //e.g. <xsl:if test="response:can-have-attribute(@count)"><xsl:copy-of select="@count" />
              //returns NULL if the node set is empty
              pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
              if (pInputNode) {
                if (pInputNode->isNodeAttribute()) {
                  //source context attribute exists, check to see if the target has it
                  sAttributeName = pInputNode->localName();
                  sNamespace     = pInputNode->queryInterface((const IXmlNamespaced *) 0)->namespaceHREF(NO_DUPLICATE);
                } else {
                  //can happen when response:can-have-attribute(element-node)
                  IFDEBUG(Debug::warn("response:can-have-attribute(node) attribute required");)
                  //throw XPathFunctionWrongArgumentType(this, MM_STRDUP("attribute"), MM_STRDUP("node"), sFunctionSignature);
                }
              } else {
                //can happen when response:can-have-attribute(/blah/@not-exist)
                //allow return of false
                //because we cannot ascertain the name of the intended source attribute
                //and the output cannot have it if it does not exist
                IFDEBUG(Debug::warn("response:can-have-attribute(attribute) empty node-set");)
              }
              break;
            }
            case XPATH_STRING: {
              sAttributeName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
              break;
            }
            default: throw XPathFunctionWrongArgumentType(this, MM_STRDUP("string"), MM_STRDUP("other"), sFunctionSignature);
          }
          break;
        }
        case 0: {
          //response:can-have-attribute()
          pInputNode = pXCtxt->contextNode(pQE); //pQE as MM
          if (pInputNode) {
            if (pInputNode->isNodeAttribute()) {
              //source context attribute exists, check to see if the target has it
              sAttributeName = pInputNode->localName();
              sNamespace     = pInputNode->queryInterface((const IXmlNamespaced *) 0)->namespaceHREF(NO_DUPLICATE);
            } else {
              //can happen when element-node => response:can-have-attribute()
              //allow false return
              IFDEBUG(Debug::warn("response:can-have-attribute() not attribute");)
            }
          } else {
            //no context at all
            //allow false return
            IFDEBUG(Debug::warn("response:can-have-attribute() not context");)
          }
          break;
        }
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      if (sAttributeName) {
        if (pTransformContext = pQE->transformContext()) {
          //outputNode() will return zero in the case of no current node (XML_DOCUMENT_NODE)
          if (pOutputNode = pTransformContext->outputNode(pQE)) {  //pQE as MM
            bCanHaveAttribute =
                 pOutputNode->isNodeElement()
              && pOutputNode->isEmpty(pQE)    //node content has not started (prevents attributes)
              && (!sAttributeName || !pOutputNode->attributeExists(pQE, sAttributeName, sNamespace));
          }
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXPath)         MMO_FREE(sXPath);
    if (sAttributeName) MMO_FREE(sAttributeName);
    if (pInputNode)     delete pInputNode;
    if (pOutputNode)    delete pOutputNode;
    //if (pTransformContext) delete pTransformContext; //pointer in to the QE
    //if (sNamespace)     MMO_FREE(sNamespace); //NO_DUPLICATE

    UNWIND_EXCEPTION_THROW;
    
    return bCanHaveAttribute ? MM_STRDUP("true") : 0;
  }

  const char *Response::xslFunction_setPostTransform(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //this IXmlBaseNode and it's IXmlBaseDoc need to survive after this transform
    //however, the pCommandNode uses a temporary LibXmlDoc created in the LibXslModule XSL extension resolution procedure
    //which will disappear after this call is complete
    //THUS: we force an absolute reference to the main Database
    NOT_COMPLETE("xslFunction_setPostTransform");
    
    /*
    static const char *sCommandSignature = "response:set-post-transform [@stylesheet-node @stylesheet-URI @stylesheet-request"];
    const IXmlBaseNode *pCommandNodeType, *pInTempTransformStylesheetNode;
    const char *sStylesheetXPath;

    //required stylesheet parameter
    pCommandNodeType  = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    if (pInTempTransformStylesheetNode = pCommandNodeType->attributeValueNode(pQE, "stylesheet")) {
      sStylesheetXPath = pInTempTransformStylesheetNode->uniqueXPathToNode(pQE);
      m_pPostTransformStylesheetDatabaseNode = db()->getSingleNode(pQE, sStylesheetXPath);

      //checks
      if (!m_pPostTransformStylesheetDatabaseNode)                             throw XPathReturnedEmptyResultSet(this, MM_STRDUP("stylesheet"), sStylesheetXPath);
      if (!m_pPostTransformStylesheetDatabaseNode->isNamespace(NAMESPACE_XSL)) throw AttributeRequired(this, MM_STRDUP("stylesheet: XSL Node stylesheet"), sCommandSignature);

      //optional
      m_sPostTransformResultStartNodeXPath = pCommandNodeType->attributeValue(pQE, "result-start-node");
    } else throw AttributeRequired(this, MM_STRDUP("stylesheet: absolute xpath"), sCommandSignature);

    //free up
    if (sStylesheetXPath) MMO_FREE(sStylesheetXPath);
    if (pInTempTransformStylesheetNode) delete pInTempTransformStylesheetNode;
    */
    
    return 0;
  }

  IXmlBaseNode *Response::xslCommand_outputLiteral(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //response:remove-CDATA needs to be output directly as a literal element in the out stream
    //postProcessXmlDocument() will work on it
    //*((LibXmlBaseNode*) pOutputNode)->m_oNode
    //*((LibXslTransformContext*) pTransformContext)->m_ctxt
    const IXslTransformContext *pTransformContext = 0;
    const IXmlBaseNode         *pResponseXMLNode  = 0;
    IXmlBaseNode               *pResponseXMLCopy  = 0;
    
    if ((pTransformContext = pQE->transformContext())
      && (pResponseXMLNode = pTransformContext->literalCommandNode(this))
    ) {
      if (pResponseXMLCopy = pOutputNode->copyChild(pQE, pResponseXMLNode, SHALLOW_CLONE)) {
        if (pOutputNode) {
          pTransformContext->continueTransformation(pResponseXMLCopy);
        } else throw NoOutputDocument(this);
      } else throw TransformWithoutCurrentNode(this);
    } else throw TransformContextRequired(this, MM_STRDUP("output literal"));
    
    //free up
    if (pResponseXMLNode) delete pResponseXMLNode;
    if (pResponseXMLCopy) delete pResponseXMLCopy;
    
    return 0;
  }
  
  const char *Response::sessionUUID() const {
    return m_pRequest->sessionUUID();
  }
  
  const char *Response::process(const Request *pRequest, const IXmlQueryEnvironment *pQE, const MessageInterpretation *pMI, IXmlBaseDoc *pAnalysedDataDoc, const bool bShouldServerSideXSLT) const {
    //already in our own thread
    IXmlBaseDoc  *pResultDoc         = 0;
    IXmlBaseNode *pDataRootNode      = 0;
    IXslDoc      *pStylesheet        = 0;
    Repository   *pStylesheetIL      = 0;
    char         *sML                = 0;
    const char   *sHeader            = 0,
                 *sDocumentStart     = 0,
                 *sFullResponse      = 0,
                 *sXSL               = 0,
                 *sStylesheetPILink  = 0;
    const BrowserQueryEnvironment bqe_output(this, pAnalysedDataDoc);
    
    //---------------------------------------------------- namespace management
    //get the Response to do it because Response knows about appropriate roots (not removed)
    if (pMI->needsMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots()) {
      pAnalysedDataDoc->moveAllPrefixedNamespaceDefinitionsToAppropriateRoots();
      //RESOURCE HUNGRY: IFDEBUG(pAnalysedDataDoc->validityCheck("check output doc after post-processing");)
    }
    
    if (pMI->needsAddAllStandardNamespaceDefinitionsToAppropriateRoots()) {
      pAnalysedDataDoc->addAllStandardNamespaceDefinitionsToAppropriateRoots();
      //RESOURCE HUNGRY: IFDEBUG(pAnalysedDataDoc->validityCheck("check output doc after post-processing");)
    }
    
    //---------------------------------------------------- last PHP debug 
    //write to the resources PHP server
    //all necessary info for a PHP transform of these files
    //for DEBUG
    IFDEBUG(
      if (sStylesheetPILink = pAnalysedDataDoc->stylesheetPILink(&bqe_output, true)) {
        if (!strstr(sStylesheetPILink, "://")) Debug::report("xml-stylesheet [%s] has no protocol (://), Repository may not understand", MM_STRDUP(sStylesheetPILink), rtWarning);
        
        pStylesheetIL  = Repository::factory_reader(&bqe_output, xmlLibrary(), 
          sStylesheetPILink, NO_PARENT_REPOSITORY, sessionUUID()
        );
        if (sXSL = pStylesheetIL->readRaw()) {
          //does not include HTTP headers
          debugWriteLastFiles(pAnalysedDataDoc, sXSL);
          MMO_FREE(sXSL); sXSL = NULL;
        } else Debug::warn("writing /last/*: failed to read stylesheet [%s]", sStylesheetPILink);
        MMO_FREE(sStylesheetPILink); sStylesheetPILink = NULL;
        MMO_DELETE(pStylesheetIL);   pStylesheetIL     = NULL;
      }
    )
    
    //---------------------------------------------------- server side XSLT return HTML
    //GDB: dynamic_cast<const LibXmlBaseDoc*>(pAnalysedDataDoc)->m_oDoc
    if (bShouldServerSideXSLT) {
      //TODO: consider postProcessXmlString()
      //TODO: consider the HTTP headers in the first pDataDoc: currently rootNode(,node_element_only) avoids them
      //TODO: PERFORMANCE: consider creating an object:Request/@name=server-side-stylesheet, [@MI-name=GET_Request_xsl]
      //  to avoid stylesheet parsing overheads
      //  response:set-post-transform(object:Request [, object:MessageInterpretation])
      //  response:set-post-transform(URI)
      if (sStylesheetPILink = pAnalysedDataDoc->stylesheetPILink(&bqe_output, true)) {
        Debug::report("Server-side XSLT with [%s]", MM_STRDUP(sStylesheetPILink));
        if (!strstr(sStylesheetPILink, "://")) Debug::report("xml-stylesheet [%s] has no protocol (://), Repository may not understand", MM_STRDUP(sStylesheetPILink), rtWarning);
        
        // xsl:stylesheet/@server-side-xslt=true append to the stylesheet request and thus the xsl:stylesheet
        //object:Response/@server-side-xslt=true to the markup the input doc so the transform can see
        //it is for HTTP + HTML, not just HTML
        if (pDataRootNode = pAnalysedDataDoc->rootNode(&bqe_output)) 
          pDataRootNode->setAttribute(&bqe_output, "server-side-xslt", "true");
        else Debug::report("Server-side XSLT failed to find root data node", rtWarning);
        
        //SECURITY: should this factory_reader() be allowed to return any Repository
        //  including access the local filesystem?
        pStylesheetIL  = Repository::factory_reader(&bqe_output, xmlLibrary(), 
          sStylesheetPILink, NO_PARENT_REPOSITORY, sessionUUID()
        );
        if (sXSL = pStylesheetIL->readRaw()) { //does not include HTTP headers
          pStylesheet = xmlLibrary()->factory_stylesheet(this, "server-side stylesheet");
          pStylesheet->loadXml(sXSL);
          
          pResultDoc = pAnalysedDataDoc->transform(
            this, pStylesheet, &bqe_output,
            NO_PARAMS_INT, NO_PARAMS_CHAR, NO_PARAMS_NODE
          );
        } else throw TransformFailed(this, MM_STRDUP("Could not load server-side stylesheet"), pAnalysedDataDoc->rootNode(&bqe_output), MM_STRDUP(sStylesheetPILink));
      } else {
        //The base XSLT has already produced its final output?
        //i.e. HTML
        //SECURITY: should we return the raw XML here in all cases? 
        Debug::report("Server-side XSLT required but no <?xml-stylesheet @href discovered in XML doc", rtWarning);
        pResultDoc = pAnalysedDataDoc;
      }
    } else pResultDoc = pAnalysedDataDoc;

    //---------------------------------------------------- document post-processing
    const XmlAdminQueryEnvironment ibqe_output(this, pResultDoc);
    //this will output HTML if the xsl:output @method=html to produce an XML_HTML_Document
    if (sML = (char*) pResultDoc->xml(&ibqe_output, pMI->noOutputEscaping())) {
      //sML might move (realloc) in the case of EXPANSIVE changes
      //most should be reductive
      //e.g. [response:token-content-length]
      //TODO: make MI configurable
      /* old all-in-one-doc system:
       * sML = trimWhiteSpace(sML);                          //affects document length calculations
       * sML = insertDocumentStart(sML);                     //allows things to be moved to the document start
       * sML = moveDocumentInstructionsToDocumentStart(sML); //affects document length
       * sML = processResponseStructureTags(sML);            //layout elements
       * sML = processResponseTokens(sML);                   //completes document length
       */
      //response:set-headers() system
      sML            = processResponseStructureTags(sML);  //layout elements, e.g. respone:remove
      sML            = trimWhiteSpace(sML);                //affects document length calculations
      sHeader        = processResponseTokens(MMO_STRDUP(m_stHeader.c_str()), sML); //completes document length
      sDocumentStart = m_pRequest->mi()->responseDocumentStart();
      sFullResponse  = prependHeader(sHeader, sDocumentStart, sML);      
    }
  
    //---------------------------------------------------- free up
    if (pDataRootNode)                  delete pDataRootNode;
    if (pResultDoc != pAnalysedDataDoc) delete pResultDoc;
    if (pStylesheet)                    delete pStylesheet;
    if (pStylesheetIL)                  delete pStylesheetIL;
    if (sXSL)                           MMO_FREE(sXSL);
    if (sStylesheetPILink)              MMO_FREE(sStylesheetPILink);
    if (sHeader && sHeader != m_stHeader.c_str()) MMO_FREE(sHeader);
    if (sML)                            MMO_FREE(sML);
    //if (sDocumentStart)                 MMO_FREE(sDocumentStart); //pointer in to the MI

    return sFullResponse;
  }
  
  char *Response::prependHeader(const char *sHeader, const char *sDocumentStartString, const char *sML) const {
    //caller frees result
    size_t iLenHeader = (sHeader ? strlen(sHeader) : 0);
    size_t iLenDSS    = (sDocumentStartString ? strlen(sDocumentStartString) : 0);
    size_t iLenML     = (sML ? strlen(sML) : 0);
    char *sFullResponse = MMO_MALLOC(iLenHeader + iLenDSS + iLenML + 1);
    
    //assemble
    memcpy(sFullResponse,                        sHeader,              iLenHeader);
    memcpy(sFullResponse + iLenHeader,           sDocumentStartString, iLenDSS);
    memcpy(sFullResponse + iLenHeader + iLenDSS, sML,                  iLenML);
    sFullResponse[iLenHeader + iLenDSS + iLenML] = '\0';
    
    return sFullResponse;
  }

  char *Response::trimWhiteSpace(char *sML) const {
    //will not remove last trailing space if there is only trailing space
    //i.e. \n will return \n
    char *sPos;
    size_t iLen;
    
    if (sML && *sML) { //length more than 0
      iLen = strlen(sML);
      
      //----------------------- leading
      sPos = sML;
      while (*sPos && (*sPos <= ' ')) 
          sPos++; 
      if (sPos != sML) {
        memmove(sML, sPos, iLen - (sPos - sML) + 1);
        Debug::warn("leading whitespace");
      }
      
      //----------------------- trailing
      sPos = sML + strlen(sML); //0 string end marker
      while (--sPos > sML && *sPos <= ' ') *sPos = 0;
    }
    
    return sML;
  }
  
  char *Response::moveDocumentInstructionsToDocumentStart(char *sML) const {
    //REDUCTIVE (no change in length)
    //<!DOCTYPE, <?xml, <?xml-stylesheet => document start
    //in the case of HTTP this can be used for moving the PIs to after the HTTP headers: \r\n\r\n
    size_t iTagsLen, iHeadersLen, iDocStartLen;
    char       *sTagEnd = 0, 
               *sLastTagEnd, 
               *sDocumentStart,
               *sBuff   = 0;
    const char *sDocumentStartString;
    
    if (*sML == '<') {
      if (sDocumentStartString = m_pRequest->mi()->responseDocumentStart()) {
        if (sDocumentStart = strstr(sML, sDocumentStartString)) {
          sTagEnd        = sML;
          do {
            sLastTagEnd = sTagEnd;
            sTagEnd     = strchr(sTagEnd + 1, '>');
          } while (sTagEnd && sTagEnd < sDocumentStart);
          
          if (sLastTagEnd != sML) {
            sLastTagEnd++;
            while (*sLastTagEnd && *sLastTagEnd < 33) sLastTagEnd++;
            iTagsLen     = sLastTagEnd - sML;
            iHeadersLen  = sDocumentStart - sLastTagEnd;
            iDocStartLen = strlen(sDocumentStartString);
            sBuff        = MMO_MALLOC(iTagsLen);
            memcpy( sBuff, sML, iTagsLen); //<statements> in to the buffer
            memmove(sML, sLastTagEnd, sDocumentStart - sLastTagEnd + iDocStartLen); //headers to start
            memcpy( sML + iHeadersLen + iDocStartLen, sBuff, iTagsLen); //<statements> before doc start
          }
        } else Debug::report("document start not found", IReportable::rtWarning);
      } else Debug::report("MI has no document start string", IReportable::rtWarning);
    }

    //free up
    if (sBuff) MMO_FREE(sBuff);
    //if (sDocumentLengthStart) MMO_FREE(sDocumentLengthStart); //pointer in to xml

    return sML;
  }
  
  char *Response::insertDocumentStart(char *sML) const {
    //caller frees sML, even if it is realloc'ated here by EXPANSIVE actions
    //using memmove(...) instead of strncpy(...) because destination is part of source
    size_t iReplacementLen;
    char *sPosOld;
    const char *sTagEnd;
    bool bFound;
    UNUSED(bFound);

    //-------------------------------------------------------------------- response:include-all
    //nothing gets translated beyond the start of this tag
    //used in the API to ensure exact results come out
    //TODO: response tags after this include-all??
    static const char *sSERVER_RESPONSE_INCLUDEALL_ATTR = " response:include-all=\"on\"";
    const char *sPosIncludeAllStart = strstr(sML, sSERVER_RESPONSE_INCLUDEALL_ATTR);
    
    //-------------------------------------------------------------------- [response:headers-end]
    static const char *sTOKEN_DOCUMENT_START = "[response:headers-end]";
    static const size_t uDS_LEN = strlen(sTOKEN_DOCUMENT_START);
    const char *sDocumentStartString = 0;

    sPosOld = sML;
    bFound  = false;
    if (sDocumentStartString = m_pRequest->mi()->responseDocumentStart()) {
      iReplacementLen = strlen(sDocumentStartString);
      if (iReplacementLen && iReplacementLen <= uDS_LEN) {
        while (sPosOld 
          && (sPosOld = strstr(sPosOld, sTOKEN_DOCUMENT_START)) 
          && (!sPosIncludeAllStart || sPosOld < sPosIncludeAllStart)
        ) {
          //we replace the whole thing with tokens
          sTagEnd = sPosOld + uDS_LEN;
          bFound  = true;
            
          //token replacement
          strcpy(sPosOld, sDocumentStartString);
          sPosOld += iReplacementLen;
          
          //remove whole tag
          memmove(sPosOld, sTagEnd, strlen(sTagEnd) + 1);
        }
      } else Debug::report("reductive token replacement overflow", IReportable::rtWarning);
    } else Debug::report("MI has no document start string", IReportable::rtWarning);
    //if (!bFound) Debug::report("[response:headers-end] token not found", IReportable::rtWarning);
    //free up
    //if (sDocumentStartString) delete sDocumentStartString; //pointer in to MI

    return sML;
  }
  
  char *Response::processResponseStructureTags(char *sML) const {
    //caller frees sML, even if it is realloc'ated here by EXPANSIVE actions
    //using memmove(...) instead of strncpy(...) because destination is part of source
    size_t iReplacementLen;
    char *sPosOld;
    const char *sTagEnd;
    bool bFound;
    UNUSED(bFound);

    //-------------------------------------------------------------------- response:include-all
    //nothing gets translated beyond the start of this tag
    //used in the API to ensure exact results come out
    //TODO: response tags after this include-all??
    static const char *sSERVER_RESPONSE_INCLUDEALL_ATTR = " response:include-all=\"on\"";
    const char *sPosIncludeAllStart = strstr(sML, sSERVER_RESPONSE_INCLUDEALL_ATTR);
    
    //-------------------------------------------------------------------- response:remove
    //to allow returning of an XML doucment with no singular root node
    //allows multiple root nodes including text nodes in the HTTP headers
    //NOTE: REDUCTIVE
    static const char *sSERVER_REMOVE_TAG = "<response:remove";
    static const size_t uSRT_LEN = strlen(sSERVER_REMOVE_TAG);
    static const char *sSERVER_REMOVE_ENDTAG = "</response:remove>";
    static const size_t uSRET_LEN = strlen(sSERVER_REMOVE_ENDTAG);
    bool bSelfClosing;
    char *sPosEnd, *sPosNextEnd;

    sPosOld = sML;
    while (
      sPosOld
      && (sPosOld = strstr(sPosOld, sSERVER_REMOVE_TAG))
      && (!sPosIncludeAllStart || sPosOld < sPosIncludeAllStart)
      && (sPosOld[uSRT_LEN] == '>' || sPosOld[uSRT_LEN] == ' ')
      && (sTagEnd = strchr(sPosOld + uSRT_LEN, '>'))
    ) {
      bSelfClosing = (sTagEnd[-1] == '/');

      //remove beginning
      memmove(sPosOld, sTagEnd + 1, strlen(sTagEnd + 1) + 1);

      if (!bSelfClosing) {
        //find the LAST matching closing tag
        sPosEnd     = sPosOld;
        sPosNextEnd = 0;
        while (sPosEnd = strstr(sPosEnd, sSERVER_REMOVE_ENDTAG)) sPosNextEnd = sPosEnd++;
        sPosEnd = sPosNextEnd; //last valid find
        if (!sPosEnd) throw ResponseTagMalformed(this, MM_STRDUP("response:remove no end tag: maybe it is self closing (not allowed)?"));

        //remove end
        memmove(sPosEnd, sPosEnd + uSRET_LEN, strlen(sPosEnd + uSRET_LEN) + 1);
      }
    }
    
    return sML;
  }
  
  char *Response::processResponseTokens(char *sHeader, const char *sBody) const {
    //caller frees result IF different
    char *sPosOld, *sTagEnd;
    
    //-------------------------------------------------------------------- [response:token-content-length]
    //LAST: because it uses the content-length of an invalid XML document!
    //and needs to work on a now invalid document with top-level root node removed
    //embed the final content-length in to the placeholder position
    //NOTE: REDUCTIVE
    static const char *sTOKEN_CONTENT_LENGTH = "[response:token-content-length]";
    static const size_t uTCL_LEN = strlen(sTOKEN_CONTENT_LENGTH);

    assert(sBody);
    
    const char *sDocumentLength = itoa(strlen(sBody));
    size_t iReplacementLen      = strlen(sDocumentLength);

    if (iReplacementLen && iReplacementLen <= uTCL_LEN) {
      sPosOld = sHeader;
      while (sPosOld && (sPosOld = strstr(sPosOld, sTOKEN_CONTENT_LENGTH))) {
        //we replace the whole thing with tokens
        sTagEnd = sPosOld + uTCL_LEN;
        
        //token replacement
        strcpy(sPosOld, sDocumentLength);
        sPosOld += iReplacementLen;
      
        //remove whole tag
        memmove(sPosOld, sTagEnd, strlen(sTagEnd) + 1);
      }
    } else Debug::report("reductive token replacement overflow", IReportable::rtWarning);
    
    //free up
    if (sDocumentLength) MMO_FREE(sDocumentLength);
    
    return sHeader;
  }
  
  char *Response::processResponseTokens(char *sML) const {
    //caller frees result IF not the same as input
    size_t iReplacementLen;
    char *sPosOld;
    const char *sTagEnd;
    bool bFound;
    UNUSED(bFound);

    //-------------------------------------------------------------------- response:include-all
    //nothing gets translated beyond the start of this tag
    //used in the API to ensure exact results come out
    //TODO: response tags after this include-all??
    static const char *sSERVER_RESPONSE_INCLUDEALL_ATTR = " response:include-all=\"on\"";
    const char *sPosIncludeAllStart = strstr(sML, sSERVER_RESPONSE_INCLUDEALL_ATTR);

    //-------------------------------------------------------------------- [response:token-content-length]
    //LAST: because it uses the content-length of an invalid XML document!
    //and needs to work on a now invalid document with top-level root node removed
    //embed the final content-length in to the placeholder position
    //NOTE: REDUCTIVE
    static const char *sTOKEN_CONTENT_LENGTH = "[response:token-content-length]";
    static const size_t uTCL_LEN = strlen(sTOKEN_CONTENT_LENGTH);
    size_t iDocumentLength;
    const char *sDocumentLength      = 0, 
               *sDocumentLengthStart = 0;

    sPosOld = sML;
    bFound  = false;
    if (sDocumentLengthStart = m_pRequest->mi()->responseDocumentStart(sML)) {
      //length calcs
      iDocumentLength = strlen(sDocumentLengthStart);
      sDocumentLength = itoa(iDocumentLength);
      iReplacementLen = strlen(sDocumentLength);
      if (iReplacementLen && iReplacementLen <= uTCL_LEN) {
        while (sPosOld 
          && (sPosOld = strstr(sPosOld, sTOKEN_CONTENT_LENGTH))
          && (!sPosIncludeAllStart || sPosOld < sPosIncludeAllStart)
        ) {
          //we replace the whole thing with tokens
          sTagEnd = sPosOld + uTCL_LEN;
          bFound  = true;
          
          //token replacement
          strcpy(sPosOld, sDocumentLength);
          sPosOld += iReplacementLen;
        
          //remove whole tag
          memmove(sPosOld, sTagEnd, strlen(sTagEnd) + 1);
        }
      } else Debug::report("reductive token replacement overflow", IReportable::rtWarning);
    } else Debug::report("unable to find [response:token-content-length]", IReportable::rtWarning);
    //if (!bFound) Debug::report("[response:token-content-length] token not found", IReportable::rtWarning);
    //free up
    if (sDocumentLength) MMO_FREE(sDocumentLength);
    //if (sDocumentLengthStart) MMO_FREE(sDocumentLengthStart); //pointer in to xml

    return sML;
  }

  void Response::debugWriteLastFiles(const IXmlBaseDoc *pXMLDoc, const char *sXSL) const { 
    XmlAdminQueryEnvironment ibqe_debug_output(this, pXMLDoc);
    struct stat info;
    ofstream transformfile;
    const char *sXML; 
    const char *sAfterHeaders;
    
    if (stat(LAST_DIR, &info) == 0) {
      sAfterHeaders = strstr(sXSL, "<");
      transformfile.open(LAST_XSL);
      transformfile << sAfterHeaders;
      transformfile.close();

      sXML = pXMLDoc->xml(&ibqe_debug_output);
      sXML = trimWhiteSpace((char*) sXML);
      sAfterHeaders = strstr(sXML, "?>");
      if (sAfterHeaders) sAfterHeaders = strstr(sAfterHeaders + 2, "?>");
      if (sAfterHeaders) sAfterHeaders += 2;
      else sAfterHeaders = sXML;
      while (*sAfterHeaders && *sAfterHeaders <= ' ') sAfterHeaders++;
      transformfile.open(LAST_XML);
      transformfile << sAfterHeaders;
      transformfile.close();
      free((void*) sXML);
      Debug::report("PHP transform => %s", MM_STRDUP(LAST_PHP));
    }
  }
          
  void Response::compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) {
    
  }
  
  const char *Response::toString() const {
    return MM_STRDUP("Response");
  }
}
