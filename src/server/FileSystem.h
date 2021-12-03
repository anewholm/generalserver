//platform agnostic file
#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "Repository.h"
#include "string.h" //required for UNIX strdup
#include <iostream>
#include <vector>
using namespace std;

#define IGNORE_MISSING_TARGET false

namespace general_server {
  class File;
  class StringVector;
  class MemoryLifetimeOwner;

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------------ Base IO classes -----------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class IOFile: implements_interface IMemoryLifetimeOwner {
  protected:
    static void generalReadLock();
    static void generalReadUnlock();
    static pthread_mutex_t m_mGeneralRead;

    //binary file handling
    //  used when streaming from Repository::readBinary(), hence virtual
    vector<char> *readBinary(const char *sFullPath) const;
    const char   *readRaw(   const char *sFullPath) const;
    //  or commiting -> copying resources
    int  cpBinary(const char *sFromFullPath, const char *sToFullPath) const;
    int  move(    const char *sFromFullPath, const char *sToFullPath, const bool bThrowIfNotThere = false) const;
    int  remove(  const char *sFullPath) const;
    bool exists( const char *sFullPath) const;

    //string XML reading and writing
    //  used when reading in or saving string XML
    int readValidXMLFrom(string &sCurrentXML, const char *sFullPath, const bool bThrowIfNotThere = false) const;
    int saveXMLAs(const char *sFullPath, const char *sContent, const bool bThrowIfThere = true) const;
  };

