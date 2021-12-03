//platform agnostic file
#ifndef _RESPONSE_H
#define _RESPONSE_H

namespace goofy {class Request;class Response;}

#include "debug.h"
#include "request.h"
#include "server.h"
#include "module.h"

namespace goofy {
	class Response: public Module {
		//stores the output from commands
		Service *m_pService;
		SOCKET m_sc;
	public:
		HRESULT fOutDocs(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fOutText(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		Response(Service *pService, SOCKET sc);
	};
}
#endif

