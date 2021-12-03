#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include <iostream>
#include <exception>
using namespace std;


#include "define.h"
#include "Utilities/strtools.h"
#include "Utilities/StringMap.h"

#include "MemoryLifetimeOwner.h" //passed on stack

#define SHARED_BREAKPOINT(t,m) ExceptionBase::shared_breakpoint(t,m) //used also by other modules, e.g. Debug::report()
#define TESTING_ONLY "__testing only__"
#define NO_SHARED_BREAKPOINT false

/* EXCEPTION UNWINDING
 * http://en.cppreference.com/w/cpp/language/try_catch
 *   E and T are the same type (ignoring top-level cv-qualifiers on T)
 *   T is an lvalue-reference to (possibly cv-qualified) E
 *   T is an unambiguous public base class of E
 *   T is a reference to an unambiguous public base class of E
 * 
 * UNWIND_EXCEPTION_BEGIN {
 *   ...
 * } UNWIND_EXCEPTION_END;
 * 
 * //free up
 * UNWIND_DELETE_IF_EXCEPTION(...);
 * UNWIND_FREE_IF_NOEXCEPTION(...);
 * 
 * UNWIND_EXCEPTION_THROW;
 */
#define UNWIND_EXCEPTION_DECLARE UnWinder unwinder
#define UNWIND_EXCEPTION_TRY     try
#define UNWIND_EXCEPTION_BEGIN   UNWIND_EXCEPTION_DECLARE; UNWIND_EXCEPTION_TRY
#define UNWIND_EXCEPTION_END     catch (ExceptionBase &eb)  {unwinder = UnWinder(eb);} \
                                 catch (exception &ex)      {unwinder = UnWinder(StandardException(this, ex));} \
                                 catch (...)                {throw;} //just in case
#define UNWIND_EXCEPTION_BEGIN_STATIC(mm) UNWIND_EXCEPTION_DECLARE; UNWIND_EXCEPTION_TRY
#define UNWIND_EXCEPTION_END_STATIC(mm)   catch (ExceptionBase &eb)  {unwinder = UnWinder(eb);} \
                                 catch (exception &ex)      {unwinder = UnWinder(StandardException(mm, ex));} \
                                 catch (...)                {throw;} //just in case

#define UNWIND_IF_EXCEPTION(x)   if (unwinder)  {x;}
#define UNWIND_IF_NOEXCEPTION(x) if (!unwinder) {x;}
#define UNWIND_DELETE_IF_EXCEPTION(x)   UNWIND_IF_EXCEPTION(if (x) {delete x; x=0;})
#define UNWIND_FREE_IF_EXCEPTION(x)     UNWIND_IF_EXCEPTION(if (x) {MMO_FREE(x); x=0;})
#define UNWIND_DELETE_IF_NOEXCEPTION(x) UNWIND_IF_EXCEPTION(if (x) {delete x; x=0;})
#define UNWIND_FREE_IF_NOEXCEPTION(x)   UNWIND_IF_EXCEPTION(if (x) {MMO_FREE(x); x=0;})

#define UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_NOEXCEPTION(x) UNWIND_IF_NOEXCEPTION(if (x) {delete x->element_destroy(); x=0;})
#define UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(x)   UNWIND_IF_EXCEPTION(  if (x) {delete x->element_destroy(); x=0;})

#define UNWIND_EXCEPTION_THROW   if (unwinder) throw unwinder;


namespace general_server {
  /* Exceptions vs asserts vs return codes
   *   Exceptions:  recoverable. Offer the caller the option of recovery
   *   assert():    non-recoverable. System coding failure
   *   return code: continuable. happens within normal operation and not regarded as a potential fatal issue
   */
  class TXml;
  interface_class IXslCommandNode;
  
  //---------------------------------------------------- sub-classes
  class ExceptionBase: public exception, public MemoryLifetimeOwner {
  private:
    const char *m_sMallocedWhat;
    const char *m_sStaticMessage;
    
    const char *formatOutput(const char *sFormat, const char *sParam1, const char *sParam2 = 0, const char *sParam3 = 0, const char *sParam4 = 0) const;
    const char *formatOutput(const char *sFormat, const int iParam1) const;

  protected:
    const ExceptionBase *m_pOuterEB;
    
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const bool bSharedBreakpoint = true);
    
