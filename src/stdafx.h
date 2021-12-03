//platform specific file (UNIX)
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include <unistd.h>

//SOCKETs winsock equivalent
//various #defines in the platform_define.h to sync socket language
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <sys/time.h>

//error reporting
#include <errno.h>
#include <err.h>
