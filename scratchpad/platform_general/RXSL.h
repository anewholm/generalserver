#ifndef _RXSL_H
#define _RXSL_H

#include "xml.h"
#include "stringMap.h"
#include "define.h"
#include <vector>
using namespace std;
#include "regexpr2.h"
using namespace regex;

#define _PAIR_DIRECTIVE pair<const char*, match_results_c*>

namespace goofy {
	class Rxsl {
		StringMap<match_results_c*> m_directives;
		vector<char*> m_vScopeTexts;
	public:
		~Rxsl();
		static const XmlDoc *rxsl(const XmlDoc *pRxsl, const char *sTextStream) {
			Rxsl rxsl;
			return rxsl.process(pRxsl, sTextStream);
		}
		const XmlDoc *process(const XmlDoc *pRxsl, const char *sTextStream);
	private:
		void recurseNode(XmlNode *pInputNode, XmlNode *pOutputNode, const char *sCurrentTextStream, match_results_c *pmResults);
		char *copyString(const char *pStart, const char *pFinish) {
			//caller freed copy of string
			assert(pStart&&pFinish);
			int iLen=(int) (pFinish-pStart);
			char *sMatch=(char*) malloc(iLen+1);
			_STRNCPY(sMatch, pStart, iLen);
			return sMatch;
		}
	};
}

#endif

