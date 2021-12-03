//platform agnostic file
#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG_FUNCTIONSTART 1 //indent++ in file output debug
#define DEBUG_BLOCKSTART 2 //indent++ in file output debug
#define DEBUG_LINE 4 //no indent, \n\r appended
#define DEBUG_CHECK 8 //no indent, ... appended, no \n\r (for checking <x>...ok\n\r)
#define DEBUG_RESULT 16 //no indent, \n\r appended
#define DEBUG_BLOCKEND 32 //indent-- in file output debug
#define DEBUG_FUNCTIONEND 64 //indent++ in file output debug
#define DEBUG_LINE_QUALIFIED 128 //surround string by {} is that the length is evident
#define DEBUG_LINE_ECHO 256 //echo string to stdout
#define DEBUG_ERROR DEBUG_LINE_ECHO | DEBUG_LINE //echo string to stdout
#define DEBUG_LINE_SERVERMSG 512 //async sub-thread server msg (appends MSG to the front)
#define DEBUG_STRING 1024 //just pump out the chars

#define DEBUG_RESULT_OK DEBUGPRINT("ok", DEBUG_RESULT)
#define DEBUG_RESULT_FAIL DEBUGPRINT("failed", DEBUG_RESULT)

#include <stdio.h>
#include "define.h"

#ifdef _DEBUG
	//debug system activated
	#define DEBUGPRINT(sMessage, iType) debugPrint(sMessage, iType)
	#define DEBUGPRINT1(sFormat, iType, a) {char sDebug[2048];_SNPRINTF1(sDebug, 2048, sFormat, a);DEBUGPRINT(sDebug, iType);}
	#define DEBUGPRINT2(sFormat, iType, a,b) {char sDebug[2048];_SNPRINTF2(sDebug, 2048, sFormat, a,b);DEBUGPRINT(sDebug, iType);}
	#define DEBUGPRINT3(sFormat, iType, a,b,c) {char sDebug[2048];_SNPRINTF3(sDebug, 2048, sFormat, a,b,c);DEBUGPRINT(sDebug, iType);}
	#define DEBUGPRINT4(sFormat, iType, a,b,c,d) {char sDebug[2048];_SNPRINTF4(sDebug, 2048, sFormat, a,b,c,d);DEBUGPRINT(sDebug, iType);}
	#define DEBUGPRINT5(sFormat, iType, a,b,c,d,e) {char sDebug[2048];_SNPRINTF5(sDebug, 2048, sFormat, a,b,c,d,e);DEBUGPRINT(sDebug, iType);}
	#define DEBUGINIT debugInit()
	#define DEBUGFINALISE debugFinalise()
#else
	//Release system
	#define DEBUGPRINT(sMessage, iType) 000
	#define DEBUGPRINT1(sFormat, iType, a) 000
	#define DEBUGPRINT2(sFormat, iType, a,b) 000
	#define DEBUGPRINT3(sFormat, iType, a,b,c) 000
	#define DEBUGPRINT4(sFormat, iType, a,b,c,d) 000
	#define DEBUGPRINT5(sFormat, iType, a,b,c,d,e) 000
	#define DEBUGINIT 000
	#define DEBUGFINALISE 000
#endif

namespace goofy {
	bool debugInit();
	bool debugFinalise();
	bool debugPrint(const char *sMessage, int iType);
}

#endif
