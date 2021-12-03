//platform agnostic file
#include "FileSystem.h"
#include "define.h"

#include "dirent.h"

#include "Server.h"
#include "IXml/IXmlBaseNode.h"
#include "Database.h"
#include "DatabaseNode.h"
#include "Utilities/container.c"
#include "Utilities/strtools.h"
#include "Utilities/StringVector.h"
#include "QueryEnvironment/RepositorySaveSecurityContext.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

#define MALLOC_RETURN NULL
#define OVERWRITE false

namespace general_server {
  //-----------------------------------------------------
  //---------------------------------------------- Low level stuff IO*
  //-----------------------------------------------------
  pthread_mutex_t IOFile::m_mGeneralRead  = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  void IOFile::generalReadLock()   {pthread_mutex_lock(&m_mGeneralRead);}
  void IOFile::generalReadUnlock() {pthread_mutex_unlock(&m_mGeneralRead);}

  int IODirectory::saveAs(const char *sFullPath) const {
    //we do not care if EEXIST, cause we will use it anyway, it's a Directory, it's the same
    return mkdir(sFullPath, S_IRWXG | S_IRWXU);
  }

  bool IODirectory::isDirectoryNavigationFile(const char *sFilename) const {
    return _STREQUAL(sFilename, ".") || _STREQUAL(sFilename, "..");
  }
  

  int IOFile::remove(const char *sFullPath) const {
    //resolve symlinks
    const char *sFullPathReal = realpath(sFullPath, MALLOC_RETURN);
    int iRet = std::remove(sFullPathReal);
    MMO_FREE(sFullPathReal);
    return iRet;
  }

  bool IOFile::exists(const char *sFullPath) const {
    struct stat statbuf;   
    return (stat(sFullPath, &statbuf) == 0 && !S_ISDIR(statbuf.st_mode));
  }

  bool IODirectory::exists(const char *sFullPath) const {
    struct stat statbuf;   
    return (stat(sFullPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode));
  }

  int IODirectory::move(const char *sFromFullPath, const char *sToFullPath, const bool bThrowIfNotThere) const {
    const char *sFromFullPathReal = 0, 
               *sToFullPathReal   = 0;
    int iRet = -1;
    UNWIND_EXCEPTION_DECLARE;
    
    generalReadLock(); {
      UNWIND_EXCEPTION_TRY {
        //resolve symlinks
        sFromFullPathReal = realpath(sFromFullPath, MALLOC_RETURN);
        sToFullPathReal   = realpath(sToFullPath,   MALLOC_RETURN);
        
        if (rmdir_recursive(sToFullPathReal) && bThrowIfNotThere) throw FailedToRemoveOldItem(this, MM_STRDUP(sToFullPathReal), SOCKETS_ERRNO);
        if (std::rename(sFromFullPathReal, sToFullPathReal))           throw FailedToSwapInPlaceholder(this, MM_STRDUP(sFromFullPathReal), MM_STRDUP(sToFullPathReal), SOCKETS_ERRNO);
        iRet = 0;
      } UNWIND_EXCEPTION_END;
    } generalReadUnlock();
    
    //free up
    if (sFromFullPathReal) MMO_FREE(sFromFullPathReal);
    if (sToFullPathReal)   MMO_FREE(sToFullPathReal);
    
    UNWIND_EXCEPTION_THROW;
    
    return iRet;
  }

  int IODirectory::cpdir_recursive(const char *sFromFullPath, const char *sToFullPath) const {
    //caller manages sFromFullPath and sToFullPath
    //both directories must exist, contents are copied
    const char *sFromFullPathReal = 0, 
               *sToFullPathReal   = 0;
    DIR *sd, *td;
    size_t frompath_len, topath_len;
    char *topath_item, *frompath_item;
    struct dirent *sp;
    int r = 0;
    UNWIND_EXCEPTION_DECLARE;
    
    assert(sFromFullPath && *sFromFullPath && sFromFullPath[strlen(sFromFullPath) - 1] == '/');
    assert(sToFullPath   && *sToFullPath   && sToFullPath[  strlen(sToFullPath) - 1]   == '/');

    generalReadLock(); {
      UNWIND_EXCEPTION_TRY {
        //resolve symlinks
        sFromFullPathReal = realpath(sFromFullPath, MALLOC_RETURN);
        sToFullPathReal   = realpath(sToFullPath,   MALLOC_RETURN);
        sd = opendir(sFromFullPath); //.../
        td = opendir(sToFullPath);   //.../

        if (sd && td) {
          while (!r && (sp = readdir(sd))) {
              if (isDirectoryNavigationFile(sp->d_name)) continue;

              //sub-item paths
              //space for directory slash if needbe in there
              frompath_len  = strlen(sFromFullPath) + strlen(sp->d_name) + 2; //space for directory slash if needbe
              frompath_item = MMO_MALLOC(frompath_len);
              snprintf(frompath_item, frompath_len, "%s%s", sFromFullPath, sp->d_name);

              topath_len    = strlen(sToFullPath) + strlen(sp->d_name) + 2;   //space for directory slash if needbe
              topath_item   = MMO_MALLOC(topath_len);
              snprintf(topath_item, topath_len, "%s%s", sToFullPath, sp->d_name);

              //actions
              switch (sp->d_type) {
                case DT_REG: {
                  r = IOFile::cpBinary(frompath_item, topath_item);
                  break;
                }
                case DT_DIR: {
                  r = IODirectory::saveAs(topath_item);
                  //add final slashes so that next recursion adds names properly
                  strcpy(frompath_item + frompath_len - 2, "/");
                  strcpy(topath_item   + topath_len   - 2, "/");
                  if (!r) r = cpdir_recursive(frompath_item, topath_item);
                  break;
                }
                default: {
                  free(topath_item);
                  throw UnknownFileType(this, frompath_item);
                }
              }

              //free up
              MMO_FREE(frompath_item);
              MMO_FREE(topath_item);
          }
          closedir(sd);
          closedir(td);
        }
        
      } UNWIND_EXCEPTION_END;
    } generalReadUnlock();
    
    //free up
    if (sFromFullPathReal) MMO_FREE(sFromFullPathReal);
    if (sToFullPathReal)   MMO_FREE(sToFullPathReal);
    
    UNWIND_EXCEPTION_THROW;

    return r;
  }

  int IODirectory::rmdir_recursive(const char *sFromFullPath, const bool bThrowIfNotThere) const {
    const char *sFromFullPathReal = 0;
    DIR *d;
    struct dirent *p;
    size_t path_len;
    int r = -1;
    UNWIND_EXCEPTION_DECLARE;
    
    generalReadLock(); {
      UNWIND_EXCEPTION_TRY {
        //resolve symlinks
        sFromFullPathReal = realpath(sFromFullPath, MALLOC_RETURN);
        d = opendir(sFromFullPathReal);
        path_len = strlen(sFromFullPathReal);

        if (d) {
            r = 0;

            while (!r && (p = readdir(d))) {
                int r2 = -1;
                char *buf;
                size_t len;

                if (isDirectoryNavigationFile(p->d_name)) continue;

                len = path_len + strlen(p->d_name) + 2;
                buf = MMO_MALLOC(len);

                if (buf) {
                  struct stat statbuf;
                  snprintf(buf, len, "%s/%s", sFromFullPathReal, p->d_name);

                  if (!stat(buf, &statbuf)) {
                      if (S_ISDIR(statbuf.st_mode)) r2 = rmdir_recursive(buf);
                      else                          r2 = unlink(buf);
                  }

                  MMO_FREE(buf);
                }

                r = r2;
            }

            closedir(d);
        } else if (bThrowIfNotThere) throw FailedToRemoveOldItem(this, sFromFullPathReal, SOCKETS_ERRNO);

        //remove the final base dir
        if (!r) r = rmdir(sFromFullPathReal);
      } UNWIND_EXCEPTION_END;
    } generalReadUnlock();
    
    //free up
    if (sFromFullPathReal) MMO_FREE(sFromFullPathReal);
    
    UNWIND_EXCEPTION_THROW;

    return r;
  }

  int IOFile::move(const char *sFromFullPath, const char *sToFullPath, const bool bThrowIfNotThere) const {
    const char *sFromFullPathReal = 0, 
               *sToFullPathReal   = 0;
    int iRet = -1;
    UNWIND_EXCEPTION_DECLARE;
    
    generalReadLock(); {
      UNWIND_EXCEPTION_TRY {
        //resolve symlinks
        sFromFullPathReal = realpath(sFromFullPath, MALLOC_RETURN);
        sToFullPathReal   = realpath(sToFullPath,   MALLOC_RETURN);
        
        if (std::remove(sToFullPathReal) && bThrowIfNotThere) throw FailedToRemoveOldItem(this, MM_STRDUP(sToFullPathReal), SOCKETS_ERRNO);
        if (std::rename(sFromFullPathReal, sToFullPathReal))  throw FailedToSwapInPlaceholder(this, MM_STRDUP(sFromFullPathReal), MM_STRDUP(sToFullPathReal), SOCKETS_ERRNO);
        iRet = 0;
      } UNWIND_EXCEPTION_END;
    } generalReadUnlock();
    
    //free up
    if (sFromFullPathReal) MMO_FREE(sFromFullPathReal);
    if (sToFullPathReal)   MMO_FREE(sToFullPathReal);
    
    UNWIND_EXCEPTION_THROW;
    
    return iRet;
  }

  int IOFile::cpBinary(const char *frompath, const char *topath) const {
    //direct copying to avoid any string conversion issues
    //this is used for copying also binary files, not just XML
    int r = 0;
    ifstream::pos_type size;
    ifstream ifile;
    ofstream ofile;
    char *memblock;

    generalReadLock(); {
      ifile.open(frompath, ios::in  | ios::binary | ios::ate);
      ofile.open(topath,   ios::out | ios::binary | ios::trunc);
      if (ifile) {
        //input
        size     = ifile.tellg();
        memblock = new char [(int) size + 1];
        ifile.seekg(0, ios::beg);
        ifile.read(memblock, size);
        ifile.close();

        //output
        ofile.write(memblock, size);
        ofile.close();

        delete[] memblock;
      }
    } generalReadUnlock();

    //don't keep lock!
    if (!ifile) throw DirectoryNotFound(this, frompath);

    return r;
  }

  int IOFile::readValidXMLFrom(string &sCurrentXML, const char *sFullPath, const bool bThrowIfNotThere) const {
    //0 = ok, found and read, 1 = error
    //this is only used for reading string XML data
    int r = 1;
    ifstream::pos_type size;
    ifstream file;
    char *sMemblock;
    char *sSkip;

    generalReadLock(); {
      file.open(sFullPath, ios::in | ios::ate);
      if (file) {
        size     = file.tellg();
        sMemblock = new char [(int) size + 1];
        file.seekg(0, ios::beg);
        file.read(sMemblock, size);
        sMemblock[size] = 0;
        file.close();
        r = 0;

        //skip processing instructions, e.g. XML declerations
        //<?xml ... ?>
        sSkip = sMemblock;
        while (sSkip && _STRNEQUAL(sSkip, "<?", 2)) {
          sSkip = strstr(sSkip, "?>");
          if (sSkip) {
            sSkip += 2;
            while (*sSkip == '\n' || *sSkip == '\r') sSkip++;
          }
        }

        //concat
        if (sSkip) sCurrentXML += sSkip;

        //loop free
        delete[] sMemblock;
      }
    } generalReadUnlock();

    //don't keep lock!
    if (!file && bThrowIfNotThere) throw DirectoryNotFound(this, sFullPath);

    return r;
  }
  
