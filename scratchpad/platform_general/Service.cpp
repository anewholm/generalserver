//platform agnostic file
#include "Service.h"

#include "Server.h"
#include "define.h"
#include "debug.h"
#include "StringMap.h"
#include "Thread.h"
#include "winsock2.h"

#define DEFAULT_BUFFLEN 1024

namespace goofy {
	//Service represents a TSR listening on a port and has actions associated with it
	//It is not the windows service interface: see goofy.cpp for that
	extern Server* g_oServer;

	DWORD WINAPI ServiceStart(LPVOID lpParam) {
		_THREADINIT; {
			Service *pService=(Service*) lpParam;
			pService->listenOnPort();
		} _THREADKILL;
		return 0;
	}

	Service::Service(const char *sServiceName, const int iPort, const bool bInteractive, const char *sSentenceFinish, const XmlDoc *pMessageInterpretation, const XmlDoc *pCommandsXSL):
		m_sServiceName(sServiceName),
		m_iPort(iPort),
		m_bInteractive(bInteractive),
		m_sSentenceFinish(sSentenceFinish),
		m_pMessageInterpretation(pMessageInterpretation),
		m_pCommandsXSL(pCommandsXSL) {
		DEBUGPRINT2("new service listener [%s] starting on %i", DEBUG_LINE, sServiceName, iPort);
	}
	Service::~Service() {
		vector<Request*>::iterator iRequest;
		for (iRequest=m_vRequests.begin();iRequest!=m_vRequests.end();iRequest++) delete *iRequest;
		delete m_pMessageInterpretation;
		delete m_pCommandsXSL;
	}

	void Service::listenOnPort() {
		//working code goes here to actually listen and stuff
		//http://msdn.microsoft.com/en-us/library/ms740668(v=vs.85).aspx
		Request *pRequest=0;
		SOCKET s=INVALID_SOCKET, sc=INVALID_SOCKET;
		SOCKADDR_IN addr; // The address structure for a TCP socket
		WSADATA w;
		WORD vVersionRequired=0x0202;
		if (!WSAStartup(vVersionRequired, &w)) {
			addr.sin_family = AF_INET;      // Address family
			addr.sin_port = htons(m_iPort);   // Assign port to this socket
			//Accept a connection from any IP using INADDR_ANY
			addr.sin_addr.s_addr = htonl (INADDR_ANY);
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
			if (s != INVALID_SOCKET)
				if (bind(s, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
					closesocket(s);
				else
					while (true) {
						//blocking socket, wait for incoming reqeuest
						if (listen(s, SOMAXCONN)==SOCKET_ERROR) {closesocket(s);WSACleanup();return;}
						else {
							//incoming request. Windows allows only one blocking socket function call per thread.
							sc=accept(s, NULL, NULL); //accept connection so that the listen doesn't return again immediately
							pRequest=new Request(this, sc);
							m_vRequests.push_back(pRequest);
							new Thread(RequestStart, pRequest);
						}
					}
			int wsae = WSAGetLastError();
			DEBUGPRINT1("WSAGetLastError [%i]", DEBUG_LINE, wsae);
			WSACleanup();
		}
	}

	const string *Service::serialise() const {
		//caller freed (needs to be efficient)
		string *psSerialise=0;
		char sConfigXML[1024];
		const string *psXML=0;
		char *sInteractive;
		if (m_bInteractive) sInteractive="true"; else sInteractive="false";
		_SNPRINTF4(sConfigXML, 1024, "<service name=\"%s\" port=\"%i\" interactive=\"%s\" sentenceFinish=\"%s\">", m_sServiceName, m_iPort, sInteractive, m_sSentenceFinish);
		psSerialise=new string(sConfigXML);
		//messageInterpretation
		psSerialise->append("<messageInterpretation>");
		char *sMessageInterpretation=m_pMessageInterpretation->xml();
		psSerialise->append(sMessageInterpretation);
		if (sMessageInterpretation) free(sMessageInterpretation);
		psSerialise->append("</messageInterpretation>");
		//messageInterpretation
		psSerialise->append("<commandsXSL>");
		char *sCommandsXSL=m_pCommandsXSL->xml();
		psSerialise->append(sCommandsXSL);
		if (sCommandsXSL) free(sCommandsXSL);
		psSerialise->append("</commandsXSL>");
		//requests
		psSerialise->append("<requests>");
		vector<Request*>::const_iterator iRequest;
		for (iRequest=m_vRequests.begin();iRequest!=m_vRequests.end();iRequest++) {
			psXML=(*iRequest)->serialise();
			psSerialise->append(*psXML);
			delete psXML;
		}
		psSerialise->append("</requests>");

		psSerialise->append("</service>");
		return psSerialise;
	}
}
