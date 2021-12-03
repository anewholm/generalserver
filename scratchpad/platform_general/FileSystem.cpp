//platform agnostic file
#include "FileSystem.h"
#include "define.h"
#include "debug.h"
#include "dirent.h"

using namespace std;

namespace goofy {
	//-----------------------------------------------------
	//---------------------------------------------- InternetLocation
	//-----------------------------------------------------
	InternetLocation::InternetLocation(const char *_url): Repository(_url) {}
	InternetLocation::~InternetLocation() {}
	const char *InternetLocation::m_type = "InternetLocation";
	const bool InternetLocation::gimme(const char *sFilePath) {
		return sFilePath && (!strncmp(sFilePath, "http://", 7) || !strncmp(sFilePath, "https://", 8));
	}
	char *InternetLocation::read(char *buffer) {
		//caller frees result
		return 0;
	}
	const char *InternetLocation::source() const {
		//do not free static result
		return m_type;
	}



	//-----------------------------------------------------
	//---------------------------------------------- Directory
	//-----------------------------------------------------
	Directory::Directory(const char *_name, const Directory *_parentDirectory, const vector<const char*> *_ext_noread): Repository(_name), m_parentDirectory(_parentDirectory), m_ext_noread(_ext_noread), m_bPopulated(false) {}
	Directory::~Directory() {
		//release Files
		iterator iFile;
		DEBUGPRINT("~Directory free(*iFile)", DEBUG_BLOCKSTART);
		for (iFile = begin(); iFile != end(); iFile++) delete *iFile;
		DEBUGPRINT("~Directory free(*iFile)", DEBUG_BLOCKEND);

		//release Directories recursive
		vector<Directory*>::iterator iDirectory;
		DEBUGPRINT("~Directory free(*iDirectory)", DEBUG_BLOCKSTART);
		for (iDirectory = m_directories.begin(); iDirectory != m_directories.end(); iDirectory++) delete *iDirectory;
		DEBUGPRINT("~Directory free(*iDirectory)", DEBUG_BLOCKEND);

		//Repository releases its m_sName
	}
	const char *Directory::m_type = "Directory";
	const bool Directory::gimme(const char *sFilePath) {
		return sFilePath && !InternetLocation::gimme(sFilePath) && issplitter(sFilePath[strlen(sFilePath) - 1]);
	}
	char *Directory::read(char *buffer) {
		//caller frees result
		//recurse the directory, sub-directories and files reading all and constructing XML
		//Read files and directories into a tree and calculate size
		char *newbuffer = 0;
		if (!buffer) {
			//we are being asked to create the whole new document ourselves
			//rather than feed in to an existing one
			const size_t iLen = 12 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 2 + strlen(NAMESPACE_REPOSITORY) + 2 + 1;
			newbuffer = buffer = (char*) malloc(populateSubTree() + iLen + 7); //include </root>
			buffer += _SNPRINTF2(buffer, iLen, "<root xmlns:%s=\"%s\">", NAMESPACE_REPOSITORY_ALIAS, NAMESPACE_REPOSITORY);
		}
		if (buffer) {
			//copy in fullpath and name
			//@TODO: no escaping happening here...
			buffer += _SNPRINTF5(buffer, 
				1 + strlen(m_xml_filename) + 1 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 11 + strlen(m_sName) + 2 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 7 + strlen(m_type) + 2 + 1, 
				"<%s %s:fullpath=\"%s\" %s:type=\"%s\">", 
				m_xml_filename, 
				NAMESPACE_REPOSITORY_ALIAS, 
				m_sName, 
				NAMESPACE_REPOSITORY_ALIAS,
				m_type);
			//recurse tree creating XML and adding content of files
			//recurse directories
			vector<Directory*>::iterator iDirectory;
			for (iDirectory = m_directories.begin(); iDirectory != m_directories.end(); iDirectory++) {
				buffer = (*iDirectory)->read(buffer);
			}

			//load files
			iterator iFile;
			for (iFile = begin(); iFile != end(); iFile++) {
				buffer = (*iFile)->read(buffer);
			}
			buffer += _SNPRINTF1(buffer, strlen(m_xml_filename) + 3 + 1, "</%s>", m_xml_filename);
		}
		if (newbuffer) {
			strcpy(buffer, "</root>");
			buffer += 7;
		}

		//return the newbuffer start if one created
		//or the current position in the one sent otherwise
		return newbuffer ? newbuffer : buffer;
	}
	size_t Directory::populateSubTree(bool bForce) {
		DIR *dp;
		struct dirent *dirp;
		Directory *subdirectory;
		File *file;
		char *filename, *fullpath;
		size_t iLen;
		size_t size_xml = 0;

		if (bForce || !m_bPopulated) {
			DEBUGPRINT1("directory [%s]", DEBUG_BLOCKSTART, m_sName);
			if (dp = opendir(m_sName)) {
				while ((dirp = readdir(dp)) != NULL) {
					filename = dirp->d_name;

					//ignore system files (., .., .*)
					if (*filename != '.') {
						//d_type is only available on BSD systems
						//see altered dirent.h for windows for more info
						//use lstat()
						switch (dirp->d_type) {
							case DT_REG: {
								iLen = strlen(m_sName) + strlen(filename) + 1;
								fullpath = (char*) malloc(iLen);
								_SNPRINTF2(fullpath, iLen, "%s%s", m_sName, filename);
								file = new File(fullpath, this); //_STDUP()s filename input
								free((void*) fullpath);

								size_xml += file->size_xml();
								push_back(file);
								DEBUGPRINT2("file [%s] %i", DEBUG_LINE, filename, file->size_xml());
								break;
							}
							case DT_DIR: {
								iLen = strlen(m_sName) + strlen(filename) + 2;
								fullpath = (char*) malloc(iLen);
								//note that windwos accepts both types of directory seperator
								_SNPRINTF3(fullpath, iLen, "%s%s%c", m_sName, filename, splitter());
								subdirectory = new Directory(fullpath, this);
								free((void*) fullpath);

								size_xml += subdirectory->size_xml();
								m_directories.push_back(subdirectory);

								//recursive: will DEBUGPRINT the directory name
								size_xml += subdirectory->populateSubTree();
								break;
							}
							default: {
								DEBUGPRINT1("unknown filetype [%s]", DEBUG_LINE, filename);
							}
						}
					}
				}
				closedir(dp);
			}
			m_bPopulated = true;
			DEBUGPRINT("directory", DEBUG_BLOCKEND);
		}

		return size_xml;
	}

