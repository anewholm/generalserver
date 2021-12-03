//platform agnostic file
#include "DB.h"

#include "debug.h"

namespace goofy {
	DB::DB(const char *sAlias, XmlDoc *pDoc, const bool bReleaseXmlDoc): m_sAlias(_STRDUP(sAlias)), m_pDoc(pDoc), m_bReleaseXmlDoc(bReleaseXmlDoc) {
		m_bReadOnly = false;
	}
	DB::DB(const char *sAlias, const XmlDoc *pDoc, const bool bReleaseXmlDoc): m_sAlias(_STRDUP(sAlias)), m_pDoc((XmlDoc*) pDoc), m_bReleaseXmlDoc(bReleaseXmlDoc) {
		m_bReadOnly = true;
	}

	DB::~DB() {
		free((void*) m_sAlias);
		if (m_bReleaseXmlDoc) {
			DEBUGPRINT("unloading XML DOM", DEBUG_LINE);
			delete m_pDoc;
		}
		DEBUGPRINT("unloading stylesheets", DEBUG_BLOCKSTART);
		{
			StringMap<XmlDoc*>::iterator iSS;
			for (iSS = m_mStyleSheets.begin(); iSS != m_mStyleSheets.end();iSS++) {
				DEBUGPRINT1("unloading stylesheet %s", DEBUG_LINE, iSS->first);
				delete iSS->second;
			}
		}
		DEBUGPRINT("unloading stylesheets", DEBUG_BLOCKEND);
		DEBUGPRINT("unloading command files", DEBUG_BLOCKSTART);
		{
			StringMap<XmlDoc*>::iterator iCF;
			for (iCF = m_mCommandFiles.begin(); iCF != m_mCommandFiles.end(); iCF++) {
				DEBUGPRINT1("unloading command file %s", DEBUG_LINE, iCF->first);
				delete iCF->second;
			}
		}
		DEBUGPRINT("unloading command files", DEBUG_BLOCKEND);
		DEBUGPRINT("unloading transforms", DEBUG_BLOCKSTART);
		{
			StringMap<StringMap<XmlDoc*>* >::iterator iTransform;
			for (iTransform = m_mTransforms.begin(); iTransform != m_mTransforms.end(); iTransform++) delete iTransform->second;
			DEBUGPRINT1("%i nodes", DEBUG_LINE, m_mTransforms.size());
		}
		DEBUGPRINT("unloading transforms", DEBUG_BLOCKEND);
	}
	XmlDoc *DB::findCommandFile(const char *sCFAlias) const {
		DEBUGPRINT1("check for command file alias [%s]", DEBUG_CHECK, sCFAlias);
		XmlDoc *pCF = 0;
		StringMap<XmlDoc*>::const_iterator iCommandFile;
		iCommandFile = m_mCommandFiles.find_const(sCFAlias);
		if (iCommandFile == m_mCommandFiles.end()) DEBUG_RESULT_FAIL;
		else {
			pCF = iCommandFile->second;
			DEBUG_RESULT_OK;
		}
		return pCF;
	}
	XmlDoc *DB::findXSLDoc(const char *sSSAlias) const {
		DEBUGPRINT1("check for stylesheet alias [%s]", DEBUG_CHECK, sSSAlias);
		XmlDoc *pSS=0;
		StringMap<XmlDoc*>::const_iterator iStyleSheet;
		iStyleSheet=m_mStyleSheets.find_const(sSSAlias);
		if (iStyleSheet==m_mStyleSheets.end()) DEBUG_RESULT_FAIL;
		else {
			pSS=iStyleSheet->second;
			DEBUG_RESULT_OK;
		}
		return pSS;
	}

