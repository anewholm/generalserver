#include "Exceptions.h"

#include "IXml/IXslDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXslNode.h"
#include "TXml.h"
#include "Server.h"

#include <string>
using namespace std;

namespace general_server {
  void ExceptionBase::shared_breakpoint(const char *sType, const char *sMessage) {
    // We do not use IFDEBUG() here because of gdb breakpoint confusion
    // Note also that startup tests will break here
    if (sMessage && !strstr(sMessage, TESTING_ONLY)) {
#ifdef WITH_DEBUG
      string stResponse("y");
      cout << sType << "::shared_breakpoint()\n" << "  " << sMessage;
      //REDIRECT IO in Kate to pass a "y" in
      //cin >> stResponse;
      if (stResponse == "n" || stResponse == "N") terminate();
#endif
    }
  }
  
  //---------------------------- non-chained exception constructors
  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(0), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase blank constructor";)
    // Only used by UnWind()ers
    // which are routinely created on the stack:
    //   UnWind eb();
    // if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sStaticMessage);
  }

  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sStaticMessage, const bool bSharedBreakpoint):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(sStaticMessage), 
    m_sMallocedWhat(0), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase static message constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), sStaticMessage);
  }

  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const int iParam, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, iParam)), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase malloced formatOutput(int) constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sMallocedWhat);
  }
  
  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1)), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase malloced formatOutput(char) constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sMallocedWhat);
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
  }
  
  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2)), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase malloced formatOutput(char,char) constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sMallocedWhat);
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
  }
  
  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2, sParam3)), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase malloced formatOutput(char,char,char) constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sMallocedWhat);
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
    if (sParam3) MM_FREE(sParam3); 
  }
  
  ExceptionBase::ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const char *sParam4, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2, sParam3, sParam4)), 
    m_pOuterEB(0)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase malloced formatOutput(char,char,char) constructor" << "[" << what() << "]\n";)
    if (bSharedBreakpoint) SHARED_BREAKPOINT(type(), m_sMallocedWhat);
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
    if (sParam3) MM_FREE(sParam3); 
    if (sParam4) MM_FREE(sParam4); 
  }
  
  //---------------------------- chained exception constructors
  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sStaticMessage, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(sStaticMessage), 
    m_sMallocedWhat(0), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained static message constructor" << "[" << what() << "]\n";)
  }

  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const int iParam, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, iParam)), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained malloced formatOutput(int) constructor" << "[" << what() << "]\n";)
  }
  
  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1)), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained malloced formatOutput(char) constructor" << "[" << what() << "]\n";)
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
  }
  
  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2)), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained malloced formatOutput(char,char) constructor" << "[" << what() << "]\n";)
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
  }
  
  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2, sParam3)), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained malloced formatOutput(char,char,char) constructor" << "[" << what() << "]\n";)
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
    if (sParam3) MM_FREE(sParam3); 
  }

  ExceptionBase::ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const char *sParam4, const bool bSharedBreakpoint): 
    MemoryLifetimeOwner(&outerEB),
    m_sStaticMessage(0), 
    m_sMallocedWhat(formatOutput(sFormat, sParam1, sParam2, sParam3, sParam4)), 
    m_pOuterEB(outerEB.clone_with_resources())
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase chained malloced formatOutput(char,char,char,char) constructor" << "[" << what() << "]\n";)
    //if (sFormat) MMO_FREE(m_sFormat); //assummed always a literal
    if (sParam1) MM_FREE(sParam1);
    if (sParam2) MM_FREE(sParam2); 
    if (sParam3) MM_FREE(sParam3); 
    if (sParam4) MM_FREE(sParam4); 
  }

  //-------------------------------------- construction / destruction other
  void ExceptionBase::factory_throw(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClass, const char *sParam1, const char *sParam2, const char *sParam3, const bool bSharedBreakpoint) {
    //TODO: change this to allowing registration of
    // <request:throw-<class-name> why="" />
    // <request:throw-Up why="" />
    if (sClass) {
      //if      (!strcmp(sClass, "NodeExceptionBase"))      {throw NodeExceptionBase(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "StandardException"))      {throw StandardException(pMemoryLifetimeOwner, sParam1);}
      if      (!strcmp(sClass, "InterfaceNotSupported"))      {throw InterfaceNotSupported(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NativeLibraryClassRequired"))      {throw NativeLibraryClassRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NotCurrentlyUsed"))      {throw NotCurrentlyUsed(sParam1);}
      else if (_STREQUAL(sClass, "NotComplete"))      {throw NotComplete(sParam1);}
      else if (_STREQUAL(sClass, "NeedsTest"))      {throw NeedsTest(sParam1);}
      else if (_STREQUAL(sClass, "CapabilityNotSupported"))      {throw CapabilityNotSupported(pMemoryLifetimeOwner, sParam1);}
      IFDEBUG(else if (_STREQUAL(sClass, "Up"))      {throw Up(pMemoryLifetimeOwner, sParam1);})
      else if (_STREQUAL(sClass, "PipeFailure"))      {throw PipeFailure::factory(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "PipeFailure_Generic"))      {throw PipeFailure_Generic(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "PipeFailure_EACCES"))      {throw PipeFailure_EACCES(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "PipeFailure_EPIPE"))      {throw PipeFailure_EPIPE(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ProtocolNotSupported"))      {throw ProtocolNotSupported(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotSendData"))      {throw CannotSendData(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotConnectSocket"))      {throw CannotConnectSocket(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotCreateSocket"))      {throw CannotCreateSocket(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotResolveHostname"))      {throw CannotResolveHostname(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ServiceRequiresASingleClass"))      {throw ServiceRequiresASingleClass(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ServiceCannotListen"))      {throw ServiceCannotListen(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ServicePortRequired"))      {throw ServicePortRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ServiceMIsRequired"))      {throw ServiceMIsRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidProtocol"))      {throw InvalidProtocol(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "BlankInputStream"))      {throw BlankInputStream(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ZeroInputStream"))      {throw ZeroInputStream(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MultipleSocketErrors"))      {throw MultipleSocketErrors(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "OutOfMemory"))      {throw OutOfMemory(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "SemaphoreFailure"))      {throw SemaphoreFailure(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MIPrimaryOutputTransformationRequired"))      {throw MIPrimaryOutputTransformationRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MIMessageTransformationRequired"))      {throw MIMessageTransformationRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ConversationFailedToRegularXInput"))      {throw ConversationFailedToRegularXInput(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FileNotFound"))      {throw FileNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DirectoryNotFound"))      {throw DirectoryNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MissingPathToFile"))      {throw MissingPathToFile(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "LoginFailed"))      {throw LoginFailed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoDatabaseSelected"))      {throw NoDatabaseSelected(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DatabaseNodeServerObjectHasNoXSDName"))      {throw DatabaseNodeServerObjectHasNoXSDName(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoServersFoundInMainDatabase"))      {throw NoServersFoundInMainDatabase(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "RepositoryTypeUnknown"))      {throw RepositoryTypeUnknown(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "RepositoryTypeMissing"))      {throw RepositoryTypeMissing(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MemoryBlockNotFound"))      {throw MemoryBlockNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ThreadTypeUnknown"))      {throw ThreadTypeUnknown(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DateFormatResultTooLong"))      {throw DateFormatResultTooLong(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToParseDate"))      {throw FailedToParseDate(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XJavaScriptFormatUnrecignised"))      {throw XJavaScriptFormatUnrecignised(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XmlIdRedefined"))      {throw XmlIdRedefined(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoEMOAvailableForExtension"))      {throw NoEMOAvailableForExtension(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoFilterTargetDocument"))      {throw NoFilterTargetDocument(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoStylesheet"))      {throw NoStylesheet(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeMaskTypeNotRecognised"))      {throw NodeMaskTypeNotRecognised(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MaskContextNotAvailableForArea"))      {throw MaskContextNotAvailableForArea(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoOutputRootNode"))      {throw NoOutputRootNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoOutputNode"))      {throw NoOutputNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MultipleRootNodes"))      {throw MultipleRootNodes(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToCompileStylesheet"))      {throw FailedToCompileStylesheet(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoOutputDocument"))      {throw NoOutputDocument(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DataSizeAbort"))      {throw DataSizeAbort(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "BlankDocument"))      {throw BlankDocument(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ZeroDocument"))      {throw ZeroDocument(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DocumentParseError"))      {throw DocumentParseError(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ResponseTagMalformed"))      {throw ResponseTagMalformed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CDATAEndNotFound"))      {throw CDATAEndNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "AttributeRequired"))      {throw AttributeRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TransformContextRequired"))      {throw TransformContextRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XSLCommandNotSupported"))      {throw XSLCommandNotSupported(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TransformWithoutCurrentNode"))      {throw TransformWithoutCurrentNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathReturnedEmptyResultSet"))      {throw XPathReturnedEmptyResultSet(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "DynamicValueCalculation"))      {throw DynamicValueCalculation(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathContextNodeRequired"))      {throw XPathContextNodeRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ContextNodeMustHaveXMLID"))      {throw ContextNodeMustHaveXMLID(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathStringArgumentRequired"))      {throw XPathStringArgumentRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathBooleanArgumentRequired"))      {throw XPathBooleanArgumentRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathArgumentCountWrong"))      {throw XPathArgumentCountWrong(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathTooFewArguments"))      {throw XPathTooFewArguments(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathTooManyArguments"))      {throw XPathTooManyArguments(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathFunctionWrongArgumentType"))      {throw XPathFunctionWrongArgumentType(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathTypeUnhandled"))      {throw XPathTypeUnhandled(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidXMLName"))      {throw InvalidXMLName(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidXML"))      {throw InvalidXML(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ValidityCheckFailure"))      {throw ValidityCheckFailure(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotReconcileNamespacePrefix"))      {throw CannotReconcileNamespacePrefix(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotReconcileDefaultNamespace"))      {throw CannotReconcileDefaultNamespace(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NamespaceIsInvalid"))      {throw NamespaceIsInvalid(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XSLTExtensionNotFound"))      {throw XSLTExtensionNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "HardLinkToNowhere"))      {throw HardLinkToNowhere(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DeviationOriginalNotFound"))      {throw DeviationOriginalNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DeviationDeviantNotFound"))      {throw DeviationDeviantNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "SessionNodeSetOutOfRange"))      {throw SessionNodeSetOutOfRange(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToResolveXSLIncludePath"))      {throw FailedToResolveXSLIncludePath(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotConvertAbsoluteHREFToXPathWithoutRoot"))      {throw CannotConvertAbsoluteHREFToXPathWithoutRoot(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotRunXPathWithoutNamespaceCache"))      {throw CannotRunXPathWithoutNamespaceCache(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DocumentMismatchForNamespaceCacheCopyOperation"))      {throw DocumentMismatchForNamespaceCacheCopyOperation(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MismatchedNamespacePrefixAssociation"))      {throw MismatchedNamespacePrefixAssociation(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MissingDefaultNamespaceDecleration"))      {throw MissingDefaultNamespaceDecleration(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XMLID_DuplicationIssue"))      {throw XMLID_DuplicationIssue(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XMLParseFailedAt"))      {throw XMLParseFailedAt(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XSLStylesheetNodeNotFound"))      {throw XSLStylesheetNodeNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UnknownHardlinkPolicy"))      {throw UnknownHardlinkPolicy(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "RelationalDatabaseConnectionFailed"))      {throw RelationalDatabaseConnectionFailed(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "XSLTException"))      {throw XSLTException(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "XSLElementException"))      {throw XSLElementException(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "XPathException"))      {throw XPathException(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathNot1Nodes"))      {throw XPathNot1Nodes(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "PositionOutOfRange"))      {throw PositionOutOfRange(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XPathStringNot1Nodes"))      {throw XPathStringNot1Nodes(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ExceptionClassRequired"))      {throw ExceptionClassRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ExceptionClassNotAllowed"))      {throw ExceptionClassNotAllowed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MultipleResourceFound"))      {throw MultipleResourceFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ResourceNotFound"))      {throw ResourceNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UserStoreNotFound"))      {throw UserStoreNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UnkownSecurityStrategyName"))      {throw UnkownSecurityStrategyName(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "WrongIssuerSecurityContext"))      {throw WrongIssuerSecurityContext(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UserDoesNotHavePermission"))      {throw UserDoesNotHavePermission(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "SessionRequired"))      {throw SessionRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeWriteableAccessDenied"))      {throw NodeWriteableAccessDenied(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeAddableAccessDenied"))      {throw NodeAddableAccessDenied(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlUnknownTransactionType"))      {throw TXmlUnknownTransactionType(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlFailedToParseTransactionXML"))      {throw TXmlFailedToParseTransactionXML(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlRequiredNodeNotFound"))      {throw TXmlRequiredNodeNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlCannotOpenTransactionStore"))      {throw TXmlCannotOpenTransactionStore(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlSerialisationParentNull"))      {throw TXmlSerialisationParentNull(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlInvalid"))      {throw TXmlInvalid(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlVolatileResult"))      {throw TXmlVolatileResult(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DynamicPathToNonRepositoryNode"))      {throw DynamicPathToNonRepositoryNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "SubRepositoryNotFoundForNode"))      {throw SubRepositoryNotFoundForNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TXmlTransformNotSupportedFor"))      {throw TXmlTransformNotSupportedFor(pMemoryLifetimeOwner, sParam1);}
      //else if (_STREQUAL(sClass, "TXmlApplicationFailed"))      {throw TXmlApplicationFailed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToRemoveOldItem"))      {throw FailedToRemoveOldItem(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToSwapInPlaceholder"))      {throw FailedToSwapInPlaceholder(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "FailedToOpenSyncDirectory"))      {throw FailedToOpenSyncDirectory(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UnknownFileType"))      {throw UnknownFileType(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "MissingRepositoryNameAttribute"))      {throw MissingRepositoryNameAttribute(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ResourcesCopyFailure"))      {throw ResourcesCopyFailure(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidCommitStyle"))      {throw InvalidCommitStyle(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidCommitStatus"))      {throw InvalidCommitStatus(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidCommitStyleName"))      {throw InvalidCommitStyleName(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "InvalidCommitStatusName"))      {throw InvalidCommitStatusName(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "UnregisteredDatabaseClass"))      {throw UnregisteredDatabaseClass(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ClassElementsMustBeElement"))      {throw ClassElementsMustBeElement(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TooManyDatabaseClassOnElement"))      {throw TooManyDatabaseClassOnElement(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DatabaseClassNameRequired"))      {throw DatabaseClassNameRequired(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DatabaseMatchFailed"))      {throw DatabaseMatchFailed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DatabaseClassNameXmlIDWrong"))      {throw DatabaseClassNameXmlIDWrong(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DatabaseClassNameWrong"))      {throw DatabaseClassNameWrong(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DuplicateDatabaseClass"))      {throw DuplicateDatabaseClass(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "DocumentNotValid"))      {throw DocumentNotValid(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoRootNodeFound"))      {throw NoRootNodeFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeNotFound"))      {throw NodeNotFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoTmpNodeFound"))      {throw NoTmpNodeFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "CannotCreateInTmpNode"))      {throw CannotCreateInTmpNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NoDocumentNodeFound"))      {throw NoDocumentNodeFound(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "ReplaceNodeRequiresConstantXMLID"))      {throw ReplaceNodeRequiresConstantXMLID(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XmlGenericException"))      {throw XmlGenericException(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "XmlParserNamespaceNotDeclared"))      {throw XmlParserNamespaceNotDeclared(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TransientNode"))      {throw TransientNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "OrphanNode"))      {throw OrphanNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NonTransientNode"))      {throw NonTransientNode(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "TransformFailed"))      {throw TransformFailed(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeLockPreventedFree"))      {throw NodeLockPreventedFree(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeLockHasNoLocks"))      {throw NodeLockHasNoLocks(pMemoryLifetimeOwner, sParam1);}
      else if (_STREQUAL(sClass, "NodeLockFailed"))      {throw NodeLockFailed(pMemoryLifetimeOwner, sParam1);}
      throw ExceptionClassNotAllowed(pMemoryLifetimeOwner, sClass);
    }
    
    throw ExceptionClassRequired(pMemoryLifetimeOwner);
  }
  
  ExceptionBase& ExceptionBase::operator=(const ExceptionBase& eb) {
    MemoryLifetimeOwner::operator=(eb);
    m_pOuterEB       = (eb.m_pOuterEB ? eb.m_pOuterEB->clone_with_resources() : 0);
    m_sMallocedWhat  = (eb.m_sMallocedWhat ? MM_STRDUP(eb.m_sMallocedWhat) : 0);
    m_sStaticMessage = eb.m_sStaticMessage;

    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase assigment [" << what() << "]\n";)
    return *this;
  }

  ExceptionBase::ExceptionBase(const ExceptionBase& eb):
    MemoryLifetimeOwner(eb.mmParent()), //addChild()
    m_pOuterEB(eb.m_pOuterEB ? eb.m_pOuterEB->clone_with_resources() : 0),
    m_sMallocedWhat(eb.m_sMallocedWhat ? MM_STRDUP(eb.m_sMallocedWhat) : 0),
    m_sStaticMessage(eb.m_sStaticMessage)
  {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase copy constructor [" << what() << "]\n";)
  }

  ExceptionBase::~ExceptionBase() throw() {
    IFDEBUG_EXCEPTIONS(cout << "ExceptionBase ~destructor [" << what() << "]\n";)
    
    if (m_sMallocedWhat) {
      IFDEBUG_EXCEPTIONS(cout << "  freeing m_sMallocedWhat\n";)
      MMO_FREE(m_sMallocedWhat);
    }
    
    if (m_pOuterEB) {
      IFDEBUG_EXCEPTIONS(cout << "  freeing m_sMallocedWhat\n";)
      delete m_pOuterEB;
    }
  }

  //-------------------------------------- methods
  const char *ExceptionBase::type() const {
    return "ExceptionBase";
  }

  const ExceptionBase *ExceptionBase::outerException() const {
    return m_pOuterEB;
  }
  
  const ExceptionBase *ExceptionBase::outestException() const {
    return m_pOuterEB ? m_pOuterEB->outestException() : this;
  }
  
  const bool ExceptionBase::involvesType(const char *sType) const {
    //recursive
    return _STREQUAL(sType, type()) || (m_pOuterEB && m_pOuterEB->involvesType(sType));
  }
  
  const char *ExceptionBase::formatOutput(const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const char *sParam4) const {
    //caller frees local inputs
    //because we don't know which are static and which are malloc
    //our m_sMallocedWhat freed in destructor, but copied in the copy and assignment constructors
    size_t lSize = strlen(sFormat)
      + (sParam1 ? strlen(sParam1) : 0)
      + (sParam2 ? strlen(sParam2) : 0)
      + (sParam3 ? strlen(sParam3) : 0)
      + (sParam4 ? strlen(sParam4) : 0)
      + 1;
    char *sBuffer = (char*) MMO_MALLOC(lSize); //private
    _SNPRINTF4(sBuffer, lSize, sFormat,
      (sParam1 ? sParam1 : ""),
      (sParam2 ? sParam2 : ""),
      (sParam3 ? sParam3 : ""),
      (sParam4 ? sParam4 : "")
    );
    //IFDEBUG(cout << "new ExceptionBase(" << sBuffer << ")";)
    return sBuffer;
  }

  const char *ExceptionBase::formatOutput(const char *sFormat, const int iParam) const {
    const char *sParam = itoa(iParam);
    size_t lSize = strlen(sFormat) + strlen(sParam) + 1;
    char *sBuffer = (char*) MMO_MALLOC(lSize); //private
    _SNPRINTF1(sBuffer, lSize, sFormat, sParam);
    MM_FREE(sParam);
    //IFDEBUG(cout << "new ExceptionBase(" << sBuffer << ")";)
    return sBuffer;
  }

  const char* ExceptionBase::what() const throw() {
    //caller frees
    //recursive
    const char *sSubWhat = 0;
    string stWhat = (m_sMallocedWhat ? m_sMallocedWhat : (m_sStaticMessage ? m_sStaticMessage : "-"));

    if (m_pOuterEB && (sSubWhat = m_pOuterEB->what())) {
      if (*sSubWhat) {
        stWhat += "\n  ";
        stWhat += sSubWhat;
      }
      MM_FREE(sSubWhat);
    }

    return MM_STRDUP(stWhat.c_str());
  }

  //------------------------------------------- structural BaseExceptions -------------------------------------------
  //important to avoid using the ExceptionBase::ExceptionBase(const ExceptionBase &eb) copy constructor
  //because we want to assign m_pOuterEB, not copy it
  //so we use "unwind"
  UnWinder::UnWinder(const ExceptionBase& eb):  ExceptionBase(eb, "unwind") {}

  UnWinder::UnWinder(): ExceptionBase(NO_OWNER) {}

  const char* UnWinder::what() const throw() {
    //recursive: skip this UnWinder chain link
    return m_pOuterEB->what();
  }
    
  StandardException::StandardException(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const exception &ex): 
    m_ex(ex), 
    ExceptionBase(pMemoryLifetimeOwner, "std:exception [%s]", MM_STRDUP(ex.what())) 
  {}

  StandardException *StandardException::clone_with_resources() const {return new StandardException(*this);}
  
  StandardException::~StandardException() {}
  
  const char *StandardException::type() const {return "StandardException";}
  
  //------------------------------------------- Sockets -------------------------------------------
  PipeFailure *PipeFailure::factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iErrno) {
    PipeFailure *pException = 0;
    switch (iErrno) {
      case EPIPE:  {pException = new PipeFailure_EPIPE(pMemoryLifetimeOwner); break;}
      case EACCES: {pException = new PipeFailure_EACCES(pMemoryLifetimeOwner); break;}
      default:     {pException = new PipeFailure_Generic(pMemoryLifetimeOwner, iErrno); break;}
    }

    return pException;
  }
  const char* PipeFailure::what() const throw() {
     //recursive
    const char *sSubWhat = 0;
    string stWhat;

    //chain actual what
    if (sSubWhat = ExceptionBase::what()) {
      stWhat += sSubWhat;
      MM_FREE(sSubWhat);
    }

    //our custom content
    stWhat += "\nsub-system error [";
    stWhat += strerror(m_iErrno); //static C buffer
    stWhat += "]\n";

    return MM_STRDUP(stWhat.c_str());
  }

  ServiceCannotListen::ServiceCannotListen(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sReason, const int iErrno, const int iPort):
    m_iPort(iPort),
    m_iErrno(iErrno),
    ExceptionBase(pMemoryLifetimeOwner, "Service Cannot Listen [%s]", sReason)
  {}

  const char* ServiceCannotListen::netStat(const int iPort) {
    char       *sCommand = 0;
    const char *sOutput  = 0;

    //run command
    sCommand = (char*) MMO_MALLOC(1024);
    _SNPRINTF1(sCommand, 1024, "netstat -o -n -a | grep %i", iPort);
    sOutput = Server::execServerProcess(sCommand);
    if (!sOutput) sOutput = MM_STRDUP("no socket activity on port");

    //free up
    MMO_FREE(sCommand);
    
    return sOutput;
  }

  const char* ServiceCannotListen::processes(const char *sProcessName) {
    char       *sCommand = 0;
    const char *sOutput  = 0;

    //run command
    sCommand = (char*) MMO_MALLOC(1024);
    _SNPRINTF1(sCommand, 1024, "ps -A | grep %s", sProcessName);
    sOutput = Server::execServerProcess(sCommand);
    if (!sOutput) sOutput = MM_STRDUP("no other process activity");

    //free up
    MMO_FREE(sCommand);

    return sOutput;
  }

  const char* ServiceCannotListen::what() const throw() {
     //recursive
    const char *sSubWhat = 0;
    string stWhat;

    //chain actual what
    if (sSubWhat = ExceptionBase::what()) {
      stWhat += sSubWhat;
      MM_FREE(sSubWhat);
    }

    //our custom content
    stWhat += "\nsub-system error [";
    stWhat += strerror(m_iErrno); //static C buffer
    stWhat += "]\n";

    return MM_STRDUP(stWhat.c_str());
  }

  //------------------------------------------- XSLTException -------------------------------------------
  XSLTException::XSLTException(const ExceptionBase &eb, const IXslTransformContext *pCtxt, const char *sErrorMessage):
    m_pInputNode(   pCtxt->sourceNode(this)), 
    m_pCommandNode( pCtxt->instructionNode(this)), 
    m_pTemplateNode(pCtxt->templateNode(this)),
    m_pOutputNode(  pCtxt->outputNode(this) ), 
    m_sCurrentMode( pCtxt->currentModeName()),
    ExceptionBase(eb, "XSLTException(xpath [%s]) %s", pCtxt->currentXSLCommandXPath(), sErrorMessage)
  {
    IFDEBUG_EXCEPTIONS(cout << "XSLTException constructor with COPYOUTEREXCEPTION\n";)
  }
  
  XSLTException::XSLTException(const IXslTransformContext *pCtxt, const char *sErrorMessage):
    m_pInputNode(   pCtxt->sourceNode(this)), 
    m_pCommandNode( pCtxt->instructionNode(this)), 
    m_pTemplateNode(pCtxt->templateNode(this)),
    m_pOutputNode(  pCtxt->outputNode(this) ), 
    m_sCurrentMode( pCtxt->currentModeName()),
    ExceptionBase(pCtxt, "XSLTException(xpath [%s]) %s", pCtxt->currentXSLCommandXPath(), sErrorMessage)
  {
    IFDEBUG_EXCEPTIONS(cout << "XSLTException constructor\n";)
  }
  
  XSLTException::~XSLTException() throw() {
    IFDEBUG_EXCEPTIONS(cout << "XSLTException ~destructor\n";)
    if (m_pInputNode)    MMO_DELETE(m_pInputNode);
    if (m_pCommandNode)  MMO_DELETE(m_pCommandNode);
    if (m_pTemplateNode) MMO_DELETE(m_pTemplateNode);
    if (m_pOutputNode)   MMO_DELETE(m_pOutputNode); 
    if (m_sCurrentMode)  MMO_DELETE(m_sCurrentMode);
  }
  
  XSLTException *XSLTException::clone_with_resources() const {
    return new XSLTException(*this);
  }
  
  const char *XSLTException::type() const {
    return "XSLTException";
  }
  
  XSLTException::XSLTException(const XSLTException& xte): 
    m_pInputNode(   xte.m_pInputNode    ? MMO_CLONE( xte.m_pInputNode)   : NULL), 
    m_pCommandNode( xte.m_pCommandNode  ? MMO_CLONE( xte.m_pCommandNode) : NULL), 
    m_pTemplateNode(xte.m_pTemplateNode ? MMO_CLONE( xte.m_pTemplateNode)->queryInterface((IXslCommandNode*) 0) : NULL), 
    m_pOutputNode(  xte.m_pOutputNode   ? MMO_CLONE( xte.m_pOutputNode)  : NULL), 
    m_sCurrentMode( xte.m_sCurrentMode  ? MMO_STRDUP(xte.m_sCurrentMode) : NULL),
    ExceptionBase(xte) 
  {
    IFDEBUG_EXCEPTIONS(cout << "XSLTException copy constructor [" << what() << "]\n";)
  }
  
  const char *XSLTException::contextXPath(const IXmlQueryEnvironment *pQE) {
    //caller frees result
    return m_pCommandNode ? m_pCommandNode->uniqueXPathToNode(pQE) : MM_STRDUP("<no command node>");
  }

  //------------------------------------------- XSLElementException -------------------------------------------
  XSLElementException::XSLElementException(const ExceptionBase &eb): ExceptionBase(eb, "XSLElementException") {
    IFDEBUG_EXCEPTIONS(cout << "XSLElementException constructor [" << what() << "]\n";)
  }
  XSLElementException::~XSLElementException() throw() {
    IFDEBUG_EXCEPTIONS(cout << "XSLElementException ~destructor [" << what() << "]\n";)
  }
  XSLElementException *XSLElementException::clone_with_resources() const {
    return new XSLElementException(*this);
  }

  //------------------------------------------- XPathException -------------------------------------------
  XPathException::XPathException(const ExceptionBase &outerEB, const IXslXPathFunctionContext *pXCtxt, const char *sErrorMessage): 
    m_sXPath(            pXCtxt && pXCtxt->xpath()             ? MMO_STRDUP(pXCtxt->xpath())            : NULL), 
    m_sFunctionName(     pXCtxt && pXCtxt->functionName()      ? MMO_STRDUP(pXCtxt->functionName())     : NULL), 
    m_sFunctionNamespace(pXCtxt && pXCtxt->functionNamespace() ? MMO_STRDUP(pXCtxt->functionNamespace()): NULL), 
    m_pCurrentNode(      pXCtxt ? MMO_CLONE( pXCtxt->currentNode(this))  : NULL), 
    ExceptionBase(outerEB, "XPathException [%s] [%s]", _STRDDUP(pXCtxt->xpath(), "<no xpath>"), sErrorMessage)
  {
    IFDEBUG_EXCEPTIONS(cout << "XPathException constructor [" << what() << "]\n";)
  }
  
  XPathException::XPathException(const IXslXPathFunctionContext *pXCtxt, const char *sErrorMessage): 
    m_sXPath(            pXCtxt && pXCtxt->xpath()             ? MMO_STRDUP(pXCtxt->xpath())            : NULL), 
    m_sFunctionName(     pXCtxt && pXCtxt->functionName()      ? MMO_STRDUP(pXCtxt->functionName())     : NULL), 
    m_sFunctionNamespace(pXCtxt && pXCtxt->functionNamespace() ? MMO_STRDUP(pXCtxt->functionNamespace()): NULL), 
    m_pCurrentNode(      pXCtxt ? MMO_CLONE( pXCtxt->contextNode(this))  : NULL), 
    ExceptionBase(pXCtxt->mmParent(), "XPathException [%s] [%s]", _STRDDUP(pXCtxt->xpath(), "<no xpath>"), sErrorMessage)
  {
    IFDEBUG_EXCEPTIONS(cout << "XPathException constructor [" << what() << "]\n";)
  }
  
  //TODO: output the xpath that failed
  XPathException::XPathException(const XPathException& xpe): 
    m_sXPath(            xpe.m_sXPath             ? MMO_STRDUP(xpe.m_sXPath)             : 0), 
    m_sFunctionName(     xpe.m_sFunctionName      ? MMO_STRDUP(xpe.m_sFunctionName)      : 0), 
    m_sFunctionNamespace(xpe.m_sFunctionNamespace ? MMO_STRDUP(xpe.m_sFunctionNamespace) : 0), 
    m_pCurrentNode(      xpe.m_pCurrentNode       ? MMO_CLONE( xpe.m_pCurrentNode)       : 0), 
    ExceptionBase(xpe) 
  {}
  
  XPathException::~XPathException() throw() {
    IFDEBUG_EXCEPTIONS(cout << "XPathException ~destructor [" << what() << "]\n";)
    if (m_sXPath)             MMO_FREE(m_sXPath);
    if (m_sFunctionName)      MMO_FREE(m_sFunctionName);
    if (m_sFunctionNamespace) MMO_FREE(m_sFunctionNamespace);
    if (m_pCurrentNode)       MMO_DELETE(m_pCurrentNode);
  }
  
  XPathException *XPathException::clone_with_resources() const {
    return new XPathException(*this);
  }

  XmlGenericException::XmlGenericException(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const char *sDetail):
    ExceptionBase(pMemoryLifetimeOwner, "Xml Generic Exception [%s] [%s]", sMessage, (sDetail ? sDetail : MM_STRDUP("<no details>"))) 
  {}

  //------------------------------------------- TXml -------------------------------------------
  TXmlApplicationFailed::TXmlApplicationFailed(const ExceptionBase &eb, const TXml *pTXml): 
    m_pTXml(pTXml ? MMO_CLONE(pTXml) : NULL), 
    ExceptionBase(eb) 
  {assert(m_pTXml);}

  TXmlApplicationFailed::TXmlApplicationFailed(const TXmlApplicationFailed &taf):
    m_pTXml(taf.m_pTXml ? MMO_CLONE(taf.m_pTXml) : NULL), 
    ExceptionBase(taf) 
  {}
  
  TXmlApplicationFailed::~TXmlApplicationFailed() throw() {if (m_pTXml) MMO_DELETE(m_pTXml);}
  
  const char *TXmlApplicationFailed::what() const throw() {
     //recursive
    const char *sSubWhat = 0;
    string stWhat;
    stWhat += "TXmlApplicationFailed ";
    stWhat += (m_pTXml ? m_pTXml->toString() : "no TXml\n");

    if (sSubWhat = ExceptionBase::what()) {
      stWhat += sSubWhat;
      MM_FREE(sSubWhat);
    }

    return MM_STRDUP(stWhat.c_str());
  }
  
  //------------------------------------------- NodeExceptionBase -------------------------------------------
  NodeExceptionBase::NodeExceptionBase(const ExceptionBase &outerEB, const char *sMessage, const IXmlBaseNode *pNode, const char *sLiteralParam2, const char *sLiteralParam3, const char *sLiteralParam4): 
    m_pNode(pNode->clone_with_resources()),
    ExceptionBase(outerEB, sMessage, (pNode ? pNode->localName() : MM_STRDUP("<no node details>")), sLiteralParam2, sLiteralParam3, sLiteralParam4)
  {}
  
  NodeExceptionBase::NodeExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage, const IXmlBaseNode *pNode, const char *sLiteralParam2, const char *sLiteralParam3, const char *sLiteralParam4): 
    m_pNode(pNode->clone_with_resources()),
    ExceptionBase(pMemoryLifetimeOwner, sMessage, (pNode ? pNode->localName() : MM_STRDUP("<no node details>")), sLiteralParam2, sLiteralParam3, sLiteralParam4)
  {}
  
  NodeExceptionBase::~NodeExceptionBase() {
    if (m_pNode) delete m_pNode;
  }
  
  NodeExceptionBase::NodeExceptionBase(const NodeExceptionBase &neb):
    ExceptionBase(neb.mmParent()),
    m_pNode(neb.m_pNode ? neb.m_pNode->clone_with_resources() : 0)
  {}

  //------------------------------------------- IntendedAncestorBaseNodeNotTraversed -------------------------------------------
  IntendedAncestorBaseNodeNotTraversed::IntendedAncestorBaseNodeNotTraversed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
    const char *sMessage, 
    const IXmlBaseNode *pTargetNode, 
    const IXmlBaseNode *pBaseNode,
    const char *sLiteralParam2, const char *sLiteralParam3
  ): 
    ExceptionBase(pMemoryLifetimeOwner, sMessage, 
      (pTargetNode ? pTargetNode->localName() : MM_STRDUP("<no target-node details>")), 
      (pBaseNode ? pBaseNode->localName() : MM_STRDUP("<no base-node details>")), 
      sLiteralParam2, sLiteralParam3, 
      NO_SHARED_BREAKPOINT
    ),
    m_pTargetNode(pTargetNode->clone_with_resources()),
    m_pBaseNode(pBaseNode->clone_with_resources())
  {}
  
  IntendedAncestorBaseNodeNotTraversed::~IntendedAncestorBaseNodeNotTraversed() {
    if (m_pTargetNode) MMO_DELETE(m_pTargetNode);
    if (m_pBaseNode)   MMO_DELETE(m_pBaseNode);
  }
  
  IntendedAncestorBaseNodeNotTraversed *IntendedAncestorBaseNodeNotTraversed::clone_with_resources() const {
    return new IntendedAncestorBaseNodeNotTraversed(*this);
  }
  
  IntendedAncestorBaseNodeNotTraversed::IntendedAncestorBaseNodeNotTraversed(const IntendedAncestorBaseNodeNotTraversed &neb):
    ExceptionBase(neb.mmParent(), false),
    m_pTargetNode(neb.m_pTargetNode ? neb.m_pTargetNode->clone_with_resources() : 0),
    m_pBaseNode(  neb.m_pBaseNode   ? neb.m_pBaseNode->clone_with_resources()   : 0)
  {}
}
