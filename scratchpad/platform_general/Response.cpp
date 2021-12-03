#include "response.h"

namespace goofy {
	extern Server* g_oServer;

	Response::Response(Service *pService, SOCKET sc): m_pService(pService), m_sc(sc) {
		addCommand("outDocs", (_PCOMMANDPROTOTYPE) &Response::fOutDocs);
		addCommand("outText", (_PCOMMANDPROTOTYPE) &Response::fOutText);
	}
	HRESULT Response::fOutDocs(const StringMap<const char*> *pArgs, const vector<const XmlDoc*> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		DEBUGPRINT("Response::fOutDocs", DEBUG_FUNCTIONSTART);
		const XmlDoc *pCurrentDoc=0;
		if (pChildResponseDocs->size()==1) *ppResultDoc=*(pChildResponseDocs->begin());
		else *ppResultDoc=new XmlDoc("result", pChildResponseDocs);
		const char *sXML=(*ppResultDoc)->xml();
		send(m_sc, sXML, (int) strlen(sXML), 0);
		DEBUGPRINT1("sXML:\n%s\n\n", DEBUG_LINE, sXML);
		free((void*) sXML);
		DEBUGPRINT("Response::fOutDocs", DEBUG_FUNCTIONEND);
		return S_OK;
	}
	HRESULT Response::fOutText(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		DEBUGPRINT("Response::fOutText", DEBUG_FUNCTIONSTART);
		*ppResultDoc=0;

		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		bool bTranslateEntities;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "translateEntities", 17)) bTranslateEntities=(!strncmp(sArgValue, "yes", 3));
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		//process text
		const char *sText=pCommandSpec->value();
		int iLen=(int) strlen(sText), iChar, iBytesToCopy;
		char *sProcessedText=(char*) malloc(iLen+1), *sCurrentProcessedText=sProcessedText;
		const char *sOldPosText=sText, *sPos=strstr(sText, "&#");
		if (bTranslateEntities) {
			while (sPos) {
				iBytesToCopy=(int) (sPos-sOldPosText);
				_STRNCPY(sCurrentProcessedText, sOldPosText, iBytesToCopy);
				sCurrentProcessedText+=iBytesToCopy;
				iChar=atoi(sPos+2);
				*sCurrentProcessedText=iChar;
				sCurrentProcessedText+=1;
				sOldPosText=strchr(sPos, ';')+1;
				sPos=strstr(sOldPosText, "&#");
			}
		}
		iBytesToCopy=(iLen-(int)(sOldPosText-sText));
		_STRNCPY(sCurrentProcessedText, sOldPosText, iBytesToCopy);
		sCurrentProcessedText+=iBytesToCopy;
		*sCurrentProcessedText=0;

		//send text to the socket
		send(m_sc, sProcessedText, (int) strlen(sProcessedText), 0);
		DEBUGPRINT1("sProcessedText:\n%s\n\n", DEBUG_LINE, sProcessedText);

		//free up
		free((void*) sText);
		free((void*) sProcessedText);
		DEBUGPRINT("Response::fOutText", DEBUG_FUNCTIONEND);
		return S_OK;
	}
}
