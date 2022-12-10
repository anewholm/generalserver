//platform agnostic definitions
#ifndef _DEFINE_H
#define _DEFINE_H

// ./configure generated defines from e.g. --with-debug
//incuding WITH_DEBUG, WITHOUT_LOCKING etc.
//see AC_DEFINE in autobuild.sh
#include "config.h"

//includes platform dependent SERVER_MAIN_XML_LIBRARY_PREFIX
//and the very platform dependent SOCKETS sub-system etc.
#include "platform_define.h"

//--------------------------------------------- standard Strategies
//this can be compiled in as MSXML6 or LibXML
//although Windows will need to use the shared libraries
//and UNIX can use them compiled directly in :)
#define SERVER_MAIN_XML_LIBRARY  CONCAT(SERVER_MAIN_XML_LIBRARY_PREFIX, XmlLibrary) //main server XML factory
#define SERVER_MAIN_XSL_MODULE   CONCAT(SERVER_MAIN_XML_LIBRARY_PREFIX, XslModule)  //XSL extension module inheritance

//--------------------------------------------- SECURITY
#define NONE_USERID 0U
#define ANONYMOUS_USERNAME "anonymous"
#define ANONYMOUS_UUID "7eb9b833-d5cd-4bbf-9ec8-2fcd4c90bee9"
#define ROOT_USERNAME "root"
#define ROOT_UUID "870ee77d-855f-4475-904e-7579ed53bd34"
#define SERVER_USERNAME "root"
#define SERVER_HASHED_PASSWORD "uehr76erv67trewgvw8erv"

//-------------------------------- 3rd party settings
#ifndef REGEX_POSIX
#define REGEX_POSIX
#endif

//-------------------------------- helper MACROS
//achieves MACRO name resolution after first concat
//otherwise we get SERVER_MAIN_XML_LIBRARY_PREFIXXmlLibrary
//note that ## does not resolve MACRO names on first pass
//and # deals only with string literals
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif

#ifndef FAKE_OVERLOAD
#define FAKE_OVERLOAD __attribute__((unused))
#endif

#ifndef ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH __attribute__ ((fallthrough))
#endif

#ifndef UNUSED
#define UNUSED(x) (void) x
#endif

#ifndef UNUSED_RESULT
#define UNUSED_RESULT(x) void* unused_result ATTRIBUTE_UNUSED = (void*) x
#endif

#ifndef UNUSED_NULL
#define UNUSED_NULL 0
#endif

#ifndef FUNCTION_SIGNATURE
#define FUNCTION_SIGNATURE MM_STRDUP(sFunctionSignature)
#endif

#define CONCAT2(x,y)  x ## y
#define CONCAT(x,y)  CONCAT2(x, y)

#define STR1(x) #x
#define STR(x)  STR1(x)

//-------------------------------- WITH_*
#ifdef WITH_DEBUG
  #define IFDEBUG(x) x
  #define IFNDEBUG(x)
#else
  #define IFDEBUG(x)
  #define IFNDEBUG(x) x
#endif

#ifdef WITH_DEBUG_EXCEPTIONS
  #define IFDEBUG_EXCEPTIONS(x) x
#else
  #define IFDEBUG_EXCEPTIONS(x) 
#endif

#ifdef WITH_DEBUG_RX
  #define IFDEBUG_RX(x) x
#else
  #define IFDEBUG_RX(x)
#endif

//-------------------------------- general documentation
#define DECIMAL 10
#define NOT_FORK_SHARED 0

//-------------------------------- inheritance documentation
#define implements_interface virtual public
#define implements_interface_static public
#define interface_class class
#define delete_wrapper delete
#define DELETE_IF(a) if (a) delete a

//-------------------------------- initialisation patterns
//lazy initialisation technicallay changes the object after initialisation
//but only because it couldn't be bothered to make it at the start
//so we allow a const function for access from other const situations
//in the function use:
//  lazy_init_const(QueryEnvironment)->m_iProperty = 12;
#define lazy_init_const_function const
#define lazy_init_const(X) ((X*) this)

