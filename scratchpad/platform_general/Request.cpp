//platform agnostic file
#include "Request.h"

#include "debug.h"
#include "RXSL.h"

namespace goofy {
	extern Server* g_oServer;

	DWORD WINAPI RequestStart(LPVOID lpParam) {
		_THREADINIT; {
			Request *pRequest=(Request*) lpParam;
			pRequest->process();
		} _THREADKILL;
		return 0;
	}

	Request::Request(Service *pService, SOCKET sc): m_pService(pService), m_sc(sc) {
		DEBUGPRINT("new request", DEBUG_LINE);
		m_pRequestDoc=0;
		m_pCommandDoc=0;
		m_sTextStream=0;

		m_pResponse=new Response(pService, sc);
		m_pTraceDoc=new XmlDoc("trace");

		linkStaticBuiltInModules();
		instanciateBuiltInModules();
		instanciateCustomModules();
	}

	Request::~Request() {
		if (m_pTraceDoc) delete m_pTraceDoc;
		if (m_pRequestDoc) delete m_pRequestDoc;
		if (m_pCommandDoc) delete m_pCommandDoc;
		if (m_pResponse) delete m_pResponse;
		if (m_sTextStream) free((void*) m_sTextStream);
	}
	const int Request::linkStaticBuiltInModules() {
		m_mModules.insert(_PAIR_MODULE("server", g_oServer));
		m_mModules.insert(_PAIR_MODULE("service", m_pService));
		//m_mModules.insert(_PAIR_MODULE("comms", m_pComms));
		return 2;
	}
	const int Request::instanciateBuiltInModules() {
		//add in the config, fileSystem and request dbs
		m_mModules.insert(_PAIR_MODULE("DB", new DBManager(this))); //alias for current DB
		//m_mModules.insert(_PAIR_MODULE("response", new Response(this)));
		m_mModules.insert(_PAIR_MODULE("request", this));
		m_mModules.insert(_PAIR_MODULE("response", m_pResponse));
		return 3;
	}
	const int Request::instanciateCustomModules() {
		//m_mModules.insert(_PAIR_MODULE("loadedFromDisk", new Thing()));
		return 0;
	}

	void Request::process() {
		XmlNode *pCommandRootNode=0, *pTraceRootNode=0;
		const XmlDoc *pResultDoc=0;
		//interpret commands (sending an XmlDoc and a text stream)
		if (SUCCEEDED(getTextStream(&m_sTextStream))) {
			//first translate the input http protocol text stream into an XML doc (according to the Service)
			if (m_pRequestDoc=Rxsl::rxsl(m_pService->m_pMessageInterpretation, m_sTextStream)) {
				DEBUGPRINT1("HTTP incoming call XML [%s]", DEBUG_LINE, m_pRequestDoc->xml());
				if (m_pRequestDoc->loadedAndParsed()) {
					//then transform it into an xml command list (according to the Service)
					if (m_pCommandDoc=m_pRequestDoc->transform(m_pService->m_pCommandsXSL)) {
						if (m_pCommandDoc->loadedAndParsed()) {
							//now recurse the command hierarchy running each command
							pCommandRootNode=m_pCommandDoc->rootNode_callerFree();
							pTraceRootNode=new XmlNode("trace", m_pTraceDoc);
							m_pTraceDoc->appendChild_callerFree(pTraceRootNode, false, false);
							recurseCommand(pCommandRootNode, pTraceRootNode, &pResultDoc);
							delete pCommandRootNode;
							delete pTraceRootNode;
						} else DEBUGPRINT("pCommandDoc not loadedAndParsed", DEBUG_LINE);
					}
				} else DEBUGPRINT("pRequestDoc not loadedAndParsed", DEBUG_LINE);
			}
		}

		if (!m_pService->m_bInteractive) closesocket(m_sc);
	}

