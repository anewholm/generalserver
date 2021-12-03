//std library includes
#include <map>
using namespace std;

//platform agnostic project headers
#include "Server.h"

//platform specific project headers
#include "define.h" //_WINDOWS, _LOGFILE, etc.
#include "debug.h"	//logging functions (windows event log/file)
#include "Thread.h"	//Threading

namespace goofy {
	Server *g_oServer;

	DB *Server::loadDB(const char *sDBAlias, const char *sRelativeFilePath) {
		//callers resposibilty to free their sDBAlias and sRelativeFilePath
		//DB and XmlDoc make copies for their own uses
		DEBUGPRINT2("loading database %s [%s]", DEBUG_CHECK, sDBAlias, sRelativeFilePath);
		size_t iLen = strlen(_CONFIGFILE) + strlen(sRelativeFilePath);
		char *sFilePath=(char*) malloc(iLen+1);
		DB *pDB=0;
		_SNPRINTF2(sFilePath, iLen+1, "%s%s", _CONFIGFILE, sRelativeFilePath);
		DEBUGPRINT1("from [%s]", DEBUG_LINE, sFilePath);
		XmlDoc *pXmlDoc = new XmlDoc(sDBAlias, sFilePath); //~XmlDoc will release the repository
		if (pXmlDoc->loadedAndParsed()) {
			DEBUG_RESULT_OK;
			pDB=new DB(sDBAlias, pXmlDoc);
			//add the DB into the servers map
			m_mDBs.insert(_PAIR_ALIAS_DB(_STRDUP(sDBAlias), pDB));
		} else {
			DEBUG_RESULT_FAIL;
			delete pXmlDoc;
		}
		free(sFilePath);
		return pDB;
	}

	HRESULT Server::unloadDB(const char *sDBAlias) {
		HRESULT hResult=E_FAIL;
		DEBUGPRINT1("finding database %s", DEBUG_CHECK, sDBAlias);
		StringMap<DB*>::iterator iDB=m_mDBs.find(sDBAlias);
		if (iDB!=m_mDBs.end()) {
			DEBUG_RESULT_OK;
			delete iDB->second; //the delete will trigger the destruction of the smart pointer and its resources
			m_mDBs.erase(iDB);
			hResult=S_OK;
		} else DEBUG_RESULT_FAIL;
		return hResult;
	}

	Index *Server::createIndexToDB(DB *pDB, const char *sIXAlias, const char *sType, const char *sXPath) {
		return pDB->createIndex(sIXAlias, sType, sXPath);
	}

	XmlDoc *Server::addXSLToDB(DB *pDB, const char *sSSAlias, const char *sSSFilePath) {
		DEBUGPRINT2("loading stylesheet(s) %s [%s]", DEBUG_CHECK, sSSAlias, sSSFilePath);
		XmlDoc *pXSLDoc=new XmlDoc(sSSAlias, sSSFilePath);
		if (pXSLDoc->loadedAndParsed()) {
			DEBUG_RESULT_OK;
			pDB->addStyleSheet(_PAIR_STYLESHEET(sSSAlias, pXSLDoc));
		} else {
			delete pXSLDoc;
			DEBUG_RESULT_FAIL;
		}
		return pXSLDoc;
	}
	XmlDoc *Server::addCommandFileToDB(DB *pDB, const char *sCFAlias, const char *sCFFilePath) {
		DEBUGPRINT2("loading command file(s) %s [%s]", DEBUG_CHECK, sCFAlias, sCFFilePath);
		XmlDoc *pCFDoc=new XmlDoc(sCFAlias, sCFFilePath);
		if (pCFDoc->loadedAndParsed()) {
			DEBUG_RESULT_OK;
			pDB->addCommandFile(_PAIR_STYLESHEET(sCFAlias, pCFDoc));
		} else {
			delete pCFDoc;
			DEBUG_RESULT_FAIL;
		}
		return pCFDoc;
	}

	const int Server::associateTransformsToNodes(DB *pDB, const char *sRef, const char *sXPathToNodes, const char *sSSAlias) {
		return pDB->associateTransformsToNodes(sRef, sXPathToNodes, sSSAlias);
	}