	Index *DB::createIndex(const char *sIXAlias, const char *sType, const char *sXPath) {
		Index *pIndex=0;
		//if (Index::create(sType, &pIndex)) {
		//}
		return pIndex;
	}
	const int DB::associateTransformsToNodes(const char *sRef, const char *sXPathToNodes, const char *sSSAlias) {
		int iNodesAffected=0;
		XmlNodeList *vNodes=0;
		XmlNodeList::iterator iNode;
		XmlNode *oNode=0;
		const char *sNodeID=0;
		StringMap<StringMap<XmlDoc*>* >::iterator iCurrent;
		StringMap<XmlDoc*>* pmNodeStageTransforms;
		DEBUGPRINT3("loading transform ref:%s SS:%s nodes:%s", DEBUG_BLOCKSTART, sRef, sSSAlias, sXPathToNodes);
		//see if the stylesheet alias is valid
		XmlDoc *pSS=0;
		if (pSS=findXSLDoc(sSSAlias)) {
			//get the nodes in question
			vNodes=m_pDoc->getMultipleNodes(sXPathToNodes);
			for (iNode=vNodes->begin();iNode!=vNodes->end();iNode++) {
				//get the node
				oNode=(*iNode);
				sNodeID=oNode->attribute("id")->value();
				//DEBUGPRINT1("%s", DEBUG_LINE, sNodeID);
				iCurrent=m_mTransforms.find(sNodeID);
				if (iCurrent==m_mTransforms.end()) {
					pmNodeStageTransforms=new StringMap<XmlDoc*>;
					m_mTransforms.insert(_PAIR_NODETRANSFORMS(sNodeID, pmNodeStageTransforms));
				} else pmNodeStageTransforms=iCurrent->second;
				pmNodeStageTransforms->insert(_PAIR_STAGETRANSFORMS(sRef, pSS));
			}
			iNodesAffected=(const int) vNodes->size();
			delete vNodes;
			DEBUGPRINT1("%i nodes", DEBUG_LINE, iNodesAffected);
		}
		DEBUGPRINT("loading transform", DEBUG_BLOCKEND);
		return iNodesAffected;
	}


	const string *DB::serialise() const {
		//caller freed (needs to be efficient)
		string *psSerialise=new string("");
		char sConfigXML[1024];
		_SNPRINTF6(sConfigXML, 1024, "<DB alias=\"%s\" file=\"%s\" loadedAndParsed=\"%s\" resolveExternals=\"%s\" specified=\"%s\" validateOnParse=\"%s\">", 
			m_sAlias, 
			m_pDoc->repository(),
			(m_pDoc->loadedAndParsed()?"yes":"no"),
			(m_pDoc->resolveExternals()?"yes":"no"),
			(m_pDoc->specified()?"yes":"no"),
			(m_pDoc->validateOnParse()?"yes":"no")
			);
		psSerialise->append(sConfigXML);
		//--------------------------- stylesheets
		psSerialise->append("<stylesheets>");
		StringMap<XmlDoc*>::const_iterator iStylesheet;
		for (iStylesheet = m_mStyleSheets.begin();iStylesheet!=m_mStyleSheets.end();iStylesheet++) {
			_SNPRINTF2(sConfigXML, 1024, "<stylesheet alias=\"%s\" file=\"%s\"/>", iStylesheet->second->m_sAlias, iStylesheet->second->repository());
			psSerialise->append(sConfigXML);
		}
		psSerialise->append("</stylesheets>");
		//--------------------------- command files
		psSerialise->append("<commandFiles>");
		StringMap<XmlDoc*>::const_iterator iCommandFile;
		for (iCommandFile=m_mCommandFiles.begin();iCommandFile!=m_mCommandFiles.end();iCommandFile++) {
			_SNPRINTF2(sConfigXML, 1024, "<commandFile alias=\"%s\" file=\"%s\"/>", iCommandFile->second->m_sAlias, iCommandFile->second->repository());
			psSerialise->append(sConfigXML);
		}
		psSerialise->append("</commandFiles>");
		//--------------------------- transforms
		psSerialise->append("<transforms>");
		StringMap<StringMap<XmlDoc*>* >::const_iterator iTransform;
		for (iTransform=m_mTransforms.begin();iTransform!=m_mTransforms.end();iTransform++) {
			_SNPRINTF1(sConfigXML, 1024, "<transform nodeID=\"%s\"/>", iTransform->first);
			psSerialise->append(sConfigXML);
		}
		psSerialise->append("</transforms>");
		psSerialise->append("</DB>");
		return psSerialise;
	}