//-------------------------------- code statuses
//this directive is used to highlight areas of code that are not currently used
//so that programmers can tell that it needs not be considered now
#define NOT_CURRENTLY_USED(sWhat) throw NotCurrentlyUsed(MM_STRDUP(sWhat))
#define NOT_COMPLETE(sWhat)       throw NotComplete(MM_STRDUP(sWhat))
#define NEEDS_TEST(sWhat)         throw NeedsTest(MM_STRDUP(sWhat))

//-------------------------------- resource management
#define NOT_OURS false
#define OURS true
#define NO_DUPLICATE false

//-------------------------------- capabilities
#define CAPABILITY_NOT_IMPLEMENTED false
#define CAPABILITY_IMPLEMENTED     true

//-------------------------------- types
#define RFC822_LEN 40
#define DATE_FORMAT_XML "%FT%T%z"               //2015-09-27T13:50:03+00:00
#define DATE_FORMAT_RFC822 "%a, %d %b %Y %T %Z" //Wed, 29 Apr 2015 13:50:35 GMT
#define DATE_FORMAT_RFC850 "%A, %d-%b-%y %T %Z"
#define DATE_FORMAT_ANSI   "%a %b  %d %T %Y"

//-------------------------------- NAMESPACEs
//external library namespaces used
#define ATTRIBUTE_SPACE ' '
#define NAMESPACE_NONE 0
#define NAMESPACE_XHTML "http://www.w3.org/1999/xhtml"
#define NAMESPACE_XHTML_ALIAS "html"
#define NAMESPACE_XSCHEMA "http://www.w3.org/2001/XMLSchema"
#define NAMESPACE_XSCHEMA_ALIAS "xsd"
#define NAMESPACE_XSL "http://www.w3.org/1999/XSL/Transform"
#define NAMESPACE_XSL_ALIAS "xsl"
#define NAMESPACE_XXX "http://general_server.org/xmlnamespaces/dummyxsl/2006"
#define NAMESPACE_XXX_ALIAS "xxx"
#define NAMESPACE_DYN "http://exslt.org/dynamic"
#define NAMESPACE_DYN_ALIAS "dyn"
#define NAMESPACE_DATE "http://exslt.org/dates-and-times" 
#define NAMESPACE_DATE_ALIAS "date" 
 
//http://www.w3.org/XML/1998/namespace
#define NAMESPACE_XML "http://www.w3.org/XML/1998/namespace"
#define NAMESPACE_XML_ALIAS "xml"

//base strings for GS namespaces
#define NAMESPACE_BASE "http://general_server.org/xmlnamespaces/"
#define NAMESPACE_YEAR "/2006"

//base default namespace
#define NAMESPACE_GS NAMESPACE_BASE "general_server" NAMESPACE_YEAR
#define NAMESPACE_GS_ALIAS "gs"

//general markup namespaces
//  these do not provide extended functionality *during* XSLT
//  but do have meaning in other layers in terms of JS interaction
//object namespace: all "objects" are linked to ServerObject's or DatabaseElement's functionality
//  the root object namespace node for each object definition indicates the object class
#define NAMESPACE_META_ALIAS "meta"
#define NAMESPACE_META NAMESPACE_BASE NAMESPACE_META_ALIAS NAMESPACE_YEAR
#define NAMESPACE_OBJECT_ALIAS "object"
#define NAMESPACE_OBJECT NAMESPACE_BASE NAMESPACE_OBJECT_ALIAS NAMESPACE_YEAR
#define NAMESPACE_INTERFACE_ALIAS "interface"
#define NAMESPACE_INTERFACE NAMESPACE_BASE NAMESPACE_INTERFACE_ALIAS NAMESPACE_YEAR
#define NAMESPACE_JAVASCRIPT_ALIAS "javascript"
#define NAMESPACE_JAVASCRIPT NAMESPACE_BASE NAMESPACE_JAVASCRIPT_ALIAS NAMESPACE_YEAR
#define NAMESPACE_CLASS_ALIAS "class"
#define NAMESPACE_CLASS NAMESPACE_BASE NAMESPACE_CLASS_ALIAS NAMESPACE_YEAR
#define NAMESPACE_CSS_ALIAS "css"
#define NAMESPACE_CSS NAMESPACE_BASE NAMESPACE_CSS_ALIAS NAMESPACE_YEAR
#define NAMESPACE_XMLSECURITY_ALIAS "xmlsecurity"
#define NAMESPACE_XMLSECURITY NAMESPACE_BASE NAMESPACE_XMLSECURITY_ALIAS NAMESPACE_YEAR
#define NAMESPACE_XMLTRANSACTION_ALIAS "xmltransaction"
#define NAMESPACE_XMLTRANSACTION NAMESPACE_BASE NAMESPACE_XMLTRANSACTION_ALIAS NAMESPACE_YEAR