	const size_t Directory::size_xml() {
		//<%s %s:fullpath=\"%s\" %s:type=\"%s\">
		//</%s>
		return 1 + strlen(m_xml_filename) + 1 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 11 + strlen(m_sName) + 2 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 7 + strlen(m_type) + 2
			+ strlen(m_xml_filename) + 3
			+ 1;
	}
	const char *Directory::source() const {
		//do not free static result
		return m_type;
	}




	//-----------------------------------------------------
	//---------------------------------------------- File
	//-----------------------------------------------------
	File::File(const char *_name, const Directory *_parentDirectory): Repository(_name), m_parentDirectory(_parentDirectory) {}
	File::~File() {}
	const char *File::m_type = "File";
	const bool File::gimme(const char *sFilePath) {
		return sFilePath && !InternetLocation::gimme(sFilePath) && !issplitter(sFilePath[strlen(sFilePath) - 1]);
	}
	const size_t File::size_xml() {
		//<%s %s:fullpath=\"%s\" %s:type=\"%s\">
		//</%s>
		if (!m_lSizeXML) {
			m_lSizeXML = 1 + strlen(m_xml_filename) + 1 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 11 + strlen(m_sName) + 2 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 7 + strlen(m_type) + 2
				+ strlen(m_xml_filename) + 3
				+ 1;
			FILE *pFile;
			if (_FOPEN(pFile, m_sName, "r")) {
				fseek(pFile, 0, SEEK_END);
				m_lSizeXML += ftell(pFile); //add contents of the file to the XML size
				fclose(pFile);
			}
		}
		return m_lSizeXML;
	}
	char *File::read(char *buffer) {
		//caller responsible for freeing result
		FILE *pFile;
		size_t lSize = size_xml();
		size_t read_result;
		char *newbuffer = 0;

		if (!buffer) {
			//new document happening here
			//copy root with namespaces
			const size_t iLen = 12 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 2 + strlen(NAMESPACE_REPOSITORY) + 2 + 1;
			newbuffer = buffer = (char*) malloc(sizeof(char)*lSize + iLen + 7); //include </root>
			buffer += _SNPRINTF2(buffer, iLen, "<root xmlns:%s=\"%s\">", NAMESPACE_REPOSITORY_ALIAS, NAMESPACE_REPOSITORY);
		}
		if (buffer) {
			//copy in fullpath and name
			//@TODO: no escaping happening here...
			buffer += _SNPRINTF5(buffer, 
				1 + strlen(m_xml_filename) + 1 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 11 + strlen(m_sName) + 2 + strlen(NAMESPACE_REPOSITORY_ALIAS) + 7 + strlen(m_type) + 2 + 1, 
				"<%s %s:fullpath=\"%s\" %s:type=\"%s\">", 
				m_xml_filename, 
				NAMESPACE_REPOSITORY_ALIAS, 
				m_sName, 
				NAMESPACE_REPOSITORY_ALIAS,
				m_type);
			if (_FOPEN(pFile, m_sName, "r")) {
				read_result = fread(buffer, 1, lSize, pFile);
				if (read_result > 0) buffer += read_result;
				fclose(pFile);
			}
			buffer += _SNPRINTF1(buffer, strlen(m_xml_filename) + 3 + 1, "</%s>", m_xml_filename);
		}
		if (newbuffer) {
			strcpy(buffer, "</root>"); //copy from static: no free required
			buffer += 7;
		}

		//return the newbuffer start if one created
		//or the current position in the one sent otherwise
		return newbuffer ? newbuffer : buffer;
	}
	const char *File::source() const {
		//do not free static result
		return m_type;
	}
}