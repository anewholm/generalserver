//platform agnostic file
#include "DBManager.h"

namespace goofy {
	extern Server* g_oServer;

	DBManager::DBManager(Request *pRequest): m_pRequest(pRequest) {
		pCurrentDefaultDB=0;
		addCommand("default", (_PCOMMANDPROTOTYPE) &DBManager::fDefault);
		addCommand("select", (_PCOMMANDPROTOTYPE) &DBManager::fSelect);
		addCommand("delete", (_PCOMMANDPROTOTYPE) &DBManager::fRemove); //note that we use fRemove because delete is a reserved word
		addCommand("insert", (_PCOMMANDPROTOTYPE) &DBManager::fInsert);
		addCommand("update", (_PCOMMANDPROTOTYPE) &DBManager::fUpdate);
		addCommand("unload", (_PCOMMANDPROTOTYPE) &DBManager::fUnload);
		addCommand("load", (_PCOMMANDPROTOTYPE) &DBManager::fLoad);
	}
	DB *DBManager::getDB(const char *sDB, const bool *bCallerFree) {
		DB *pDB=0;
		return pDB;
	}

	//dispatch functions
	//note that we use fRemove because delete is a reserved word
	HRESULT DBManager::fRemove(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		*ppResultDoc=0;
		return E_FAIL;
	}
	HRESULT DBManager::fInsert(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fInsert", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0, *sContext=0, *sWhat=0;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
			else if (!strncmp(sArgName, "context", 7)) sContext=sArgValue;
			else if (!strncmp(sArgName, "what", 4)) sWhat=sArgValue;
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		/*act
		if we have db="current" then the select is from the childrens result docs
		if we have no db, or db="default" then it will be the default.
			If no default then the instruction is ignored
		if no from then take whole document (allows ID: indexing)
		if no what then take document from from node (allows ref:stylesheet)
		*/
		DB *pDB=0, *pTempDB=0;
		const XmlDoc *pCurrentDoc=0;
		if (!sDb) {
			//construct a new DB from the pChildResponseDocs
			DEBUGPRINT("setting database to current", DEBUG_LINE);
			if (pChildResponseDocs->size()==1) pCurrentDoc=*(pChildResponseDocs->begin());
			else pCurrentDoc=new XmlDoc("result", pChildResponseDocs);
			// do not delete the current doc in ~DB() in this case as it will be used as the return doc
			//(to be passed to the next instruction up)
			pTempDB=pDB=new DB(_STRDUP("current"), (XmlDoc*) pCurrentDoc, false); //will have a root node <root>
		} else {
			//look for the DB
			DEBUGPRINT1("setting database to [%s]", DEBUG_CHECK, sDb);
			if (pDB=g_oServer->findDB(sDb)) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}
		//need a db to do an insert
		if (pDB) {
			DEBUGPRINT2("attempting db call [%s,%s]", DEBUG_CHECK, sContext, sWhat);
			if (SUCCEEDED(hResult=pDB->insert(sContext, sWhat, ppResultDoc))) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}

		//if it is an insert into the current doc then return a pointer to the currentDoc instead
		if (!sDb) *ppResultDoc=pCurrentDoc;

		//free up
		if (pTempDB) delete pTempDB;
		DEBUGPRINT("DBManager::fSelect", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DBManager::fUpdate(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fUpdate", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0, *sContext=0, *sWhat=0, *sTo=0;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
			else if (!strncmp(sArgName, "context", 7)) sContext=sArgValue;
			else if (!strncmp(sArgName, "what", 4)) sWhat=sArgValue;
			else if (!strncmp(sArgName, "to", 2)) sTo=sArgValue;
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		/*act
		if we have db="current" then the select is from the childrens result docs
		if we have no db, or db="default" then it will be the default.
			If no default then the instruction is ignored
		if no from then take whole document (allows ID: indexing)
		if no what then take document from from node (allows ref:stylesheet)
		*/
		DB *pDB=0, *pTempDB=0;
		const XmlDoc *pCurrentDoc=0;
		if (!sDb) {
			//construct a new DB from the pChildResponseDocs
			DEBUGPRINT("setting database to current", DEBUG_LINE);
			if (pChildResponseDocs->size()==1) pCurrentDoc=*(pChildResponseDocs->begin());
			else pCurrentDoc=new XmlDoc("result", pChildResponseDocs);
			// do not delete the current doc in ~DB() in this case as it will be used as the return doc
			//(to be passed to the next instruction up)
			pTempDB=pDB=new DB(_STRDUP("current"), (XmlDoc*) pCurrentDoc, false); //will have a root node <root>
		} else {
			//look for the DB
			DEBUGPRINT1("setting database to [%s]", DEBUG_CHECK, sDb);
			if (pDB=g_oServer->findDB(sDb)) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}
		//need a db to do an insert
		if (pDB) {
			DEBUGPRINT3("attempting db call [%s,%s,%s]", DEBUG_CHECK, sContext, sWhat, sTo);
			if (SUCCEEDED(hResult=pDB->update(sContext, sWhat, sTo, ppResultDoc))) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}

		//if it is an insert into the current doc then return a pointer to the currentDoc instead
		if (!sDb) *ppResultDoc=pCurrentDoc;

		//free up
		if (pTempDB) delete pTempDB;
		DEBUGPRINT("DBManager::fUpdate", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DBManager::fSelect(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fSelect", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0, *sContext=0, *sWhat=0;
		bool bDeepCloneXPath=false;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
			else if (!strncmp(sArgName, "what", 4)) sWhat=sArgValue;
			else if (!strncmp(sArgName, "context", 7)) sContext=sArgValue;
			else if (!strncmp(sArgName, "deepCloneXPath", 14)) bDeepCloneXPath=(!strncmp(sArgValue, "yes", 3));
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		/*act
		if we have db="current" then the select is from the childrens result docs
		if we have no db, or db="default" then it will be the default.
			If no default then the instruction is ignored
		if no from then take whole document (allows ID: indexing)
		if no what then take document from from node (allows ref:stylesheet)
		*/
		const DB *pDB=0, *pTempDB=0;
		if (!sDb) {
			//construct a new DB from the pChildResponseDocs
			DEBUGPRINT("setting database to current", DEBUG_LINE);
			const XmlDoc *pCurrentDoc=0;
			if (pChildResponseDocs->size()==1) pCurrentDoc=*(pChildResponseDocs->begin());
			else pCurrentDoc=new XmlDoc("result", pChildResponseDocs);
			pTempDB=pDB=new DB(_STRDUP("current"), (XmlDoc*) pCurrentDoc); //will have a root node <root>
		//------------- dynamic pseudo databases
		} else if (!strncmp(sDb, "config", 6)) {
			DEBUGPRINT("setting database to pseudo DB config", DEBUG_LINE);
			const string *stConfigXML=g_oServer->serialise();
			const char *sXML=stConfigXML->c_str();
			XmlDoc *pXmlDocConfig=new XmlDoc("config", sXML);
			pTempDB=pDB=new DB(_STRDUP("config"), pXmlDocConfig);
			delete stConfigXML;
		} else if (!strncmp(sDb, "request", 7)) {
			DEBUGPRINT("setting database to pseudo DB request", DEBUG_LINE);
			pTempDB=pDB=new DB(_STRDUP("request"), m_pRequest->m_pRequestDoc, false); //no XmlDoc release at ~DB()
		} else if (!strncmp(sDb, "default", 7)) {
			DEBUGPRINT("setting database to default", DEBUG_LINE);
			pDB=pCurrentDefaultDB;
		} else {
			//look for the DB
			DEBUGPRINT1("setting database to [%s]", DEBUG_CHECK, sDb);
			if (pDB=g_oServer->findDB(sDb)) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}
		//need a db to do a select
		if (pDB) {
			DEBUGPRINT2("attempting db call [%s,%s]", DEBUG_CHECK, sContext, sWhat);
			if (SUCCEEDED(hResult=pDB->select(sContext, sWhat, ppResultDoc, bDeepCloneXPath))) DEBUG_RESULT_OK;
			else DEBUG_RESULT_FAIL;
		}

		//free up
		if (pTempDB) delete pTempDB;
		DEBUGPRINT("DBManager::fSelect", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DBManager::fDefault(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fDefault", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		//act
		if (sDb) {
			DEBUGPRINT1("setting default database to [%s]", DEBUG_CHECK, sDb);
			if (pCurrentDefaultDB=g_oServer->findDB(sDb)) {
				DEBUG_RESULT_OK;
				hResult=S_OK;
			} else DEBUG_RESULT_FAIL;
		}

		//free up
		DEBUGPRINT("DBManager::fDefault", DEBUG_FUNCTIONEND);
		return hResult;
	}

	HRESULT DBManager::fUnload(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fUnload", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		//act
		if (sDb) {
			DEBUGPRINT1("unloading database to [%s]", DEBUG_CHECK, sDb);
			if (SUCCEEDED(g_oServer->unloadDB(sDb))) {
				DEBUG_RESULT_OK;
				hResult=S_OK;
			} else DEBUG_RESULT_FAIL;
		}

		//free up
		DEBUGPRINT("DBManager::fUnload", DEBUG_FUNCTIONEND);
		return hResult;
	}

	HRESULT DBManager::fLoad(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		DEBUGPRINT("DBManager::fLoad", DEBUG_FUNCTIONSTART);
		DEBUGPRINT("unpacking args", DEBUG_BLOCKSTART);
		StringMap<const char*>::const_iterator iArg;
		const char *sArgName=0, *sArgValue=0;
		const char *sDb=0, *sFilepath=0;
		for (iArg=pArgs->begin();iArg!=pArgs->end();iArg++) {
			//unpack
			sArgName=iArg->first;
			sArgValue=iArg->second;
			DEBUGPRINT2("%s=%s", DEBUG_LINE, sArgName, sArgValue);
			if (!strncmp(sArgName, "db", 2)) sDb=sArgValue;
			else if (!strncmp(sArgName, "filepath", 8)) sFilepath=sArgValue;
		}
		DEBUGPRINT("unpacking args", DEBUG_BLOCKEND);

		//act
		if (sDb&&sFilepath) {
			DEBUGPRINT2("loading database to %s [%s]", DEBUG_CHECK, sDb, sFilepath);
			if (g_oServer->loadDB(sDb, sFilepath)) {
				DEBUG_RESULT_OK;
				hResult=S_OK;
			} else DEBUG_RESULT_FAIL;
		}

		//free up
		DEBUGPRINT("DBManager::fLoad", DEBUG_FUNCTIONEND);
		return hResult;
	}
}
