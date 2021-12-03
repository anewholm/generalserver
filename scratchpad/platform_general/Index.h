//platform agnostic file
#ifndef _INDEX_H
#define _INDEX_H

//std library includes
#include <vector>
#include <map>
using namespace std;

namespace goofy {
	class Index {
		//External indexing for the XML database, Various types:
		//numeric array: sequential slots with gaps, gap re-use is ok with version number to avoid ghosts
		//  useful for webpage id requests /?id=345 -> 32 bit webpage node pointer in DB
	};
}
#endif