	Server::Server() {
		//------------------------------------ startup process --------------------------------------
		DEBUGPRINT("goofyStartup", DEBUG_BLOCKSTART);
		{
			//Singleton
			g_oServer = this;
			DEBUGPRINT4("version: %i.%i [%i] %s", DEBUG_LINE, _VERSION_MAJOR, _VERSION_MINOR, _VERSION_BUILD, _VERSION_TYPE);
#ifdef _DEBUG
			DEBUGPRINT("(debug mode)", DEBUG_LINE);
#endif

			DEBUGPRINT("loading configuration files", DEBUG_BLOCKSTART);
			{
				//base configuration
				//base configuration should be a directory which will cause the Repository::factory to return a Directory() subclass
				//this will then read in everything and parse the XML into an XML DOC
				//all databases are read here so don't put stuff here if you dont want it in memory
				DEBUGPRINT1("_CONFIGFILE [%s]", DEBUG_LINE, _CONFIGFILE);
				m_config = new XmlDoc("config", _CONFIGFILE, true); //true simply indicates that it is a file name
				//const char *xml = m_config->xml(); //DEBUG: show XML

				//------------------------------------------------------
				//------------------------------------------------------ enumerate databases
				//------------------------------------------------------
				DEBUGPRINT("enumerate databases", DEBUG_BLOCKSTART);
				{
					//get all the directories under databases
					DB *pDB = 0;					//DB object
					XmlNode *pDBNode = 0;			//database folder
					XmlNode *pXmlDBDataNode = 0;	//node under the database for db.xml
					XmlDoc *pXmlDBDataDoc = 0;		//actual database xml
					const char *sName;

					XmlNodeList::iterator iNode;
					//XmlNodeList will delete pDBNodes after loop
					XmlNodeList *vDatabases = m_config->getMultipleNodes("//databases/*");
					for (iNode = vDatabases->begin(); iNode!=vDatabases->end(); iNode++) {
						pDBNode = (*iNode);
						sName = pDBNode->name();

						//create the DB with a doc pointing to its db.xml node in config is available
						//this will remove the node from config and place it in the DB document
						//we allow empty databases
						//only the first root node is valid
						//pXmlDBDataNode will get appended to pXmlDBDataDoc and released when Doc is released
						pXmlDBDataNode = pDBNode->getSingleNode_callerFree("db_xml/*[1]");
						if (pXmlDBDataNode) {
							//link node in
							//~XmlDoc does not delete pXmlDBDataNode
							pXmlDBDataDoc = new XmlDoc(sName, pXmlDBDataNode); 
						} else pXmlDBDataDoc = 0;
						pDB = new DB(sName, pXmlDBDataDoc); //DB releases its doc on delete
						m_mDBs.insert(_PAIR_ALIAS_DB(_STRDUP(sName), pDB));


						//successful load of the primary DB XML Document, load other DB components
						//------------------------------------------------------ load general transforms
						DEBUGPRINT("loading personal stylesheets", DEBUG_BLOCKSTART);
						{
							XmlNodeList *vStyleSheetNodes;
							XmlNode *pStyleSheetNode = 0;
							XmlNode *pStyleSheetRoot = 0;
							XmlNodeList::iterator iStyleSheet;
							const char *sStyleSheetName;

							vStyleSheetNodes = pDBNode->getMultipleNodes("transforms/*");
							for (iStyleSheet = vStyleSheetNodes->begin(); iStyleSheet != vStyleSheetNodes->end(); iStyleSheet++) {
								pStyleSheetNode = (*iStyleSheet);
								if (pStyleSheetRoot = pStyleSheetNode->getSingleNode_serverFree("*[1]")) {
									sStyleSheetName = pStyleSheetNode->name();
									pDB->addStyleSheet(_PAIR_STYLESHEET(_STRDUP(sStyleSheetName), new XmlDoc(sStyleSheetName, pStyleSheetRoot)));
									free((void*) sStyleSheetName);
								}
							}
							delete vStyleSheetNodes;
						}
						DEBUGPRINT("loading personal stylesheets", DEBUG_BLOCKEND);

						//------------------------------------------------------ calculate indexes
						/*
						DEBUGPRINT("loading indexes", DEBUG_BLOCKSTART);
						{
							XmlNodeList *vIndexSpecs;
							XmlNode *pIndexSpec=0; //specification lines in the config file
							XmlNodeList::iterator iIndex;
							const char *sIXAlias=0, *sType=0, *sXPath=0;

							vIndexSpecs=pDBSpec->getMultipleNodes(".//index");
							for (iIndex=vIndexSpecs->begin();iIndex!=vIndexSpecs->end();iIndex++) {
								pIndexSpec=(*iIndex);
								sType=pIndexSpec->attribute("type")->value();
								sIXAlias=pIndexSpec->attribute("name")->value();
								sXPath=pIndexSpec->attribute("scope")->value();
								createIndexToDB(pDB, sIXAlias, sType, sXPath);
								free((void*) sIXAlias);
								free((void*) sType);
								free((void*) sXPath);
							}
							delete vIndexSpecs;
						}
						DEBUGPRINT("loading indexes", DEBUG_BLOCKEND);
						*/
						free((void*) sName);
					}
					//delete vDatabases will delete all pDBNode (*iNode)
					delete vDatabases;
				}
				DEBUGPRINT("databases", DEBUG_BLOCKEND);

				//------------------------------------------------------
				//------------------------------------------------------ load ports and input text stream transforms
				//------------------------------------------------------
				DEBUGPRINT("services", DEBUG_BLOCKSTART);
				{
					XmlNodeList *vServices = m_config->getMultipleNodes("//services/*");
					XmlNode *pServiceNode = 0; //service directory
					XmlNodeList::iterator iNode;
					XmlNode *pMessageInterpretationSpec = 0, *pCommandsTransformationSpec = 0;
					const char *sServiceName = 0;
					const char *sPort = 0;
					const char *sSentenceFinish = "\n";
					const char *sInteractive = "true";
					bool bInteractive = true;
					int iPort;
					Service *pService = 0;

					for (iNode = vServices->begin(); iNode != vServices->end(); iNode++) {
						pServiceNode    = (*iNode); //specification of the Database being loaded
						sServiceName    = pServiceNode->name();
						sPort           = strchr(sServiceName, '_') + 1;

						DEBUGPRINT3("starting service %s [%s]", DEBUG_CHECK, sServiceName, sPort, sSentenceFinish);
						//kick off a new thread for each service
						bInteractive = !strcmp(sInteractive, "true");
						iPort = atoi(sPort);
						//message interpretation (need to expand this to accept several with regex matches to decide which to run)
						/*
						pMessageInterpretationSpec = pServiceNode->getSingleNode_serverFree(".//messageInterpretation");
						DEBUGPRINT1("loading message interpretation [%s]", DEBUG_LINE, pMessageInterpretationSpec->attributeValue("name"));
						//commands transform
						pCommandsTransformationSpec = pMessageInterpretationSpec->getSingleNode_serverFree(".//commandsTransformation");
						DEBUGPRINT1("loading commands transformation [%s]", DEBUG_LINE, pCommandsTransformationSpec->attributeValue("name"));
						//create service
						DEBUGPRINT1("creating service [%s]", DEBUG_LINE, sServiceName);
						pService = new Service(sServiceName, iPort, bInteractive, sSentenceFinish,
							new XmlDoc(pMessageInterpretationSpec->firstChild_serverFree()),
							new XmlDoc(pCommandsTransformationSpec->firstChild_serverFree()));
						m_mServices.insert(_PAIR_SERVICE(sServiceName, pService));
						new Thread(ServiceStart, pService);
						*/
						DEBUG_RESULT_OK;
					}
					delete vServices;
				}
				DEBUGPRINT("services", DEBUG_BLOCKEND);
			}
			DEBUGPRINT("loading configuration files", DEBUG_BLOCKEND);
		}
		DEBUGPRINT("gsStartup", DEBUG_BLOCKEND);
	}

