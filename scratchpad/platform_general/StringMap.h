//platform agnostic file
#ifndef _STRINGMAP_H
#define _STRINGMAP_H

#include <map>
using namespace std;

namespace goofy {
	template<class T> class StringMap: public map<const char*, T> {
	public:
		typename StringMap::iterator find(const char *sKey) {
			typename StringMap::iterator iItem;
			for (iItem=begin();iItem!=end();iItem++) if (!strcmp(sKey, iItem->first)) break;
			return iItem;
		}
		typename StringMap::const_iterator find_const(const char *sKey) const {
			typename StringMap::const_iterator iItem;
			for (iItem=begin();iItem!=end();iItem++) if (!strcmp(sKey, iItem->first)) break;
			return iItem;
		}
	};
}

#endif
