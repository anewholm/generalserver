//platform agnostic file
#ifndef _DBMANAGER_H
#define _DBMANAGER_H

#include "debug.h"
#include "DB.h"
#include "Request.h"
#include "module.h"

namespace goofy {
	class DBManager: public Module {
		//wrapper for Databases, selects the current one (USE)
		Request *m_pRequest;
		DB *pCurrentDefaultDB;
	public:
		DBManager(Request *pRequest);
		DB *getDB(const char *sDB, const bool *bCallerFree);
		
		//dispatch functions
		HRESULT fSelect(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fDefault(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fRemove(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fInsert(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fUpdate(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fUnload(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
		HRESULT fLoad(const StringMap<const char*> *pArgs, const vector<const XmlDoc *> *pChildResponseDocs, const XmlDoc **ppResultDoc, const XmlNode *pCommandSpec);
	};
}

#endif