	Server::~Server() {
		DEBUGPRINT("unloading databases", DEBUG_BLOCKSTART);
		{
			StringMap<DB*>::iterator iDB;
			for (iDB = m_mDBs.begin(); iDB != m_mDBs.end(); iDB++) {
				DEBUGPRINT1("unloading database %s", DEBUG_BLOCKSTART, iDB->first);
				delete iDB->second;
				DEBUGPRINT("unloading database", DEBUG_BLOCKEND);
			}
		}
		delete m_config;
		DEBUGPRINT("unloading databases", DEBUG_BLOCKEND);
	}
	DB *Server::findDB(const char *sDBAlias) const {
		DB *pDB=0;
		const StringMap<DB*>::const_iterator iDB=m_mDBs.find_const(sDBAlias);
		if (iDB!=m_mDBs.end()) pDB=iDB->second;
		return pDB;
	}
	const string *Server::currentHDD() const {
		return new string;
	}
	const string *Server::configFile() const {
		return new string;
	}
	const string *Server::serialise() const {
		//caller freed
		string *psCurrentState=new string("<server xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" \
			xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" \
			xmlns:rxsl=\"http://www.poptech.coop/xmlnamespaces/rxsl/2006\" \
			xmlns:goofy=\"http://www.poptech.coop/xmlnamespaces/goofy/2006\" \
			xmlns:DB=\"http://www.poptech.coop/xmlnamespaces/DB/2006\" \
			xmlns:server=\"http://www.poptech.coop/xmlnamespaces/server/2006\" \
			xmlns:response=\"http://www.poptech.coop/xmlnamespaces/response/2006\" \
			xmlns:comms=\"http://www.poptech.coop/xmlnamespaces/comms/2006\" \
			>");
		const string *psXML=0;
		char sVersion[1024];

		//-------------------------------- add defines
		psCurrentState->append("<definitions>");
		psCurrentState->append("<CONFIGFILE><![CDATA[");
		psCurrentState->append(_CONFIGFILE);
		psCurrentState->append("]]></CONFIGFILE>");
		psCurrentState->append("<DEBUG>");
#ifdef _DEBUG
		psCurrentState->append("on");
#else
		psCurrentState->append("off");
#endif
		psCurrentState->append("</DEBUG>");
		psCurrentState->append("<WIN32>");
#ifdef _WIN32
		psCurrentState->append("on");
#else
		psCurrentState->append("off");
#endif
		psCurrentState->append("</WIN32>");
		psCurrentState->append("<UNIX>");
#ifdef _UNIX
		psCurrentState->append("on");
#else
		psCurrentState->append("off");
#endif
		psCurrentState->append("</UNIX>");
		_SNPRINTF4(sVersion, 1024, "version: %i.%i [%i] %s", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_BUILD, _VERSION_TYPE);
		psCurrentState->append("<VERSION><![CDATA[");
		psCurrentState->append(sVersion);
		psCurrentState->append("]]></VERSION>");
		psCurrentState->append("</definitions>");

		//-------------------------------- config
		FILE *pFile;
		char sConfigXML[20001];
		if (_FOPEN(pFile, _CONFIGFILE, "r")) {}
		size_t iBytes=fread(sConfigXML, 1, 20000, pFile);
		fclose(pFile);
		sConfigXML[iBytes]=0;
		psCurrentState->append(sConfigXML+39);

		//-------------------------------- add DBs
		StringMap<DB*>::const_iterator iDB; //due to the const nature of the function
		const DB *pDB=0;
		psCurrentState->append("<DBs>");
		for (iDB=m_mDBs.begin();iDB!=m_mDBs.end();iDB++) {
			pDB=iDB->second;
			psXML=pDB->serialise();
			psCurrentState->append(*psXML);
			delete psXML;
		}
		psCurrentState->append("</DBs>");

		//-------------------------------- add services
		StringMap<Service*>::const_iterator iService; //due to the const nature of the function
		const Service *pService=0;
		psCurrentState->append("<services>");
		for (iService=m_mServices.begin();iService!=m_mServices.end();iService++) {
			pService=iService->second;
			psXML=pService->serialise();
			psCurrentState->append(*psXML);
			delete psXML;
			//-------------------------------- add requests
		}
		psCurrentState->append("</services>");

		psCurrentState->append("</server>");
		return psCurrentState;
	}
}
