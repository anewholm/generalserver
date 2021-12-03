//platform agnostic file
#include <iostream>
#include "debug.h"
using namespace std;

namespace goofy {
	FILE *g_fDebug;
	
	bool debugPrint(const char *sMessage, int iType) {
		static int s_iIndent = 0;
		bool bFinishLine=true;
		bool bShowIndent=true;
		bool bShowMessage=true;
		char *sPrefix=0, *sPostfix=0;
		unsigned int iNewIndent=s_iIndent;
	
		if (iType&DEBUG_STRING){
			bFinishLine=false;
			bShowIndent=false;
		}
		if (iType&DEBUG_LINE_SERVERMSG){
			sPrefix="";
			iType=iType|DEBUG_LINE_ECHO;
		}
		if (iType&DEBUG_LINE_ECHO) {
			cout << sMessage << "\n";
		}
		if (iType&DEBUG_LINE_QUALIFIED) {
			sPostfix="}";
			sPrefix="{";
		}
		if (iType&DEBUG_FUNCTIONSTART) {
			sPostfix="]";
			sPrefix="[";
			iNewIndent=s_iIndent+1;
		}
		if (iType&DEBUG_BLOCKSTART) {
			iNewIndent=s_iIndent+1;
		}
		if (iType&DEBUG_CHECK) {
			sPostfix="...";
			//bFinishLine=false;
			iNewIndent=s_iIndent+1;
		}
		if (iType&DEBUG_BLOCKEND
			||iType&DEBUG_FUNCTIONEND) {
			bFinishLine=false;
			bShowIndent=false;
			bShowMessage=false;
			if (s_iIndent>0) iNewIndent=--s_iIndent;
		}
		if (iType&DEBUG_RESULT) {
			//bShowIndent=false;
			sPostfix=")";
			sPrefix="(";
			if (s_iIndent>0) iNewIndent=--s_iIndent;
		}
	
		//indent
		if (s_iIndent&&bShowIndent) {
			char *sIndent=(char*) malloc(s_iIndent+1);
			memset(sIndent, '\t', s_iIndent);
			sIndent[s_iIndent]='\0';
			fputs(sIndent, g_fDebug);
			free(sIndent);
		}
		
		//pre
		if (sPrefix) fputs(sPrefix, g_fDebug);
		//message
		if (bShowMessage) {
			fputs(sMessage, g_fDebug);
		}
		//post
		if (sPostfix) fputs(sPostfix, g_fDebug);
		//EOL
		if (bFinishLine) fputs("\n", g_fDebug);
		fflush(g_fDebug);
		s_iIndent=iNewIndent;
	
		return true;
	}
	bool debugInit() {
		if (_FOPEN(g_fDebug, _LOGFILE, "w" )) {}
		return (g_fDebug!=NULL);
	}
	bool debugFinalise() {
		fclose(g_fDebug);
		return true;
	}
}
