//platform agnostic file
#ifndef _MESSAGE_INTERPRETATION_H
#define _MESSAGE_INTERPRETATION_H

//standard library includes
#include <vector>
#include <map>
using namespace std;
#include "Utilities/regexpr2.h"
using namespace regex;

//messageTransforms return XmlDocuments
#include "DatabaseNodeServerObject.h" //direct inheritance
#include "MemoryLifetimeOwner.h"      //direct inheritance
#include "Exceptions.h"               //passed by reference

namespace general_server {
  //aheads
  class Server;
  class DatabaseNode;
  class Service;
  interface_class IXmlBaseNode;
  interface_class IXmlBaseDoc;

  class MessageInterpretation: public GeneralServerDatabaseNodeServerObject, virtual public MemoryLifetimeOwner {
  public:
    enum sentence_end_type {
      text, //default
      stream_regex
    };
    static sentence_end_type sentenceEndType(const char *sString);
    enum sentence_end_format {
      length, //default
      match
    };
    static sentence_end_format sentenceEndFormat(const char *sString);
    class Comp { //for ordering MIs
    public:
      bool operator() (const MessageInterpretation *mi1, const MessageInterpretation *mi2) const {return mi1->m_iPriority < mi2->m_iPriority;}
    };

  private:
    const Service *m_pService;
    const char *m_sMIName;
    int m_iPriority;
    const rpattern_c *m_rxAcceptedFormats;
    bool m_bHasAcceptedFormat; //needed to say if the regex was valid

    const char *m_sSentenceEnd;
    const char *m_sHeadersEnd; //defaults to m_sSentenceEnd
    bool m_bMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots;
    bool m_bAddAllStandardNamespaceDefinitionsToAppropriateRoots;
    const rpattern_c *m_rxSentenceEnd;
    bool m_bHasSentenceEnd; //needed to say if the regex was valid
    sentence_end_type m_iType;
    sentence_end_format m_iFormat;
    const char *m_sAfter;
    const rpattern_c *m_rxREST_session;
    const rpattern_c *m_rxREST_configurationFlags; //dynamic, request based config
    const rpattern_c *m_rxREST_clientSoftwareIdentifier;
    const rpattern_c *m_rxREST_resourcesAreaIdentifier;
    bool m_bHasREST_session; //needed to say if the regex was valid
    bool m_bHasREST_configurationFlags;
    bool m_bHasREST_clientSoftwareIdentifier;
    bool m_bHasREST_resourcesAreaIdentifier;
    bool m_bCloseConnection; //can be overridden by Request
    const char *m_sSummary;  //@summary of accepted format
    StringMap<const char*> m_mPaths; //configurable auto-global XSLT variables

    const DatabaseNode *m_poPrimaryOutputTransformation;
    const DatabaseNode *m_poAnalyserOutputTransformation;
    const DatabaseNode *m_poMessageTransformation;
    const DatabaseNode *m_poMessageValidityTest;
    const DatabaseNode *m_poDefaultErrorHandlers;
    
    bool m_bNoOutputEscaping;

    const char *matchFirst(const rpattern_c *pPattern, const char *sTextStream) const;

  public:
    MessageInterpretation(DatabaseNode *pNode, const Service *pService);
    ~MessageInterpretation();

    //accessors
    const char *name() const {return m_sMIName;}
    static const char *xsd_name() {return "object:MessageInterpretation";}
    const DatabaseNode *primaryOutputTransformation()  const {return m_poPrimaryOutputTransformation;}
    const DatabaseNode *analyserOutputTransformation() const {return m_poAnalyserOutputTransformation;}
    const DatabaseNode *messageTransformation()        const {return m_poMessageTransformation;}
    IXmlBaseNode *messageTransform(const char *sTextStream, IXmlBaseNode *pDestinationNode) const;
    bool hasAutoSession() const {return m_bHasREST_session;}
    bool closeConnection() const {return m_bCloseConnection;}
    bool noOutputEscaping() const {return m_bNoOutputEscaping;}
    const char *responseDocumentStart(const char *sXML = 0) const;
    bool needsMoveAllPrefixedNamespaceDefinitionsToAppropriateRoots() const;
    bool needsAddAllStandardNamespaceDefinitionsToAppropriateRoots() const;
    const DatabaseNode *errorHandler(ExceptionBase& eb) const;

    bool gimme(const char *sTextStream) const;
    bool isSenteceEnd(const char *sTextStream) const;
    const char *sessionID(const char *sTextStream) const;
    const char *configurationFlags(const char *sTextStream) const;
    const char *clientSoftwareIdentifier(const char *sTextStream) const;
    const char *resourcesAreaIdentifier(const char *sTextStream) const;
    const StringMap<const char*> *paths() const;

    class GimmePred {
      const char *m_sTextStream; //caller manages this
    public:
      GimmePred(const char *sTextStream): m_sTextStream(sTextStream) {}
      bool operator()(const MessageInterpretation* mi) const {return mi->gimme(m_sTextStream);}
    };

    const char *toString() const;
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;

    //regex comparison predicates etc
    bool operator==(const char *sTextStream) const {return gimme(sTextStream);}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
