//platform agnostic file
#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "Repository.h"
#include <vector>

using namespace std;

namespace goofy {
	class File;

	class InternetLocation: public Repository {

	private:
	protected:
		static const char *m_type;
	public:
		InternetLocation(const char *_url);
		~InternetLocation();

		static const bool gimme(const char *sFilePath);
		char *read(char *buffer = 0);
		const char *source() const;
	};

	class Directory: public Repository, vector<File*> {
		//iterate through files (only), directories must be known
		const vector<const char*> *m_ext_noread; //files to not read in
		const Directory *m_parentDirectory;
		vector<Directory*> m_directories;
		bool m_bPopulated;

	private:
		size_t populateSubTree(bool bForce = false);
	protected:
		static const char *m_type;
	public:
		Directory(const char *_name, const Directory *_parentDirectory = 0, const vector<const char*> *_ext_noread = 0);
		~Directory();

		vector<File*> filesOfType(const char *type);
		vector<File*> filesFromRX(const char *RX);

		static const bool gimme(const char *sFilePath);
		const size_t size_xml();
		char *read(char *buffer = 0);
		const char *source() const;
	};

	class File: public Repository {
		const Directory *m_parentDirectory;

	private:
	protected:
		static const char *m_type;
	public:
		File(const char *_name, const Directory *_parentDirectory = 0);
		~File();

		static const bool gimme(const char *sFilePath);
		const size_t size_xml();
		char *read(char *buffer = 0);
		const char *source() const;
	};
}

#endif