  const char *IOFile::readRaw(const char *sFullPath) const {
    //used for streaming resources
    char *sData = 0;
    ifstream::pos_type iSize;
    ifstream pFile;

    generalReadLock(); {
      pFile.open(sFullPath, ios::in | ios::binary | ios::ate);
      if (pFile) {
        iSize  = pFile.tellg();
        sData = MM_MALLOC_FOR_RETURN(iSize); //protected
        pFile.seekg(0, ios::beg);
        pFile.read(sData, iSize);
        pFile.close();
      } //else do not keep lock!
    } generalReadUnlock();

    if (!pFile) throw DirectoryNotFound(this, sFullPath);

    return sData;
  }

  vector<char> *IOFile::readBinary(const char *sFullPath) const {
    //used for streaming resources
    vector<char> *vData = 0;
    ifstream::pos_type size;
    ifstream file;

    generalReadLock(); {
      file.open(sFullPath, ios::in | ios::binary | ios::ate);
      if (file) {
        size  = file.tellg();
        vData = new vector<char>((int) size);
        file.seekg(0, ios::beg);
        file.read(vData->data(), size);
        file.close();
      } //do not keep lock!
    } generalReadUnlock();

    if (!file) throw DirectoryNotFound(this, sFullPath);

    return vData;
  }

  int IOFile::saveXMLAs(const char *sFullPath, const char *sContent, const bool bThrowIfThere) const {
    //this is only used to write XML data
    //will throw a DirectoryNotFound() if a Directory is not present
    int iRet = -1;
    ofstream file;
    UNWIND_EXCEPTION_DECLARE;

    generalReadLock(); {
      UNWIND_EXCEPTION_TRY {
        if (bThrowIfThere && exists(sFullPath)) throw FileExists(this, sFullPath);
        
        file.open(sFullPath, ios::out | ios::trunc);
        if (file) {
          file.write(sContent, strlen(sContent));
          file.close();
          iRet = 0;
        } else throw FileNotFound(this, sFullPath);
      } UNWIND_EXCEPTION_END;
    } generalReadUnlock();

    UNWIND_EXCEPTION_THROW;

    return iRet;
  }

