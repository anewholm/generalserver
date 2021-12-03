//platform agnostic file
#ifndef _REQUEST_H
#define _REQUEST_H

namespace goofy {class Request;class Response;}

#include <vector>
using namespace std;

#include "stdafx.h" //WINDOWS: winsock2.h

#define DEFAULT_BUFFLEN 1024

#include "Server.h"
#include "Service.h"
#include "DB.h"
#include "XML.h"
#include "Module.h"
#include "DBManager.h"
#include "Response.h"

namespace goofy {
	DWORD WINAPI RequestStart(LPVOID lpParam);

	//each new request is on a new thread and accepts a text command stream
	//this gets converted into (multiple, hierarchical) command(s)
	//The server is responsible only for kicking off the thread, passing through the communications socket
	class Request: public Module {
	private:
		Service *m_pService;
		SOCKET m_sc;
		const char *m_sTextStream;
		const XmlDoc *m_pCommandDoc;
		XmlDoc *m_pTraceDoc;
		StringMap<Module*> m_mModules;
		Response *m_pResponse;
	public:
		const XmlDoc *m_pRequestDoc;
	private:
		//conversion of input text stream into a CommandList ready for execution
		HRESULT getTextStream(const char **sTextStream) const;
	public:
		~Request();
		Request(Service *pService, SOCKET s);
		void process();
		//Modules are the Objects which house the commands that this Request can run
		const int linkStaticBuiltInModules();
		const int instanciateBuiltInModules();
		const int instanciateCustomModules();
		HRESULT recurseCommand(const XmlNode *pCommandSpec, XmlNode *pParentTraceNode, const XmlDoc **ppResultDoc);
		const string *serialise() const;
	};
}

#endif