//XslModule namespaces providing extended functionality to GS XSLTs
#define NAMESPACE_DEBUG_ALIAS "debug"
#define NAMESPACE_DEBUG NAMESPACE_BASE NAMESPACE_DEBUG_ALIAS NAMESPACE_YEAR
#define NAMESPACE_DATABASE_ALIAS "database"
#define NAMESPACE_DATABASE NAMESPACE_BASE NAMESPACE_DATABASE_ALIAS NAMESPACE_YEAR
#define NAMESPACE_SERVER_ALIAS "server"
#define NAMESPACE_SERVER NAMESPACE_BASE NAMESPACE_SERVER_ALIAS NAMESPACE_YEAR
#define NAMESPACE_SERVICE_ALIAS "service"
#define NAMESPACE_SERVICE NAMESPACE_BASE NAMESPACE_SERVICE_ALIAS NAMESPACE_YEAR
#define NAMESPACE_SESSION_ALIAS "session"
#define NAMESPACE_SESSION NAMESPACE_BASE NAMESPACE_SESSION_ALIAS NAMESPACE_YEAR
#define NAMESPACE_CONVERSATION_ALIAS "conversation"
#define NAMESPACE_CONVERSATION NAMESPACE_BASE NAMESPACE_CONVERSATION_ALIAS NAMESPACE_YEAR
#define NAMESPACE_USER_ALIAS "user"
#define NAMESPACE_USER NAMESPACE_BASE NAMESPACE_USER_ALIAS NAMESPACE_YEAR
#define NAMESPACE_REQUEST_ALIAS "request"
#define NAMESPACE_REQUEST NAMESPACE_BASE NAMESPACE_REQUEST_ALIAS NAMESPACE_YEAR
#define NAMESPACE_RESPONSE_ALIAS "response"
#define NAMESPACE_RESPONSE NAMESPACE_BASE NAMESPACE_RESPONSE_ALIAS NAMESPACE_YEAR
#define NAMESPACE_RX_ALIAS "rx"
#define NAMESPACE_RX NAMESPACE_BASE NAMESPACE_RX_ALIAS NAMESPACE_YEAR
#define NAMESPACE_REPOSITORY_ALIAS "repository"
#define NAMESPACE_REPOSITORY NAMESPACE_BASE NAMESPACE_REPOSITORY_ALIAS NAMESPACE_YEAR
#define NAMESPACE_XJAVASCRIPT_ALIAS "xjavascript"
#define NAMESPACE_XJAVASCRIPT NAMESPACE_BASE NAMESPACE_XJAVASCRIPT_ALIAS NAMESPACE_YEAR

//custom C++ extensions to EXSLT (no javascript supported for things like exslt::regexp)
#define NAMESPACE_EXSLT_REGEXP "http://exslt.org/regular-expressions"
#define NAMESPACE_EXSLT_REGEXP_ALIAS "regexp"
#define NAMESPACE_EXSLT_FLOW "http://exslt.org/flow"
#define NAMESPACE_EXSLT_FLOW_ALIAS "flow"
#define NAMESPACE_EXSLT_STRINGS "http://exslt.org/strings"
#define NAMESPACE_EXSLT_STRINGS_ALIAS "str"
#define NAMESPACE_EXSLT_DATEANDTIMES "http://exslt.org/dates-and-times"
#define NAMESPACE_EXSLT_DATEANDTIMES_ALIAS "date"