  //-----------------------------------------------------
  //---------------------------------------------- InternetLocation
  //-----------------------------------------------------
  InternetLocation::InternetLocation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sUrl, const Repository *pParent, const char *sSessionID): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    Repository(pMemoryLifetimeOwner, pLib, sUrl, pParent),
    m_sSessionID(sSessionID)
  {}
  
  InternetLocation::InternetLocation(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    Repository(pMemoryLifetimeOwner, pLib, pNode, pRootRepository),
    m_sSessionID(0)
  {}
  
  const Repository::repositoryType InternetLocation::m_iRepositoryType = tInternetLocation;
  const char *InternetLocation::m_sTypeName = "InternetLocation";
  bool InternetLocation::gimme(const char *sFullPath) {
    return sFullPath && (
         !strncmp(sFullPath, "http://",  7) 
      || !strncmp(sFullPath, "https://", 8)
      || !strncmp(sFullPath, "ftp://",   6)
      || !strncmp(sFullPath, "sftp://",  7)
    );
  }
  void InternetLocation::appendXmlTo_content(  string &sCurrentXML, const bool bIsTop) const {
    NOT_COMPLETE(""); //InternetLocation::appendXmlTo_content()
  }
  const char *InternetLocation::protocol() const {
    //caller frees result unless zero
    const char *sProtocolStart;
    char *sProtocol = 0;
    size_t iProtocolLen;

    if (sProtocolStart = strstr(m_sFullPath, "://")) {
      if (iProtocolLen = sProtocolStart - m_sFullPath)
        sProtocol = strndup(m_sFullPath, iProtocolLen);
    }

    return sProtocol;
  }
  
  const char *InternetLocation::hostname() const {
    //caller frees result unless zero
    const char *sProtocolStart, *sHostNameEnd;
    char *sHostname = 0;
    size_t iHostnameLen;

    if (sProtocolStart = strstr(m_sFullPath, "://")) {
      sHostNameEnd = strchrs(sProtocolStart + 3, ":/");
      if (!sHostNameEnd) sHostNameEnd = m_sFullPath + strlen(m_sFullPath);

      if (iHostnameLen = sHostNameEnd - sProtocolStart - 3)
        sHostname = strndup(sProtocolStart + 3, iHostnameLen);
    }

    return sHostname;
  }
  
  const char *InternetLocation::port()     const {
    //caller frees result
    const char *sProtocolStart, *sHostNameEnd, *sPortEnd;
    char *sPort = 0;
    size_t iPortLen;

    if (sProtocolStart = strstr(m_sFullPath, "://")) {
      sHostNameEnd = strchrs(sProtocolStart + 3, ":/");
      if (!sHostNameEnd) sHostNameEnd = m_sFullPath + strlen(m_sFullPath);

      if (*sHostNameEnd == ':') {
        sPortEnd = strchr(sHostNameEnd, '/');
        if (!sPortEnd) sPortEnd = m_sFullPath + strlen(m_sFullPath);

        if (iPortLen = sPortEnd - sHostNameEnd - 1)
          sPort = strndup(sHostNameEnd + 1, iPortLen);
      }
    }
    if (!sPort) sPort = MMO_STRDUP("80");

    return sPort;
  }
  
  const char *InternetLocation::path()     const {
    //caller frees result
    const char *sProtocolStart, *sHostNameEnd;
    char *sPath = 0;
    size_t iPathLen;

    if (sProtocolStart = strstr(m_sFullPath, "://")) {
      sHostNameEnd = strchr(sProtocolStart + 3, '/');
      if (!sHostNameEnd) sHostNameEnd = m_sFullPath + strlen(m_sFullPath);

      if (iPathLen = strlen(m_sFullPath) - (sHostNameEnd - m_sFullPath - 1) - 1)
        sPath = strndup(sHostNameEnd, iPathLen);
    }

    if (!sPath) sPath = MM_STRDUP("/");

    return sPath;
  }
  const char *InternetLocation::query()    const {
    NOT_COMPLETE(""); //InternetLocation::query()
  }
  const char *InternetLocation::anchor()   const {
    NOT_COMPLETE(""); //InternetLocation::anchor()
  }

  void InternetLocation::readValidXML(string &sCurrentXML) const {
    const char *sContent    = 0;
    const char *sXMLContent = 0;

    if (sContent = readRaw()) {
      //this parses the HTML in to a htmlDoc
      //and then writes it out with the XML writer
      sXMLContent = m_pLib->convertHTMLToXML(sContent);
    }

    if (sXMLContent) sCurrentXML += sXMLContent;

    //free up
    if (sContent)    MMO_FREE(sContent);
    if (sXMLContent) MMO_FREE(sXMLContent);
  }

  const char *InternetLocation::readRaw() const {
    // set family and socket type
    SOCKET sc = INVALID_SOCKET;
    struct addrinfo adHints, *adResult = 0;
    char sResponse[4096];
    size_t iBytes;
    char sHeaders[2048];
    string stContent;
    const char *sBodyStart;

    //disect request URL
    const char *sProtocol = protocol(); //defaults to http
    const char *sHostname = hostname(); //defaults to localhost SECURITY: will this cause a default privileged internal request on some systems?
    const char *sPort     = port();     //defaults to 80
    const char *sPath     = path();     //defaults to /
    //const bool  bInternal = _STREQUAL(sHostname, "localhost");
    static const char *sUserAgent = "User-Agent: Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0)";

    Debug::report("InternetLocation request: [%s] :// [%s] : [%s] [%s]", sProtocol, sHostname, sPort, sPath);

    UNWIND_EXCEPTION_BEGIN {
      if (!sProtocol) throw ProtocolNotSupported(this, MM_STRDUP("no protocol"));
      if (strcmp(sProtocol, "http")) throw ProtocolNotSupported(this, MM_STRDUP(sProtocol));

      // resolve hostname to IP address: specify host and port number (in char not int)
      memset(&adHints, 0, sizeof adHints);
      adHints.ai_family   = AF_INET;
      adHints.ai_socktype = SOCK_STREAM;
      if (!sHostname) throw CannotResolveHostname(this, MM_STRDUP("no hostname"));
      if (getaddrinfo(sHostname, sPort, &adHints, &adResult) != 0) throw CannotResolveHostname(this, MM_STRDUP(sHostname));

      // create socket and free addrinfo memory
      sc = socket(adResult->ai_family, adResult->ai_socktype, 0);
      if (sc == (SOCKET) SOCKET_ERROR) throw CannotCreateSocket(this);

      // set socket timeouts
      struct timeval timeout;
      memset(&timeout, 0, sizeof(timeout)); // zero timeout struct before use
      timeout.tv_sec  = 5;
      timeout.tv_usec = 0;
      setsockopt(sc, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)); // send timeout
      setsockopt(sc, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); // receive timeout

      // connect to website
      if (connect(sc, adResult->ai_addr, adResult->ai_addrlen) == SOCKET_ERROR) throw CannotConnectSocket(this, MM_STRDUP(sHostname));

      //assemble headers request
      _SNPRINTF4(
        sHeaders,
        sizeof(sHeaders),
        "GET %s HTTP/1.1\r\nHost: %s\r\n%s\r\nConnection: Close\r\nCookie: GSSESSID=%s\r\n\r\n",
        sPath,
        sHostname,
        sUserAgent,
        (m_sSessionID ? m_sSessionID : "")
      );

      // send headers to connection
      if ((send(sc, sHeaders, strlen(sHeaders), 0)) == SOCKET_ERROR) throw CannotSendData(this);

      //recieve
      while (iBytes = recv(sc, sResponse, sizeof(sResponse), 0)) {
        sResponse[iBytes] = 0;
        stContent += sResponse;
      }
    } UNWIND_EXCEPTION_END;

    //post-process content (full HTTP response)
    //<!DOCTYPE might also be present
    sBodyStart = strstr(stContent.c_str(), "\r\n\r\n");

    //clean up
    if (adResult)             freeaddrinfo(adResult);
    if (sc != INVALID_SOCKET) CLOSE_SOCKET(sc);
    if (sProtocol)            MMO_FREE(sProtocol);
    if (sHostname)            MMO_FREE(sHostname);
    if (sPort)                MMO_FREE(sPort);
    if (sPath)                MMO_FREE(sPath);

    UNWIND_EXCEPTION_THROW;

    return sBodyStart ? MM_STRDUP(sBodyStart + 4) : 0;
  }


  //-----------------------------------------------------
  //---------------------------------------------- Directory
  //-----------------------------------------------------
  pthread_mutex_t Directory::m_mTransactionFileAccess = PTHREAD_MUTEX_INITIALIZER;
  void Directory::transactionFileAccessLock() {
    pthread_mutex_lock(&m_mTransactionFileAccess);
  }
  void Directory::transactionFileAccessUnlock() {
    pthread_mutex_unlock(&m_mTransactionFileAccess);
  }

  Directory::Directory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg, const StringVector *_ext_noread): MemoryLifetimeOwner(pMemoryLifetimeOwner), Repository(pMemoryLifetimeOwner, pLib, sFullPath, pParent), m_ext_noread(_ext_noread), m_bPopulated(false), m_pTransactionFile(0), m_pCommitStatusFile(0) {}
  Directory::Directory(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), Repository(pMemoryLifetimeOwner, pLib, pNode, pRootRepository), m_bPopulated(false), m_pTransactionFile(0), m_pCommitStatusFile(0) {}
  Directory::~Directory() {
    //release Files and directories
    lazyCloseTransactionsFile();
    if (m_pCommitStatusFile) fclose(m_pCommitStatusFile);

    //should be recursive
    vector_element_destroy(*this);

    //Repository releases its m_sFullPath
  }
  const Repository::repositoryType Directory::m_iRepositoryType = tDirectory;
  const char *Directory::m_sTypeName = "Directory";

  const char *Directory::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    string sDynamicFileSystemPath = baseFileSystemPath(pSubRepositoryNode, pRootRepository);
    sDynamicFileSystemPath += splitter();
    return MM_STRDUP(sDynamicFileSystemPath.c_str());
  }

  TXml::transactionResult Directory::commitCreatePlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) {
    //so the primary repository is a Directory
    vector<AffectedRepositoryDetails>::iterator iRepository;
    vector<AffectedRepositoryDetails> vAffectedSubRepositorys;
    AffectedRepositoryDetails ardetails;
    Repository *pNearestRepository;
    TXml::transactionResult ret = TXml::transactionFailure;

    //save the unique Repositories
    Debug::report("      ----- affected nodes -> sub-repositories and rationalisation");
    affectedSubRepositories(&vAffectedSubRepositorys, pvAffectedNodes);
    rationaliseAffectedRepositoriesVector(&vAffectedSubRepositorys);

    for (iRepository = vAffectedSubRepositorys.begin(); iRepository != vAffectedSubRepositorys.end(); iRepository++) {
      ardetails          = *iRepository;
      pNearestRepository = ardetails.pRelatedRepository;
      Debug::report("      ----- %s repository", pNearestRepository->getRepositoryTypeName());
      switch (ardetails.iAffectedScope) {
        case TXml::saveTree: {
          ret = pNearestRepository->createSaveTreePlaceholder();
          break;
        }
        case TXml::saveElementOnly: {
          //only works with Directories (sameNode)
          ret = pNearestRepository->createElementOnlyPlaceholder();
          break;
        }
        case TXml::syncImmediateChildren: {
          Debug::report("      no preparation to do for syncImmediateChildren");
          ret = TXml::transactionSuccess;
          break;
        }
        case TXml::linkChild: NOT_COMPLETE("TXml::linkChild"); break;
      }
      delete pNearestRepository;
    }

    return ret;
  }
  
  TXml::transactionResult Directory::commitGetPersistentStatus(TXml::commitStatus *cs, TXml::commitStyle *ct) {
    //save as XML: <repository:status commit-status="beginCommitCreatePlaceholders" commit-style="full" />
    //*original* Directory location + dbstatus.metaxml
    //note that m_sFullPath of parent Repository has the placeholder name in it
    DatabaseStatusFile dsf(this, m_pLib, m_pNode, this);
    dsf.setDynamicFullPath(); //original path to root of Repository
    dsf.read(cs, ct); //need to save as XML (cs and ct)
    dsf.relinquishDatabaseNode();

    return TXml::transactionSuccess;
  }
  TXml::transactionResult Directory::commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct) {
    //save as XML: <repository:status commit-status="beginCommitCreatePlaceholders" commit-style="full" />
    //*original* Directory location + dbstatus.metaxml
    //note that m_sFullPath of parent Repository has the placeholder name in it
    DatabaseStatusFile dsf(this, m_pLib, m_pNode, this);
    dsf.setDynamicFullPath(); //original path to root of Repository
    dsf.saveOrDelete(cs, ct); //need to save as XML (cs and ct)
    dsf.relinquishDatabaseNode();

    return TXml::transactionSuccess;
  }
  
  TXml::transactionResult Directory::commitSwapInPlaceholders(const IXmlBaseDoc *pDoc, const vector<TXml*> *pvTXmls, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) {
    //so the primary repository is a Directory
    vector<AffectedRepositoryDetails>::iterator iRepository;
    vector<AffectedRepositoryDetails> vAffectedSubRepositorys;
    AffectedRepositoryDetails ardetails;
    Repository *pNearestRepository;
    TXml::transactionResult ret = TXml::transactionFailure;

    //save the unique Repositories
    Debug::report("      ----- affected nodes -> sub-repositories and rationalisation");
    affectedSubRepositories(&vAffectedSubRepositorys, pvAffectedNodes);
    rationaliseAffectedRepositoriesVector(&vAffectedSubRepositorys);

    for (iRepository = vAffectedSubRepositorys.begin(); iRepository != vAffectedSubRepositorys.end(); iRepository++) {
      ardetails          = *iRepository;
      pNearestRepository = ardetails.pRelatedRepository;
      Debug::report("      ----- %s repository", pNearestRepository->getRepositoryTypeName());
      switch (ardetails.iAffectedScope) {
        case TXml::saveTree: {
          ret = pNearestRepository->swapInSaveTreePlaceholder();
          break;
        }
        case TXml::syncImmediateChildren: {
          //this should only carry out deletes for nodes that are not present anymore
          ret = pNearestRepository->syncImmediateChildren();
          break;
        }
        case TXml::saveElementOnly: {
          //only works with Directories (sameNode)
          ret = pNearestRepository->swapInElementOnlyPlaceholder();
          break;
        }
        case TXml::linkChild: NOT_COMPLETE("linkChild"); break;
      }
      delete pNearestRepository;
    }

    return ret;
  }

  TXml::transactionResult Directory::syncImmediateChildren() {
    //traverse the Files and Directories underneath this node
    //delete any File or Directory that does not have a corresponding child::@repository:name

    //used by:
    //  commit process
    const XmlAdminQueryEnvironment ibqe(this, m_pNode->document_const(), new RepositorySaveSecurityContext()); //commit process: Directory::syncImmediateChildren() compare nodes to files

    DIR *dp;
    struct dirent *dirp;
    char *sFilename, *sFullPath;
    size_t iLen;
    size_t iRemoved = 0;

    XmlNodeList<DatabaseNode> *pvChildren;
    XmlNodeList<DatabaseNode>::iterator iChild;
    const DatabaseNode *pChildNode;
    StringVector vNodeRepositoryNames;
    StringVector::iterator iEnd;
    const char *sRepositoryName;
    string sNames;
    Repository *pSubRepository;

    //node children
    //ALL node children MUST be Sub-Repositories because this is a container Directory
    pvChildren = m_pNode->children(&ibqe);
    vNodeRepositoryNames.reserve(pvChildren->size());
    for (iChild = pvChildren->begin(); iChild != pvChildren->end(); iChild++) {
      pChildNode = *iChild;
      sRepositoryName = pChildNode->attributeValueDirect(&ibqe, "name", NAMESPACE_REPOSITORY);
      if (sRepositoryName) {
        vNodeRepositoryNames.push_back(sRepositoryName);
      } else throw MissingRepositoryNameAttribute(this, pChildNode->uniqueXPathToNode(&ibqe, NO_BASE_NODE, INCLUDE_TRANSIENT));
    }
    iEnd = vNodeRepositoryNames.end();

    //loop filesystem directory contents
    if (dp = opendir(m_sFullPath)) {
      while ((dirp = readdir(dp)) != NULL) {
        sFilename = dirp->d_name;

        //ignore directory navigation files (., ..)
        if (!isDirectoryNavigationFile(sFilename)) {
          if (vNodeRepositoryNames.find(sFilename) == iEnd) {
            //not found in child nodes! - remove the file
            iLen      = strlen(m_sFullPath) + strlen(sFilename) + 1;
            sFullPath = MMO_MALLOC(iLen);
            _SNPRINTF2(sFullPath, iLen, "%s%s", m_sFullPath, sFilename);

            //check what sort of Repository this is...
            pSubRepository = Repository::factory_reader(mmParent(), m_pLib, sFullPath, this);

            if (pSubRepository && pSubRepository->removeDuringSync()) {
              switch (dirp->d_type) {
                case DT_REG: {
                  Debug::report("      sync: removing file [%s]", sFullPath);
                  std::remove(sFullPath);
                  break;
                }
                case DT_DIR: {
                  Debug::report("      sync: removing directory [%s]", sFullPath);
                  rmdir_recursive(sFullPath);
                  break;
                }
                default: throw UnknownFileType(this, sFullPath);
              }
              iRemoved++;
            }

            //free up
            MMO_FREE(sFullPath);
            delete pSubRepository;
          }
        }
      }
      closedir(dp);
    } else throw FailedToOpenSyncDirectory(this, m_sFullPath);

    //report
    if (iRemoved) {
      Debug::report("      sync: @repository:name's: [%s]", &vNodeRepositoryNames);
    } else {
      Debug::report("      sync: nothing to remove [%s] ...", &vNodeRepositoryNames);
    }

    //free up
    vector_element_destroy(pvChildren);
    vector_element_free(vNodeRepositoryNames);

    return TXml::transactionSuccess;
  }

  TXml::transactionResult Directory::createElementOnlyPlaceholder() {
    const char *sPlaceholderName;
    DirectoryDetails dd(this, m_pLib, m_pNode, this);
    dd.setDynamicFullPath();
    sPlaceholderName = dd.placeholderName();
    //for the member node -> to the placeholder position, there is no normal place
    Debug::report("      create element only placeholder [%s] ...", sPlaceholderName);
    dd.saveXMLAs(sPlaceholderName);

    //free up
    MMO_FREE(sPlaceholderName);
    dd.relinquishDatabaseNode(); //it's owned by the Directory, sets m_pNode = 0

    return TXml::transactionSuccess;
  }
  TXml::transactionResult Directory::swapInElementOnlyPlaceholder() {
    const char *sPlaceholderName, *sNormalName;
    DirectoryDetails dd(this, m_pLib, m_pNode, this);
    dd.setDynamicFullPath();
    sPlaceholderName = dd.placeholderName();
    sNormalName      = dd.placeholderName(NO_PLACEHOLDER);
    //directory.metaxml may not be there already
    Debug::report("      swap in element only placeholder [%s] -> [%s] ...", sPlaceholderName, sNormalName);
    IOFile::move(sPlaceholderName, sNormalName, IGNORE_MISSING_TARGET);

    //free up
    MMO_FREE(sPlaceholderName);
    MMO_FREE(sNormalName);
    dd.relinquishDatabaseNode(); //it's owned by the Directory, sets m_pNode = 0

    return TXml::transactionSuccess;
  }

  TXml::transactionResult Directory::commitDeleteTransactions(const vector<TXml*> *pvTXmls) {
    size_t iLen = strlen(m_sFullPath) + strlen(TRANSACTION_FILE) + 1;
    char *sPath = MMO_MALLOC(iLen);
    _SNPRINTF2(sPath, iLen, "%s%s", m_sFullPath, TRANSACTION_FILE);
    std::remove(sPath);
    MMO_FREE(sPath);
    return TXml::transactionSuccess;
  }

  TXml::transactionResult Directory::createSaveTreePlaceholder(const bool bAddPlaceholderPrefix) {
    //works directly off the node related to this Repository
    //  which must be instanciated with the constructor::(m_pNode)
    assert(m_pNode);
    //we will traverse all sub-repositories and save everything
    //this Directory will createSaveTreePlaceholder() -> __directory-name
    //but subsequent sub-repositories (Directories and Files) will just save() without createSaveTreePlaceholder()

    //used by:
    //  commit process
    const XmlAdminQueryEnvironment ibqe(this, m_pNode->document_const(), new RepositorySaveSecurityContext()); //commit process: Directory::createSaveTreePlaceholder() create _dir_placeholder_000

    char *sPlaceholderName;
    const char *sName;
    size_t iFullPathLen;
    XmlNodeList<DatabaseNode> *pvChildren = 0;
    XmlNodeList<DatabaseNode>::iterator iChildNode;
    const DatabaseNode *pChildNode;
    Repository *pChildRepository;
    const DatabaseNode *pDTDNode;

    UNWIND_EXCEPTION_BEGIN {
      //-------------------- .../0_commit_placholder_users/
      sPlaceholderName = (char*) placeholderName(bAddPlaceholderPrefix);
      Debug::report("      saving/creating Directory placeholder [%s] ...", sPlaceholderName);
      //mkdir(sFullPath, S_IRWXG | S_IRWXU);
      saveAs(sPlaceholderName);
      //switch our m_sFullPath to the placeholder name or the rest of this objects life
      //child path queries will now build off this placholder location instead
      //~Repository() destructor will free it
      if (m_sFullPath) MMO_FREE(m_sFullPath);
      m_sFullPath = MM_STRDUP(sPlaceholderName);

      //-------------------- .../0_commit_placholder_users/directory.metaxml
      iFullPathLen     = strlen(sPlaceholderName);
      sPlaceholderName = (char*) realloc(sPlaceholderName, iFullPathLen + DirectoryDetails::m_iFilenameLen + 1);
      strcpy(sPlaceholderName + iFullPathLen, DirectoryDetails::m_sFilename);
      Debug::report("      + saving/creating DirectoryDetails [%s] ...", sPlaceholderName);
      DirectoryDetails dd(this, m_pLib, m_pNode, this);
      //for the member node -> to the placeholder position, there is no normal place
      dd.saveXMLAs(sPlaceholderName);
      dd.relinquishDatabaseNode(); //it's owned by the Directory

      //-------------------- .../config/DOCTYPE.dtd
      //in General Server FileSystem::Directory the DTD is owned by the root directory / node
      //and must be output in to this directory also
      if (m_pNode->isRootNode()) {
        if (pDTDNode = m_pNode->db()->dtdNode()) {
          sPlaceholderName = (char*) realloc(sPlaceholderName, iFullPathLen + DOCTypeDefinition::m_iFilenameLen + 1);
          strcpy(sPlaceholderName + iFullPathLen, DOCTypeDefinition::m_sFilename);
          Debug::report("      + saving/creating DTD [%s] ...", sPlaceholderName);
          DOCTypeDefinition dtd(this, m_pLib, pDTDNode, this);
          dtd.saveXMLAs(sPlaceholderName);
        } else Debug::report("      + no Database DTD not found [%s]");
      }

      //-------------------- traverse
      //working from the XML, not the FS
      pvChildren = m_pNode->children(&ibqe);
      Debug::report("      traversing [%s] sub-repositories raw [%s] ...", m_sFullPath, pvChildren->size());
      for (iChildNode = pvChildren->begin(); iChildNode != pvChildren->end(); iChildNode++) {
        pChildNode = *iChildNode;
        //Directory
        //ALL nodes underneath it MUST have @repository:name and @repository:type as well
        //throws RepositoryTypeMissing
        pChildRepository = Repository::factory_writable(mmParent(), m_pLib, pChildNode, this);
        //change rootRepository to this so that the children work if our new m_sFullPath
        //SUB_PLACEHOLDER = false which forces normal filename without placeholder prefix
        pChildRepository->createSaveTreePlaceholder(NO_PLACEHOLDER); //recursive
        delete pChildRepository;
        //delete pChildNode; //~Repository has deleted it
      }

      //-------------------- extra tasks, like resources inclusion
      createSaveTreePlaceholderAdditions(); //virtual
    } UNWIND_EXCEPTION_END;
    
    //free up
    MMO_FREE(sPlaceholderName);
    if (pvChildren) MMO_DELETE(pvChildren); //elements already deleted
    //delete pChildNode; //~Repository will delete it
    //dd is local

    UNWIND_EXCEPTION_THROW;
    
    return TXml::transactionSuccess;
  }

  const char *Directory::placeholderName(const bool bAddPlaceholderPrefix) const {
    char *sPlaceholderName;
    size_t iFullPathLen, iNameLen;

    //placeholder name (.../0_commit_placeholder_users/)
    iFullPathLen     = strlen(m_sFullPath);
    iNameLen         = strlen(m_sName);
    sPlaceholderName = MM_MALLOC_FOR_RETURN(CommitFileTemporary::m_iCommitPrefixLen + iFullPathLen + 1);
    strcpy(sPlaceholderName, m_sFullPath);
    if (bAddPlaceholderPrefix) {
      _SNPRINTF2(sPlaceholderName + iFullPathLen - iNameLen - 1, //-1 pertains to the ending slash on the Directory path
               CommitFileTemporary::m_iCommitPrefixLen + iNameLen + 2,
               "%s%s/",
               CommitFileTemporary::m_sCommitPrefix,
               m_sName
              );
    }
    return sPlaceholderName;
  }

  TXml::transactionResult Directory::swapInSaveTreePlaceholder() {
    //we only ever swap in the top placeholder
    const char *sPlaceholderName = placeholderName(true);
    Debug::report("      swap in placeholder [%s] -> [%s] ...", sPlaceholderName, m_sFullPath);
    move(sPlaceholderName, m_sFullPath);
    MMO_FREE(sPlaceholderName);
    return TXml::transactionSuccess;
  }

  bool Directory::gimme(const char *sFullPath) {
    return sFullPath && !InternetLocation::gimme(sFullPath) && issplitter(sFullPath[strlen(sFullPath) - 1]);
  }

  void Directory::appendXmlTo_preCursor(string &sCurrentXML, const bool bIsTop) const {
    const Repository *fSubItem;

    if (bIsTop) {
      if (fSubItem = firstChild(DOCTypeDefinition::m_iRepositoryType)) {
        fSubItem->appendXmlTo_content(sCurrentXML);
      }
    }
  }

  const char *Directory::readRaw() const {
    //this is a contents request, not a element request
    DIR *dp;
    struct dirent *dirp;
    char *sFilename, *sFullPath;
    string stContents;
    size_t iLen;

    //recurseChildren() dont recurse resource directories
    if (dp = opendir(m_sFullPath)) {
      while ((dirp = readdir(dp)) != NULL) {
        sFilename = dirp->d_name;
        sFullPath = 0;

        //ignore directory navigation files (., ..)
        if (!isDirectoryNavigationFile(sFilename)) {
          //d_type is only available on BSD systems
          //http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
          //see altered dirent.h for windows for more info
          //use lstat()
          switch (dirp->d_type) {
            case DT_LNK: //symbolic link
            case DT_REG: {
              iLen      = strlen(m_sFullPath) + strlen(sFilename) + 1;
              sFullPath = MMO_MALLOC(iLen);
              _SNPRINTF2(sFullPath, iLen, "%s%s", m_sFullPath, sFilename);
              break;
            }
            case DT_DIR: {
              iLen      = strlen(m_sFullPath) + strlen(sFilename) + 2;
              sFullPath = MMO_MALLOC(iLen);
              //note that windwos accepts both types of directory seperator
              _SNPRINTF3(sFullPath, iLen, "%s%s%c", m_sFullPath, sFilename, splitter());
              break;
            }
            case DT_FIFO: //A named pipe, or FIFO. See FIFO Special Files.
            case DT_SOCK: //A local-domain socket.
            case DT_CHR:  //A character device.
            case DT_BLK:   //A block device.
            case DT_UNKNOWN:
            default: throw UnknownFileType(this, sFilename);
          }

          if (sFullPath) {
            stContents += sFilename;
            stContents += '\n';
            MMO_FREE(sFullPath);
          }
        }
      }
      closedir(dp);
    }

    return MM_STRDUP(stContents.c_str());
  }

  void Directory::readValidXML(string &sCurrentXML) const {
    //this is a contents request, not a element request
    //this is a contents request, not a element request
    DIR *dp;
    struct dirent *dirp;
    char *sFilename, *sFullPath;
    const char *sElementName;
    size_t iLen;

    appendXmlTo_startTag(sCurrentXML, NOT_TOP);
    //recurseChildren() dont recurse resource directories
    if (dp = opendir(m_sFullPath)) {
      while ((dirp = readdir(dp)) != NULL) {
        sFilename = dirp->d_name;
        sFullPath = 0;

        //ignore directory navigation files (., ..)
        if (!isDirectoryNavigationFile(sFilename)) {
          //d_type is only available on BSD systems
          //http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
          //see altered dirent.h for windows for more info
          //use lstat()
          switch (dirp->d_type) {
            case DT_LNK: //symbolic link
            case DT_REG: {
              /*
              iLen      = strlen(m_sFullPath) + strlen(sFilename) + 1;
              sFullPath = MMO_MALLOC(iLen);
              _SNPRINTF2(sFullPath, iLen, "%s%s", m_sFullPath, sFilename);
              */
              break;
            }
            case DT_DIR: {
              iLen      = strlen(m_sFullPath) + strlen(sFilename) + 2;
              sFullPath = MMO_MALLOC(iLen);
              //note that windwos accepts both types of directory seperator
              _SNPRINTF3(sFullPath, iLen, "%s%s%c", m_sFullPath, sFilename, splitter());
              break;
            }
            case DT_FIFO: //A named pipe, or FIFO. See FIFO Special Files.
            case DT_SOCK: //A local-domain socket.
            case DT_CHR:  //A character device.
            case DT_BLK:   //A block device.
            case DT_UNKNOWN:
            default: throw UnknownFileType(this, sFilename);
          }

          if (sFullPath) {
            sElementName = xmlLibrary()->xml_element_name(sFilename);
            sCurrentXML += "<repository:";
            sCurrentXML += sElementName;
            //sCurrentXML += " database:on-before-read=\"database:parse-xml(repository:read-valid-xml('";
            //sCurrentXML += sFullPath;
            //sCurrentXML += "'), /object:Server/repository:test/repository:directory)\"";
            sCurrentXML += " />";

            MMO_FREE(sFullPath);
            MMO_FREE(sElementName);
          }
        }
      }
      closedir(dp);
    }
    appendXmlTo_endTag(sCurrentXML, NOT_TOP);
  }

  const char *Directory::toString() const {
    const_iterator iChild;
    string stOut;
    const Repository *pChild;
    
    stOut += "Directory [";
    stOut += m_sName;
    stOut += "]\n";
    for (iChild = begin(); iChild != end(); iChild++) {
      pChild = *iChild;
      stOut += "  ";
      stOut += pChild->getRepositoryTypeName();
      stOut += " [";
      stOut += pChild->name();
      stOut += "]\n";
    }
    return MM_STRDUP(stOut.c_str());
  }

  void Directory::appendXmlTo_startTag(string &sCurrentXML, const bool bIsTop) const {
    const Repository *fSubItem;
    bool bHasRepositoryXMLNS = false,
         bHasAnyXMLNS        = false,
         bHasDefaultXMLNS    = false,
         bHasRepositoryNameAttribute = false,
         bHasRepositoryTypeAttribute = false;
    size_t iContentsStart;
    const char *sLocalNameStart     = 0, 
               *sNameAttributeStart = 0;
    size_t iLocalNameStart, iNameAttributeStart;
    size_t iLastCloser, iTagStart;

    if (fSubItem = firstChild(DirectoryDetails::m_iRepositoryType)) {
      //we have a directory specification directory.metaxml
      //opening tag specification for the directory
      iContentsStart = sCurrentXML.size();
      fSubItem->appendXmlTo_startTag(sCurrentXML);
      //remove closer ready for the attributes
      iLastCloser = sCurrentXML.rfind('>');
      if (iLastCloser != sCurrentXML.npos && iLastCloser >= iContentsStart) sCurrentXML.erase(iLastCloser, 1);

      //repository namespace exists?
      iTagStart = sCurrentXML.rfind('<');
      if (iTagStart != sCurrentXML.npos && iTagStart >= iContentsStart) {
        iLocalNameStart     = sCurrentXML.find(":",  iTagStart);
        sLocalNameStart     = (iLocalNameStart == sCurrentXML.npos ? 0 : sCurrentXML.c_str() + iLocalNameStart + 1);
        iNameAttributeStart = sCurrentXML.find(" name=\"",  iTagStart);
        sNameAttributeStart = (iNameAttributeStart == sCurrentXML.npos ? 0 : sCurrentXML.c_str() + iNameAttributeStart + 7);
        bHasRepositoryXMLNS = sCurrentXML.find(NAMESPACE_REPOSITORY_XMLNS, iTagStart) != sCurrentXML.npos;
        bHasAnyXMLNS        = sCurrentXML.find(" xmlns:",   iTagStart)                != sCurrentXML.npos;
        bHasDefaultXMLNS    = sCurrentXML.find(" xmlns=\"", iTagStart)                != sCurrentXML.npos;
        bHasRepositoryNameAttribute = sCurrentXML.find(" repository:name=\"",  iTagStart) != sCurrentXML.npos;
        bHasRepositoryTypeAttribute = sCurrentXML.find(" repository:type=\"",  iTagStart) != sCurrentXML.npos;
      }
    } else {
      //copy in fullpath and name
      sCurrentXML += '<';
      sCurrentXML += NAMESPACE_REPOSITORY_ALIAS;
      sCurrentXML += ':';
      sCurrentXML += m_xml_filename;
    }

    //namespaces
    if (bIsTop) {
      //object:Server directory.metaxml that may contain namespace decelrations
      if (!bHasDefaultXMLNS) {
        sCurrentXML += ATTRIBUTE_SPACE;
        sCurrentXML += NAMESPACE_DEFAULT;
      }
      //if namespaces have been saved here in the root then they had better be right!
      if (!bHasAnyXMLNS) {
        sCurrentXML += ATTRIBUTE_SPACE;
        sCurrentXML += NAMESPACE_ALL;
      }
    } else {
      //sub-directory, possibly a directory.metaxml, that may contain the repository xmlns decleration
      if (!bHasRepositoryXMLNS) {
        sCurrentXML += ATTRIBUTE_SPACE;
        sCurrentXML += NAMESPACE_REPOSITORY_XMLNS;
      }
    }

    //copy paste fool check
    IFDEBUG(
      if (fSubItem) { //i.e. there is a directory.xml with a different name
        if (sLocalNameStart) {
          //let's check to see if it is the same as the actual directory name
          //or if the @name is correct
          if (  !_STRNEQUAL(sLocalNameStart, m_sName, strlen(m_sName))
            && (!_STRNEQUAL(sNameAttributeStart, m_sName, strlen(m_sName)))
          ) {
            sLocalNameStart = strndup(sLocalNameStart, strlen(m_sName));
            Debug::report("directory has different local-name() AND @name [%s] => [%s]", m_sName, sLocalNameStart);
            MMO_FREE(sLocalNameStart);
          }
        } else {
          Debug::report("unable to locate local-name for directory");
        }
      }
    )

    //every directory needs a @name in order to be navigated
    //<repository:users name="users"
    //@repository:* are not for general use
    //  they are system indicators for permanent HDD saving only
    //  and subject to CHANGE!
    if (!sNameAttributeStart) {
      sCurrentXML += ATTRIBUTE_SPACE;
      sCurrentXML += "name=\"";
      sCurrentXML += (m_sName ? m_sName : "");
      sCurrentXML += '"';
    }

    //print repository attributes
    //these are essentially hints for the commit process to save to disk
    if (!bHasRepositoryNameAttribute) {
      sCurrentXML += ATTRIBUTE_SPACE;
      sCurrentXML += "repository:name=\"";
      sCurrentXML += (m_sName ? m_sName : "");
      sCurrentXML += '"';
    }
    if (!bHasRepositoryTypeAttribute) {
      sCurrentXML += ATTRIBUTE_SPACE;
      sCurrentXML += "repository:type=\"";
      sCurrentXML += getRepositoryTypeName();
      sCurrentXML += '"';
    }

    sCurrentXML += ">";

    //free up
    //if (sLocalNameStart) MMO_FREE(sLocalNameStart); //pointer in to string
    //if (sNameAttributeStart) MMO_FREE(sNameAttributeStart); //pointer in to string
  }

  void Directory::appendXmlTo_endTag(string &sCurrentXML, const bool bIsTop) const {
    const Repository *fSubItem;

    if (fSubItem = firstChild(DirectoryDetails::m_iRepositoryType)) {
      //we have a directory specification directory.metaxml
      //closing tag specification for the directory
      fSubItem->appendXmlTo_endTag(sCurrentXML);
    } else {
      //copy in fullpath and name
      sCurrentXML += "</";
      sCurrentXML += NAMESPACE_REPOSITORY_ALIAS;
      sCurrentXML += ':';
      sCurrentXML += m_xml_filename;
      sCurrentXML += '>';
    }
  }

  int Directory::inputReadCallback(StreamingContext *pStreamingContext, char* sBuffer, int iBufferLen) const {
    //streaming -> xml -> doc parser system
    //gets called constantly until it returns zero with an empty buffer
    //buffer available space is typically 4000
    //sub-tree populated during factory construction (only if using correct contructor)
    //needs to be hierarchical in order to add start, middle and end tags around children
    //but we know parent and children, so we can recurse without function recursion
    const Repository*& pRootRepository       = pStreamingContext->m_pRootRepository;
    const Repository*& pCurrentSubRepository = pStreamingContext->m_pCurrentRepository;
    string& sCurrentXML                      = pStreamingContext->m_sCurrentXML;
    size_t& iCurrentXMLPosition              = pStreamingContext->m_iCurrentXMLPosition;

    static bool bDebug                  = false;
    int iLenSent                        = 0;
    bool bIsTop;
    bool bFoundNextAncestorSiblingOrEnd = false;
    const Repository *pParentRepository = 0;
    Repository::const_iterator iChildRepository, iChildrenEnd;

    //get NEW content if needed (not all calls will require more content)
    //will trigger sub-repository traversals
    //a zero pCurrentSubRepository result indicates the traversal has returned to the root Repository
    //this only happens at the start, it will not land on it again
    //  except when traversing back up WITHIN this loop
    while (iCurrentXMLPosition >= sCurrentXML.size() && pCurrentSubRepository) {
      //reset, this loop will only repeat if sCurrentXML remains empty
      sCurrentXML = "";
      iCurrentXMLPosition = 0;
      bIsTop      = (pCurrentSubRepository == pRootRepository);

      //add the new Repository in to the mix
      //this will normally be only one malloc from one read
      //might yeild a zero result which is why we are looping. we need data before the next step
      //also might be at top (will have the END_TAG of the top)
      if (bDebug) cout << "current startTag:" << pCurrentSubRepository->fullPath() << "\n";
      pCurrentSubRepository->appendXmlTo_preCursor(sCurrentXML, bIsTop); //e.g. DTD and <root xmlns...>
      pCurrentSubRepository->appendXmlTo_startTag( sCurrentXML, bIsTop); //e.g. Directory starts / includeFileNode() starts
      pCurrentSubRepository->appendXmlTo_content(  sCurrentXML, bIsTop); //e.g. File contents

      //non-recursive walk hierarchy appending any close tags to new output stream
      //we always want to arrive at a new START_TAG point
      //  because then we know the next step in this linear walking of the hierarchy
      //order of traverse:
      //  down tree: first child
      //  across tree: next sibling (TODO: order of siblings?: <filename> (order_1).<extension>?)
      //  up tree: parent next sibling
      //  exit (pCurrentSubRepository == 0)

      //try down tree (there might not be any children)
      //we save the current position for next traversal stage in case of no children
      pParentRepository = pCurrentSubRepository;
      iChildRepository  = pParentRepository->begin();
      iChildrenEnd      = pParentRepository->end();
      while (iChildRepository != iChildrenEnd
        && (pCurrentSubRepository = *iChildRepository)
        && !pCurrentSubRepository->includeInXML()
      ) iChildRepository++;

      if (iChildRepository == iChildrenEnd) {
        //across and up tree: next sibling / parent
        pCurrentSubRepository = pParentRepository; //change back because of previous down-children traversal check
        bFoundNextAncestorSiblingOrEnd = false;
        //parent is required to find next sibling of that parent
        //  all hierarchies must have a singular root (Directory)
        while (!bFoundNextAncestorSiblingOrEnd) {
          //this node has no [more] children so output its endTag
          //its parent child list will be tested next and that output too if at the end
          bIsTop = (pCurrentSubRepository == pRootRepository);
          pCurrentSubRepository->appendXmlTo_endTag(sCurrentXML, bIsTop); //e.g. Directory closes
          if (bDebug) cout << "leaving endTag:  " << pCurrentSubRepository->fullPath() << "\n";

          if (pParentRepository = pCurrentSubRepository->parent()) {
            //find this child in the parent vector and move to the next one
            //we have already put out the endTag for the current sibling
            iChildRepository = pParentRepository->find(pCurrentSubRepository);
            iChildrenEnd     = pParentRepository->end();

            //move to the next sibling with includeInXML()
            //if not at the end
            do ++iChildRepository;
            while (iChildRepository != iChildrenEnd
              && (pCurrentSubRepository = *iChildRepository)
              && !pCurrentSubRepository->includeInXML()
            );
            if (iChildRepository != iChildrenEnd) bFoundNextAncestorSiblingOrEnd = true; //found Ancestor-Sibling
            else {
              //we have reached the end of the children list, so move back up
              //up tree: parent next sibling
              //returns zero at the top
              pCurrentSubRepository = pParentRepository;
            }
          } else {
            //we have reached the root again, time to exit and finish
            //allowing the last string to go out
            if (bDebug) cout << "leaving root\n";
            pCurrentSubRepository          = 0;
            bFoundNextAncestorSiblingOrEnd = true; //found End (root)
          }
        }
      }
    }

    //write to output and increase pointer
    //if sCurrentXML is empty then *buffer = 0 and iLenSent = 0 which signals an exit
    _STRNCPY(sBuffer, sCurrentXML.c_str() + iCurrentXMLPosition, iBufferLen); //returns destination unfortunately
    iLenSent             = sCurrentXML.size() - iCurrentXMLPosition;
    iLenSent             = (iLenSent > iBufferLen ? iBufferLen : iLenSent);
    //point to next character, or zero termination character
    //if we hit this again without new / more content then the process will return zeros and exit
    iCurrentXMLPosition += iLenSent;

    return iLenSent;
  }

  void Directory::populateChildren(const bool bForce) {
    //causes recursive behaviour in the contrustors of the sub-repositories
    DIR *dp;
    struct dirent *dirp;
    Repository *pSubRepository;
    Repository *pDirectoryDetails = 0;
    char *sFilename, *sFullPath;
    size_t iLen;
    vector<const char*> vDirectoryContents;
    vector<const char*>::const_iterator iFile;

    //do not recurse resource directories
    if (bForce || !m_bPopulated) {
      if (dp = opendir(m_sFullPath)) {
        //---------------------------------- recurseChildren() 
        //NOTE: that the files are loaded in their natural order
        //  ll -U will list directory contents in natural order
        //when the Directory::createPlaceholders() function runs it will also save them in the correct order
        //so NO NEED to carry out manual ordering
        //TODO: a temporary name ordering is carried out by the ajax javascript loader
        //this should be removed when front-end ordering is active
        while ((dirp = readdir(dp)) != NULL) {
          sFilename = dirp->d_name;
          sFullPath = 0;

          //ignore directory navigation files (., ..)
          if (!isDirectoryNavigationFile(sFilename)) {
            //d_type is only available on BSD systems
            //http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
            //see altered dirent.h for windows for more info
            //use lstat()
            switch (dirp->d_type) {
              case DT_LNK: //symbolic link
              case DT_REG: {
                iLen      = strlen(m_sFullPath) + strlen(sFilename) + 1;
                sFullPath = MMO_MALLOC(iLen);
                _SNPRINTF2(sFullPath, iLen, "%s%s", m_sFullPath, sFilename);
                break;
              }
              case DT_DIR: {
                iLen      = strlen(m_sFullPath) + strlen(sFilename) + 2;
                sFullPath = MMO_MALLOC(iLen);
                //note that windwos accepts both types of directory seperator
                _SNPRINTF3(sFullPath, iLen, "%s%s%c", m_sFullPath, sFilename, splitter());
                break;
              }
              case DT_FIFO:    //A named pipe, or FIFO. See FIFO Special Files.
              case DT_SOCK:    //A local-domain socket.
              case DT_CHR:     //A character device.
              case DT_BLK:     //A block device.
              case DT_UNKNOWN:
              default: throw UnknownFileType(this, sFilename);
            }
            
            if (sFullPath) vDirectoryContents.push_back(sFullPath);
          }
        }
        closedir(dp);
        
        //---------------------------------- create the repositories
        //recursive calls populateChildren() in contructor
        //which therefore marks them as m_bPopulated
        //throws RepositoryTypeUnknown()
        for (iFile = vDirectoryContents.begin(); iFile != vDirectoryContents.end(); iFile++) {
          pSubRepository = Repository::factory_reader(this, m_pLib, *iFile, this);
          if (shouldIncludeSubRepository(pSubRepository)) {
            push_back(pSubRepository);
            if (pSubRepository->type() == DirectoryDetails::m_iRepositoryType) 
              pDirectoryDetails = pSubRepository;
          } else delete pSubRepository;
        }

        //---------------------------------- ordering and any other directive requirements
        if (pDirectoryDetails) pDirectoryDetails->applyDirectives(this);
      }

      m_bPopulated = true;
    }
    
    //free up
    vector_element_free(vDirectoryContents);
  }

  bool Directory::lazyOpenTransactionsFile() {
    transactionFileAccessLock(); {
      if (!m_pTransactionFile) {
        size_t iLen = strlen(m_sFullPath) + strlen(TRANSACTION_FILE) + 1;
        char *sPath = MMO_MALLOC(iLen);
        _SNPRINTF2(sPath, iLen, "%s%s", m_sFullPath, TRANSACTION_FILE);
        Debug::report("opening transactions file [%s]", sPath);
        _FOPEN(m_pTransactionFile, sPath, "a");
        if (!m_pTransactionFile) throw TXmlCannotOpenTransactionStore(this, MM_STRDUP("Directory"));
        MMO_FREE(sPath);
      }
    } transactionFileAccessUnlock();

    return true;
  }

  bool Directory::lazyCloseTransactionsFile() {
    transactionFileAccessLock(); {
      if (m_pTransactionFile) {
        Debug::report("closing transactions file");
        fclose(m_pTransactionFile);
        m_pTransactionFile = 0;
      }
    } transactionFileAccessUnlock();

    return true;
  }

  TXml::transactionResult Directory::rollback() {
    transactionFileAccessLock(); {
      //close if open
      if (m_pTransactionFile) {
        fclose(m_pTransactionFile);
        m_pTransactionFile = 0;
      }

      //delete if there
      size_t iLen = strlen(m_sFullPath) + strlen(TRANSACTION_FILE) + 1;
      char *sPath = MMO_MALLOC(iLen);
      _SNPRINTF2(sPath, iLen, "%s%s", m_sFullPath, TRANSACTION_FILE);
      Debug::report("deleting transactions file [%s]", sPath);
      IOFile::remove(sPath);
      MMO_FREE(sPath);
    } transactionFileAccessUnlock();

    return TXml::transactionSuccess;
  }


  TXml::transactionResult Directory::applyTransaction(TXml *t, IXmlBaseNode *pSerialisationNode, const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED) {
    //directory store uses stored transaction knowledge which are applied and saved to disc during re-startup
    //  whereas an IXmlDoc or InternetLocation might apply it immediately
    TXml::transactionResult tr = TXml::transactionDisabled;
    
    if (enabled()) {
      lazyOpenTransactionsFile(); //m_pTransactionFile

      //write TXML
      if (pSerialisationNode) {
        const char *sTXml = t->serialise(pSerialisationNode);
        fwrite(sTXml, sizeof(char), strlen(sTXml), m_pTransactionFile);
        fputc('\n', m_pTransactionFile);
        fflush(m_pTransactionFile);
        MMO_FREE(sTXml);
        tr = TXml::transactionSuccess;
      } else throw TXmlSerialisationParentNull(this, MM_STRDUP("Directory repository"));
    }

    return tr;
  }



  //-----------------------------------------------------
  //---------------------------------------------- File
  //-----------------------------------------------------
  File::File(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    Repository(pMemoryLifetimeOwner, pLib, sFullPath, pParent) 
  {
    assert(m_sName && m_sFullPath && *m_sName && *m_sFullPath);
  }
  
  File::File(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    Repository(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) 
  {}
  
  File::~File() {}
  
  const Repository::repositoryType File::m_iRepositoryType = tFile;

  const char *File::m_sTypeName = "File";
  const char *File::placeholderName(const bool bAddPlaceholderPrefix) const {
    char *sPlaceholderName;
    size_t iFullPathLen, iNameLen;

    if (bAddPlaceholderPrefix) {
      //this is the main and only File being saved in this createSaveTreePlaceholder()
      iFullPathLen     = strlen(m_sFullPath);
      iNameLen         = strlen(m_sName);
      sPlaceholderName = MM_MALLOC_FOR_RETURN(CommitFileTemporary::m_iCommitPrefixLen + iFullPathLen + 1);
      strcpy(sPlaceholderName, m_sFullPath);
      _SNPRINTF2(sPlaceholderName + iFullPathLen - iNameLen,
                 CommitFileTemporary::m_iCommitPrefixLen + iNameLen + 1,
                 "%s%s",
                 CommitFileTemporary::m_sCommitPrefix,
                 m_sName
                );
    } else {
      //this is being saved as part of a parent createSaveTreePlaceholder() and thus does not need a 0_commit_placeholder prefix
      sPlaceholderName = MM_STRDUP(m_sFullPath);
    }
    return sPlaceholderName;
  }
  
  const char *File::readRaw() const {
    return IOFile::readRaw(m_sFullPath);
  }

  vector<char> *File::readBinary() const {
    return IOFile::readBinary(m_sFullPath);
  }

  TXml::transactionResult File::createSaveTreePlaceholder(const bool bAddPlaceholderPrefix) {
    //works directly off the node related to this Repository
    //  which must be instanciated with the constructor::(m_pNode)
    assert(m_pNode);

    const char *sPlaceholderName;
    const char *sXML = nodeXML(m_pNode);

    sPlaceholderName = placeholderName(bAddPlaceholderPrefix);

    //save
    Debug::report("      saving/creating File placeholder [%s] ...", sPlaceholderName);
    IOFile::saveXMLAs(sPlaceholderName, sXML);

    //free up
    MMO_FREE(sXML);
    MMO_FREE(sPlaceholderName);

    return TXml::transactionSuccess;
  }
  TXml::transactionResult File::swapInSaveTreePlaceholder() {
    //we only ever swap in the top placeholder
    const char *sPlaceholderName = placeholderName(true);
    Debug::report("      swap in placeholder [%s] -> [%s] ...", sPlaceholderName, m_sFullPath);
    move(sPlaceholderName, m_sFullPath);
    MMO_FREE(sPlaceholderName);
    return TXml::transactionSuccess;
  }

  bool File::gimme(const char *sFullPath) {
    return sFullPath && !InternetLocation::gimme(sFullPath) && !issplitter(sFullPath[strlen(sFullPath) - 1]);
  }

  void File::readValidXML(string &sCurrentXML) const {
    readValidXMLFrom(sCurrentXML, m_sFullPath);
  }

  void File::appendXmlTo_content(string &sCurrentXML, const bool bIsTop) const {
    //reading directly in to main buffer
    //this is a gamble that we can remalloc() more space directly because the main buffer may be big
    //instead of using an intermediate buffer to play with first
    const char *sName = 0, *sNameAttribute, *sNameAttributeStart = 0;
    size_t iNameAttributeStart;
    string sInserts;
    size_t iContentsStart;
    size_t iTagStart, iTagNameEnd, iTagEnd, iXMLNSStart;
    bool bClassSkip = false;

    //remember start and read contents in
    iContentsStart = sCurrentXML.size();
    readValidXML(sCurrentXML);

    //inject repository attributes and namespace
    //if we have a start tag...
    iTagStart = sCurrentXML.find('<', iContentsStart);
    if (iTagStart != sCurrentXML.npos) {
      iTagNameEnd = sCurrentXML.find_first_of("/> ", iTagStart);
      iTagEnd     = sCurrentXML.find('>', iTagNameEnd);
      if (iTagNameEnd != sCurrentXML.npos && iTagEnd != sCurrentXML.npos) {
        //@name
        iNameAttributeStart = sCurrentXML.find(" name=\"",  iTagStart);
        sNameAttributeStart = (iNameAttributeStart == sCurrentXML.npos || iNameAttributeStart > iTagEnd ? 0 : sCurrentXML.c_str() + iNameAttributeStart + 7);

        //NAMESPACE_REPOSITORY_XMLNS
        iXMLNSStart = sCurrentXML.find(NAMESPACE_REPOSITORY_XMLNS, iTagNameEnd);
        if (iXMLNSStart > iTagEnd || iXMLNSStart == sCurrentXML.npos) {
          sInserts += ATTRIBUTE_SPACE;
          sInserts += NAMESPACE_REPOSITORY_XMLNS;
        }

        //repository:name
        iXMLNSStart = sCurrentXML.find(" repository:name=", iTagNameEnd);
        if (iXMLNSStart > iTagEnd || iXMLNSStart == sCurrentXML.npos) {
          sInserts += ATTRIBUTE_SPACE;
          sInserts += "repository:name=\"";
          sInserts += (m_sName ? m_sName : "");
          sInserts += '"';
        }

        //repository:type
        iXMLNSStart = sCurrentXML.find(" repository:type=", iTagNameEnd);
        if (iXMLNSStart > iTagEnd || iXMLNSStart == sCurrentXML.npos) {
          sInserts += ATTRIBUTE_SPACE;
          sInserts += "repository:type=\"";
          sInserts += getRepositoryTypeName();
          sInserts += '"';
        }

        //repository:size
        /*
        iSize          = sCurrentXML.size() - iContentsStart;
        iXMLNSStart = sCurrentXML.find(" repository:size=", iTagNameEnd);
        if (iXMLNSStart > iTagEnd || iXMLNSStart == sCurrentXML.npos) {
          sInserts += ATTRIBUTE_SPACE;
          sInserts += "repository:size=\"";
          sInserts += iSize.toString();
          sInserts += '"';
        }
        */

        //copy paste fool check
        if (sNameAttributeStart) {
          //let's check to see if it is the same as the actual file name
          //or if the @name is correct
          sName          = m_sName;
          sNameAttribute = sNameAttributeStart;
          if (_STRNEQUAL(sNameAttribute, "Class__", 7)) {
            sNameAttribute = strchr(sNameAttribute + 7, '_');
            if (sNameAttribute) sNameAttribute++;
            bClassSkip = true;
          }
          while (strchr("_0123456789", *sName)) sName++; //skip ordering
          while (*sName && *sName != '.' && (*sName == '_' || *sName == *sNameAttribute)) {
            sName++;
            sNameAttribute++;
          }
          
          if (*sName && *sName != '.') {
            if (sCurrentXML.find(" meta:different-file-name=\"ok\"",  iTagStart) == sCurrentXML.npos
              && !_STRNEQUAL(sNameAttributeStart, "[@meta:", 7)
              && !_STRNEQUAL(m_sName, "transactions.xml", 16)
            ) {
              sNameAttributeStart = strndup(sNameAttributeStart, strlen(m_sName));
              Debug::report("file has different file-name AND @name [%s] => [%s]", m_sName, sNameAttributeStart, rtWarning);
              if (bClassSkip) Debug::report("(Class__<name>_ skipped)", rtWarning);
              MMO_FREE(sNameAttributeStart);
            }
          }
          
        } else {
          //files without @name are ok
          //Debug::report("unable to locate @name for file");
        }

        sCurrentXML.insert(iTagNameEnd, sInserts);
      }
    }

    //free up
    //if (sName)               MMO_FREE(sName); //pointer in to string
    //if (sNameAttributeStart) MMO_FREE(sNameAttributeStart); //pointer in to string
  }


  //-----------------------------------------------------
  //---------------------------------------------- MetaFile
  //-----------------------------------------------------
  MetaFile::MetaFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    File(pMemoryLifetimeOwner, pLib, sFullPath, pParent) 
  {}
  
  MetaFile::MetaFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    File(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) 
  {}
  
  const Repository::repositoryType MetaFile::m_iRepositoryType = tMetaFile;
  const char *MetaFile::m_sTypeName = "MetaFile";
  bool MetaFile::gimme(const char *sFullPath) {
    const char *sExtension = extensionStandard(sFullPath); //server freed
    const bool isMetaFile = sExtension && !strcmp(sExtension, "metaxml");
    return isMetaFile;
  }


  //-----------------------------------------------------
  //---------------------------------------------- EditorBackupFile
  //-----------------------------------------------------
  EditorBackupFile::EditorBackupFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  EditorBackupFile::EditorBackupFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const Repository::repositoryType EditorBackupFile::m_iRepositoryType = tEditorBackupFile;
  const char *EditorBackupFile::m_sTypeName = "EditorBackupFile";
  bool EditorBackupFile::gimme(const char *sFullPath) {
    return *sFullPath && sFullPath[strlen(sFullPath)-1] == '~';
  }


  //-----------------------------------------------------
  //---------------------------------------------- DirectoryLayout
  //-----------------------------------------------------
  const char   *DirectoryLayout::m_sFilename    = ".directory";
  const size_t  DirectoryLayout::m_iFilenameLen = strlen(DirectoryLayout::m_sFilename);
  DirectoryLayout::DirectoryLayout(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  DirectoryLayout::DirectoryLayout(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const Repository::repositoryType DirectoryLayout::m_iRepositoryType = tDirectoryLayout;
  const char *DirectoryLayout::m_sTypeName = "DirectoryLayout";
  bool DirectoryLayout::gimme(const char *sFullPath) {
    const char *sName = nameStandard(sFullPath);
    const bool isDirectoryLayout = !strcmp(sName, m_sFilename);
    MM_FREE(sName);
    return isDirectoryLayout;
  }


  //-----------------------------------------------------
  //---------------------------------------------- DirectoryDetails
  //-----------------------------------------------------
  const char   *DirectoryDetails::m_sFilename    = "directory.metaxml";
  const size_t  DirectoryDetails::m_iFilenameLen = strlen(DirectoryDetails::m_sFilename);
  const Repository::repositoryType DirectoryDetails::m_iRepositoryType = tDirectoryDetails;
  const char *DirectoryDetails::m_sTypeName = "DirectoryDetails";

  DirectoryDetails::DirectoryDetails(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  DirectoryDetails::DirectoryDetails(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}

  bool DirectoryDetails::gimme(const char *sFullPath) {
    const char *sName = nameStandard(sFullPath);
    const bool isDirectoryDetails = !strcmp(sName, m_sFilename);
    MM_FREE(sName);
    return isDirectoryDetails;
  }
  const char *DirectoryDetails::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    //only one directory.metaxml allowed per directory
    string sDynamicFileSystemPath = baseFileSystemPath(pSubRepositoryNode, pRootRepository);
    sDynamicFileSystemPath += splitter();
    sDynamicFileSystemPath += m_sFilename;
    return MM_STRDUP(sDynamicFileSystemPath.c_str());
  }
  const char *DirectoryDetails::placeholderName(const bool bAddPlaceholderPrefix) const {
    //used during saveElementOnly calls only
    //saveTree calls create there own placeholderName() including the placeholder of the ancestor saving directory
    size_t iFullPathLen    = strlen(m_sFullPath); //.../groups/directory.metaxml
    size_t iNameLen        = strlen(m_sName);
    size_t iLen            = iFullPathLen + CommitFileTemporary::m_iCommitPrefixLen + 1;
    char *sPlaceholderName = MM_MALLOC_FOR_RETURN(iLen);
    strcpy(sPlaceholderName, m_sFullPath);

    if (bAddPlaceholderPrefix) {
      _SNPRINTF2(sPlaceholderName + iFullPathLen - iNameLen,
               iLen,
               "%s%s",
               CommitFileTemporary::m_sCommitPrefix,
               DirectoryDetails::m_sFilename
              );
    }
    return sPlaceholderName;
  }
  
  void DirectoryDetails::applyDirectives(Repository *pTarget) {
    assert(pTarget); //DirectoryDetails should always be given a valid parent
    
    string stDirectiveXML;
    size_t iStart, iEnd;
        
    if (pTarget) {
      readValidXML(stDirectiveXML);
      
      //---------------------------------- sorting of target sub-repositories
      //NOTE: use ls -U1 to generate the natural order filelist
      iStart = stDirectiveXML.find("<xml:sort");
      if (iStart != stDirectiveXML.npos) {
        iStart = stDirectiveXML.find(">", iStart);
        if (iStart != stDirectiveXML.npos) {
          iStart++;
          iEnd = stDirectiveXML.find("<", iStart);
          if (iEnd != stDirectiveXML.npos) {
            string stFilelist = stDirectiveXML.substr(iStart, iEnd-iStart);
            if (stFilelist.size()) {
              //we have a sort situation
              vector<string> vFiles = split(stFilelist.c_str());
              vector<string>::const_iterator iFile;
              vector<const Repository*>::iterator iChildRepository, iCurrent;
              const char *sFilename;
              const Repository *pChildRepository;
              
              if (vFiles.size()) {
                Debug::report("applying repository xml:sort to [%s]", MMO_STRDUP(pTarget->name()));
                iCurrent = pTarget->begin();
                for (iFile = vFiles.begin(); iFile != vFiles.end(); iFile++) {
                  sFilename = iFile->c_str();
                  if (sFilename && *sFilename) {
                    iChildRepository = find_if(pTarget->begin(), pTarget->end(), Repository::FilenamePred(sFilename));
                    if (iChildRepository != pTarget->end()) {
                      //found the target repository
                      //move it to before iCurrent++
                      //PERFORMANCE: this is NOT efficient
                      //IFDEBUG(Debug::report("xml:sort file [%s]", MM_STRDUP(sFilename)));
                      pChildRepository = *iChildRepository;
                      pTarget->erase(iChildRepository);
                      pTarget->insert(iCurrent++, pChildRepository);
                    } else Debug::warn("xml:sort file not found [%s]", MM_STRDUP(sFilename));
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  int DirectoryDetails::saveXMLAs(const char *sFullPath, const bool bThrowIfThere) const {
    assert(m_pNode);
    assert(sFullPath);
    //m_sFullPath always points to the directory, not the directory.metaxml
    //all files are saved in the placeholder directories or names so we always pass in the sFullPath
    //but we can decide the content here

    const char *sXML = nodeXML(m_pNode, ELEMENT_ONLY);
    IOFile::saveXMLAs(sFullPath, sXML, bThrowIfThere);

    //free up
    MMO_FREE(sXML);
    
    return 0;
  }

  void DirectoryDetails::appendXmlTo_startTag(string &sCurrentXML, const bool bIsTop) const {
    //e.g. 
    //  <repository:requests gs:transient-area="yes" /> (/ will be removed)
    //  <repository:requests gs:transient-area="yes">
    //  <repository:requests gs:transient-area="yes">
    //    other things, e.g. sorting
    //  </repository:requests>
    size_t iFirstClosingTagEnd;
    size_t iContentsStart = sCurrentXML.size();
    
    readValidXML(sCurrentXML); 
    iFirstClosingTagEnd = sCurrentXML.find('>', iContentsStart);
    if (iFirstClosingTagEnd != sCurrentXML.npos) {
      //change content to a start tag if necessary
      if (sCurrentXML[iFirstClosingTagEnd - 1] == '/')
        sCurrentXML[--iFirstClosingTagEnd] = '>';
      sCurrentXML.resize(iFirstClosingTagEnd + 1);
    } else throw XMLParseFailedAt(mmParent(), m_sFullPath);
  }
  
  void DirectoryDetails::appendXmlTo_endTag(string &sCurrentXML, const bool bIsTop) const {
    size_t iTagNameEnd, iFirstTagOpen;
    size_t iContentsStart = sCurrentXML.size();
    
    readValidXML(sCurrentXML); //e.g. <repository:requests gs:transient-area="yes" />
    iFirstTagOpen = sCurrentXML.find('<', iContentsStart);
    if (iFirstTagOpen != sCurrentXML.npos) {
      //find first end of element name
      //if it is a space then it MUST be the first space before an attribute of tag end
      //if it is a / in an attribute then the space will catch it first
      iTagNameEnd = sCurrentXML.find_first_of(" />", iFirstTagOpen);
      if (iTagNameEnd != sCurrentXML.npos) {
        sCurrentXML.resize(iTagNameEnd);
        sCurrentXML.insert(iFirstTagOpen + 1, "/");
        sCurrentXML += '>';
      } else throw XMLParseFailedAt(mmParent(), m_sFullPath);
    } else throw XMLParseFailedAt(mmParent(), m_sFullPath);
  }


  //-----------------------------------------------------
  //---------------------------------------------- DatabaseStatusFile
  //-----------------------------------------------------
  const char   *DatabaseStatusFile::m_sFilename    = "dbstatus.metaxml";
  const size_t  DatabaseStatusFile::m_iFilenameLen = strlen(DatabaseStatusFile::m_sFilename);

  DatabaseStatusFile::DatabaseStatusFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  DatabaseStatusFile::DatabaseStatusFile(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), MetaFile(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const Repository::repositoryType DatabaseStatusFile::m_iRepositoryType = tDatabaseStatusFile;
  const char *DatabaseStatusFile::m_sTypeName = "DatabaseStatusFile";
  bool DatabaseStatusFile::gimme(const char *sFullPath) {
    const char *sName = nameStandard(sFullPath);
    const bool isDirectoryDetails = !strcmp(sName, "dbstatus.metaxml");
    MM_FREE(sName);
    return isDirectoryDetails;
  }
  const char *DatabaseStatusFile::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    //only one dbstatus.metaxml allowed in a directory
    string sDynamicFileSystemPath = baseFileSystemPath(pSubRepositoryNode, pRootRepository);
    sDynamicFileSystemPath += splitter();
    sDynamicFileSystemPath += m_sFilename;
    return MM_STRDUP(sDynamicFileSystemPath.c_str());
  }
  
  void DatabaseStatusFile::saveOrDelete(TXml::commitStatus cs, TXml::commitStyle ct) const {
    //always saves to the original place, not the placeholder position
    //<repository:status commit-status="beginCommitCreatePlaceholders" commit-style="full" />
    const char *sCommitStatusName = 0,
               *sCommitStyleName  = 0;
    string sXML;

    if (!m_sFullPath) throw MissingPathToFile(this);

    //contents
    if (cs == TXml::commitFinished) {
      //end of commit, so delete file completely
      IOFile::remove(m_sFullPath);
    } else {
      //write mid-status in XML
      sCommitStatusName = getCommitStatusName(cs);
      sCommitStyleName  = getCommitStyleName(ct);
      sXML += "<repository:status commit-status=\"";
      sXML += sCommitStatusName;
      sXML += "\" commit-style=\"";
      sXML += sCommitStyleName;
      sXML += "\" />";
      IOFile::saveXMLAs(m_sFullPath, sXML.c_str(), OVERWRITE);
    }

    //free up
    //if (sCommitStatusName) MMO_FREE(sCommitStatusName); //constants!
    //if (sCommitStyleName)  MMO_FREE(sCommitStyleName);  //constants!
  }

  void DatabaseStatusFile::read(TXml::commitStatus *cs, TXml::commitStyle *ct) const {
    string sCurrentXML;
    const char *sCommitStatusName = 0;
    const char *sCommitStyleName  = 0;

    if (!m_sFullPath) throw MissingPathToFile(this);
    *cs = TXml::commitStatusUnknown;
    *ct = TXml::notRequired;

    //idiot check the the path is the original place
    assert(m_sFullPath && !strstr(m_sFullPath, "0_commit_placeholder"));

    if (readValidXMLFrom(sCurrentXML, m_sFullPath)) {
      //non-zero response indicates error: file not there
      *cs = TXml::commitFinished;
    } else {
      //easy quick string handling XML analysis functions
      //return zro if not found
      sCommitStatusName = Repository::tagAttributeValue(sCurrentXML.c_str(), "commit-status");
      sCommitStyleName  = Repository::tagAttributeValue(sCurrentXML.c_str(), "commit-style");

      //TODO: DatabaseStatusFile: these don't throw when there is an un-parsed string
      //TODO: DatabaseStatusFile: we are also not checking for zero
      *cs = parseCommitStatus(sCommitStatusName);
      *ct = parseCommitStyle( sCommitStyleName);
    }

    //free up
    if (sCommitStatusName) MMO_FREE(sCommitStatusName);
    if (sCommitStyleName)  MMO_FREE(sCommitStyleName);
  }

  //-----------------------------------------------------
  //---------------------------------------------- CommitFileTemporary (includes directories)
  //-----------------------------------------------------
  const char  *CommitFileTemporary::m_sCommitPrefix    = "0_commit_placeholder_";
  const size_t CommitFileTemporary::m_iCommitPrefixLen = strlen(m_sCommitPrefix);

  CommitFileTemporary::CommitFileTemporary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), File(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  CommitFileTemporary::CommitFileTemporary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), File(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const Repository::repositoryType CommitFileTemporary::m_iRepositoryType = tCommitFileTemporary;
  const char *CommitFileTemporary::m_sTypeName = "CommitFileTemporary";
  bool CommitFileTemporary::gimme(const char *sFullPath) {
    const char *sName = nameStandard(sFullPath);
    const bool isDirectoryDetails = !strncmp(sName, m_sCommitPrefix, m_iCommitPrefixLen);
    MM_FREE(sName);
    return isDirectoryDetails;
  }


  //-----------------------------------------------------
  //---------------------------------------------- DOCTypeDefinition
  //-----------------------------------------------------
  const char   *DOCTypeDefinition::m_sFilename    = "DOCTYPE.dtd";
  const size_t  DOCTypeDefinition::m_iFilenameLen = strlen(DOCTypeDefinition::m_sFilename);

  DOCTypeDefinition::DOCTypeDefinition(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    MetaFile(pMemoryLifetimeOwner, pLib, sFullPath, pParent) 
  {}
  
  DOCTypeDefinition::DOCTypeDefinition(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    MetaFile(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) 
  {}
  
  const Repository::repositoryType DOCTypeDefinition::m_iRepositoryType = tDOCTypeDefinition;
  const char *DOCTypeDefinition::m_sTypeName = "DOCTypeDefinition";
  bool DOCTypeDefinition::gimme(const char *sFullPath) {
    const char *sName = nameStandard(sFullPath);
    const bool isDTD = !strcmp(sName, m_sFilename);
    MM_FREE(sName);
    return isDTD;
  }
  const char *DOCTypeDefinition::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    //only one DOCTYPE.dtd allowed in a directory
    string sDynamicFileSystemPath = baseFileSystemPath(pSubRepositoryNode, pRootRepository);
    sDynamicFileSystemPath += splitter();
    sDynamicFileSystemPath += m_sFilename;
    return MM_STRDUP(sDynamicFileSystemPath.c_str());
  }
  const char *DOCTypeDefinition::placeholderName(const bool bAddPlaceholderPrefix) const {
    size_t iLen = strlen(m_sFullPath)
                  + (bAddPlaceholderPrefix ? CommitFileTemporary::m_iCommitPrefixLen : 0)
                  + DOCTypeDefinition::m_iFilenameLen
                  + 1;
    char *sPlaceholderName = MM_MALLOC_FOR_RETURN(iLen);
    _SNPRINTF3(sPlaceholderName,
               iLen,
               "%s%s%s",
               m_sFullPath,
               (bAddPlaceholderPrefix ? CommitFileTemporary::m_sCommitPrefix : ""),
               DOCTypeDefinition::m_sFilename
              );
    return sPlaceholderName;
  }

  int DOCTypeDefinition::saveXMLAs(const char *sFullPath, const bool bThrowIfThere) const {
    assert(m_pNode);
    assert(sFullPath);
    //m_sFullPath always points to the directory, not the directory.metaxml
    //all files are saved in the placeholder directories or names so we always pass in the sFullPath
    //but we can decide the content here

    const char *sXML = nodeXML(m_pNode, ELEMENT_ONLY);
    IOFile::saveXMLAs(sFullPath, sXML, bThrowIfThere);

    //free up
    MMO_FREE(sXML);
    
    return 0;
  }

  void DOCTypeDefinition::appendXmlTo_content(string &sCurrentXML, const bool bIsTop) const {
    readValidXML(sCurrentXML);
  }


  //-----------------------------------------------------
  //---------------------------------------------- ResourcesStore
  //-----------------------------------------------------
  const char   *ResourcesStore::m_sFilename    = "resources";
  const size_t  ResourcesStore::m_iFilenameLen = strlen(ResourcesStore::m_sFilename);
  const Repository::repositoryType ResourcesStore::m_iRepositoryType = tResourcesStore;

  ResourcesStore::ResourcesStore(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), Directory(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  ResourcesStore::ResourcesStore(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), Directory(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const char *ResourcesStore::m_sTypeName = "ResourcesStore";
  bool ResourcesStore::gimme(const char *sFullPath) {
    //ResourcesStore used to contain binary information on the GS
    //But this is illegal now
    return false;
    
    /*
    const char *sName = nameStandard(sFullPath);
    const bool isResources = !strcmp(sName, m_sFilename);
    MMO_FREE(sName);
    return isResources;
    */
  }
  
  bool ResourcesStore::shouldIncludeSubRepository(const Repository *pSubRepository) const {
    return (pSubRepository->type() == tDirectoryDetails);
  }

  TXml::transactionResult ResourcesStore::createSaveTreePlaceholderAdditions() {
    //copy original location files -> placeholder directory
    //only called in recursive Directory situation with placeholders in m_pParent::m_sFullPath
    //m_pNode   = repository:resoruces
    //m_pParent = with placeholder name
    const char *sOriginalPath    = dynamicFileSystemPath(); //path to this node from the root repository of the Database
    const char *sPlaceholderName = dynamicFileSystemPath(m_pNode, m_pParent);

    //idiot check that these are not the same place
    assert(strcmp(sOriginalPath, sPlaceholderName));

    //copy everything from sOriginalPath -> sPlaceholderName
    //both directories already exist
    //don't delete the directories first because we may have a directory.metaxml in there already
    Debug::report("      + recursive resources copy...");
    if (cpdir_recursive(sOriginalPath, sPlaceholderName)) //recursive re-create
      throw ResourcesCopyFailure(this, sOriginalPath, sPlaceholderName, SOCKETS_ERRNO);

    return TXml::transactionSuccess;
  }


  //-----------------------------------------------------
  //---------------------------------------------- Hardlink
  //-----------------------------------------------------
  const char   *Hardlink::m_sExtension    = "hardlink";
  const size_t  Hardlink::m_iExtensionLen = strlen(Hardlink::m_sExtension);
  const Repository::repositoryType Hardlink::m_iRepositoryType = tHardlink;

  Hardlink::Hardlink(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent, const char *sArg): MemoryLifetimeOwner(pMemoryLifetimeOwner), File(pMemoryLifetimeOwner, pLib, sFullPath, pParent) {}
  Hardlink::Hardlink(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): MemoryLifetimeOwner(pMemoryLifetimeOwner), File(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) {}
  const char *Hardlink::m_sTypeName = "Hardlink";
  bool Hardlink::gimme(const char *sFullPath) {
    bool isHardlink        = false;
    //TODO: need to enable hardlink removal...
    const char *sExtension = extensionStandard(sFullPath);
    if (sExtension) isHardlink = _STREQUAL(sExtension, m_sExtension);
    //no free sExtension because it is part of filename
    return isHardlink;
  }

  int Hardlink::saveXMLAs(const char *sFullPath, const bool bThrowIfThere) const {
    assert(m_pNode);
    assert(sFullPath);

    const char *sXML = nodeXML(m_pNode, ELEMENT_ONLY);
    IOFile::saveXMLAs(sFullPath, sXML, bThrowIfThere);

    //free up
    MMO_FREE(sXML);
    
    return 0;
  }

  const char *Hardlink::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    //.../.../.../Class__DatabaseElement.hardlink
    //used by:
    //  commit process
    const XmlAdminQueryEnvironment ibqe(this, m_pNode->document_const()); //commit process: Directory::syncImmediateChildren() compare nodes to files
    const DatabaseNode *pParentDirectoryNode;
    const char *sName, 
               *sFilename, 
               *sPosition = 0;
    size_t iPosition;
    string sDynamicFileSystemPath;

    //directory path to the Directory holding the hardlink, not the hardlink
    pParentDirectoryNode   = pSubRepositoryNode->parentNode(&ibqe);
    sDynamicFileSystemPath = baseFileSystemPath(pParentDirectoryNode, pRootRepository);
    sName = m_pNode->xmlID(&ibqe);
    if (!sName || !*sName) sName = m_pNode->fullyQualifiedName();
    if (!sName || !*sName) sName = MM_STRDUP("unknown");
    iPosition = m_pNode->position();
    sPosition = itoa(iPosition);
    sFilename = xmlLibrary()->xml_element_name(sName);

    sDynamicFileSystemPath += splitter();
    sDynamicFileSystemPath += sFilename;
    sDynamicFileSystemPath += "_";
    sDynamicFileSystemPath += sPosition; //ensure unique when multiple same hardlinks next to each other
    sDynamicFileSystemPath += ".";
    sDynamicFileSystemPath += m_sExtension;

    //free up
    if (pParentDirectoryNode) delete pParentDirectoryNode;
    if (sName)                MMO_FREE(sName);
    if (sFilename)            MMO_FREE(sFilename);
    if (sPosition)            MMO_FREE(sPosition);

    return MM_STRDUP(sDynamicFileSystemPath.c_str());
  }

  const char *Hardlink::placeholderName(const bool bAddPlaceholderPrefix) const {
    char *sPlaceholderName;
    size_t iFullPathLen, iNameLen;

    if (bAddPlaceholderPrefix) {
      //this is the main and only File being saved in this createSaveTreePlaceholder()
      iFullPathLen     = strlen(m_sFullPath);
      iNameLen         = strlen(m_sName);
      sPlaceholderName = MM_MALLOC_FOR_RETURN(CommitFileTemporary::m_iCommitPrefixLen + iFullPathLen + 1);
      strcpy(sPlaceholderName, m_sFullPath);
      _SNPRINTF2(sPlaceholderName + iFullPathLen - iNameLen,
                 CommitFileTemporary::m_iCommitPrefixLen + iNameLen + 1,
                 "%s%s",
                 CommitFileTemporary::m_sCommitPrefix,
                 m_sName
                );
    } else {
      //this is being saved as part of a parent createSaveTreePlaceholder() and thus does not need a 0_commit_placeholder prefix
      sPlaceholderName = MM_STRDUP(m_sFullPath);
    }
    return sPlaceholderName;
  }
}
