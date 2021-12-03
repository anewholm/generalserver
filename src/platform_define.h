//platform specific definitions (UNIX)

//DO NOT directly include this file, it is only included by define.h
#include "stdafx.h" //sockets etc. (auto-included by WIN32)

//----------------------------------------------------- windows data types (used in the socket stuff)
typedef int INT_PTR;
typedef unsigned int UINT_PTR;
typedef UINT_PTR SOCKET;
#define WORD unsigned int
#define __thiscall //windows calling convention

//----------------------------------------------------- SOCKETS (replacement winsock defs)
#define INVALID_SOCKET (SOCKET) 0
#define SOCKET_ERROR -1
#define CLOSE_SOCKET(s) close(s)
#define SOCKADDR_IN sockaddr_in
#define LPSOCKADDR sockaddr*
#define WSADATA int //throw away
//#define GS_SO_RCVTIMEO 3
//SOCKETS init and clean
#define SOCKETS_CLEANUP()
#define SOCKETS_INIT(v,w) 0 //return zero to indicate success
//error reporting
#define ERR(x,y)  err(x,y)  //http://man7.org/linux/man-pages/man3/err.3.html
#define WARN(x,y) warn(x,y) //http://man7.org/linux/man-pages/man3/err.3.html
#define SOCKETS_ERRNO errno
#define PIPE_READ  0
#define PIPE_WRITE 1
//function call status macros - these are already defined by DEVENV in WINDOWS
#define S_OK 0x00000000L
#define E_FAIL 0x80004005L
#define E_NOTIMPL 0x80004001L
#define HRESULT int
#define SUCCEEDED(t) (t)==0
#define FAILED(t) !SUCCEEDED(t)
#define VALID(t) (t)!=0

//----------------------------------------------------- misc
//#define _PATH_SPLITTER '/' //define as \ in windows (default /)

//----------------------------------------------------- misc
//TODO: place in configure.ac
#define LAST_DIR "/var/www/general_resources_server/last"
#define LAST_XML LAST_DIR "/response.xml"
#define LAST_XSL LAST_DIR "/stylesheet.xsl"
#define LAST_PHP "http://general-resources-server.localhost/last/index.php"

//----------------------------------------------------- function aliases
//these are platform dependent 
//because Windows introduced a new range of calls to combat buffer overflow
#define _STREQUAL(a,b) ((!a && !b) || (a && b && !strcmp((const char*) a, (const char*) b)))
#define _STRDIFFERENT(a,b) !_STREQUAL(a,b)
#define _STRNEQUAL(a,b,i) ((!a && !b) || (a && b && !strncmp(a,b,i)))
#define _STRDUP(a) (a ? strdup((const char*) a) : 0)
#define _STRDDUP(a, d) (a ? strdup((const char*) a) : strdup((const char*) d)) //d = default
#define _STRNDUP(a,i) (a ? strndup((const char*) a,i) : 0)
#define _STRNDDUP(a,d,i) (a ? strndup((const char*) a,i) : strndup((const char*) d,i)) //d = default
#define _STRNCPY(d, s, i) strncpy((char*) d,(const char*) s,i);d[i]=0;
#define _SNPRINTF1(a,b,c,d) snprintf(a,b,c,d)
#define _SNPRINTF2(a,b,c,d,e) snprintf(a,b,c,d,e)
#define _SNPRINTF3(a,b,c,d,e,f) snprintf(a,b,c,d,e,f)
#define _SNPRINTF4(a,b,c,d,e,f,g) snprintf(a,b,c,d,e,f,g)
#define _SNPRINTF5(a,b,c,d,e,f,g,h) snprintf(a,b,c,d,e,f,g,h)
#define _SNPRINTF6(a,b,c,d,e,f,g,h,i) snprintf(a,b,c,d,e,f,g,h,i)
#define _SNPRINTF7(a,b,c,d,e,f,g,h,i,j) snprintf(a,b,c,d,e,f,g,h,i,j)
#define _SNPRINTF8(a,b,c,d,e,f,g,h,i,j,k) snprintf(a,b,c,d,e,f,g,h,i,j,k)
#define _SNPRINTF9(a,b,c,d,e,f,g,h,i,j,k,l) snprintf(a,b,c,d,e,f,g,h,i,j,k,l)
#define _SNPRINTF10(a,b,c,d,e,f,g,h,i,j,k,l,m) snprintf(a,b,c,d,e,f,g,h,i,j,k,l,m)
#define _FOPEN(pFile,Name,Mode) pFile=fopen(Name,Mode)

