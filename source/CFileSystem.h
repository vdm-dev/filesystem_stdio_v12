#pragma once


#include "CPathList.h"
#include "CBspArchive.h"


#define LogFunction(message) (this->Log(0, __LINE__, __FILE__, __func__, message))


class CFinder;


class CFileSystem : public IFileSystem
{
public:
     CFileSystem();
     ~CFileSystem();

     //
     // IAppSystem
     //
     bool Connect(CreateInterfaceFn factory);
     void Disconnect();
 
     void* QueryInterface(const char* pInterfaceName);
 
     InitReturnVal_t Init();
     void Shutdown();


     //
     // IBaseFileSystem
     //
     virtual int Read(void* pOutput, int size, FileHandle_t file);
     virtual int Write(void const* pInput, int size, FileHandle_t file);

     virtual FileHandle_t Open(const char* pFileName, const char* pOptions, const char* pathID = 0);
     virtual void Close(FileHandle_t file);

     virtual void Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType);
     virtual unsigned int Tell(FileHandle_t file);
     virtual unsigned int Size(FileHandle_t file);
     virtual unsigned int Size(const char* pFileName, const char* pPathID = 0);

     virtual void Flush(FileHandle_t file);
     virtual bool Precache(const char* pFileName, const char* pPathID = 0);

     virtual bool FileExists(const char* pFileName, const char* pPathID = 0);
     virtual bool IsFileWritable(char const* pFileName, const char* pPathID = 0);
     virtual bool SetFileWritable(char const* pFileName, bool writable, const char* pPathID = 0);

     virtual long GetFileTime(const char* pFileName, const char* pPathID = 0);


     //
     // IFileSystem
     //
     virtual void RemoveAllSearchPaths(void);

     virtual void AddSearchPath(const char* pPath, const char* pathID, SearchPathAdd_t addType = PATH_ADD_TO_TAIL);
     virtual bool RemoveSearchPath(const char* pPath, const char* pathID = 0);

     virtual void RemoveFile(char const* pRelativePath, const char* pathID = 0);
     virtual void RenameFile(char const* pOldPath, char const* pNewPath, const char* pathID = 0);

     virtual void CreateDirHierarchy(const char* path, const char* pathID = 0);

     virtual bool IsDirectory(const char* pFileName, const char* pathID = 0);
     virtual void FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime);
     virtual bool IsOk(FileHandle_t file);
     virtual bool EndOfFile(FileHandle_t file);
     virtual char* ReadLine(char* pOutput, int maxChars, FileHandle_t file);
     virtual int FPrintf(FileHandle_t file, char* pFormat, ...);

     virtual CSysModule* LoadModule(const char* pFileName, const char* pPathID = 0, bool bValidatedDllOnly = true);
     virtual void UnloadModule(CSysModule* pModule);

     virtual const char* FindFirst(const char* pWildCard, FileFindHandle_t* pHandle);
     virtual const char* FindNext(FileFindHandle_t handle);
     virtual bool FindIsDirectory(FileFindHandle_t handle);
     virtual void FindClose(FileFindHandle_t handle);

     virtual const char* GetLocalPath(const char* pFileName, char* pLocalPath, int localPathBufferSize);
     virtual bool FullPathToRelativePath(const char* pFullpath, char* pRelative, int maxlen);
     virtual bool GetCurrentDirectory(char* pDirectory, int maxlen);

     virtual void PrintOpenedFiles(void);
     virtual void PrintSearchPaths(void);

     virtual void SetWarningFunc(void (*pfnWarning)(const char* fmt, ...));
     virtual void SetWarningLevel(FileWarningLevel_t level);
     virtual void AddLoggingFunc(void (*pfnLogFunc)(const char* fileName, const char* accessType));
     virtual void RemoveLoggingFunc(FileSystemLoggingFunc_t logFunc);

     virtual fsasync_t AsyncFilePrefetch(int numFiles, const asyncFileList_t* fileListPtr);
     virtual fsasync_t AsyncFileFinish(int asyncID, bool wait);
     virtual fsasync_t AsyncFileAbort(int asyncID);
     virtual fsasync_t AsyncFileStatus(int asyncID);
     virtual fsasync_t AsyncFlush();

     virtual const FileSystemStatistics* GetFilesystemStatistics();

     virtual WaitForResourcesHandle_t WaitForResources(const char* resourcelist);
     virtual bool GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress /* out */, bool* complete /* out */);
     virtual void CancelWaitForResources(WaitForResourcesHandle_t handle);

     virtual int HintResourceNeed(const char* hintlist, int forgetEverything);
     virtual bool IsFileImmediatelyAvailable(const char* pFileName);

     virtual void GetLocalCopy(const char* pFileName);

     virtual FileNameHandle_t FindOrAddFileName(char const* pFileName);
     virtual bool String(const FileNameHandle_t& handle, char* buf, int buflen);

     virtual bool IsOk2(FileHandle_t file);
     virtual void RemoveSearchPaths(const char* szPathID);

     virtual bool IsSteam() const;

     virtual FilesystemMountRetval_t MountSteamContent(int nExtraAppId = -1);

     virtual const char* FindFirstEx(const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle);

     virtual void MarkPathIDByRequestOnly(const char* pPathID, bool bRequestOnly);

     virtual fsasync_t AsyncFileSetPriority(int asyncID, int newPriority);

     virtual bool AddPackFile(const char* fullpath, const char* pathID);

     virtual fsasync_t AsyncFileWrite(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory);
     virtual fsasync_t AsyncFileAppendFile(const char* pDestFileName, const char* pSrcFileName);
     virtual void AsyncFileFinishAllWrites();

private:
    void Log(int severity, const char* format, ...) const;
    void Log(int severity, int line = 0, const char* fileName = NULL, const char* function = NULL, const char* message = NULL) const;

private:
    CPathList _paths;
    CBspArchive _map;
    CFinder* _finder;

    void (*_consoleWrite)(const char* fmt, ...);

    int _severity;
};

