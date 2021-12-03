//platform specific definitions (WINDOWS)
#ifndef _DEFINEPLATFORM_H
#define _DEFINEPLATFORM_H

//_DEBUG, _UNIX and _WIN32 are defined by the makefile/compiler
#define _LOGFILE "C:\\Users\\anewholm\\Documents\\Visual Studio 2005\\Projects\\goofy2\\general_server.log"
#define _CONFIGFILE "x:\\general_server\\config\\"

#define THREADED 1

//SOCKETS
#define CLOSE_SOCKET(s) closesocket(s)
#define SOCKETS_INIT(v,w) WSAStartup(v,w)
#define SOCKET_CLEANUP() WSACleanup()
//error reporting
#define ERR(x,y) cout << "[err:" << x << "]\n"
#define SOCKETS_ERRNO WSAGetLastError()

//function aliases - windows prefers use of _snprintf_s with additional maxLength parameter
#define _PATH_SPLITTER '\\'
#define _STRDUP(a) _strdup(a)
#define _STRNCPY(sDest, sSource, iChars) strncpy_s(sDest,iChars+1,sSource,iChars);sDest[iChars]=0;
#define _SNPRINTF1(a,b,c,d) _snprintf_s(a,b,b-1,c,d)
#define _SNPRINTF2(a,b,c,d,e) _snprintf_s(a,b,b-1,c,d,e)
#define _SNPRINTF3(a,b,c,d,e,f) _snprintf_s(a,b,b-1,c,d,e,f)
#define _SNPRINTF4(a,b,c,d,e,f,g) _snprintf_s(a,b,b-1,c,d,e,f,g)
#define _SNPRINTF5(a,b,c,d,e,f,g,h) _snprintf_s(a,b,b-1,c,d,e,f,g,h)
#define _SNPRINTF6(a,b,c,d,e,f,g,h,i) _snprintf_s(a,b,b-1,c,d,e,f,g,h,i)
#define _SNPRINTF7(a,b,c,d,e,f,g,h,i,j) _snprintf_s(a,b,b-1,c,d,e,f,g,h,i,j)
//fopen_s returns 0 on success whereas C++ fopen returns a valid FILE*
//http://msdn.microsoft.com/en-us/library/z5hh6ee9%28v=vs.80%29.aspx
#define _FOPEN(pFile,Name,Mode) !fopen_s(&pFile,Name,Mode) //fopen depreciated for fopen_s
#define _THREADINIT CoInitializeEx(NULL, COINIT_MULTITHREADED)
#define _THREADKILL CoUninitialize()

#endif
