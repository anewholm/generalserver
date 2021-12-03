//platform agnostic file
#ifndef _RESPONSE_H
#define _RESPONSE_H

namespace general_server {class Request;class Response;}

#include "Request.h"
#include "Server.h"
#include "MemoryLifetimeOwner.h" //direct inheritance

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class Response: public SERVER_MAIN_XSL_MODULE, virtual public MemoryLifetimeOwner {
    //stores the output from commands
    Request *m_pRequest;
    string m_stHeader;
    const DatabaseNode *m_pPostTransformStylesheetDatabaseNode;   //post transforms
    const char *m_sPostTransformResultStartNodeXPath; //post transforms

    const IXmlLibrary *xmlLibrary() const;
    void debugWriteLastFiles(const IXmlBaseDoc *pXMLDoc, const char *sXSL) const;

    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

  protected:
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;

    //output management
    const char *sessionUUID() const;
    char *moveDocumentInstructionsToDocumentStart(char *sML) const;
    char *insertDocumentStart(  char *sML) const;
    char *processResponseStructureTags(char *sML) const;                        //XML -> removals string processing
    char *processResponseTokens(char *sML) const;                               //XML -> token replacements
    char *processResponseTokens(char *sHeader, const char *sBody) const;      //XML -> token replacements
    char *trimWhiteSpace(  char *sML) const;
    char *prependHeader(const char *sHeader, const char *sDocumentStartString, const char *sML) const;
    
  public:
    Response(Request *pRequest);
    ~Response();

    Database *db() const;
    const char *header() const;

    //output commands
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;

    //non-documented for future use
    IXmlBaseNode *xslCommand_outputLiteral(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_cannotHaveAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_canHaveAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_setPostTransform(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_setHeader(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    //output management
    const char *process(const Request *pRequest, const IXmlQueryEnvironment *pQE, const MessageInterpretation *pMI, IXmlBaseDoc *pAnalysedDataDoc, const bool bShouldServerSideXSLT = false) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput);
    const char *toString() const;
  };
}
#endif

