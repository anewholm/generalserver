//platform agnostic file
#include "MessageInterpretation.h"
#include "define.h"
#include "RegularX.h"
#include "Database.h"
#include "Service.h"
#include "DatabaseNode.h"

#include <string>

namespace general_server {
  MessageInterpretation::MessageInterpretation(DatabaseNode *pNode, const Service *pService):
    MemoryLifetimeOwner(pService, pNode),
    GeneralServerDatabaseNodeServerObject(pNode),
    m_pService(pService),
    m_sSummary(0),
    m_sSentenceEnd(0),
    m_sHeadersEnd(0),
    m_sAfter(0),
    m_bMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots(true),
    m_bAddAllStandardNamespaceDefinitionsToAppropriateRoots(true),
    
    m_poPrimaryOutputTransformation(0),
    m_poAnalyserOutputTransformation(0),
    m_bNoOutputEscaping(false),

    m_rxAcceptedFormats(0),
    m_bHasSentenceEnd(0),
    m_rxSentenceEnd(0),
    m_bHasREST_session(0),
    m_rxREST_session(0),
    m_bHasREST_configurationFlags(0),
    m_rxREST_configurationFlags(0),
    m_bHasREST_clientSoftwareIdentifier(0),
    m_rxREST_clientSoftwareIdentifier(0),
    m_bHasREST_resourcesAreaIdentifier(0),
    m_rxREST_resourcesAreaIdentifier(0)
  {
    static const char *sElementSignature = "object:MessageInterpretation @name [@priority]";
    const DatabaseNode *pCheckNode         = 0;       //deleted when used (several times)
    const char *sCloseConnection           = 0,       //used only for boolean calcs
               *srxAcceptedFormats         = 0,       //regex string, not stored
               *srxREST_session            = 0,       //
               *srxREST_configurationFlags = 0,       //
               *srxREST_clientSoftwareIdentifier = 0, //
               *srxREST_resourcesAreaIdentifier  = 0; //
    const char *sMethod                    = 0,
               *sType                      = 0,
               *sFormat                    = 0;       //stored as enums
    XmlNodeList<const DatabaseNode> *pvPaths = 0;
    XmlNodeList<const DatabaseNode>::const_iterator iPath;

    UNWIND_EXCEPTION_BEGIN {
      //--------------------- MessageInterpretation config nodes and values
      m_sMIName   = m_pNode->attributeValue(m_pIBQE_serverStartup, "name");
      m_iPriority = m_pNode->attributeValueInt(m_pIBQE_serverStartup, "priority");

      if (m_sMIName) {
        if (pCheckNode = m_pNode->getSingleNode(m_pIBQE_serverStartup, "gs:accepted-format")) {
          //<accepted-format ...>
          srxAcceptedFormats = pCheckNode->value(m_pIBQE_serverStartup);
          m_sSummary         = pCheckNode->attributeValue(m_pIBQE_serverStartup, "summary");
          delete pCheckNode;
        }

        if (pCheckNode = m_pNode->getSingleNode(m_pIBQE_serverStartup, "gs:end")) {
          //<gs:end ...>
          sType                 = pCheckNode->attributeValue(m_pIBQE_serverStartup, "type");            //string (default) or stream_regex
          sFormat               = pCheckNode->attributeValue(m_pIBQE_serverStartup, "format");          //length or match (default)
          srxREST_session       = pCheckNode->attributeValue(m_pIBQE_serverStartup, "rest-session");
          srxREST_configurationFlags       = pCheckNode->attributeValue(m_pIBQE_serverStartup, "rest-configuration-flags");
          srxREST_clientSoftwareIdentifier = pCheckNode->attributeValue(m_pIBQE_serverStartup, "client-software-identifier");
          srxREST_resourcesAreaIdentifier  = pCheckNode->attributeValue(m_pIBQE_serverStartup, "resources-area-identifier");
          sCloseConnection      = pCheckNode->attributeValue(m_pIBQE_serverStartup, "close-connection"); //after message (default: no)
          m_sSentenceEnd        = pCheckNode->value(m_pIBQE_serverStartup);
          m_sHeadersEnd         = pCheckNode->attributeValue(m_pIBQE_serverStartup, "headers-end");
          m_sAfter              = pCheckNode->attributeValue(m_pIBQE_serverStartup, "after");
          //attributes with -normalise in will get auto-normalised by LibXmlBaseNode
          if (!m_sAfter)         m_sAfter         = pCheckNode->attributeValue(m_pIBQE_serverStartup, "after-normalise"); 
          if (!m_sHeadersEnd) {
            m_sHeadersEnd = pCheckNode->attributeValue(m_pIBQE_serverStartup, "headers-end-normalise"); 
            if (!m_sHeadersEnd && m_sSentenceEnd) m_sHeadersEnd = m_sSentenceEnd;
          }
          m_bMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots = pCheckNode->attributeValueBoolDynamicString(m_pIBQE_serverStartup, "move-all-prefixed-namespace-definitions-to-appropriate-roots", NAMESPACE_NONE, true);
          m_bAddAllStandardNamespaceDefinitionsToAppropriateRoots  = pCheckNode->attributeValueBoolDynamicString(m_pIBQE_serverStartup, "add-all-standard-namespace-definitions-to-appropriate-roots",  NAMESPACE_NONE, true);
          delete pCheckNode;
        }
        IFDEBUG(if (!srxREST_configurationFlags) srxREST_configurationFlags = MM_STRDUP("[\?&amp;]configuration-flags=([^&amp; ]+)"));

        //--------------------- inputs processing
        //simple
        m_iType              = MessageInterpretation::sentenceEndType(sType);
        m_iFormat            = MessageInterpretation::sentenceEndFormat(sFormat);
        m_bCloseConnection   = xmlLibrary()->textEqualsTrue(sCloseConnection);

        //reg exs
        m_bHasAcceptedFormat = (srxAcceptedFormats && *srxAcceptedFormats);
        m_rxAcceptedFormats  = new rpattern_c((srxAcceptedFormats ? srxAcceptedFormats : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);
        m_bHasSentenceEnd    = (m_sSentenceEnd && *m_sSentenceEnd);
        m_rxSentenceEnd      = new rpattern_c((m_sSentenceEnd ? m_sSentenceEnd : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);
        m_bHasREST_session   = (srxREST_session && *srxREST_session);
        m_rxREST_session     = new rpattern_c((srxREST_session ? srxREST_session : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);
        m_bHasREST_configurationFlags       = (srxREST_configurationFlags && *srxREST_configurationFlags);
        m_rxREST_configurationFlags         = new rpattern_c((srxREST_configurationFlags ? srxREST_configurationFlags : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);
        m_bHasREST_clientSoftwareIdentifier = (srxREST_clientSoftwareIdentifier && *srxREST_clientSoftwareIdentifier);
        m_rxREST_clientSoftwareIdentifier   = new rpattern_c((srxREST_clientSoftwareIdentifier ? srxREST_clientSoftwareIdentifier : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);
        m_bHasREST_resourcesAreaIdentifier  = (srxREST_resourcesAreaIdentifier && *srxREST_resourcesAreaIdentifier);
        m_rxREST_resourcesAreaIdentifier    = new rpattern_c((srxREST_resourcesAreaIdentifier ? srxREST_resourcesAreaIdentifier : ""), MULTILINE | NORMALIZE, MODE_DEFAULT);

        //nodes
        m_poPrimaryOutputTransformation  = m_pNode->getSingleNode(m_pIBQE_serverStartup, "name::controller");
        m_poAnalyserOutputTransformation = m_pNode->getSingleNode(m_pIBQE_serverStartup, "name::analyser");
        m_poMessageTransformation        = m_pNode->getSingleNode(m_pIBQE_serverStartup, "object:Request");
        m_poMessageValidityTest          = m_pNode->getSingleNode(m_pIBQE_serverStartup, "xsd:element");
        m_poDefaultErrorHandlers         = m_pNode->getSingleNode(m_pIBQE_serverStartup, "repository:error_handlers");

        //@disable-output-escaping xsl:text
        if (m_poPrimaryOutputTransformation) {
          if (pCheckNode = m_poPrimaryOutputTransformation->getSingleNode(m_pIBQE_serverStartup, "xsl:output")) {
            if (sMethod = pCheckNode->attributeValue(m_pIBQE_serverStartup, "method")) {
              m_bNoOutputEscaping = _STREQUAL(sMethod, "text");
            }
            delete pCheckNode;
          }
        }
        
        //paths
        //e.g. <gs_website_root>/object:Server/repository:websites/object:Website[request:resource-match()]</gs_website_root>
        //evaluated at point of request
        if (pCheckNode = m_pNode->getSingleNode(m_pIBQE_serverStartup, "repository:paths")) {
          if (pvPaths = pCheckNode->children(m_pIBQE_serverStartup)) {
            for (iPath = pvPaths->begin(); iPath != pvPaths->end(); iPath++) {
              m_mPaths.insert((*iPath)->localName(), (*iPath)->value(m_pIBQE_serverStartup));
            }
            delete pvPaths->element_destroy();
          }
          delete pCheckNode;
        }
      } else throw AttributeRequired(this, MM_STRDUP("@name"), sElementSignature);

      //--------------------- requirements
      if (!m_poPrimaryOutputTransformation) throw MIPrimaryOutputTransformationRequired(this, m_sMIName);
      if (!m_poMessageTransformation)       throw MIMessageTransformationRequired(this, m_sMIName);
    } UNWIND_EXCEPTION_END;

    //free up temp vars
    if (sType)              MMO_FREE(sType);
    if (sFormat)            MMO_FREE(sFormat);
    if (sCloseConnection)   MMO_FREE(sCloseConnection);
    if (srxAcceptedFormats) MMO_FREE(srxAcceptedFormats);
    if (srxREST_session)    MMO_FREE(srxREST_session);
    if (srxREST_configurationFlags)       MMO_FREE(srxREST_configurationFlags);
    if (srxREST_clientSoftwareIdentifier) MMO_FREE(srxREST_clientSoftwareIdentifier);
    if (srxREST_resourcesAreaIdentifier)  MMO_FREE(srxREST_resourcesAreaIdentifier);
    if (sMethod)            MMO_FREE(sMethod);

    UNWIND_EXCEPTION_THROW;
  }

  MessageInterpretation::~MessageInterpretation() {
    if (m_rxAcceptedFormats)             delete m_rxAcceptedFormats;
    if (m_rxSentenceEnd)                 delete m_rxSentenceEnd;
    if (m_rxREST_session)                delete m_rxREST_session;
    if (m_rxREST_configurationFlags)     delete m_rxREST_configurationFlags;
    if (m_rxREST_clientSoftwareIdentifier) delete m_rxREST_clientSoftwareIdentifier;
    if (m_rxREST_resourcesAreaIdentifier)  delete m_rxREST_resourcesAreaIdentifier;
    if (m_sMIName)                       MMO_FREE(m_sMIName);
    if (m_sSentenceEnd)                  MMO_FREE(m_sSentenceEnd);
    if (m_sHeadersEnd && m_sHeadersEnd != m_sSentenceEnd) MMO_FREE(m_sHeadersEnd);
    if (m_sAfter)                        MMO_FREE(m_sAfter);
    if (m_poPrimaryOutputTransformation) delete m_poPrimaryOutputTransformation;
    if (m_poAnalyserOutputTransformation) delete m_poAnalyserOutputTransformation;
    if (m_poMessageTransformation)       delete m_poMessageTransformation;
    if (m_poMessageValidityTest)         delete m_poMessageValidityTest;
    if (m_poDefaultErrorHandlers)        delete m_poDefaultErrorHandlers;
    if (m_sSummary)                      MMO_FREE(m_sSummary);
    m_mPaths.elements_free();
  }

  const char *MessageInterpretation::toString() const {
    stringstream sOut;
    sOut << "["
         << (m_bCloseConnection ? "c" : " ")
         << (m_bHasREST_session ? "s" : " ")
         << (m_bHasREST_configurationFlags ? "f" : " ")
         << (m_bHasREST_clientSoftwareIdentifier ? "ua" : "  ")
         << (m_bMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots ? "^" : " ")
         << (m_poPrimaryOutputTransformation ? "x" : "!")
         << (m_poAnalyserOutputTransformation ? "2" : "!")
         << (m_poDefaultErrorHandlers ? "e" : " ")
         << m_mPaths.size()
         << " "
         << (m_iType == text ? "text" : "regx") << " "
         << (m_iFormat == length ? "len" : "mat")
         << "] "
         << m_sMIName
         << " (" << m_iPriority << ") ";
    if (m_sSummary) sOut << " (" << m_sSummary << ")";
    return MM_STRDUP(sOut.str().c_str());
  }

  const StringMap<const char*> *MessageInterpretation::paths() const {return &m_mPaths;}

  MessageInterpretation::sentence_end_type MessageInterpretation::sentenceEndType(const char *sString) {
    if (sString && !strncmp(sString, "stream-regex", 12)) return stream_regex;
    return text;
  }
  MessageInterpretation::sentence_end_format MessageInterpretation::sentenceEndFormat(const char *sString) {
    if (sString && !strncmp(sString, "match", 5)) return match;
    return length;
  }

  bool MessageInterpretation::gimme(const char *sTextStream) const {
    size_t iCount = m_rxAcceptedFormats->count(sTextStream);
    return m_bHasAcceptedFormat ? iCount != 0 : false;
  }
  bool MessageInterpretation::needsMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots() const {
    return m_bMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots;
  }
  
  bool MessageInterpretation::needsAddAllStandardNamespaceDefinitionsToAppropriateRoots() const {
    return m_bAddAllStandardNamespaceDefinitionsToAppropriateRoots;
  }

  const char *MessageInterpretation::responseDocumentStart(const char *sXML) const {
    //do not free result!
    //without a parameter responseDocumentStart() returns the document start string
    //with a parameter responseDocumentStart(sXML) it finds the document start in the sXML
    //TODO: regex matching on start
    const char *sRet;
    
    if (sXML) {
      if (m_sHeadersEnd) {
        sRet = strstr(sXML, m_sHeadersEnd);
        if (sRet) sRet += strlen(m_sHeadersEnd);
      } else sRet = sXML;
    } else 
      sRet = m_sHeadersEnd;
    
    return sRet;
  }
  
  bool MessageInterpretation::isSenteceEnd(const char *sTextStream) const {
    bool bEnd = false;
    size_t iBufferLen = strlen(sTextStream);
    size_t iSentenceEndLength;

    switch (m_iType) {
      case text: { 
        //------------------------- end-of-buffer string to match
        if (!m_sSentenceEnd) bEnd = true; //if we have no string to match then the conversation is over!
        else {
          iSentenceEndLength = strlen(m_sSentenceEnd); //cached
          if (iBufferLen >= iSentenceEndLength && !strcmp(sTextStream + iBufferLen - iSentenceEndLength, m_sSentenceEnd)) bEnd = true;
        }
        break;
      }

      case stream_regex: {
        //------------------------- regex match on current stream to understand when it ends
        if (!m_sSentenceEnd) bEnd = true; //if we have no string to match then the conversation is over!
        else {
          //match our regex
          const char *sMatch = 0;
          match_results_c mResults;
          rpattern_c::backref_type br = m_rxSentenceEnd->match( sTextStream, mResults );
          if (br.matched && (int) mResults.cbackrefs() == 2) {
            //get string
            if (br = mResults.backref(1)) {
              if (br.first && br.second) {
                if (sMatch = RegularX::copyString(br.first, br.second)) {
                  //interpret results
                  switch (m_iFormat) {
                    case match: {
                      //simple if exists
                      if (sMatch && *sMatch) bEnd = true;
                      break;
                    }
                    case length: {
                      //not simple: match = length
                      size_t iBufferLengthSizeEnd = atoi(sMatch);
                      const char *sFrom;
                      if (m_sAfter) {
                        //ok we need to calculate this size after a certain string has occurred
                        if (sFrom = strstr(sTextStream, m_sAfter)) 
                          iBufferLengthSizeEnd += (sFrom - sTextStream) + strlen(m_sAfter);
                      }
                      if (iBufferLen >= iBufferLengthSizeEnd) bEnd = true;
                    }
                  }
                }
              }
            }
          }
          //free
          if (sMatch) {MMO_FREE(sMatch);sMatch = 0;}
        }
        break;
      }
    }
    return bEnd;
  }
  
  IXmlBaseNode *MessageInterpretation::messageTransform(const char *sTextStream, IXmlBaseNode *pDestinationNode) const {
    //caller frees result
    //PUBLIC function
    IXmlBaseNode *pResultNode = 0;
    match_results_c mResults;

    //used by:
    //  request creation process: request -> messageTransform() always wants to bypass security
    const XmlAdminQueryEnvironment ibqe_request_creation(this, pDestinationNode->document()); //request creation process: request -> messageTransform() always wants to bypass security

    if (m_bHasAcceptedFormat && m_poMessageTransformation) {
      m_rxAcceptedFormats->match(sTextStream, mResults);
      pResultNode = RegularX::rx(&ibqe_request_creation, m_poMessageTransformation->node_const(), sTextStream, pDestinationNode, &mResults);
    }

    return pResultNode;
  }

  const DatabaseNode *MessageInterpretation::errorHandler(ExceptionBase& eb) const {
    //IReportableErrorClass exceptions have a virtual type() that corresponds to an error report
    const DatabaseNode *pErrorReportNode = 0;
    const char *sType = 0;
    bool bFirst = true;
    string stXpathToType;
    const ExceptionBase *pEB;

    if (m_poDefaultErrorHandlers) {
      stXpathToType = "*[@name='";
      pEB = &eb;
      do {
        if (sType = pEB->type()) {
          if (!bFirst) stXpathToType += "' or @name='";
          bFirst = false;
          stXpathToType += sType;
        }
      } while (pEB = pEB->outerException());
      if (bFirst) stXpathToType += "default";
      stXpathToType += "']";

      const XmlAdminQueryEnvironment ibqe(this, m_poDefaultErrorHandlers->document_const()); //error handler reading
      pErrorReportNode = m_poDefaultErrorHandlers->getSingleNode(&ibqe, stXpathToType.c_str());

      //tidy up
      //MMO_FREE(sType); //constants
    }

    return pErrorReportNode;
  }

  const char *MessageInterpretation::matchFirst(const rpattern_c *pPattern, const char *sTextStream) const {
    //caller frees non-zero result
    const char *sMatch = 0;
    match_results_c mResults;
    rpattern_c::backref_type br = pPattern->match(sTextStream, mResults);

    if (br.matched && (int) mResults.cbackrefs() == 2) {
      //matched!, get string
      if (br = mResults.backref(1)) {
        if (br.first && br.second) sMatch = RegularX::copyString(br.first, br.second);
      }
    }

    return (sMatch && *sMatch ? sMatch : 0);
  }

  const char *MessageInterpretation::clientSoftwareIdentifier(const char *sTextStream) const {
    //caller frees non-zero result
    return matchFirst(m_rxREST_clientSoftwareIdentifier, sTextStream);
  }

  const char *MessageInterpretation::configurationFlags(const char *sTextStream) const {
    //caller frees non-zero result
    return matchFirst(m_rxREST_configurationFlags, sTextStream);
  }

  const char *MessageInterpretation::sessionID(const char *sTextStream) const {
    //caller frees non-zero result
    return matchFirst(m_rxREST_session, sTextStream);
  }

  const char *MessageInterpretation::resourcesAreaIdentifier(const char *sTextStream) const {
    //caller frees non-zero result
    return matchFirst(m_rxREST_resourcesAreaIdentifier, sTextStream);
  }

  void MessageInterpretation::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const {
    NOT_COMPLETE("");
  }
}
