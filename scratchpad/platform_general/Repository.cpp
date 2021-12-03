#include "Repository.h"
#include "FileSystem.h"

#include <cctype>

namespace goofy {
	Repository *Repository::factory(const char *sFilePath) {
		//URL, e.g. http://google.com
		if (InternetLocation::gimme(sFilePath)) return new InternetLocation(sFilePath);

		//directory, e.g. c:/windows/
		if (Directory::gimme(sFilePath)) return new Directory(sFilePath);

		//file, e.g. c:\windows\test.xml
		if (File::gimme(sFilePath)) return new File(sFilePath);
		
		return 0;
	}
	Repository::Repository(const char *_name): m_sName(_STRDUP(_name)), m_xml_filename(xml_filename()), m_lSizeXML(0) {
		//caller frees their copy of _name
	}
	Repository::~Repository() {
		if (m_sName) {free((void*) m_sName);m_sName = 0;}
		if (m_xml_filename) {free((void*) m_xml_filename);m_xml_filename = 0;}
	};
	const size_t Repository::size_xml() {return 0;}
	const char *Repository::source() const {
		//caller responsible for freeing description
		size_t iLen = strlen(m_sName) + 13;
		char *sDescription = (char*) malloc(iLen);
		_SNPRINTF1(sDescription, iLen, "Repository:%s", m_sName);
		return sDescription;
	}
	const char *Repository::xml_filename() const {
		//caller frees result if non-zero
		//by default take the last part of the string after /
		char *xml_filename;
		char *position;
		size_t iLen;

		if (!m_sName) {
			xml_filename = _STRDUP("unassigned");
		} else {
			iLen = strlen(m_sName);
			if (!iLen) {
				xml_filename = _STRDUP("empty");
			} else {
				//ok, we have a valid non-empty sring
				//find the first character after the last seperator
				position = (char*) m_sName + iLen - 1; //points to the last character
				if (iLen != 1 && issplitter(*position)) position--;
				while (position > m_sName && !issplitter(*position)) position--;
				if (issplitter(*position) && position[1]) position++;
				xml_filename = _STRDUP(position);
				//make alpha numeric lowercase
				for (position = xml_filename; *position; position++) {
					if (!isalnum(*position)) *position = '_';
					else if (!islower(*position)) *position = tolower(*position);
				}

				//valid first character
				if (!isalpha(*xml_filename)) *xml_filename = 'a';

				//remove trailing _ 
				//decreases length of string but not to 0
				position = xml_filename + strlen(xml_filename);
				while (--position > xml_filename && *position == '_') *position = 0;
			}
		}

		return xml_filename;
	}
}
