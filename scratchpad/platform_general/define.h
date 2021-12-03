//platform agnostic definitions
#ifndef _DEFINE_H
#define _DEFINE_H

#include "definePlatform.h"

//_DEBUG, _UNIX and _WIN32 are defined by the makefile/compiler
#define _VERSION_MAJOR 1
#define _VERSION_MINOR 0
#define _VERSION_BUILD 1
#define _VERSION_TYPE "Alpha"

#define PTHREAD_RUN_FINALISER 1

//#define _RECORD_REQUESTS

#define REGEX_POSIX

//-------------------------------- NAMESPACEs
#define NAMESPACE_XSCHEMA "http://www.w3.org/2001/XMLSchema"
#define NAMESPACE_XSL "http://www.w3.org/1999/XSL/Transform"

#define NAMESPACE_BASE "http://general_server.org/xmlnamespaces/"
#define NAMESPACE_YEAR "/2006"

#define NAMESPACE_GS NAMESPACE_BASE##"general_server"##NAMESPACE_YEAR
#define NAMESPACE_GS_ALIAS "general_server"
#define NAMESPACE_DB NAMESPACE_BASE##"DB"##NAMESPACE_YEAR
#define NAMESPACE_DB_ALIAS "DB"
#define NAMESPACE_SERVER NAMESPACE_BASE##"server"##NAMESPACE_YEAR
#define NAMESPACE_SERVER_ALIAS "server"
#define NAMESPACE_RESPONSE NAMESPACE_BASE##"response"##NAMESPACE_YEAR
#define NAMESPACE_RESPONSE_ALIAS "response"
#define NAMESPACE_COMMS NAMESPACE_BASE##"comms"##NAMESPACE_YEAR
#define NAMESPACE_COMMS_ALIAS "comms"
#define NAMESPACE_RXSL NAMESPACE_BASE##"rxsl"##NAMESPACE_YEAR
#define NAMESPACE_RXSL_ALIAS "rxsl"
#define NAMESPACE_REPOSITORY NAMESPACE_BASE##"repository"##NAMESPACE_YEAR
#define NAMESPACE_REPOSITORY_ALIAS "repository"

#endif
