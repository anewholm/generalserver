//platform specific file (WINDOWS)
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

//Additional linker dependencies: WS2_32.lib 
#include "winsock2.h"

//COM smart pointers
//Additional linker dependencies: comsuppw.lib
#import "msxml6.dll" 
