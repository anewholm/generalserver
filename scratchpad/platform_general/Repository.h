//platform agnostic file
#ifndef _REPOSITORY_H
#define _REPOSITORY_H

#include "StringMap.h"
#include "define.h"

namespace goofy {
	class Repository {
		//Permanent Store of information
		//Usually a wrapper for the FileSystem, but could be a wrapper for another store

	private:
	protected:
		const char *m_sName;
		const char *m_xml_filename; //completed on creation
		size_t m_lSizeXML;
		Repository(const char *_name);
		virtual const char *xml_filename() const;

	public:
		//Factory pattern: http://en.wikipedia.org/wiki/Factory_method_pattern#Definition
		//either create a subclass based on the format of the sFilePath
		//or directly create a subclass, e.g. Directory
		static Repository *factory(const char *sFilePath);
		~Repository();

		static const bool gimme(const char *sFilePath) {return false;}
		virtual const size_t size_xml();
		virtual char *read(char *buffer = 0) = 0;
		virtual const char *source() const;

		//for URLs, UNIX and Windows
		static const int issplitter(char c) {return c == '/' || c == '\\';}
		static const char splitter() {return _PATH_SPLITTER;}
	};
}

#endif