  class IODirectory: public IOFile {
  protected:
    int saveAs(const char *sFullPath) const;
    int move(           const char *sFromFullPath, const char *sToFullPath, const bool bThrowIfNotThere = false) const;
    int rmdir_recursive(const char *sFromFullPath, const bool bThrowIfNotThere = false) const;
    int cpdir_recursive(const char *sFromFullPath, const char *sToFullPath) const;
    bool exists( const char *sFullPath) const;
    bool isDirectoryNavigationFile(const char *sFilename) const; //.. or .
  };

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- Repository types ---------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class InternetLocation: public Repository {
    const char *m_sSessionID;
    
  protected:
    static const Repository::repositoryType m_iRepositoryType;
  public:
    InternetLocation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);
    InternetLocation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sUrl, const Repository *pParent = NULL, const char *sSessionID = NULL);

    //type specific full path information
    const char *protocol() const;
    const char *hostname() const;
    const char *port()     const;
    const char *path()     const;
    const char *query()    const;
    const char *anchor()   const;

    //Repository interface
    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    Repository::repositoryType type() const {return m_iRepositoryType;}
    bool removeDuringSync() const {return true;}
    const char *readRaw() const;
    void readValidXML(string &sCurrentXML) const;
    void appendXmlTo_content(  string &sCurrentXML, const bool bIsTop = false) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class Directory: public Repository, protected IODirectory {
    //iterate through files (only), directories must be known
    const StringVector *m_ext_noread; //files to not read in
    bool m_bPopulated;
    FILE *m_pTransactionFile;
    FILE *m_pCommitStatusFile;
    static pthread_mutex_t m_mTransactionFileAccess;

    const char *placeholderName(const bool bAddPlaceholderPrefix) const;
    bool lazyOpenTransactionsFile();
    bool lazyCloseTransactionsFile();
    static void transactionFileAccessLock();
    static void transactionFileAccessUnlock();

  protected:
    static const Repository::repositoryType m_iRepositoryType;

    //causes recursive behaviour in the contrustors of the sub-repositories
    void populateChildren(const bool bForce = false);

  public:
    Directory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL, const StringVector *_ext_noread = 0);
    Directory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);
    ~Directory();

    static const char *m_sTypeName;
    const char *dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode = 0, const Repository *pRootRepository = 0) const;

    TXml::transactionResult commitCreatePlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes);
    TXml::transactionResult commitSwapInPlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes);
    TXml::transactionResult commitGetPersistentStatus(TXml::commitStatus *cs, TXml::commitStyle *ct);
    TXml::transactionResult commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct);
    TXml::transactionResult commitDeleteTransactions(const vector<TXml*> *pvTXmls = 0);

    static bool gimme(const char *sFilePath);
    bool canSaveElementOnly() const {return true;}
    bool shouldIncludeSubRepository(const Repository *pSubRepository) const {return true;}
    bool removeDuringSync() const {return true;}

    const char *readRaw() const;
    void readValidXML(string &sCurrentXML) const;

    int inputReadCallback(StreamingContext *pStreamingContext, char* sBuffer, int iLen) const;
    void appendXmlTo_preCursor(string &sCurrentXML, const bool bIsTop = NOT_TOP) const; //DTD
    void appendXmlTo_startTag( string &sCurrentXML, const bool bIsTop = NOT_TOP) const; //Directory starts (adjusted with @repository:* attributes)
    void appendXmlTo_endTag(   string &sCurrentXML, const bool bIsTop = NOT_TOP) const; //Directory closes
    Repository::repositoryType type() const {return m_iRepositoryType;}

    TXml::transactionResult createSaveTreePlaceholder(const bool bAddPlaceholderPrefix = true); //requires a m_pNode
    TXml::transactionResult swapInSaveTreePlaceholder();
    TXml::transactionResult createElementOnlyPlaceholder();
    TXml::transactionResult swapInElementOnlyPlaceholder();
    TXml::transactionResult syncImmediateChildren();
    TXml::transactionResult applyTransaction(TXml *t, IXmlBaseNode *pSerialisationNode = 0, const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED = 0);
    TXml::transactionResult rollback();

    const char *toString() const;
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class ResourcesStore: public Directory {
    friend class Directory;

  protected:
    static const Repository::repositoryType m_iRepositoryType;
    static const char   *m_sFilename;
    static const size_t  m_iFilenameLen;

  public:
    ResourcesStore(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    ResourcesStore(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFullPath);
    Repository::repositoryType type() const {return m_iRepositoryType;}
   
    bool shouldIncludeSubRepository(const Repository *pSubRepository) const;
    TXml::transactionResult createSaveTreePlaceholderAdditions();

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class File: public Repository, protected IOFile {
    friend class Directory;

  protected:
    static const Repository::repositoryType m_iRepositoryType;

    void readValidXML(string &sCurrentXML) const;     //-> IOFile::readFrom(...a)
    const char *placeholderName(const bool bAddPlaceholderPrefix) const;

  public:
    File(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    File(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);
    ~File();

    static const char *m_sTypeName;
    void appendXmlTo_content(string &sCurrentXML, const bool bIsTop = false) const; //File contents (adjusted with @repository:* attributes)
    void saveAs(const char *sFullPath) const; //-> IOFile::saveAs(...)
    const char *readRaw() const;
    vector<char> *readBinary() const;
    bool removeDuringSync() const {return true;}

    static bool gimme(const char *sFilePath);
    Repository::repositoryType type() const {return m_iRepositoryType;}
    TXml::transactionResult createSaveTreePlaceholder(const bool bAddPlaceholderPrefix = true); //requires a m_pNode
    TXml::transactionResult swapInSaveTreePlaceholder();     //important ATOMIC part

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class MetaFile: public File {
  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    MetaFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    MetaFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    bool removeDuringSync() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class DirectoryDetails: public MetaFile {
    friend class Directory;
    static const char   *m_sFilename;
    static const size_t  m_iFilenameLen;

  protected:
    static const Repository::repositoryType m_iRepositoryType;


  public:
    DirectoryDetails(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    DirectoryDetails(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}
    const char *dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode = 0, const Repository *pRootRepository = 0) const;
    int saveXMLAs(const char *sFullPath, const bool bThrowIfThere = false) const;
    const char *placeholderName(const bool bAddPlaceholderPrefix = true) const;
    void appendXmlTo_startTag( string &sCurrentXML, const bool bIsTop = false) const; //e.g. Directory starts / File node starts
    void appendXmlTo_endTag(   string &sCurrentXML, const bool bIsTop = false) const; //e.g. Directory closes

    void applyDirectives(Repository *pTarget = 0);
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class DatabaseStatusFile: public MetaFile {
    friend class Directory;
  protected:
    static const Repository::repositoryType m_iRepositoryType;
    static const char   *m_sFilename;
    static const size_t  m_iFilenameLen;

  public:
    DatabaseStatusFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    DatabaseStatusFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    const char *dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode = 0, const Repository *pRootRepository = 0) const;
    void saveOrDelete(TXml::commitStatus cs, TXml::commitStyle ct) const;
    void read(TXml::commitStatus *cs, TXml::commitStyle *ct) const;

    bool includeInXML() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class EditorBackupFile: public MetaFile {
    //gedit css.xsl~ backup files
    friend class Directory;

  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    EditorBackupFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    EditorBackupFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class DirectoryLayout: public MetaFile {
    //KUbuntu .directory
    friend class Directory;

  protected:
    static const char   *m_sFilename;
    static const size_t  m_iFilenameLen;
    static const Repository::repositoryType m_iRepositoryType;

  public:
    DirectoryLayout(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    DirectoryLayout(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class CommitFileTemporary: public File {
    friend class Directory;
  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    static const char  *m_sCommitPrefix;
    static const size_t m_iCommitPrefixLen;

    CommitFileTemporary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    CommitFileTemporary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    bool removeDuringSync() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class DOCTypeDefinition: public MetaFile {
    friend class Directory;
    static const char   *m_sFilename;
    static const size_t  m_iFilenameLen;

  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    DOCTypeDefinition(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    DOCTypeDefinition(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool includeInXML() const {return false;}
    bool removeDuringSync() const {return false;}
    const char *dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode = 0, const Repository *pRootRepository = 0) const;
    int saveXMLAs(const char *sFullPath, const bool bThrowIfThere = false) const;
    const char *placeholderName(const bool bAddPlaceholderPrefix = true) const;
    Repository::repositoryType type() const {return m_iRepositoryType;}
    void appendXmlTo_content(string &sCurrentXML, const bool bIsTop = false) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  class Hardlink: public File {
    friend class Directory;
    static const char   *m_sExtension;
    static const size_t  m_iExtensionLen;

  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    Hardlink(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent = NULL, const char *sArg = NULL);
    Hardlink(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    bool removeDuringSync() const {return false;}
    Repository::repositoryType type() const {return m_iRepositoryType;}

    const char *dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const;
    int saveXMLAs(const char *sFullPath, const bool bThrowIfThere = true) const;
    const char *placeholderName(const bool bAddPlaceholderPrefix = true) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
