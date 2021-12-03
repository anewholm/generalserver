//platform agnostic file
#ifndef _SERVICE_H
#define _SERVICE_H

namespace goofy {class Service;}

#include "request.h"
#include "xml.h"
#include <vector>
#include <map>
using namespace std;

#include "module.h"

#define _PAIR_SERVICE pair<const char*, Service*>

namespace goofy {
	DWORD WINAPI ServiceStart(LPVOID lpParam);

	class Service: public Module {
	private:
		const char *m_sServiceName;
		const int m_iPort;
		vector<Request*> m_vRequests;
	public:
		const bool m_bInteractive;
		const char *m_sSentenceFinish;
		const XmlDoc *m_pMessageInterpretation;
		const XmlDoc *m_pCommandsXSL;
	public:
		Service(
			const char *sServiceName,
			const int iPort, 
			const bool bInteractive, 
			const char *sSentenceFinish, 
			const XmlDoc *pMessageInterpretation,
			const XmlDoc *pCommandsXSL);
		~Service();
		const string *serialise() const;

		void listenOnPort();
	};
}

#endif