	//commands
	HRESULT DB::select(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc, const bool bDeepCloneXPath) const {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		char *sFromPlace=0;
		XmlNode *pStartNode=0;
		DEBUGPRINT3("DB::select(%s, %s, %s)", DEBUG_FUNCTIONSTART, m_sAlias, sContext, sWhat);

		//from
		if (!sContext||!*sContext) {
			DEBUGPRINT("no from, starting from the root", DEBUG_LINE);
			pStartNode=m_pDoc->rootNode_callerFree();
		} else {
			if (!strncmp(sContext, "ID:", 3)) {
				//we have an ID request to use as the start point for the selection
				DEBUGPRINT1("starting from (ID) %s", DEBUG_LINE, sContext+3);
				pStartNode=m_pDoc->nodeFromID_callerFree(sContext+3);
			} else if (!strncmp(sContext, "xpath:", 6)) {
				//we have an ID request to use as the start point for the selection
				DEBUGPRINT1("starting from %s", DEBUG_LINE, sContext+6);
				pStartNode=m_pDoc->getSingleNode_callerFree(sContext+6);
			} else {
				DEBUGPRINT("not implemented", DEBUG_LINE);
				hResult=E_NOTIMPL;
			}
		}

		//what
		DEBUGPRINT("checking start node", DEBUG_CHECK);
		if (pStartNode) {
			DEBUG_RESULT_OK;
			if (!sWhat||!*sWhat) {
				DEBUGPRINT("copying complete DB from startNode", DEBUG_LINE);
				//select whole document
				*ppResultDoc = new XmlDoc("result", pStartNode);
				hResult=S_OK;
			} else if (!strncmp(sWhat, "xsl:", 4)) {
				DEBUGPRINT("loaded xsl transformation", DEBUG_LINE);
				DEBUGPRINT1("looking for xsl [%s]", DEBUG_CHECK, sWhat+4);
				XmlDoc *pXSL=findXSLDoc(sWhat+4);
				if (pXSL) {
					DEBUG_RESULT_OK;
					hResult=S_OK;
					*ppResultDoc=pStartNode->transform(pXSL);
				} else DEBUG_RESULT_FAIL;
			} else if (!strncmp(sWhat, "xpath:", 6)) {
				DEBUGPRINT("direct xpath request", DEBUG_LINE);
				XmlNodeList *pResultnodes=pStartNode->getMultipleNodes(sWhat+6);
				DEBUGPRINT("running XPath", DEBUG_CHECK);
				if (pResultnodes) {
					DEBUG_RESULT_OK;
					hResult=S_OK;
					*ppResultDoc=new XmlDoc("result", pResultnodes, "root", bDeepCloneXPath);
					delete pResultnodes;
#ifdef _DEBUG
					const char *sXML=(*ppResultDoc)->xml();
					DEBUGPRINT1("doc:\n%s\n\n", DEBUG_LINE, sXML);
					free((void*) sXML);
#endif
				} else DEBUG_RESULT_FAIL;
			} else {
				DEBUGPRINT("not implemented", DEBUG_LINE);
				hResult=E_NOTIMPL;
			}
		} else DEBUG_RESULT_FAIL;

		//free up
		if (pStartNode) delete pStartNode;
		DEBUGPRINT("DB::select", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DB::insert(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		char *sFromPlace=0;
		XmlNode *pStartNode=0;
		DEBUGPRINT3("DB::insert(%s, %s, %s)", DEBUG_FUNCTIONSTART, m_sAlias, sContext, sWhat);

		//from
		if (!sContext||!*sContext) {
			DEBUGPRINT("no from, starting from the root", DEBUG_LINE);
			pStartNode=m_pDoc->rootNode_callerFree();
		} else {
			if (!strncmp(sContext, "ID:", 3)) {
				//we have an ID request to use as the start point for the selection
				char *sXPathFormat="//*[@id='%s']";
				int iLen=(int) (strlen(sContext+3)+strlen(sXPathFormat));
				sFromPlace=(char*) malloc(iLen+1);
				_SNPRINTF1(sFromPlace, iLen, sXPathFormat, sContext+3);
				DEBUGPRINT1("starting from %s", DEBUG_LINE, sFromPlace);
				pStartNode=m_pDoc->getSingleNode_callerFree(sFromPlace);
				free(sFromPlace);
			} else if (!strncmp(sContext, "xpath:", 6)) {
				//we have an ID request to use as the start point for the selection
				DEBUGPRINT1("starting from %s", DEBUG_LINE, sContext+6);
				pStartNode=m_pDoc->getSingleNode_callerFree(sContext+6);
			} else {
				DEBUGPRINT("not implemented", DEBUG_LINE);
				hResult=E_NOTIMPL;
			}
		}

		//what
		DEBUGPRINT("checking start node", DEBUG_CHECK);
		if (pStartNode) {
			DEBUG_RESULT_OK;
			if (!sWhat||!*sWhat) {
				DEBUGPRINT("nothing to insert", DEBUG_LINE);
				*ppResultDoc=0;
				hResult=E_INVALIDARG;
			} else {
				DEBUGPRINT("attempting parse of new node", DEBUG_CHECK);
				XmlDoc *pNewSubTree=new XmlDoc(sWhat);
				if (!pNewSubTree->loadedAndParsed()) {
					*ppResultDoc=0;
					hResult=E_INVALIDARG;
					DEBUG_RESULT_FAIL;
					delete pNewSubTree;
				} else {
					DEBUG_RESULT_OK;
					XmlNode *pNewRootNode=pNewSubTree->rootNode_callerFree();
					XmlNode *pNewNode=pStartNode->appendChild_callerFree(pNewRootNode, true, true);
					delete pNewNode;
					delete pNewRootNode;
					*ppResultDoc=pNewSubTree;
					hResult=S_OK;
				}
			}
		} else DEBUG_RESULT_FAIL;

		//free up
		if (pStartNode) delete pStartNode;
		DEBUGPRINT("DB::insert", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DB::update(const char *sContext, const char *sWhat, const char *sTo, const XmlDoc **ppResultDoc) {
		HRESULT hResult=E_FAIL;
		*ppResultDoc=0;
		char *sFromPlace=0;
		XmlNode *pStartNode=0;
		DEBUGPRINT4("DB::update(%s, %s, %s, %s)", DEBUG_FUNCTIONSTART, m_sAlias, sContext, sWhat, sTo);

		//from
		if (!strncmp(sContext, "ID:", 3)) {
			//we have an ID request to use as the start point for the selection
			char *sXPathFormat="//*[@id='%s']";
			int iLen=(int) (strlen(sContext+3)+strlen(sXPathFormat));
			sFromPlace=(char*) malloc(iLen+1);
			_SNPRINTF1(sFromPlace, iLen, sXPathFormat, sContext+3);
			DEBUGPRINT1("starting from %s", DEBUG_LINE, sFromPlace);
			pStartNode=m_pDoc->getSingleNode_callerFree(sFromPlace);
			free(sFromPlace);
		} else if (!strncmp(sContext, "xpath:", 6)) {
			//we have an ID request to use as the start point for the selection
			DEBUGPRINT1("starting from %s", DEBUG_LINE, sContext+6);
			pStartNode=m_pDoc->getSingleNode_callerFree(sContext+6);
		} else {
			DEBUGPRINT("not implemented", DEBUG_LINE);
			hResult=E_NOTIMPL;
		}

		//what
		DEBUGPRINT("checking start node", DEBUG_CHECK);
		if (pStartNode) {
			DEBUG_RESULT_OK;
			if (!sWhat||!*sWhat) {
				DEBUGPRINT("nothing to insert", DEBUG_LINE);
				*ppResultDoc=0;
				hResult=E_INVALIDARG;
			} else {
				DEBUGPRINT("checking protocol of what", DEBUG_CHECK);
				if (!strncmp(sWhat, "xpath:", 6)) {
					DEBUG_RESULT_OK;
					DEBUGPRINT("getting node to update", DEBUG_CHECK);
					XmlNode *pNodeToUpdate=pStartNode->getSingleNode_callerFree(sWhat+6);
					if (pNodeToUpdate) {
						DEBUG_RESULT_OK;
						DEBUGPRINT("setting value", DEBUG_LINE);
						if (!sTo) sTo="";
						pNodeToUpdate->value(sTo);
						delete pNodeToUpdate;
						*ppResultDoc=0;
						hResult=S_OK;
					} else {
						DEBUG_RESULT_FAIL;
						*ppResultDoc=0;
						hResult=E_FAIL;
					}
				} else {
					DEBUGPRINT("not implemented", DEBUG_RESULT);
					*ppResultDoc=0;
					hResult=E_NOTIMPL;
				}
			}
		} else DEBUG_RESULT_FAIL;

		//free up
		if (pStartNode) delete pStartNode;
		DEBUGPRINT("DB::update", DEBUG_FUNCTIONEND);
		return hResult;
	}
	HRESULT DB::remove(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc) {
		*ppResultDoc=0;
		return E_NOTIMPL;
	}


}
