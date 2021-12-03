//platform agnostic file
#ifndef _SERVER_H
#define _SERVER_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
using namespace std;

//platform specific project headers
#include "DB.h"	//MSXML6 (Windows COM) or libxml2 (Redhat 9.0)
#include "module.h"
#include "Service.h"
#include "index.h"
#include "Repository.h"

#define _PAIR_ALIAS_DB pair<const char*, DB*>

//see server.cpp for the main comment block introduction
namespace goofy {
	class Server: public Module {
	private:
		StringMap<DB*> m_mDBs; //collection of DBs
		StringMap<Service*> m_mServices; //collection of services
		XmlDoc *m_config;

	public:
		Server();
		~Server();

		//Database initialisation and setup (run from goofy.config.xml or incoming commands)
		//these commands are also used from the interactive native service commands
		DB *loadDB(const char *sDBAlias, const char *sFilePath);
		HRESULT unloadDB(const char *sDBAlias);
		XmlDoc *addXSLToDB(DB *pDB, const char *sSSAlias, const char *sSSFilePath);
		XmlDoc *addCommandFileToDB(DB *pDB, const char *sCFAlias, const char *sCFFilePath);
		Index *createIndexToDB(DB *pDB, const char *sIXAlias, const char *sType, const char *sXPath="");

		const int associateTransformsToNodes(DB *pDB, const char *sRef, const char *sXPathToNodes, const char *sSSAlias);
		//querying
		DB *findDB(const char *sDBAlias) const;

		//native service commands (runnable from the services, native to goofy, entered in the commands list)
		//DB setup
		HRESULT loadDB(const vector<char*>*);
		HRESULT addXSLToDB(const vector<char*>*);
		HRESULT associateTransformsToNodes(const vector<char*>*);

		//state management
		const string *currentHDD() const;
		const string *configFile() const;
		const string *serialise() const;
	};
}

#endif