	HRESULT Request::recurseCommand(const XmlNode *pCommandSpec, XmlNode *pParentTraceNode, const XmlDoc **ppResultDoc) {
		//return variables
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		//current node information
		XmlNode *pTraceNode=0;
		const char *sName=0, *sPrefix=0, *sBaseName=0;
		//children recursion
		XmlNodeList *aChildNodes=0;
		XmlNodeList::iterator iNode;
		//and result collection
		vector<const XmlDoc *> pChildResponseDocs;
		vector<const XmlDoc *>::iterator iChildResultDoc;
		const XmlDoc *pChildResultDoc=0;
		//finding the module function
		StringMap<Module*>::const_iterator iModules;
		Module *pModule=0;
		//sending args
		StringMap<const char*> vArgs;
		StringMap<const char*>::iterator iArg;
		XmlNodeList *vAttributes;
		XmlNodeList::iterator iArgNode;
		XmlNode *pArgNode=0;
		const char *sArgName=0, *sArgValue=0;

		//add this node into the results doc now (to maintain the correct hierarchy)
		sName=pCommandSpec->name();
		sPrefix=pCommandSpec->namespacePrefix();
		sBaseName=pCommandSpec->basename();
		DEBUGPRINT2("processing command [%s:%s]", DEBUG_BLOCKSTART, sPrefix, sBaseName);
		DEBUGPRINT("create trace node", DEBUG_LINE);
		pTraceNode=new XmlNode(sName, pParentTraceNode->m_pDoc);
		pParentTraceNode->appendChild_serverFree(pTraceNode, false, false); //no clone result=pTraceNode

		//recurse children first as their results will be fed into this nodes parameters
		DEBUGPRINT("recursing children", DEBUG_LINE);
		aChildNodes=pCommandSpec->getMultipleNodes("*"); //children::* (no comments or processing instructions)
		for (iNode=aChildNodes->begin();iNode!=aChildNodes->end();iNode++) {
			if (SUCCEEDED(recurseCommand(*iNode, pTraceNode, &pChildResultDoc))&&pChildResultDoc) {
				pChildResponseDocs.push_back(pChildResultDoc);
			}
		}
		delete aChildNodes;

		//process this node (passing it the results from the children)
		//construct the arguments map: client frees
		DEBUGPRINT("packing arguments", DEBUG_BLOCKSTART);
		vAttributes=pCommandSpec->getMultipleNodes("@*");
		for (iArgNode=vAttributes->begin();iArgNode!=vAttributes->end();iArgNode++) {
			pArgNode=*iArgNode;
			sArgName=pArgNode->basename();
			sArgValue=pArgNode->value();
			DEBUGPRINT2("inserting arg [%s]=[%s]", DEBUG_LINE, sArgName, sArgValue);
			vArgs.insert(_PAIR_ARG(sArgName, sArgValue));
		}
		delete vAttributes;
		DEBUGPRINT("packing arguments", DEBUG_BLOCKEND);

		//look for the module
		DEBUGPRINT1("looking for module [%s]", DEBUG_CHECK, sPrefix);
		if (sPrefix) {
			iModules=m_mModules.find(sPrefix);
			if (iModules!=m_mModules.end()) {
				DEBUG_RESULT_OK;
				//found the Module, get a pointer to it and look for the function...
				pModule=iModules->second;
				DEBUGPRINT("running command", DEBUG_CHECK);
				hResult=pModule->runCommand(sBaseName, &vArgs, &pChildResponseDocs, ppResultDoc, pCommandSpec);
				if (SUCCEEDED(hResult)) {
					DEBUG_RESULT_OK;
					DEBUGPRINT("checking for response document", DEBUG_CHECK);
					if (*ppResultDoc) {
						DEBUG_RESULT_OK;
#ifdef _RECORD_REQUESTS
						XmlNode *pResultDocRootNode=(*ppResultDoc)->rootNode_callerFree();
						pTraceNode->appendChild_serverFree(pResultDocRootNode, true, true);
						delete pResultDocRootNode;
#endif
					} else DEBUG_RESULT_FAIL;
				} else DEBUG_RESULT_FAIL;
			} else DEBUG_RESULT_FAIL;
		} else DEBUG_RESULT_FAIL;

		//free up
		for (iArg=vArgs.begin();iArg!=vArgs.end();iArg++) {free((void*) iArg->first);free((void*) iArg->second);}
		for (iChildResultDoc=pChildResponseDocs.begin();iChildResultDoc!=pChildResponseDocs.end();iChildResultDoc++) delete *iChildResultDoc;
		free((void*) sName);
		free((void*) sPrefix);
		free((void*) sBaseName);
		DEBUGPRINT("processing command", DEBUG_BLOCKEND);
		return hResult;
	}