//default namespace is placed in the root of all documents
#define NAMESPACE_DEFAULT "xmlns=\"" NAMESPACE_GS "\""
//NAMESPACE_ALL placed in the root node of all documents by the plugin library
//  and in all xpath selects
//NOTE: TODO: recompile everything to include the new NAMESPACE in the xpath resolver
//  <xsl:param @name=gs_exclude_namespace_prefixes select=...
#define NAMESPACE_ALL "\
xmlns:" NAMESPACE_META_ALIAS  "=\"" NAMESPACE_META "\" \
xmlns:" NAMESPACE_XHTML_ALIAS "=\"" NAMESPACE_XHTML "\" \
xmlns:" NAMESPACE_XML_ALIAS "=\"" NAMESPACE_XML "\" \
xmlns:" NAMESPACE_GS_ALIAS "=\"" NAMESPACE_GS "\" \
xmlns:" NAMESPACE_XSCHEMA_ALIAS "=\"" NAMESPACE_XSCHEMA "\" \
xmlns:" NAMESPACE_XSL_ALIAS "=\"" NAMESPACE_XSL "\" \
xmlns:" NAMESPACE_DYN_ALIAS "=\"" NAMESPACE_DYN "\" \
xmlns:" NAMESPACE_DEBUG_ALIAS "=\"" NAMESPACE_DEBUG "\" \
xmlns:" NAMESPACE_DATABASE_ALIAS "=\"" NAMESPACE_DATABASE "\" \
xmlns:" NAMESPACE_SERVER_ALIAS "=\"" NAMESPACE_SERVER "\" \
xmlns:" NAMESPACE_SERVICE_ALIAS "=\"" NAMESPACE_SERVICE "\" \
xmlns:" NAMESPACE_SESSION_ALIAS "=\"" NAMESPACE_SESSION "\" \
xmlns:" NAMESPACE_CONVERSATION_ALIAS "=\"" NAMESPACE_CONVERSATION "\" \
xmlns:" NAMESPACE_USER_ALIAS "=\"" NAMESPACE_USER "\" \
xmlns:" NAMESPACE_REQUEST_ALIAS "=\"" NAMESPACE_REQUEST "\" \
xmlns:" NAMESPACE_RESPONSE_ALIAS "=\"" NAMESPACE_RESPONSE "\" \
xmlns:" NAMESPACE_RX_ALIAS "=\"" NAMESPACE_RX "\" \
xmlns:" NAMESPACE_REPOSITORY_ALIAS "=\"" NAMESPACE_REPOSITORY "\" \
xmlns:" NAMESPACE_OBJECT_ALIAS "=\"" NAMESPACE_OBJECT "\" \
xmlns:" NAMESPACE_INTERFACE_ALIAS "=\"" NAMESPACE_INTERFACE "\" \
xmlns:" NAMESPACE_JAVASCRIPT_ALIAS "=\"" NAMESPACE_JAVASCRIPT "\" \
xmlns:" NAMESPACE_XJAVASCRIPT_ALIAS "=\"" NAMESPACE_XJAVASCRIPT "\" \
xmlns:" NAMESPACE_CLASS_ALIAS "=\"" NAMESPACE_CLASS "\" \
xmlns:" NAMESPACE_CSS_ALIAS "=\"" NAMESPACE_CSS "\" \
xmlns:" NAMESPACE_XMLSECURITY_ALIAS "=\"" NAMESPACE_XMLSECURITY "\" \
xmlns:" NAMESPACE_XMLTRANSACTION_ALIAS "=\"" NAMESPACE_XMLTRANSACTION "\" \
xmlns:" NAMESPACE_EXSLT_REGEXP_ALIAS "=\"" NAMESPACE_EXSLT_REGEXP "\" \
xmlns:" NAMESPACE_EXSLT_FLOW_ALIAS "=\"" NAMESPACE_EXSLT_FLOW "\" \
xmlns:" NAMESPACE_EXSLT_STRINGS_ALIAS "=\"" NAMESPACE_EXSLT_STRINGS "\" \
xmlns:" NAMESPACE_EXSLT_DATEANDTIMES_ALIAS "=\"" NAMESPACE_EXSLT_DATEANDTIMES "\""
// TODO: this date namespace conflicts with the EXSLT date ns
// xmlns:" NAMESPACE_DATE_ALIAS "=\"" NAMESPACE_DATE "\"

#endif
