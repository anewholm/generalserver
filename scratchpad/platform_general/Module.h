//platform agnostic file
#ifndef _MODULE_H
#define _MODULE_H

//std library includes
#include <iostream>
#include <vector>
using namespace std;

#include "debug.h"
#include "StringMap.h"

//dereferncing member function pointers requires the class:: prefix
#define _PCOMMANDPROTOTYPE HRESULT (__thiscall Module::*)(const StringMap<const char*> *, const vector<const XmlDoc *>*, const XmlDoc**, const XmlNode*)
#define _PCOMMANDPROTOTYPE_ARG HRESULT (__thiscall Module::*pFunc)(const StringMap<const char*> *, const vector<const XmlDoc *>*, const XmlDoc**, const XmlNode*)
#define _PAIR_PCOMMANDPROTOTYPE pair<const char*, _PCOMMANDPROTOTYPE>
#define _PAIR_MODULE pair<const char*, Module*>
#define _PAIR_ARG pair<const char*, const char*>

namespace goofy {
	class Module {
		//Modules are the Objects which house the commands that this Request can run
		//Modules may also do other stuff but also provide an interface to incoming Requests that have commands to run
	public:
		StringMap<_PCOMMANDPROTOTYPE> m_commands;
		HRESULT addCommand(const char *sCommandName, _PCOMMANDPROTOTYPE_ARG) {
			DEBUGPRINT1("inserting command [%s]", DEBUG_LINE, sCommandName);
			m_commands.insert(_PAIR_PCOMMANDPROTOTYPE(sCommandName, pFunc));
			return S_OK;
		}
		HRESULT runCommand(const char *sCommandName, const StringMap<const char*>* mArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
			HRESULT iResult=E_NOTIMPL;
			StringMap<_PCOMMANDPROTOTYPE>::iterator iCommand=m_commands.find(sCommandName);
			DEBUGPRINT1("looking for command [%s]", DEBUG_CHECK, sCommandName);
			if (iCommand!=m_commands.end()) {
				DEBUG_RESULT_OK;
				_PCOMMANDPROTOTYPE_ARG=iCommand->second;
				iResult=(this->*pFunc)(mArgs, pChildResponseDocs, ppResultDoc, pCommandSpec);
			} else DEBUG_RESULT_FAIL;
			return iResult;
		}
	};
}

#endif