    //first exceptions
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sStaticMessage, const bool bSharedBreakpoint = true);
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const int iParam, const bool bSharedBreakpoint = true);
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const bool bSharedBreakpoint = true);
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const bool bSharedBreakpoint = true);
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const bool bSharedBreakpoint = true);
    ExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const char *sParam4, const bool bSharedBreakpoint = true);

    //chained exceptions
    ExceptionBase(const ExceptionBase &outerEB, const char *sStaticMessage, const bool bSharedBreakpoint = true);
    ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const int iParam, const bool bSharedBreakpoint = true);
    ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const bool bSharedBreakpoint = true);
    ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const bool bSharedBreakpoint = true);
    ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const bool bSharedBreakpoint = true);
    ExceptionBase(const ExceptionBase &outerEB, const char *sFormat, const char *sParam1, const char *sParam2, const char *sParam3, const char *sParam4, const bool bSharedBreakpoint = true);

    //copy constructor to ensure that resources are duplicated properly
    ExceptionBase(const ExceptionBase& eb);
    //assignment operator to ensure that resources are duplicated properly
    ExceptionBase& operator=(const ExceptionBase& eb);

  public:
    //~destructors are run after copy constructor or assignment operator on the local stack during throw processing
    //release all resources here as the copy and assignment constructors will copy them
    virtual ~ExceptionBase() throw();
    static void shared_breakpoint(const char *sType, const char *sMessage = 0);

    //factory
    static void factory_throw(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClass, const char *sParam1, const char *sParam2 = 0, const char *sParam3 = 0, const bool bSharedBreakpoint = true);

    //runs the copy constructor via new T(*this)
    //used by this class within the chaining procedure
    //and by others like the un-winding pattern
    virtual ExceptionBase *clone_with_resources() const = 0;

    //client-reportable nature
    virtual const char *type() const;
    virtual StringMap<const char*> *variables() const {return 0;}
    virtual const char *contextXPath() {return 0;}

    //override the std:exception
    const char *toString() const {return what();}
    virtual const char* what() const throw();
    const ExceptionBase *outerException()  const;
    const ExceptionBase *outestException() const;
    const bool           involvesType(const char *sType) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class UnWinder: public ExceptionBase {
    //use to re-throw ExceptionBase(this, pMemoryLifetimeOwner) during un-wind procedures
  protected:
    UnWinder *clone_with_resources() const {return new UnWinder(*this);}
  public:
    //this exception, by definition, is NOT in a catch {}
    //so we need to set the chain manualy
    UnWinder(); //without an outer exception will return false below
    UnWinder(const ExceptionBase& eb);
    
    operator const bool() const {return m_pOuterEB;} //if (unwinder) do_something();
    const char *type() const {return "UnWinder";}
    const char *what() const throw();
  };

  //---------------------------------------------------- exceptions structural bases
  class NodeExceptionBase: public ExceptionBase { //or UnLinked or NoParent
    const IXmlBaseNode *m_pNode;
  protected:
    virtual NodeExceptionBase *clone_with_resources() const = 0;
    NodeExceptionBase(const ExceptionBase &outerEB, const char *sMessage, const IXmlBaseNode *pNode = 0, const char *sLiteralParam2 = 0, const char *sLiteralParam3 = 0, const char *sLiteralParam4 = 0);
    NodeExceptionBase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage, const IXmlBaseNode *pNode = 0, const char *sLiteralParam2 = 0, const char *sLiteralParam3 = 0, const char *sLiteralParam4 = 0);
    ~NodeExceptionBase();
    NodeExceptionBase(const NodeExceptionBase &neb);
  };
 
  //---------------------------------------------------- C++ structural exceptions
  class StandardException: public ExceptionBase {
    //used simply to contain information on chained std:exceptions which don't natively support chaining in C++98
    const exception m_ex;
  protected:
    StandardException *clone_with_resources() const;
  public:
    StandardException(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const exception &ex);
    ~StandardException();
    const char *type() const;
  };
  
  class InterfaceNotSupported: public ExceptionBase {
    //thrown by inappropriate queryInterface(type) operations
  protected:
    InterfaceNotSupported *clone_with_resources() const {return new InterfaceNotSupported(*this);}
  public:
    InterfaceNotSupported(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sClassName): ExceptionBase(pMemoryLifetimeOwner,"Interface Not Supported [%s]", sClassName) {}
    const char *type() const {return "InterfaceNotSupported";}
  };
  
  class NativeLibraryClassRequired: public ExceptionBase {
    //thrown after failed dynamic casts that assume a specific native super class
  protected:
    NativeLibraryClassRequired *clone_with_resources() const {return new NativeLibraryClassRequired(*this);}
  public:
    NativeLibraryClassRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sClassName): ExceptionBase(pMemoryLifetimeOwner,"Native Library Class required [%s]", sClassName) {}
    const char *type() const {return "NativeLibraryClassRequired";}
  };
  
  class NotCurrentlyUsed: public ExceptionBase {
  protected:
    NotCurrentlyUsed *clone_with_resources() const {return new NotCurrentlyUsed(*this);}
  private:
    //thrown by inappropriate functionality requests
  public:
    NotCurrentlyUsed(const char* sWhat = 0): ExceptionBase(NO_OWNER,"Not Currently Used [%s]", sWhat) {}
    const char *type() const {return "NotCurrentlyUsed";}
  };
  
  class NotComplete: public ExceptionBase {
  protected:
    NotComplete *clone_with_resources() const {return new NotComplete(*this);}
  private:
    //thrown by inappropriate functionality requests
  public:
    NotComplete(const char* sWhat = 0): ExceptionBase(NO_OWNER,"Not Complete [%s]", sWhat) {}
    const char *type() const {return "NotComplete";}
  };
  
  class NeedsTest: public ExceptionBase {
  protected:
    NeedsTest *clone_with_resources() const {return new NeedsTest(*this);}
  private:
    //thrown by inappropriate functionality requests
  public:
    NeedsTest(const char* sWhat = 0): ExceptionBase(NO_OWNER,"Needs Test [%s]", sWhat) {}
    const char *type() const {return "NeedsTest";}
  };

  class CapabilityNotSupported: public ExceptionBase {
  protected:
    CapabilityNotSupported *clone_with_resources() const {return new CapabilityNotSupported(*this);}
  private:
    //thrown by inappropriate functionality requests
  public:
    CapabilityNotSupported(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sCapability): ExceptionBase(pMemoryLifetimeOwner,"Capability Not Supported [%s]", sCapability) {}
    const char *type() const {return "CapabilityNotSupported";}
  };

  IFDEBUG( //throw Up(this) is not allowed for the production version
    class Up: public ExceptionBase {
    protected:
      Up *clone_with_resources() const {return new Up(*this);}
    public:
      Up(const ExceptionBase &outerEB, const char* sStaticMessage): ExceptionBase(outerEB, sStaticMessage) {}
      Up(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sStaticMessage): ExceptionBase(pMemoryLifetimeOwner,sStaticMessage) {}
      Up(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFormat, const char *sParam1): ExceptionBase(pMemoryLifetimeOwner,sFormat, sParam1) {}

      const char *type() const {return "Up";}
    };
  )

  //---------------------------------------------------- Sockets
  class PipeFailure: public ExceptionBase {
  protected:
    const int m_iErrno;

    PipeFailure *clone_with_resources() const {return new PipeFailure(*this);}
    PipeFailure(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iErrno): m_iErrno(iErrno), ExceptionBase(pMemoryLifetimeOwner,"Pipe Failure [%s]", MMO_STRDUP(strerror(iErrno))) {}
  public:
    static PipeFailure *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iErrno);
    static PipeFailure *factory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sErrno) {return factory(pMemoryLifetimeOwner, atoi(sErrno));}
    virtual const char* what() const throw();
    const char *type() const {return "PipeFailure";}
  };

  class PipeFailure_Generic: public PipeFailure {
    friend class PipeFailure;
    PipeFailure_Generic *clone_with_resources() const {return new PipeFailure_Generic(*this);}
    PipeFailure_Generic(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iErrno): PipeFailure(pMemoryLifetimeOwner, iErrno) {}
  };

  class PipeFailure_EACCES: public PipeFailure {
    friend class PipeFailure;
    PipeFailure_EACCES *clone_with_resources() const {return new PipeFailure_EACCES(*this);}
    PipeFailure_EACCES(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): PipeFailure(pMemoryLifetimeOwner, EACCES) {}
  };

  class PipeFailure_EPIPE: public PipeFailure {
    //Broken Pipe
    friend class PipeFailure;
    PipeFailure_EPIPE *clone_with_resources() const {return new PipeFailure_EPIPE(*this);}
    PipeFailure_EPIPE(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): PipeFailure(pMemoryLifetimeOwner,EPIPE) {}
  };

  /*
EAGAIN or EWOULDBLOCK
EBADF
ECONNRESET
EDESTADDRREQ
EFAULT
EINTR
EINVAL
EISCONN
EMSGSIZE
ENOBUFS
ENOMEM
ENOTCONN
ENOTSOCK
EOPNOTSUPP
*/

  class ProtocolNotSupported: public ExceptionBase {
  protected:
    ProtocolNotSupported *clone_with_resources() const {return new ProtocolNotSupported(*this);}
  public:
    ProtocolNotSupported(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sProtocol): ExceptionBase(pMemoryLifetimeOwner,"Protocol Not Supported [%s]", sProtocol) {}
    const char *type() const {return "ProtocolNotSupported";}
  };
  
  class CannotSendData: public ExceptionBase {
  protected:
    CannotSendData *clone_with_resources() const {return new CannotSendData(*this);}
  public:
    CannotSendData(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Cannot Send Data [%s]", sWhy) {}
    const char *type() const {return "CannotSendData";}
  };
  
  class CannotConnectSocket: public ExceptionBase {
  protected:
    CannotConnectSocket *clone_with_resources() const {return new CannotConnectSocket(*this);}
  public:
    CannotConnectSocket(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Cannot Connect Socket [%s]", sWhy) {}
    const char *type() const {return "CannotConnectSocket";}
  };

  class CannotCreateSocket: public ExceptionBase {
  protected:
    CannotCreateSocket *clone_with_resources() const {return new CannotCreateSocket(*this);}
  public:
    CannotCreateSocket(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Cannot Create Socket [%s]", sWhy) {}
    const char *type() const {return "CannotCreateSocket";}
  };

  class CannotResolveHostname: public ExceptionBase {
  protected:
    CannotResolveHostname *clone_with_resources() const {return new CannotResolveHostname(*this);}
  public:
    CannotResolveHostname(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sHostname = 0): ExceptionBase(pMemoryLifetimeOwner,"Cannot Resolve Hostname [%s]", sHostname) {}
    const char *type() const {return "CannotResolveHostname";}
  };

  class ServiceRequiresASingleClass: public ExceptionBase {
  protected:
    ServiceRequiresASingleClass *clone_with_resources() const {return new ServiceRequiresASingleClass(*this);}
  public:
    ServiceRequiresASingleClass(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Service Requires A Single Class [%s]", sWhy) {}
    const char *type() const {return "ServiceRequiresASingleClass";}
  };
  
  class ServiceAssignedSocketZero: public ExceptionBase {
    int m_iPort;
    int m_iErrno;
  protected:
    ServiceAssignedSocketZero *clone_with_resources() const {return new ServiceAssignedSocketZero(*this);}
  public:
    ServiceAssignedSocketZero(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Service Assigned Socket Zero [%s]", sReason) {}
    const char *type() const {return "ServiceAssignedSocketZero";}
  };
  
  class ServiceCannotListen: public ExceptionBase {
    int m_iPort;
    int m_iErrno;
  protected:
    ServiceCannotListen *clone_with_resources() const {return new ServiceCannotListen(*this);}
  public:
    ServiceCannotListen(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sReason = 0, const int iErrno = 0, const int iPort = 0);
    static const char* netStat(const int iPort);
    static const char* processes(const char *sProcessName);
    const char* what() const throw();
    const char *type() const {return "ServiceCannotListen";}
  };

  class ServicePortRequired: public ExceptionBase {
  protected:
    ServicePortRequired *clone_with_resources() const {return new ServicePortRequired(*this);}
  public:
    ServicePortRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sName): ExceptionBase(pMemoryLifetimeOwner,"Service Port Required [%s]", sName) {}
    const char *type() const {return "ServicePortRequired";}
  };

  class ServiceMIsRequired: public ExceptionBase {
  protected:
    ServiceMIsRequired *clone_with_resources() const {return new ServiceMIsRequired(*this);}
  public:
    ServiceMIsRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sName): ExceptionBase(pMemoryLifetimeOwner,"Service MIs Required [%s]", sName) {}
    const char *type() const {return "ServiceMIsRequired";}
  };

  class InvalidProtocol: public ExceptionBase {
  protected:
    InvalidProtocol *clone_with_resources() const {return new InvalidProtocol(*this);}
  public:
    InvalidProtocol(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sTextStream): ExceptionBase(pMemoryLifetimeOwner,"Invalid Protocol [%s]", sTextStream) {}
    const char *type() const {return "InvalidProtocol";}
  };

  class BlankInputStream: public ExceptionBase {
  protected:
    BlankInputStream *clone_with_resources() const {return new BlankInputStream(*this);}
  public:
    BlankInputStream(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Blank text input stream [%s]", sWhy) {}
    const char *type() const {return "BlankInputStream";}
  };

  class ZeroInputStream: public ExceptionBase {
  protected:
    ZeroInputStream *clone_with_resources() const {return new ZeroInputStream(*this);}
  public:
    ZeroInputStream(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"0x0 Text input stream [%s]", sWhy) {}
    const char *type() const {return "ZeroInputStream";}
  };

  class MultipleSocketErrors: public ExceptionBase {
  protected:
    MultipleSocketErrors *clone_with_resources() const {return new MultipleSocketErrors(*this);}
  public:
    MultipleSocketErrors(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sReason): ExceptionBase(pMemoryLifetimeOwner,"Multiple Socket Errors [%s]", sReason) {}
    const char *type() const {return "MultipleSocketErrors";}
  };

  //---------------------------------------------------- Server, Services and Requests
  class TestsFailed: public ExceptionBase {
  protected:
    TestsFailed *clone_with_resources() const {return new TestsFailed(*this);}
  public:
    TestsFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Tests Failed [%s]", sReason) {}
    const char *type() const {return "TestsFailed";}
  };

  class OutOfMemory: public ExceptionBase {
  protected:
    OutOfMemory *clone_with_resources() const {return new OutOfMemory(*this);}
  public:
    OutOfMemory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sReason): ExceptionBase(pMemoryLifetimeOwner,"Out Of Memory [%s]", sReason) {}
    const char *type() const {return "OutOfMemory";}
  };

  class SemaphoreFailure: public ExceptionBase {
  protected:
    SemaphoreFailure *clone_with_resources() const {return new SemaphoreFailure(*this);}
  public:
    SemaphoreFailure(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iErrno):   ExceptionBase(pMemoryLifetimeOwner,"Semaphore Failure [%s]", MMO_STRDUP(strerror(iErrno))) {}
    SemaphoreFailure(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sErrno): ExceptionBase(pMemoryLifetimeOwner,"Semaphore Failure [%s]", MMO_STRDUP(strerror(atoi(sErrno)))) {}
    const char *type() const {return "SemaphoreFailure";}
  };

  class MIPrimaryOutputTransformationRequired: public ExceptionBase {
  protected:
    MIPrimaryOutputTransformationRequired *clone_with_resources() const {return new MIPrimaryOutputTransformationRequired(*this);}
  public:
    MIPrimaryOutputTransformationRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"Message Interpretation Primary Output Transformation Required [%s]", sName) {}
    const char *type() const {return "MIPrimaryOutputTransformationRequired";}
  };

  class MIMessageTransformationRequired: public ExceptionBase {
  protected:
    MIMessageTransformationRequired *clone_with_resources() const {return new MIMessageTransformationRequired(*this);}
  public:
    MIMessageTransformationRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"Message Interpretation Message Transform Required [%s]", sName) {}
    const char *type() const {return "MIMessageTransformationRequired";}
  };

  class ConversationFailedToRegularXInput: public ExceptionBase {
  protected:
    ConversationFailedToRegularXInput *clone_with_resources() const {return new ConversationFailedToRegularXInput(*this);}
  public:
    ConversationFailedToRegularXInput(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sInputType, const char* sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Request Failed To RegularX Input [%s %s]", sInputType, sReason ) {}
    const char *type() const {return "ConversationFailedToRegularXInput";}
  };

  class FileExists: public ExceptionBase {
  protected:
    FileExists *clone_with_resources() const {return new FileExists(*this);}
  public:
    FileExists(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFullPath): ExceptionBase(pMemoryLifetimeOwner,"File Exists [%s]", sFullPath) {}
    const char *type() const {return "FileExists";}
  };

  class FileNotFound: public ExceptionBase {
  protected:
    FileNotFound *clone_with_resources() const {return new FileNotFound(*this);}
  public:
    FileNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFullPath): ExceptionBase(pMemoryLifetimeOwner,"File Not Found [%s]", sFullPath) {}
    const char *type() const {return "FileNotFound";}
  };

  class DirectoryNotFound: public ExceptionBase {
  protected:
    DirectoryNotFound *clone_with_resources() const {return new DirectoryNotFound(*this);}
  public:
    DirectoryNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFullPath): ExceptionBase(pMemoryLifetimeOwner,"Directory Not Found [%s]", sFullPath) {}
    const char *type() const {return "DirectoryNotFound";}
  };

  class MissingPathToFile: public ExceptionBase {
  protected:
    MissingPathToFile *clone_with_resources() const {return new MissingPathToFile(*this);}
  public:
    MissingPathToFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Missing Path To File [%s]", sWhy ) {}
    const char *type() const {return "MissingPathToFile";}
  };

  class LoginFailed: public ExceptionBase {
  protected:
    LoginFailed *clone_with_resources() const {return new LoginFailed(*this);}
  public:
    LoginFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Login Failed [%s]", sWhy) {}
    const char *type() const {return "LoginFailed";}
  };

  class NoDatabaseSelected: public ExceptionBase {
  protected:
    NoDatabaseSelected *clone_with_resources() const {return new NoDatabaseSelected(*this);}
  public:
    NoDatabaseSelected(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"No Database Selected [%s]", sWhy) {}
    const char *type() const {return "NoDatabaseSelected";}
  };

  class DatabaseNodeServerObjectHasNoXSDName: public ExceptionBase {
  protected:
    DatabaseNodeServerObjectHasNoXSDName *clone_with_resources() const {return new DatabaseNodeServerObjectHasNoXSDName(*this);}
  public:
    DatabaseNodeServerObjectHasNoXSDName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"DatabaseNodeServerObject has no XSD name [%s]", sWhy) {}
    const char *type() const {return "DatabaseNodeServerObjectHasNoXSDName";}
  };

  class NoServersFoundInMainDatabase: public ExceptionBase {
  protected:
    NoServersFoundInMainDatabase *clone_with_resources() const {return new NoServersFoundInMainDatabase(*this);}
  public:
    NoServersFoundInMainDatabase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sElementName): ExceptionBase(pMemoryLifetimeOwner,"No Servers Found In Main Database [%s]", sElementName) {}
    const char *type() const {return "NoServersFoundInMainDatabase";}
  };

  class RepositoryTypeUnknown: public ExceptionBase {
  protected:
    RepositoryTypeUnknown *clone_with_resources() const {return new RepositoryTypeUnknown(*this);}
  public:
    RepositoryTypeUnknown(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sElementName): ExceptionBase(pMemoryLifetimeOwner,"Repository Type Unknown [%s]", sElementName) {}
    const char *type() const {return "RepositoryTypeUnknown";}
  };

  class RepositoryTypeMissing: public ExceptionBase {
  protected:
    RepositoryTypeMissing *clone_with_resources() const {return new RepositoryTypeMissing(*this);}
  public:
    RepositoryTypeMissing(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Repository Type Missing [%s]", sWhy) {}
    const char *type() const {return "RepositoryTypeMissing";}
  };

  class MemoryBlockNotFound: public ExceptionBase {
  protected:
    MemoryBlockNotFound *clone_with_resources() const {return new MemoryBlockNotFound(*this);}
  public:
    MemoryBlockNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage): ExceptionBase(pMemoryLifetimeOwner,"Memory Block Not Found [%s]", sMessage) {}
    const char *type() const {return "MemoryBlockNotFound";}
  };

  class ThreadTypeUnknown: public ExceptionBase {
  protected:
    ThreadTypeUnknown *clone_with_resources() const {return new ThreadTypeUnknown(*this);}
  public:
    ThreadTypeUnknown(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sOwner): ExceptionBase(pMemoryLifetimeOwner,"Thread Type Unknown in [%s]", sOwner) {}
    const char *type() const {return "ThreadTypeUnknown";}
  };

  class DateFormatResultTooLong: public ExceptionBase {
  protected:
    DateFormatResultTooLong *clone_with_resources() const {return new DateFormatResultTooLong(*this);}
  public:
    DateFormatResultTooLong(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFormat): ExceptionBase(pMemoryLifetimeOwner,"Date Format Result Too Long [%s]", sFormat) {}
    const char *type() const {return "DateFormatResultTooLong";}
  };

  class FailedToParseDate: public ExceptionBase {
  protected:
    FailedToParseDate *clone_with_resources() const {return new FailedToParseDate(*this);}
  public:
    FailedToParseDate(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sValue): ExceptionBase(pMemoryLifetimeOwner,"Failed To Parse Date [%s]", sValue) {}
    const char *type() const {return "FailedToParseDate";}
  };


  //---------------------------------------------------- XML module
  class XJavaScriptFormatUnrecignised: public ExceptionBase {
  protected:
    XJavaScriptFormatUnrecignised *clone_with_resources() const {return new XJavaScriptFormatUnrecignised(*this);}
  public:
    XJavaScriptFormatUnrecignised(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sParameterInput): ExceptionBase(pMemoryLifetimeOwner,"XJavaScript Format Unrecignised [%s]", sParameterInput) {}
    const char *type() const {return "XJavaScriptFormatUnrecignised";}
  };

  class InputLibraryNodeFreed: public ExceptionBase {
  protected:
    InputLibraryNodeFreed *clone_with_resources() const {return new InputLibraryNodeFreed(*this);}
  public:
    InputLibraryNodeFreed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage = 0, const char* sLiteralParam1 = 0): 
      ExceptionBase(pMemoryLifetimeOwner,"Input Library Node Freed [%s]", sMessage, sLiteralParam1) {}
    const char *type() const {return "InputLibraryNodeFreed";}
  };
  
  class XmlIdRedefined: public ExceptionBase {
  protected:
    XmlIdRedefined *clone_with_resources() const {return new XmlIdRedefined(*this);}
  public:
    XmlIdRedefined(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sXmlId): ExceptionBase(pMemoryLifetimeOwner,"Xml Id Redefined [%s]", sXmlId) {}
    const char *type() const {return "XmlIdRedefined";}
  };
  
  class TextNodeMergedAndFreed: public InputLibraryNodeFreed {
  protected:
    TextNodeMergedAndFreed *clone_with_resources() const {return new TextNodeMergedAndFreed(*this);}
  public:
    TextNodeMergedAndFreed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage = 0): 
      InputLibraryNodeFreed(pMemoryLifetimeOwner, "Text Node Merged And Freed", sMessage) 
    {}
    const char *type() const {return "TextNodeMergedAndFreed";}
  };
  
  class StructuralChangeInappropriate: public NodeExceptionBase {
  protected:
    StructuralChangeInappropriate *clone_with_resources() const {return new StructuralChangeInappropriate(*this);}
  public:
    StructuralChangeInappropriate(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseNode* pNode = 0, const char *sMessage = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Structural Change Inappropriate [%s] [%s]", pNode, sMessage) {}
    const char *type() const {return "StructuralChangeInappropriate";}
  };
  
  class NoEMOAvailableForExtension: public ExceptionBase {
  protected:
    NoEMOAvailableForExtension *clone_with_resources() const {return new NoEMOAvailableForExtension(*this);}
  public:
    NoEMOAvailableForExtension(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFunctionName): ExceptionBase(pMemoryLifetimeOwner,"No EMO Available For Extension [%s]", sFunctionName) {}
    const char *type() const {return "NoEMOAvailableForExtension";}
  };

  class NoFilterTargetDocument: public ExceptionBase {
  protected:
    NoFilterTargetDocument *clone_with_resources() const {return new NoFilterTargetDocument(*this);}
  public:
    NoFilterTargetDocument(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"No Filter Target Document for creating nodes [%s]", sWhy) {}
    const char *type() const {return "NoFilterTargetDocument";}
  };
  
  class NoStylesheet: public ExceptionBase {
  protected:
    NoStylesheet *clone_with_resources() const {return new NoStylesheet(*this);}
  public:
    NoStylesheet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"No Stylesheet [%s]", sWhy) {}
    const char *type() const {return "NoStylesheet";}
  };
  
  class NodeMaskTypeNotRecognised: public ExceptionBase {
  protected:
    NodeMaskTypeNotRecognised *clone_with_resources() const {return new NodeMaskTypeNotRecognised(*this);}
  public:
    NodeMaskTypeNotRecognised(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sNodeMaskType = 0): ExceptionBase(pMemoryLifetimeOwner,"Node Mask Type Not Recognised [%s]", sNodeMaskType) {}
    const char *type() const {return "NodeMaskTypeNotRecognised";}
  };

  class MaskContextNotAvailableForArea: public ExceptionBase {
  protected:
    MaskContextNotAvailableForArea *clone_with_resources() const {return new MaskContextNotAvailableForArea(*this);}
  public:
    MaskContextNotAvailableForArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): ExceptionBase(pMemoryLifetimeOwner,"MaskContext Not Available For Area") {}
    MaskContextNotAvailableForArea(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy): ExceptionBase(pMemoryLifetimeOwner,"MaskContext Not Available For Area [%s]", sWhy) {}
    const char *type() const {return "MaskContextNotAvailableForArea";}
  };
  
  class NoOutputRootNode: public ExceptionBase {
  protected:
    NoOutputRootNode *clone_with_resources() const {return new NoOutputRootNode(*this);}
  public:
    NoOutputRootNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhich): ExceptionBase(pMemoryLifetimeOwner,"No Output Root Node for [%s]", sWhich) {}
    const char *type() const {return "NoOutputRootNode";}
  };

  class NoOutputNode: public ExceptionBase {
  protected:
    NoOutputNode *clone_with_resources() const {return new NoOutputNode(*this);}
  public:
    NoOutputNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"No Output Node [%s]", sWhy) {}
    const char *type() const {return "NoOutputNode";}
  };

  class MultipleRootNodes: public ExceptionBase {
  protected:
    MultipleRootNodes *clone_with_resources() const {return new MultipleRootNodes(*this);}
  public:
    MultipleRootNodes(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sAlias): ExceptionBase(pMemoryLifetimeOwner,"Multiple Root Nodes [%s]", sAlias) {}
    const char *type() const {return "MultipleRootNodes";}
  };
  
  class FailedToCompileStylesheet: public ExceptionBase {
  protected:
    FailedToCompileStylesheet *clone_with_resources() const {return new FailedToCompileStylesheet(*this);}
  public:
    FailedToCompileStylesheet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Failed To Compile Stylesheet [%s]", sWhy) {}
    const char *type() const {return "FailedToCompileStylesheet";}
  };
  
  class FailedToSetBreakpoint: public ExceptionBase {
  protected:
    FailedToSetBreakpoint *clone_with_resources() const {return new FailedToSetBreakpoint(*this);}
  public:
    FailedToSetBreakpoint(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Failed To Set Breakpoint [%s]", sWhy) {}
    const char *type() const {return "FailedToSetBreakpoint";}
  };
  
  class NoOutputDocument: public ExceptionBase {
  protected:
    NoOutputDocument *clone_with_resources() const {return new NoOutputDocument(*this);}
  public:
    NoOutputDocument(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Attempt to append output to NULL output document during server XSLT [%s]", sWhy) {}
    const char *type() const {return "NoOutputDocument";}
  };
  
  class DataSizeAbort: public ExceptionBase {
  protected:
    DataSizeAbort *clone_with_resources() const {return new DataSizeAbort(*this);}
  public:
    DataSizeAbort(const IMemoryLifetimeOwner *pMemoryLifetimeOwner,size_t iDataSize): ExceptionBase(pMemoryLifetimeOwner,"Return Data Size is too large [%s], this will cause computer to crash!: aborting", (int) iDataSize) {}
    DataSizeAbort(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sDataSize = 0): ExceptionBase(pMemoryLifetimeOwner,"Return Data Size is too large [%s], this will cause computer to crash!: aborting", atoi(sDataSize)) {}
    const char *type() const {return "DataSizeAbort";}
  };
  
  class BlankDocument: public ExceptionBase {
  protected:
    BlankDocument *clone_with_resources() const {return new BlankDocument(*this);}
  public:
    BlankDocument(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhich): ExceptionBase(pMemoryLifetimeOwner,"Blank Document [%s]", sWhich) {}
    const char *type() const {return "BlankDocument";}
  };
  
  class ZeroDocument: public ExceptionBase {
  protected:
    ZeroDocument *clone_with_resources() const {return new ZeroDocument(*this);}
  public:
    ZeroDocument(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhich): ExceptionBase(pMemoryLifetimeOwner,"Zero Document [%s]", sWhich) {}
    const char *type() const {return "ZeroDocument";}
  };
  
  class DocumentParseError: public ExceptionBase {
  protected:
    DocumentParseError *clone_with_resources() const {return new DocumentParseError(*this);}
  public:
    DocumentParseError(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhich): ExceptionBase(pMemoryLifetimeOwner,"Document Parse Error [%s]", sWhich) {}
    const char *type() const {return "DocumentParseError";}
  };
  
  class ResponseTagMalformed: public ExceptionBase {
  protected:
    ResponseTagMalformed *clone_with_resources() const {return new ResponseTagMalformed(*this);}
  public:
    ResponseTagMalformed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sHint): ExceptionBase(pMemoryLifetimeOwner,"Response Tag Malformed [%s]", sHint) {}
    const char *type() const {return "ResponseTagMalformed";}
  };
  
  class CDATAEndNotFound: public ExceptionBase {
  protected:
    CDATAEndNotFound *clone_with_resources() const {return new CDATAEndNotFound(*this);}
  public:
    CDATAEndNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sXML): ExceptionBase(pMemoryLifetimeOwner,"CDATA End Not Found [%s]", sXML) {}
    const char *type() const {return "CDATAEndNotFound";}
  };

  class AttributeRequired: public ExceptionBase {
  protected:
    AttributeRequired *clone_with_resources() const {return new AttributeRequired(*this);}
  public:
    AttributeRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sAttributeName, const char *sCommandSignature = 0): ExceptionBase(pMemoryLifetimeOwner,"Attribute Required [%s] in [%s]", sAttributeName, sCommandSignature) {}
    const char *type() const {return "AttributeRequired";}
  };

  class TransformContextRequired: public ExceptionBase {
  protected:
    TransformContextRequired *clone_with_resources() const {return new TransformContextRequired(*this);}
  public:
    TransformContextRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFor = "transform"): ExceptionBase(pMemoryLifetimeOwner,"TransformContext Required for [%s]", sFor) {}
    const char *type() const {return "TransformContextRequired";}
  };

  class XSLCommandNotSupported: public ExceptionBase {
  protected:
    XSLCommandNotSupported *clone_with_resources() const {return new XSLCommandNotSupported(*this);}
  public:
    XSLCommandNotSupported(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sCommandSignature): ExceptionBase(pMemoryLifetimeOwner,"XSL Command [%s] Not Supported", sCommandSignature) {}
    const char *type() const {return "XSLCommandNotSupported";}
  };
  
  class TransformWithoutCurrentNode: public ExceptionBase {
  protected:
    TransformWithoutCurrentNode *clone_with_resources() const {return new TransformWithoutCurrentNode(*this);}
  public:
    TransformWithoutCurrentNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Transform Without Current Node", sWhy) {}
    const char *type() const {return "TransformWithoutCurrentNode";}
  };
  
  class XPathReturnedEmptyResultSet: public ExceptionBase {
  protected:
    XPathReturnedEmptyResultSet *clone_with_resources() const {return new XPathReturnedEmptyResultSet(*this);}
  public:
    XPathReturnedEmptyResultSet(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sUsage, const char* sXpath = 0): ExceptionBase(pMemoryLifetimeOwner,"XPath Returned Empty Result Set [%s => %s]", sUsage, sXpath) {}
    const char *type() const {return "XPathReturnedEmptyResultSet";}
  };

  class DynamicValueCalculation: public ExceptionBase {
  protected:
    DynamicValueCalculation *clone_with_resources() const {return new DynamicValueCalculation(*this);}
  public:
    DynamicValueCalculation(const ExceptionBase &outerEB, const char* sDynamicString): 
      ExceptionBase(outerEB, "Dynamic Value Calculation [%s]", sDynamicString) 
    {}
    const char *type() const {return "DynamicValueCalculation";}
  };

  class XmlNodeListCannotContainNULL: public ExceptionBase {
  protected:
    XmlNodeListCannotContainNULL *clone_with_resources() const {return new XmlNodeListCannotContainNULL(*this);}
  public:
    XmlNodeListCannotContainNULL(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhat = 0): ExceptionBase(pMemoryLifetimeOwner,"XmlNode List Cannot Contain NULL [%s]", sWhat) {}
    const char *type() const {return "XmlNodeListCannotContainNULL";}
  };

  class XPathContextNodeRequired: public ExceptionBase {
  protected:
    XPathContextNodeRequired *clone_with_resources() const {return new XPathContextNodeRequired(*this);}
  public:
    XPathContextNodeRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFunctionSignature): ExceptionBase(pMemoryLifetimeOwner,"XPath Context Node Required [%s]", sFunctionSignature) {}
    const char *type() const {return "XPathContextNodeRequired";}
  };

  class ContextNodeMustHaveXMLID: public ExceptionBase {
  protected:
    ContextNodeMustHaveXMLID *clone_with_resources() const {return new ContextNodeMustHaveXMLID(*this);}
  public:
    ContextNodeMustHaveXMLID(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sFunctionSignature): ExceptionBase(pMemoryLifetimeOwner,"Context Node Must Have @xml:id [%s]", sFunctionSignature) {}
    const char *type() const {return "ContextNodeMustHaveXMLID";}
  };

  class XPathStringArgumentRequired: public ExceptionBase {
  protected:
    XPathStringArgumentRequired *clone_with_resources() const {return new XPathStringArgumentRequired(*this);}
  public:
    XPathStringArgumentRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* xpath): ExceptionBase(pMemoryLifetimeOwner,"XPath String Argument Required [%s]", xpath) {}
    const char *type() const {return "XPathStringArgumentRequired";}
  };

  class XPathBooleanArgumentRequired: public ExceptionBase {
  protected:
    XPathBooleanArgumentRequired *clone_with_resources() const {return new XPathBooleanArgumentRequired(*this);}
  public:
    XPathBooleanArgumentRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* xpath): ExceptionBase(pMemoryLifetimeOwner,"XPath Boolean Argument Required [%s]", xpath) {}
    const char *type() const {return "XPathBooleanArgumentRequired";}
  };

  class XPathArgumentCountWrong: public ExceptionBase {
  protected:
    XPathArgumentCountWrong *clone_with_resources() const {return new XPathArgumentCountWrong(*this);}
  public:
    XPathArgumentCountWrong(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* xpath): ExceptionBase(pMemoryLifetimeOwner,"XPath Argument Count Wrong [%s]", xpath) {}
    const char *type() const {return "XPathArgumentCountWrong";}
  };

  class XPathTooFewArguments: public ExceptionBase {
  protected:
    XPathTooFewArguments *clone_with_resources() const {return new XPathTooFewArguments(*this);}
  public:
    XPathTooFewArguments(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* xpath): ExceptionBase(pMemoryLifetimeOwner,"XPath Too Few Arguments [%s]", xpath) {}
    const char *type() const {return "XPathTooFewArguments";}
  };

  class XPathTooManyArguments: public ExceptionBase {
  protected:
    XPathTooManyArguments *clone_with_resources() const {return new XPathTooManyArguments(*this);}
  public:
    XPathTooManyArguments(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* xpath): ExceptionBase(pMemoryLifetimeOwner,"XPath Too Many Arguments [%s]", xpath) {}
    const char *type() const {return "XPathTooManyArguments";}
  };

  class XPathFunctionWrongArgumentType: public ExceptionBase {
    //TODO: accept XPATH_type, not strings
  protected:
    XPathFunctionWrongArgumentType *clone_with_resources() const {return new XPathFunctionWrongArgumentType(*this);}
  public:
    XPathFunctionWrongArgumentType(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sTypeRequired, const char* sTypeGot = 0, const char *sFunctionSignature = 0): ExceptionBase(pMemoryLifetimeOwner,"XPath Function Wrong Argument Type: wanted [%s], got [%s] in [%s]", sTypeRequired, sTypeGot, sFunctionSignature) {}
    const char *type() const {return "XPathFunctionWrongArgumentType";}
  };

  class XPathTypeUnhandled: public ExceptionBase {
  protected:
    XPathTypeUnhandled *clone_with_resources() const {return new XPathTypeUnhandled(*this);}
  public:
    XPathTypeUnhandled(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iType): ExceptionBase(pMemoryLifetimeOwner,"XPath Type Unhandled [%i]", iType) {}
    XPathTypeUnhandled(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sType = 0): ExceptionBase(pMemoryLifetimeOwner,"XPath Type Unhandled [%s]", sType) {}
    const char *type() const {return "XPathTypeUnhandled";}
  };

  class InvalidXMLName: public ExceptionBase {
  protected:
    InvalidXMLName *clone_with_resources() const {return new InvalidXMLName(*this);}
  public:
    InvalidXMLName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName = 0): ExceptionBase(pMemoryLifetimeOwner,"Invalid XML-Name [%s]", sName) {}
    const char *type() const {return "InvalidXMLName";}
  };

  class InvalidXML: public ExceptionBase {
  protected:
    InvalidXML *clone_with_resources() const {return new InvalidXML(*this);}
  public:
    InvalidXML(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sXML = 0): ExceptionBase(pMemoryLifetimeOwner,"Invalid XML [%s]", sXML) {}
    const char *type() const {return "InvalidXML";}
  };

  class ValidityCheckFailure: public ExceptionBase {
  protected:
    ValidityCheckFailure *clone_with_resources() const {return new ValidityCheckFailure(*this);}
  public:
    ValidityCheckFailure(const ExceptionBase &outerEB, const char* sContext): ExceptionBase(outerEB, "Validity Check Failure [%s]", sContext) {}
    ValidityCheckFailure(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sContext = 0): ExceptionBase(pMemoryLifetimeOwner,"Validity Check Failure [%s]", sContext) {}
    const char *type() const {return "ValidityCheckFailure";}
  };

  class CannotReconcileNamespaces: public NodeExceptionBase {
  protected:
    CannotReconcileNamespaces *clone_with_resources() const {return new CannotReconcileNamespaces(*this);}
  public:
    CannotReconcileNamespaces(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage, const IXmlBaseNode *pNode = 0, const char *sLiteralParam2 = 0, const char *sLiteralParam3 = 0): 
      NodeExceptionBase(pMemoryLifetimeOwner,"Cannot Reconcile Namespaces  [%s: %s%s]", pNode, MM_STRDUP(sMessage), sLiteralParam2, sLiteralParam3) {}
    const char *type() const {return "CannotReconcileNamespaces";}
  };

  class CannotReconcileNamespacePrefix: public CannotReconcileNamespaces {
  protected:
    CannotReconcileNamespacePrefix *clone_with_resources() const {return new CannotReconcileNamespacePrefix(*this);}
  public:
    CannotReconcileNamespacePrefix(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage, const IXmlBaseNode *pNode = 0, const char *sLiteralParam2 = 0): 
      CannotReconcileNamespaces(pMemoryLifetimeOwner, "Cannot Reconcile Namespace Prefix [%s: %s%s]", pNode, sMessage, sLiteralParam2) {}
    const char *type() const {return "CannotReconcileNamespacePrefix";}
  };

  class CannotReconcileDefaultNamespace: public CannotReconcileNamespaces {
  protected:
    CannotReconcileDefaultNamespace *clone_with_resources() const {return new CannotReconcileDefaultNamespace(*this);}
  public:
    CannotReconcileDefaultNamespace(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage, const IXmlBaseNode *pNode = 0, const char *sLiteralParam2 = 0): 
      CannotReconcileNamespaces(pMemoryLifetimeOwner,"Cannot Reconcile Default Namespace [%s: %s%s]", pNode, sMessage, sLiteralParam2) {}
    const char *type() const {return "CannotReconcileDefaultNamespace";}
  };

  class NamespaceIsInvalid: public ExceptionBase {
  protected:
    NamespaceIsInvalid *clone_with_resources() const {return new NamespaceIsInvalid(*this);}
  public:
    NamespaceIsInvalid(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeName, const char *sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Namespace Is Invalid [%s: %s]", sNodeName, sReason) {}
    const char *type() const {return "NamespaceIsInvalid";}
  };

  class NodeWithoutNamespace: public ExceptionBase {
  protected:
    NodeWithoutNamespace *clone_with_resources() const {return new NodeWithoutNamespace(*this);}
  public:
    NodeWithoutNamespace(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeName = 0): ExceptionBase(pMemoryLifetimeOwner,"Node Without Namespace [%s]", sNodeName) {}
    const char *type() const {return "NodeWithoutNamespace";}
  };

  class XSLTExtensionNotFound: public ExceptionBase {
  protected:
    XSLTExtensionNotFound *clone_with_resources() const {return new XSLTExtensionNotFound(*this);}
  public:
    XSLTExtensionNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNamespace, const char *sElement = 0): ExceptionBase(pMemoryLifetimeOwner,"XSLT Extension Not Found [%s: %s]", sNamespace, sElement) {}
    const char *type() const {return "XSLTExtensionNotFound";}
  };

  class HardLinkToNowhere: public ExceptionBase {
  protected:
    HardLinkToNowhere *clone_with_resources() const {return new HardLinkToNowhere(*this);}
  public:
    HardLinkToNowhere(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPath, const char *sParentXmlId = 0): ExceptionBase(pMemoryLifetimeOwner,"Hard Link To Nowhere [%s] under [%s]", sXPath, sParentXmlId) {}
    const char *type() const {return "HardLinkToNowhere";}
  };
  
  class IntendedAncestorBaseNodeNotTraversed: public ExceptionBase {
    const IXmlBaseNode *m_pTargetNode;
    const IXmlBaseNode *m_pBaseNode;
    
  public:
    IntendedAncestorBaseNodeNotTraversed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      const char *sMessage, 
      const IXmlBaseNode *pTargetNode = 0, 
      const IXmlBaseNode *pBaseNode   = 0,
      const char *sLiteralParam2 = 0, const char *sLiteralParam3 = 0);
    ~IntendedAncestorBaseNodeNotTraversed();
    
    IntendedAncestorBaseNodeNotTraversed *clone_with_resources() const;
    IntendedAncestorBaseNodeNotTraversed(const IntendedAncestorBaseNodeNotTraversed &neb);
    const char *type() const {return "IntendedAncestorBaseNodeNotTraversed";}
  };

  class DeviationOriginalNotFound: public ExceptionBase {
  protected:
    DeviationOriginalNotFound *clone_with_resources() const {return new DeviationOriginalNotFound(*this);}
  public:
    DeviationOriginalNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXmlID): ExceptionBase(pMemoryLifetimeOwner,"Deviation Original Not Found [%s]", sXmlID) {}
    const char *type() const {return "DeviationOriginalNotFound";}
  };

  class DeviationDeviantNotFound: public ExceptionBase {
  protected:
    DeviationDeviantNotFound *clone_with_resources() const {return new DeviationDeviantNotFound(*this);}
  public:
    DeviationDeviantNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXmlID): ExceptionBase(pMemoryLifetimeOwner,"Deviation Deviant Not Found [%s]", sXmlID) {}
    const char *type() const {return "DeviationDeviantNotFound";}
  };
  
  class SessionNodeSetOutOfRange: public ExceptionBase {
  protected:
    SessionNodeSetOutOfRange *clone_with_resources() const {return new SessionNodeSetOutOfRange(*this);}
  public:
    SessionNodeSetOutOfRange(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Session Node Set Out Of Range", sWhy) {}
    const char *type() const {return "SessionNodeSetOutOfRange";}
  };  

  class FailedToResolveXSLIncludePath: public ExceptionBase {
  protected:
    FailedToResolveXSLIncludePath *clone_with_resources() const {return new FailedToResolveXSLIncludePath(*this);}
  public:
    FailedToResolveXSLIncludePath(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPath = 0): ExceptionBase(pMemoryLifetimeOwner,"Failed To Resolve XSL Include Path [%s]", sXPath) {}
    const char *type() const {return "FailedToResolveXSLIncludePath";}
  };

  class CannotConvertAbsoluteHREFToXPathWithoutRoot: public ExceptionBase {
  protected:
    CannotConvertAbsoluteHREFToXPathWithoutRoot *clone_with_resources() const {return new CannotConvertAbsoluteHREFToXPathWithoutRoot(*this);}
  public:
    CannotConvertAbsoluteHREFToXPathWithoutRoot(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPath): ExceptionBase(pMemoryLifetimeOwner,"Cannot Convert Absolute HREF To XPath Without Root [%s]", sXPath) {}
    const char *type() const {return "CannotConvertAbsoluteHREFToXPathWithoutRoot";}
  };

  class CannotRunXPathWithoutNamespaceCache: public ExceptionBase {
  protected:
    CannotRunXPathWithoutNamespaceCache *clone_with_resources() const {return new CannotRunXPathWithoutNamespaceCache(*this);}
  public:
    CannotRunXPathWithoutNamespaceCache(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Cannot Run XPath Without Namespace Cache", sWhy) {}
    const char *type() const {return "CannotRunXPathWithoutNamespaceCache";}
  };

  class DocumentMismatchForNamespaceCacheCopyOperation: public ExceptionBase {
  protected:
    DocumentMismatchForNamespaceCacheCopyOperation *clone_with_resources() const {return new DocumentMismatchForNamespaceCacheCopyOperation(*this);}
  public:
    DocumentMismatchForNamespaceCacheCopyOperation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Document Mismatch For Namespace Cache Copy Operation", sWhy) {}
    const char *type() const {return "DocumentMismatchForNamespaceCacheCopyOperation";}
  };

  class MismatchedNamespacePrefixAssociation: public ExceptionBase {
  protected:
    MismatchedNamespacePrefixAssociation *clone_with_resources() const {return new MismatchedNamespacePrefixAssociation(*this);}
  public:
    MismatchedNamespacePrefixAssociation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sPrefix, const char *sHREF = 0, const char *sPath = 0): ExceptionBase(pMemoryLifetimeOwner,"Mismatched Namespace Prefix Association [%s] with [xmlns:%s=%s]", sPath, sPrefix, sHREF) {}
    const char *type() const {return "MismatchedNamespacePrefixAssociation";}
  };

  class MissingDefaultNamespaceDecleration: public ExceptionBase {
  protected:
    MissingDefaultNamespaceDecleration *clone_with_resources() const {return new MissingDefaultNamespaceDecleration(*this);}
  public:
    MissingDefaultNamespaceDecleration(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sHREF, const char *sPath = 0): ExceptionBase(pMemoryLifetimeOwner,"Missing Default Namespace Decleration [%s] for [%s]", sHREF, sPath) {}
    const char *type() const {return "MissingDefaultNamespaceDecleration";}
  };

  class XMLID_DuplicationIssue: public ExceptionBase {
  protected:
    XMLID_DuplicationIssue *clone_with_resources() const {return new XMLID_DuplicationIssue(*this);}
  public:
    XMLID_DuplicationIssue(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sReason): ExceptionBase(pMemoryLifetimeOwner,"xml:id Duplication Issue [%s]", sReason) {}
    const char *type() const {return "XMLID_DuplicationIssue";}
  };

  class XMLParseFailedAt: public ExceptionBase {
    XMLParseFailedAt *clone_with_resources() const {return new XMLParseFailedAt(*this);}
  public:
    XMLParseFailedAt(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const char* sCurrent = 0, const char* sXMLID = 0):
      ExceptionBase(pMemoryLifetimeOwner,"XML Parse Failed At [%s] with [%s] at [%s]", (sXMLID ? sXMLID : MM_STRDUP("unknown")), sMessage, sCurrent) {}
    const char *type() const {return "XMLParseFailedAt";}
  };

  class XSLStylesheetNodeNotFound: public ExceptionBase {
    XSLStylesheetNodeNotFound *clone_with_resources() const {return new XSLStylesheetNodeNotFound(*this);}
  public:
    XSLStylesheetNodeNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"XSL Stylesheet Node Not Found [%s]", sWhy) {}
    const char *type() const {return "XSLStylesheetNodeNotFound";}
  };

  class UnknownHardlinkPolicy: public ExceptionBase {
    UnknownHardlinkPolicy *clone_with_resources() const {return new UnknownHardlinkPolicy(*this);}
  public:
    UnknownHardlinkPolicy(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sPolicy):
      ExceptionBase(pMemoryLifetimeOwner,"Unknown Hardlink Policy [%s]", (sPolicy ? sPolicy : "(zero pointer)")) {}
    const char *type() const {return "UnknownHardlinkPolicy";}
  };


  //---------------------------------------------------- foregin systems
  class RelationalDatabaseConnectionFailed: public ExceptionBase {
    RelationalDatabaseConnectionFailed *clone_with_resources() const {return new RelationalDatabaseConnectionFailed(*this);}
  public:
    RelationalDatabaseConnectionFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sDatabaseType, const char* sError = 0):
      ExceptionBase(pMemoryLifetimeOwner,"Relational Database Connection [%s] Failed with [%s]", sDatabaseType, sError) {}
    const char *type() const {return "RelationalDatabaseConnectionFailed";}
  };


  //---------------------------------------------------- XSLT execution errors
  interface_class IXslTransformContext;
  interface_class IXslXPathFunctionContext;

  class XSLTException: public ExceptionBase {
    const IXmlBaseNode    *m_pInputNode;
    const IXmlBaseNode    *m_pCommandNode;
    const IXslCommandNode *m_pTemplateNode;
    const IXmlBaseNode    *m_pOutputNode;
    const char *m_sCurrentMode;
  protected:
    virtual XSLTException *clone_with_resources() const;
  public:
    XSLTException(const ExceptionBase &eb, const IXslTransformContext *pCtxt, const char *sErrorMessage = 0);
    XSLTException(const IXslTransformContext *pCtxt, const char *sErrorMessage = 0);
    XSLTException(const XSLTException& xte);
    virtual ~XSLTException() throw();
    virtual const char *type() const;
    virtual const char *contextXPath(const IXmlQueryEnvironment *pQE);
  };

  //these always include IXSLTransformContext which has the xml:id of the XSL command and more
  class XSLElementException: public ExceptionBase {
  protected:
    virtual XSLElementException *clone_with_resources() const;
  public:
    XSLElementException(const ExceptionBase &outerEB);
    virtual ~XSLElementException() throw();
    virtual const char *type() const {return "XSLElementException";}
  };

  class XPathException: public ExceptionBase {
    const char *m_sXPath;
    const char *m_sFunctionName;
    const char *m_sFunctionNamespace;
    const IXmlBaseNode *m_pCurrentNode;
    
  protected:
    virtual XPathException *clone_with_resources() const;
    
  public:
    XPathException(const ExceptionBase &outerEB, const IXslXPathFunctionContext *pXCtxt, const char *sErrorMessage = 0);
    XPathException(const IXslXPathFunctionContext *pXCtxt, const char *sErrorMessage = 0);
    XPathException(const XPathException& xpe);
    virtual ~XPathException() throw();
    virtual const char *type() const {return "XPathException";}
  };

  class XSLTThrow: public ExceptionBase {
  protected:
    XSLTThrow *clone_with_resources() const {return new XSLTThrow(*this);}
  public:
    XSLTThrow(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sParameter = 0): ExceptionBase(pMemoryLifetimeOwner,"XSLT Throw [%s]", sParameter) {}
    const char *type() const {return "XSLTThrow";}
  };
  
  class SubPositioningOnNonElement: public NodeExceptionBase {
  protected:
    SubPositioningOnNonElement *clone_with_resources() const {return new SubPositioningOnNonElement(*this);}
  public:
    SubPositioningOnNonElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Sub Positioning On Non Element [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "SubPositioningOnNonElement";}
  };
  
  class XPathNot1Nodes: public ExceptionBase {
  protected:
    XPathNot1Nodes *clone_with_resources() const {return new XPathNot1Nodes(*this);}
  public:
    XPathNot1Nodes(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFunctionName = 0): ExceptionBase(pMemoryLifetimeOwner,"XPath Multiple Nodes in function [%s]", sFunctionName ? sFunctionName : MM_STRDUP("not specified")) {}
    const char *type() const {return "XPathNot1Nodes";}
  };

  class XPathStringNot1Nodes: public NodeExceptionBase {
  protected:
    XPathStringNot1Nodes *clone_with_resources() const {return new XPathStringNot1Nodes(*this);}
  public:
    XPathStringNot1Nodes(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"XPath String returned not 1 Nodes [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "XPathStringNot1Nodes";}
  };

  class PositionOutOfRange: public ExceptionBase {
  protected:
    PositionOutOfRange *clone_with_resources() const {return new PositionOutOfRange(*this);}
  public:
    PositionOutOfRange(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const int iPosition): ExceptionBase(pMemoryLifetimeOwner,"Position Out Of Range [%s]", iPosition) {}
    PositionOutOfRange(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sPosition = 0): ExceptionBase(pMemoryLifetimeOwner,"Position Out Of Range [%s]", sPosition) {}
    const char *type() const {return "Position Out Of Range";}
  };

  class CommandNodeRequired: public ExceptionBase {
  protected:
    CommandNodeRequired *clone_with_resources() const {return new CommandNodeRequired(*this);}
  public:
    CommandNodeRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage): ExceptionBase(pMemoryLifetimeOwner,"Command Node Required [%s]", sMessage) {}
    const char *type() const {return "Command Node Required";}
  };

  //---------------------------------------------------- Resources and dynamic XSL throws
  class ExceptionClassRequired: public ExceptionBase {
  protected:
    ExceptionClassRequired *clone_with_resources() const {return new ExceptionClassRequired(*this);}
  public:
    ExceptionClassRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Exception Class Required [%s]", sWhy) {}
    const char *type() const {return "ExceptionClassRequired";}
  };
  
  class ExceptionClassNotAllowed: public ExceptionBase {
  protected:
    ExceptionClassNotAllowed *clone_with_resources() const {return new ExceptionClassNotAllowed(*this);}
  public:
    ExceptionClassNotAllowed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClassName = 0): ExceptionBase(pMemoryLifetimeOwner,"Exception Class Not Allowed [%s]", sClassName) {}
    const char *type() const {return "ExceptionClassNotAllowed";}
  };
  
  class MultipleResourceFound: public ExceptionBase {
  protected:
    MultipleResourceFound *clone_with_resources() const {return new MultipleResourceFound(*this);}
  public:
    const char *type() const {return "MultipleResourceFound";}
    MultipleResourceFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sURL): ExceptionBase(pMemoryLifetimeOwner,"Multiple Resource Found [%s]", sURL) {}
  };
  
  class ResourceNotFound: public ExceptionBase {
  protected:
    ResourceNotFound *clone_with_resources() const {return new ResourceNotFound(*this);}
  public:
    const char *type() const {return "ResourceNotFound";}
    ResourceNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sURL): ExceptionBase(pMemoryLifetimeOwner,"Resource Not Found [%s]", sURL) {}
  };


  //---------------------------------------------------- Security
  class UserStoreNotFound: public ExceptionBase {
  protected:
    UserStoreNotFound *clone_with_resources() const {return new UserStoreNotFound(*this);}
  public:
    const char *type() const {return "UserStoreNotFound";}
    UserStoreNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPathToStore): ExceptionBase(pMemoryLifetimeOwner,"User Store Not Found [%s]", sXPathToStore) {}
  };

  class UnkownSecurityStrategyName: public ExceptionBase {
  protected:
    UnkownSecurityStrategyName *clone_with_resources() const {return new UnkownSecurityStrategyName(*this);}
  public:
    const char *type() const {return "UnkownSecurityStrategyName";}
    UnkownSecurityStrategyName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sName): ExceptionBase(pMemoryLifetimeOwner,"Unkown Security Strategy Name [%s]", sName) {}
  };

  class WrongIssuerSecurityContext: public ExceptionBase {
  protected:
    WrongIssuerSecurityContext *clone_with_resources() const {return new WrongIssuerSecurityContext(*this);}
  public:
    const char *type() const {return "WrongIssuerSecurityContext";}
    WrongIssuerSecurityContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Wrong Issuer SecurityContext [%s]", sWhy) {}
  };

  class UserDoesNotHavePermission: public ExceptionBase {
  protected:
    UserDoesNotHavePermission *clone_with_resources() const {return new UserDoesNotHavePermission(*this);}
  public:
    const char *type() const {return "UserDoesNotHavePermission";}
    UserDoesNotHavePermission(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sPermission): ExceptionBase(pMemoryLifetimeOwner,"User Does Not Have Permission [%s]", sPermission) {}
  };

  
  class SessionsCollectionRequired: public ExceptionBase {
  protected:
    SessionsCollectionRequired *clone_with_resources() const {return new SessionsCollectionRequired(*this);}
  public:
    SessionsCollectionRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Sessions Collection Required [%s]", sReason) {}
    const char *type() const {return "SessionsCollectionRequired";}
  };
  
  class SessionRequired: public ExceptionBase {
  protected:
    SessionRequired *clone_with_resources() const {return new SessionRequired(*this);}
  public:
    const char *type() const {return "SessionRequired";}
    SessionRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sReason = 0): ExceptionBase(pMemoryLifetimeOwner,"Session Required [%s]", "undefined reason") {}
  };

  class NodeWriteableAccessDenied: public ExceptionBase {
  protected:
    NodeWriteableAccessDenied *clone_with_resources() const {return new NodeWriteableAccessDenied(*this);}
  public:
    const char *type() const {return "NodeWriteableAccessDenied";}
    NodeWriteableAccessDenied(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Node Writeable Access Denied [%s]", sWhy) {}
  };

  class NodeAddableAccessDenied: public ExceptionBase {
  protected:
    NodeAddableAccessDenied *clone_with_resources() const {return new NodeAddableAccessDenied(*this);}
  public:
    const char *type() const {return "NodeAddableAccessDenied";}
    NodeAddableAccessDenied(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Node Addable Access Denied [%s]", sWhy) {}
  };


  //---------------------------------------------------- TXML module
  class TXmlUnknownTransactionType: public ExceptionBase {
  protected:
    TXmlUnknownTransactionType *clone_with_resources() const {return new TXmlUnknownTransactionType(*this);}
  public:
    TXmlUnknownTransactionType(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sBadType = 0): ExceptionBase(pMemoryLifetimeOwner,"TXml Unknown Transaction Type [%s]", sBadType) {}
    const char *type() const {return "TXmlUnknownTransactionType";}
  };
  
  class TXmlDestinationMustBeElement: public ExceptionBase {
  protected:
    TXmlDestinationMustBeElement *clone_with_resources() const {return new TXmlDestinationMustBeElement(*this);}
  public:
    TXmlDestinationMustBeElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sBadType = 0): ExceptionBase(pMemoryLifetimeOwner,"TXmlDestination Must Be Element [%s]", sBadType) {}
    const char *type() const {return "TXmlDestinationMustBeElement";}
  };
  
  class TXmlFailedToParseTransactionXML: public ExceptionBase {
  protected:
    TXmlFailedToParseTransactionXML *clone_with_resources() const {return new TXmlFailedToParseTransactionXML(*this);}
  public:
    TXmlFailedToParseTransactionXML(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sXML): ExceptionBase(pMemoryLifetimeOwner,"TXml Failed To Parse Transaction XML [%s]", sXML) {}
    const char *type() const {return "TXmlFailedToParseTransactionXML";}
  };

  class TXmlSelectNodeRequired: public ExceptionBase {
  protected:
    TXmlSelectNodeRequired *clone_with_resources() const {return new TXmlSelectNodeRequired(*this);}
  public:
    TXmlSelectNodeRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner, MM_STRDUP("TXml Select Node Required [%s]")) {}
    const char *type() const {return "TXmlSelectNodeRequired";}
  };

  class TXmlRequiredNodeNotFound: public ExceptionBase {
  protected:
    TXmlRequiredNodeNotFound *clone_with_resources() const {return new TXmlRequiredNodeNotFound(*this);}
  public:
    TXmlRequiredNodeNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sNodeType, const char *sDocContext = 0): ExceptionBase(pMemoryLifetimeOwner,"TXml Required Node Not Found [%s] in Doc [%s]", sNodeType, sDocContext) {}
    const char *type() const {return "TXmlRequiredNodeNotFound";}
  };

  class TXmlCannotOpenTransactionStore: public ExceptionBase {
  protected:
    TXmlCannotOpenTransactionStore *clone_with_resources() const {return new TXmlCannotOpenTransactionStore(*this);}
  public:
    TXmlCannotOpenTransactionStore(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sContext): ExceptionBase(pMemoryLifetimeOwner,"TXml Cannot Open Transaction Store [%s]", sContext) {}
    const char *type() const {return "TXmlCannotOpenTransactionStore";}
  };

  class TXmlSerialisationParentNull: public ExceptionBase {
  protected:
    TXmlSerialisationParentNull *clone_with_resources() const {return new TXmlSerialisationParentNull(*this);}
  public:
    TXmlSerialisationParentNull(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sContext): ExceptionBase(pMemoryLifetimeOwner,"TXml Serialisation Parent Null [%s]", sContext) {}
    const char *type() const {return "TXmlSerialisationParentNull";}
  };

  class TXmlInvalid: public ExceptionBase {
  protected:
    TXmlInvalid *clone_with_resources() const {return new TXmlInvalid(*this);}
  public:
    TXmlInvalid(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sBadType = 0): ExceptionBase(pMemoryLifetimeOwner,"TXml Invalid [%s]", sBadType) {}
    const char *type() const {return "TXmlInvalid";}
  };

  class TXmlVolatileResult: public ExceptionBase {
  protected:
    TXmlVolatileResult *clone_with_resources() const {return new TXmlVolatileResult(*this);}
  public:
    TXmlVolatileResult(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"TXml Volatile Result [%s]", sWhy) {}
    const char *type() const {return "TXmlVolatileResult";}
  };

  class DynamicPathToNonRepositoryNode: public ExceptionBase {
  protected:
    DynamicPathToNonRepositoryNode *clone_with_resources() const {return new DynamicPathToNonRepositoryNode(*this);}
  public:
    DynamicPathToNonRepositoryNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPathToNode): ExceptionBase(pMemoryLifetimeOwner,"Dynamic Path To Non-Repository Node [%s]", sXPathToNode) {}
    const char *type() const {return "DynamicPathToNonRepositoryNode";}
  };

  class SubRepositoryNotFoundForNode: public ExceptionBase {
  protected:
    SubRepositoryNotFoundForNode *clone_with_resources() const {return new SubRepositoryNotFoundForNode(*this);}
  public:
    SubRepositoryNotFoundForNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sXPathToNode): ExceptionBase(pMemoryLifetimeOwner,"Sub-Repository Not Found For Node [%s]", sXPathToNode) {}
    const char *type() const {return "SubRepositoryNotFoundForNode";}
  };

  class TXmlTransformNotSupportedFor: public ExceptionBase {
  protected:
    TXmlTransformNotSupportedFor *clone_with_resources() const {return new TXmlTransformNotSupportedFor(*this);}
  public:
    TXmlTransformNotSupportedFor(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sUpdateType): ExceptionBase(pMemoryLifetimeOwner,"TXml Transform Not Supported For [%s]", sUpdateType) {}
    const char *type() const {return "TXmlTransformNotSupportedFor";}
  };

  class TXmlApplicationFailed: public ExceptionBase {
    const TXml *m_pTXml;
  protected:
    TXmlApplicationFailed *clone_with_resources() const {return new TXmlApplicationFailed(*this);}
  public:
    TXmlApplicationFailed(const ExceptionBase &eb, const TXml *pTXml);
    TXmlApplicationFailed(const TXmlApplicationFailed &eb);
    ~TXmlApplicationFailed() throw();

    const char *type() const {return "TXmlApplicationFailed";}
    const char *what() const throw();
  };


  //commits
  class FailedToRemoveOldItem: public ExceptionBase {
  protected:
    FailedToRemoveOldItem *clone_with_resources() const {return new FailedToRemoveOldItem(*this);}
  public:
    FailedToRemoveOldItem(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sToFullPath, const int iErrno = 0): ExceptionBase(pMemoryLifetimeOwner,"Failed To Remove Old Item (%s) [%s]", MMO_STRDUP(strerror(iErrno)), sToFullPath) {}
    const char *type() const {return "FailedToRemoveOldItem";}
  };

  class FailedToSwapInPlaceholder: public ExceptionBase {
  protected:
    FailedToSwapInPlaceholder *clone_with_resources() const {return new FailedToSwapInPlaceholder(*this);}
  public:
    FailedToSwapInPlaceholder(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFromFullPath, const char *sToFullPath = 0, const int iErrno = 0): ExceptionBase(pMemoryLifetimeOwner,"Failed To Swap In Placeholder (%s) for [%s] -> [%s]", MMO_STRDUP(strerror(iErrno)), sFromFullPath, sToFullPath) {}
    const char *type() const {return "FailedToSwapInPlaceholder";}
  };

  class FailedToOpenSyncDirectory: public ExceptionBase {
  protected:
    FailedToOpenSyncDirectory *clone_with_resources() const {return new FailedToOpenSyncDirectory(*this);}
  public:
    FailedToOpenSyncDirectory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFullPath): ExceptionBase(pMemoryLifetimeOwner,"Failed To Open Sync Directory [%s]", sFullPath) {}
    const char *type() const {return "FailedToOpenSyncDirectory";}
  };

  class UnknownFileType: public ExceptionBase {
  protected:
    UnknownFileType *clone_with_resources() const {return new UnknownFileType(*this);}
  public:
    UnknownFileType(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFullPath): ExceptionBase(pMemoryLifetimeOwner,"Unknown File Type [%s]", sFullPath) {}
    const char *type() const {return "UnknownFileType";}
  };

  class MissingRepositoryNameAttribute: public ExceptionBase {
  protected:
    MissingRepositoryNameAttribute *clone_with_resources() const {return new MissingRepositoryNameAttribute(*this);}
  public:
    MissingRepositoryNameAttribute(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFullPath): ExceptionBase(pMemoryLifetimeOwner,"Missing Repository Name Attribute [%s]", sFullPath) {}
    const char *type() const {return "MissingRepositoryNameAttribute";}
  };

  class ResourcesCopyFailure: public ExceptionBase {
  protected:
    ResourcesCopyFailure *clone_with_resources() const {return new ResourcesCopyFailure(*this);}
  public:
    ResourcesCopyFailure(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sFromPath, const char *sToPath = 0, int iErrno = 0): ExceptionBase(pMemoryLifetimeOwner,"Resources Copy Failure (%s) [%s] -> [%s]", MMO_STRDUP(strerror(iErrno)), sFromPath, sToPath) {}
    const char *type() const {return "ResourcesCopyFailure";}
  };

  class InvalidCommitStyle: public ExceptionBase {
  protected:
    InvalidCommitStyle *clone_with_resources() const {return new InvalidCommitStyle(*this);}
  public:
    InvalidCommitStyle(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *ct): ExceptionBase(pMemoryLifetimeOwner,"Invalid Commit Style [%s]", ct) {}
    const char *type() const {return "InvalidCommitStyle";}
  };

  class InvalidCommitStatus: public ExceptionBase {
  protected:
    InvalidCommitStatus *clone_with_resources() const {return new InvalidCommitStatus(*this);}
  public:
    InvalidCommitStatus(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *cs): ExceptionBase(pMemoryLifetimeOwner,"Invalid Commit Status [%s]", cs) {}
    const char *type() const {return "InvalidCommitStatus";}
  };

  class InvalidCommitStyleName: public ExceptionBase {
  protected:
    InvalidCommitStyleName *clone_with_resources() const {return new InvalidCommitStyleName(*this);}
  public:
    InvalidCommitStyleName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *ct): ExceptionBase(pMemoryLifetimeOwner,"Invalid Commit Style Name [%s]", ct) {}
    const char *type() const {return "InvalidCommitStyleName";}
  };

  class InvalidCommitStatusName: public ExceptionBase {
  protected:
    InvalidCommitStatusName *clone_with_resources() const {return new InvalidCommitStatusName(*this);}
  public:
    InvalidCommitStatusName(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *cs): ExceptionBase(pMemoryLifetimeOwner,"Invalid Commit Status Name [%s]", cs) {}
    const char *type() const {return "InvalidCommitStatusName";}
  };



  //---------------------------------------------------- Database
  class UnregisteredDatabaseClass: public ExceptionBase {
  protected:
    UnregisteredDatabaseClass *clone_with_resources() const {return new UnregisteredDatabaseClass(*this);}
  public:
    UnregisteredDatabaseClass(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sClassName): ExceptionBase(pMemoryLifetimeOwner,"Unregistered Database Class [%s]", sClassName) {}
    const char *type() const {return "UnregisteredDatabaseClass";}
  };

  class ClassElementsMustBeElement: public ExceptionBase {
  protected:
    ClassElementsMustBeElement *clone_with_resources() const {return new ClassElementsMustBeElement(*this);}
  public:
    ClassElementsMustBeElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sElementsXPath): ExceptionBase(pMemoryLifetimeOwner,"Class Elements Must Be Element [%s]", sElementsXPath) {}
    const char *type() const {return "ClassElementsMustBeElement";}
  };

  class TooManyDatabaseClassOnElement: public ExceptionBase {
  protected:
    TooManyDatabaseClassOnElement *clone_with_resources() const {return new TooManyDatabaseClassOnElement(*this);}
  public:
    TooManyDatabaseClassOnElement(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sElementName): ExceptionBase(pMemoryLifetimeOwner,"Too Many DatabaseClass On Element [%s]", sElementName) {}
    const char *type() const {return "TooManyDatabaseClassOnElement";}
  };

  class DatabaseClassNameRequired: public ExceptionBase {
  protected:
    DatabaseClassNameRequired *clone_with_resources() const {return new DatabaseClassNameRequired(*this);}
  public:
    DatabaseClassNameRequired(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"Database ClassName Required [%s]", sWhy) {}
    const char *type() const {return "DatabaseClassNameRequired";}
  };
  
  class DatabaseMatchFailed: public ExceptionBase {
  protected:
    DatabaseMatchFailed *clone_with_resources() const {return new DatabaseMatchFailed(*this);}
  public:
    DatabaseMatchFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClassName, const char *sWhy = 0): ExceptionBase(pMemoryLifetimeOwner,"DatabaseClass [%s] Match Failed [%s]", sClassName, sWhy) {}
    const char *type() const {return "DatabaseMatchFailed";}
  };
  
  class DatabaseClassNameXmlIDWrong: public ExceptionBase {
  protected:
    DatabaseClassNameXmlIDWrong *clone_with_resources() const {return new DatabaseClassNameXmlIDWrong(*this);}
  public:
    DatabaseClassNameXmlIDWrong(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClassName): ExceptionBase(pMemoryLifetimeOwner,"Database @xml:id Wrong [%s]", sClassName) {}
    const char *type() const {return "DatabaseMatchFailed";}
  };

  class DatabaseClassNameWrong: public ExceptionBase {
  protected:
    DatabaseClassNameWrong *clone_with_resources() const {return new DatabaseClassNameWrong(*this);}
  public:
    DatabaseClassNameWrong(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sClassName): ExceptionBase(pMemoryLifetimeOwner,"Database @name Wrong [%s]", sClassName) {}
    const char *type() const {return "DatabaseMatchFailed";}
  };

  class DuplicateDatabaseClass: public ExceptionBase {
  protected:
    DuplicateDatabaseClass *clone_with_resources() const {return new DuplicateDatabaseClass(*this);}
  public:
    DuplicateDatabaseClass(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"Duplicate Database Class [%s]", sName) {}
    const char *type() const {return "DuplicateDatabaseClass";}
  };

  class DocumentNotValid: public ExceptionBase {
  protected:
    DocumentNotValid *clone_with_resources() const {return new DocumentNotValid(*this);}
  public:
    DocumentNotValid(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"Document Not Valid [%s]", sName) {}
    const char *type() const {return "DocumentNotValid";}
  };

  class NoRootNodeFound: public ExceptionBase {
  protected:
    NoRootNodeFound *clone_with_resources() const {return new NoRootNodeFound(*this);}
  public:
    NoRootNodeFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"No Root Node Found in document [%s]", sName) {}
    const char *type() const {return "NoRootNodeFound";}
  };

  class NodeNotFound: public ExceptionBase {
  protected:
    NodeNotFound *clone_with_resources() const {return new NodeNotFound(*this);}
  public:
    NodeNotFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sWhat): ExceptionBase(pMemoryLifetimeOwner,"Node Not Found [%s]", sWhat) {}
    const char *type() const {return "NodeNotFound";}
  };

  class NoTmpNodeFound: public ExceptionBase {
  protected:
    NoTmpNodeFound *clone_with_resources() const {return new NoTmpNodeFound(*this);}
  public:
    NoTmpNodeFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"No Tmp Node Found in document [%s]", sName) {}
    const char *type() const {return "NoTmpNodeFound";}
  };
  
  class CannotCreateInTmpNode: public ExceptionBase {
  protected:
    CannotCreateInTmpNode *clone_with_resources() const {return new CannotCreateInTmpNode(*this);}
  public:
    CannotCreateInTmpNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"Cannot Create In Tmp Node in document [%s]", sName) {}
    const char *type() const {return "CannotCreateInTmpNode";}
  };
  
  class NoDocumentNodeFound: public ExceptionBase {
  protected:
    NoDocumentNodeFound *clone_with_resources() const {return new NoDocumentNodeFound(*this);}
  public:
    NoDocumentNodeFound(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sName): ExceptionBase(pMemoryLifetimeOwner,"No Document Node Found in document [%s]", sName) {}
    const char *type() const {return "NoDocumentNodeFound";}
  };
  
  class ReplaceNodeRequiresConstantXMLID: public ExceptionBase {
  protected:
    ReplaceNodeRequiresConstantXMLID *clone_with_resources() const {return new ReplaceNodeRequiresConstantXMLID(*this);}
  public:
    ReplaceNodeRequiresConstantXMLID(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sXMLID_previous, const char* sXMLID_new = 0): ExceptionBase(pMemoryLifetimeOwner,"ReplaceNode Requires Constant XML:ID [%s -> %s]", sXMLID_previous, sXMLID_new) {}
    const char *type() const {return "ReplaceNodeRequiresConstantXMLID";}
  };


  //XmlGenericException from The XMLLibrary
  class XmlGenericException: public ExceptionBase {
  protected:
    XmlGenericException *clone_with_resources() const {return new XmlGenericException(*this);}
  public:
    XmlGenericException(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const char *sDetail = 0);
    const char *type() const {return "XmlGenericException";}
  };
  
  //Xml Library overlay exceptions
  class XmlParserNamespaceNotDeclared: public ExceptionBase {
  protected:
    XmlParserNamespaceNotDeclared *clone_with_resources() const {return new XmlParserNamespaceNotDeclared(*this);}
  public:
    XmlParserNamespaceNotDeclared(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage): ExceptionBase(pMemoryLifetimeOwner,"XmlParser Namespace Not Declared [%s]", sMessage) {}
    const char *type() const {return "XmlParserNamespaceNotDeclared";}
  };
  
  class TransientNode: public NodeExceptionBase {
  protected:
    TransientNode *clone_with_resources() const {return new TransientNode(*this);}
  public:
    TransientNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Transient Node [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "TransientNode";}
  };
  
  class OrphanNode: public NodeExceptionBase { //or UnLinked or NoParent
  protected:
    OrphanNode *clone_with_resources() const {return new OrphanNode(*this);}
  public:
    OrphanNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Orphan Node [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "OrphanNode";}
  };

  class NonTransientNode: public NodeExceptionBase {
  protected:
    NonTransientNode *clone_with_resources() const {return new NonTransientNode(*this);}
  public:
    NonTransientNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Not a Transient Node [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "NonTransientNode";}
  };
  
  class TransformFailed: public NodeExceptionBase {
  protected:
    TransformFailed *clone_with_resources() const {return new TransformFailed(*this);}
  public:
    TransformFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char* sMessage, const IXmlBaseNode* pNode = 0, const char *sLiteralArg1 = 0): NodeExceptionBase(pMemoryLifetimeOwner,"Transform Failed [%s] [%s %s]", pNode, sMessage, sLiteralArg1) {}
    const char *type() const {return "TransformFailed";}
  };
  
  //Node Locking
  class NodeLockPreventedFree: public NodeExceptionBase {
  protected:
    NodeLockPreventedFree *clone_with_resources() const {return new NodeLockPreventedFree(*this);}
  public:
    NodeLockPreventedFree(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseNode* pNode = 0, const char *sMessage = 0): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Prevented Free [%s] [%s]", pNode, sMessage) {}
    NodeLockPreventedFree(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Prevented Free [%s]", 0, sMessage) {}
    const char *type() const {return "NodeLockPreventedFree";}
  };
  
  class NodeLockHasNoLocks: public NodeExceptionBase {
  protected:
    NodeLockHasNoLocks *clone_with_resources() const {return new NodeLockHasNoLocks(*this);}
  public:
    NodeLockHasNoLocks(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseNode* pNode = 0, const char *sMessage = 0): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Has No Locks [%s] [%s]", pNode, sMessage) {}
    NodeLockHasNoLocks(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Has No Locks [%s]", 0, sMessage) {}
    const char *type() const {return "NodeLockHasNoLocks";}
  };

  class NodeLockFailed: public NodeExceptionBase {
  protected:
    NodeLockFailed *clone_with_resources() const {return new NodeLockFailed(*this);}
  public:
    NodeLockFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlBaseNode* pNode = 0, const char *sMessage = 0): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Failed [%s] [%s]", pNode, sMessage) {}
    NodeLockFailed(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const char *sMessage): NodeExceptionBase(pMemoryLifetimeOwner,"NodeLock Failed [%s]", 0, sMessage) {}
    const char *type() const {return "NodeLockFailed";}
  };
}

#endif