	HRESULT Request::getTextStream(const char **sTextStream) const {
		//caller freed sTextStream
		HRESULT hResult=E_FAIL;
		int iBytesRead=0, iRemainingBufferSpace=0, iBufferUsed=0;
		char *sRecvBuff=0, *sRecvBuffPointer=0;
		*sTextStream=0;
		if (sRecvBuffPointer=(char*) malloc(DEFAULT_BUFFLEN)) {
			//read from the new client socket
			*sTextStream=sRecvBuff=sRecvBuffPointer;
			*sRecvBuffPointer=0; //null terminate the buffer immediately
			hResult=S_OK;
			do {
				//calculate the remaining buffer space we have (leaving space for the 0 terminating character)
				iBufferUsed=(int) (sRecvBuffPointer-sRecvBuff);
				iRemainingBufferSpace=(DEFAULT_BUFFLEN-iBufferUsed-1);
				iBytesRead=recv(m_sc, sRecvBuffPointer, iRemainingBufferSpace, 0);
				if (iBytesRead>0) {
					sRecvBuffPointer[iBytesRead]=0; //terminate the new request string
					DEBUGPRINT1("[%s]\n", DEBUG_LINE, sRecvBuffPointer);
					//increment pointer in recieve buffer
					sRecvBuffPointer+=iBytesRead;
					//check for return indicating completion of message (HTTP protocol)
					if (iBytesRead>3&&!strcmp(sRecvBuffPointer-4, "\r\n\r\n")) {
						iBytesRead=0; //exit loop
						DEBUGPRINT1("recieved all:\n%s\n\n", DEBUG_LINE, sRecvBuff);
					}
					if (iBytesRead==iRemainingBufferSpace) {iBytesRead=0;} //buffer limit reached!
				} else if (iBytesRead < 0) {
					//negative or null response indicating error
					DEBUGPRINT("socket error", DEBUG_LINE);
					hResult=E_UNEXPECTED;
					iBytesRead=0;
				}
			} while (iBytesRead > 0);
		} else hResult=E_OUTOFMEMORY;
		return hResult;
	}

	const string *Request::serialise() const {
		//caller freed (needs to be efficient)
		string *psSerialise=0;
		psSerialise=new string("<request>");
		//input stream
		psSerialise->append("<inputStream>");
		psSerialise->append(m_sTextStream);
		psSerialise->append("</inputStream>");
		//request doc
		psSerialise->append("<requestDoc>");
		char *sXML=m_pRequestDoc->xml();
		psSerialise->append(sXML);
		if (sXML) free(sXML);
		psSerialise->append("</requestDoc>");
		//request doc
		psSerialise->append("<commandsDoc>");
		sXML=m_pCommandDoc->xml();
		psSerialise->append(sXML);
		if (sXML) free(sXML);
		psSerialise->append("</commandsDoc>");
		//output analysis
		psSerialise->append("<trace>");
		sXML=m_pTraceDoc->xml();
		psSerialise->append(sXML);
		if (sXML) free(sXML);
		psSerialise->append("</trace>");

		psSerialise->append("</request>");
		return psSerialise;
	}
}

