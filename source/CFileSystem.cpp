#include "CFileSystem.h"

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "CFile.h"
#include "CFinder.h"
#include "ExLib.h"

#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif

static CFileSystem g_FileSystem;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION, g_FileSystem);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CFileSystem, IBaseFileSystem, BASEFILESYSTEM_INTERFACE_VERSION, g_FileSystem);


CFileSystem::CFileSystem()
    : _finder(NULL)
    , _consoleWrite(NULL)
    , _severity(2)
{
    _finder = new CFinder(_paths.List());
}

CFileSystem::~CFileSystem()
{
    if (_finder)
    {
        delete _finder;
        _finder = NULL;
    }
}

void CFileSystem::RemoveAllSearchPaths(void)
{
    LogFunction("");
    _paths.Clear();
}


void CFileSystem::AddSearchPath(const char* pPath, const char* pathID, SearchPathAdd_t addType)
{
    if (AddPackFile(pPath, pathID))
        return;

    char fullPath[CPATH_LIMIT + 1];
    memset(fullPath, 0, sizeof(fullPath));

    if (CPath::IsAbsolute(pPath))
    {
        exs_strcpy(fullPath, sizeof(fullPath), pPath);
        DWORD attributes = GetFileAttributesA(fullPath);
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            return;
    }
    else
    {
        GetModuleFileNameA(NULL, fullPath, sizeof(fullPath));
        CPath::KeepPath(fullPath);
        exs_strcat(fullPath, sizeof(fullPath), pPath);

        DWORD attributes = GetFileAttributesA(fullPath);
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            DWORD length = GetCurrentDirectoryA(sizeof(fullPath), fullPath);
            CPath::IncludeTrailingBackslash(fullPath, length);
            exs_strcat(fullPath, sizeof(fullPath), pPath);

            attributes = GetFileAttributesA(fullPath);
            if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                return;
        }
    }

    CPath::Normalize(fullPath);
    CPath::IncludeTrailingBackslash(fullPath, strlen(fullPath));
    _paths.Append(fullPath, pathID, addType == PATH_ADD_TO_HEAD);
}

bool CFileSystem::RemoveSearchPath(const char* pPath, const char* pathID)
{
    char suffix[10];
    if (CPath::GetCompleteSuffix(suffix, sizeof(suffix), pPath) && _stricmp(suffix, ".bsp") == 0)
    {
        _map.Clear();
        return 1;
    }

    int count = _paths.Remove(pPath, pathID);

    Log(1, "RemoveSearchPath(%s, %s) -> %d", pPath, pathID, count);

    return (count > 0);
}

void CFileSystem::RemoveFile(char const* pRelativePath, const char* pathID)
{
    LogFunction(pRelativePath);
}

void CFileSystem::RenameFile(char const* pOldPath, char const* pNewPath, const char* pathID)
{
    LogFunction(pOldPath);
}


void CFileSystem::CreateDirHierarchy(const char* path, const char* pathID)
{
    LogFunction(path);
}


bool CFileSystem::IsDirectory(const char* pFileName, const char* pathID)
{
    LogFunction("");
    return false;
}

void CFileSystem::FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime)
{
    LogFunction("");
}

bool CFileSystem::IsOk(FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->IsOpen();
    return false;
}

bool CFileSystem::EndOfFile(FileHandle_t file)
{
    if (file)
        return Tell(file) >= Size(file);

    return false;
}

char* CFileSystem::ReadLine(char* pOutput, int maxChars, FileHandle_t file)
{
    if (!pOutput || !file || maxChars <= 0)
        return NULL;

    int offset = Tell(file);

    int readed = Read(pOutput, maxChars, file);
    if (readed == 0)
        return NULL;

    if (readed == maxChars)
    {
        readed--;
        pOutput[maxChars - 1] = '\0';
    }

    for (int i = 0; i < readed; ++i)
    {
        if (pOutput[i] == '\n')
        {
            Seek(file, offset + i + 1, FILESYSTEM_SEEK_HEAD);
            pOutput[i + 1] = '\0';
            return pOutput;
        }
        else if (pOutput[i] == '\r')
        {
            int a = 0;
        }
    }

    pOutput[readed] = '\0';
    return pOutput;
}

int CFileSystem::FPrintf(FileHandle_t file, char* pFormat, ...)
{
    if (!pFormat || !file)
        return 0;

    char   buffer[256];
    size_t length = sizeof(buffer);
    char* output = buffer;

    va_list arguments;
    va_start(arguments, pFormat);
    int result = vsnprintf(output, length, pFormat, arguments);
    std::vector<char> dynamic;
    while (result < 0 || result >= static_cast<int>(length))
    {
        length += 256;
        dynamic.resize(length);
        output = &dynamic[0];
        result = vsnprintf(output, length, pFormat, arguments);
    }
    va_end(arguments);

    if (result > 0)
    {
        return Write(output, result, file);
    }
    else
    {
        return 0;

    }
}


CSysModule* CFileSystem::LoadModule(const char* pFileName, const char* pPathID, bool bValidatedDllOnly)
{
    if (!pFileName)
        return NULL;

    char libraryName[CPATH_LIMIT + 1];

    if (CPath::IsAbsolute(pFileName))
    {
        strcpy(libraryName, pFileName);

        if (strstr(libraryName, ".dll") == NULL)
            strcat(libraryName, ".dll");

        HMODULE handle = LoadLibraryA(libraryName);

        if (handle)
        {
            Log(1, "LoadModule(%s, %s) -> %s", pFileName, pPathID, libraryName);
            return reinterpret_cast<CSysModule*>(handle);
        }
    }
    else
    {
        int offset = 0;
        while (true)
        {
            if (_paths.Resolve(pFileName, pPathID, libraryName, &offset) == 0)
                break;

            offset++;

            if (strstr(libraryName, ".dll") == NULL)
                strcat(libraryName, ".dll");

            HMODULE handle = LoadLibraryA(libraryName);

            if (handle)
            {
                Log(1, "LoadModule(%s, %s) -> %s", pFileName, pPathID, libraryName);
                return reinterpret_cast<CSysModule*>(handle);
            }
        }
    }

    Log(1, "LoadModule(%s, %s) -> NOT LOADED", pFileName, pPathID);
    return NULL;
}

void CFileSystem::UnloadModule(CSysModule* pModule)
{
    if (pModule)
    {
        char path[MAX_PATH];
        path[0] = '\0';
        GetModuleFileNameA(reinterpret_cast<HMODULE>(pModule), path, sizeof(path));
        Log(1, "UnloadModule(%s)", path);
        FreeLibrary(reinterpret_cast<HMODULE>(pModule));
    }
}


const char* CFileSystem::FindFirst(const char* pWildCard, FileFindHandle_t* pHandle)
{
    return FindFirstEx(pWildCard, NULL, pHandle);
}

const char* CFileSystem::FindNext(FileFindHandle_t handle)
{
    if (_finder)
        return _finder->Next(handle);

    return NULL;
}

bool CFileSystem::FindIsDirectory(FileFindHandle_t handle)
{
    if (_finder)
        return _finder->IsDirectory(handle);

    return false;
}

void CFileSystem::FindClose(FileFindHandle_t handle)
{
    if (_finder)
        return _finder->End(handle);
}


const char* CFileSystem::GetLocalPath(const char* pFileName, char* pLocalPath, int localPathBufferSize)
{
    if (!pFileName || !pLocalPath || localPathBufferSize < 1)
        return NULL;

    pLocalPath[0] = '\0';

    DWORD attributes = INVALID_FILE_ATTRIBUTES;

    char path[CPATH_LIMIT + 1];
    const char* result = path;

    if (CPath::IsAbsolute(pFileName))
    {
        result = pFileName;
        attributes = GetFileAttributesA(pFileName);
    }
    else
    {
        int offset = 0;
        while (true)
        {
            attributes = INVALID_FILE_ATTRIBUTES;

            if (_paths.Resolve(pFileName, NULL, path, &offset) == 0)
                break;

            offset++;

            attributes = GetFileAttributesA(path);

            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                break;
        }
    }

    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        size_t length = strlen(result);

        if (static_cast<int>(length) > localPathBufferSize)
            return NULL;

        strcpy(pLocalPath, result);
        Log(1, "GetLocalPath(%s, %d) -> %s", pFileName, localPathBufferSize, pLocalPath);
        return pLocalPath;
    }
    else
    {
        return NULL;
    }
}

bool CFileSystem::FullPathToRelativePath(const char* pFullpath, char* pRelative, int maxlen)
{
    LogFunction("");
    return false;
}

bool CFileSystem::GetCurrentDirectory(char* pDirectory, int maxlen)
{
    int result = static_cast<int>(::GetCurrentDirectoryA(maxlen, pDirectory));

    if (result == 0 || result > maxlen)
        return false;

    CPath::Normalize(pDirectory);

    if (pDirectory[result - 1] == '\\')
        pDirectory[result - 1] = '\0';

    return true;
}


void CFileSystem::PrintOpenedFiles(void)
{
    LogFunction("");
}

void CFileSystem::PrintSearchPaths(void)
{
    if (_consoleWrite)
    {
        _consoleWrite("Filesystem Paths:\n-----------------\n");

        int total = 0;

        if (_map.IsOpen())
        {
            total++;
            _consoleWrite("%02d. [map] %s\n", total, _map.FileName());
        }

        std::vector<CPath>& list = _paths.List();

        for (size_t i = 0; i < list.size(); ++i)
        {
            total++;
            _consoleWrite("%02d. [%s] %s\n", total, list[i].Group(), list[i].Get());
        }
    }
}

void CFileSystem::SetWarningFunc(void (*pfnWarning)(const char* fmt, ...))
{
    _consoleWrite = pfnWarning;
}

void CFileSystem::SetWarningLevel(FileWarningLevel_t level)
{
    LogFunction("");
}

void CFileSystem::AddLoggingFunc(void (*pfnLogFunc)(const char* fileName, const char* accessType))
{
    LogFunction("");
}

void CFileSystem::RemoveLoggingFunc(FileSystemLoggingFunc_t logFunc)
{
    LogFunction("");
}

fsasync_t CFileSystem::AsyncFilePrefetch(int numFiles, const asyncFileList_t* fileListPtr)
{
    for (int i = 0; i < numFiles; ++i)
    {
        CFile* file = reinterpret_cast<CFile*>(Open(fileListPtr[i].filename, "rb"));

        const asyncFileList_t* hOperation = &fileListPtr[i];

        int readed = 0;

        if (file)
        {
            asyncFileList_t* hNewOperation = new asyncFileList_t(fileListPtr[i]);
            if (hNewOperation)
            {
                int size = static_cast<int>(Size(file));
                size -= hNewOperation->fileStartOffset;
                if (size < 0)
                    size = 0;
                if (size > hNewOperation->maxDataBytes)
                    size = hNewOperation->maxDataBytes;

                hNewOperation->dataPtr = malloc(static_cast<size_t>(size));

                Seek(file, hNewOperation->fileStartOffset, FILESYSTEM_SEEK_HEAD);
                readed = Read(hNewOperation->dataPtr, size, file);

                hOperation = hNewOperation;
            }
            delete file;
        }

        if (hOperation && hOperation->callbackPtr)
        {
            hOperation->callbackPtr(hOperation, readed, FSASYNC_OK);

            if ((hOperation->flags & FSASYNC_FLAGS_FREEDATAPTR) && hOperation->dataPtr)
                free(hOperation->dataPtr);
        }
    }

    return FSASYNC_OK;
}

fsasync_t CFileSystem::AsyncFileFinish(int asyncID, bool wait)
{
    LogFunction("");
    return FSASYNC_OK;
}

fsasync_t CFileSystem::AsyncFileAbort(int asyncID)
{
    LogFunction("");
    return FSASYNC_OK;
}

fsasync_t CFileSystem::AsyncFileStatus(int asyncID)
{
    LogFunction("");
    return FSASYNC_OK;
}

fsasync_t CFileSystem::AsyncFlush()
{
    LogFunction("");
    return FSASYNC_OK;
}


const FileSystemStatistics* CFileSystem::GetFilesystemStatistics()
{
    LogFunction("");
    return NULL;
}


WaitForResourcesHandle_t CFileSystem::WaitForResources(const char* resourcelist)
{
    Log(1, "WaitForResources(%s)", resourcelist);
    return 1;
}

bool CFileSystem::GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress /* out */, bool* complete /* out */)
{
    if (handle < 1)
        return false;

    if (progress)
        *progress = 1.0f;
    if (complete)
        *complete = true;
    return true;
}

void CFileSystem::CancelWaitForResources(WaitForResourcesHandle_t handle)
{
    LogFunction("");
}


int CFileSystem::HintResourceNeed(const char* hintlist, int forgetEverything)
{
    LogFunction("");
    return 0;
}

bool CFileSystem::IsFileImmediatelyAvailable(const char* pFileName)
{
    LogFunction("");
    return true;
}


void CFileSystem::GetLocalCopy(const char* pFileName)
{
}


FileNameHandle_t CFileSystem::FindOrAddFileName(char const* pFileName)
{
    //Log(1, "FindOrAddFileName(%s)", pFileName);
    return new std::string(pFileName);
}

bool CFileSystem::String(const FileNameHandle_t& handle, char* buf, int buflen)
{
    const std::string* path = reinterpret_cast<const std::string*>(handle);
    if (path && (int) path->size() <= buflen)
    {
        strcpy(buf, path->c_str());
        //Log(1, "String() -> %s", buf);
        return true;
    }
    else
    {
        return false;
    }
}


bool CFileSystem::IsOk2(FileHandle_t file)
{
    return IsOk(file);
}

void CFileSystem::RemoveSearchPaths(const char* szPathID)
{
    RemoveSearchPath(NULL, szPathID);
}

bool CFileSystem::IsSteam() const
{
    return false;
}

FilesystemMountRetval_t CFileSystem::MountSteamContent(int nExtraAppId)
{
    LogFunction("");
    return FILESYSTEM_MOUNT_FAILED;
}

const char* CFileSystem::FindFirstEx(const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle)
{
    Log(1, "FindFirstEx(%s, %s)", pWildCard, pPathID);
    if (_finder)
        return _finder->Begin(pWildCard, pPathID, pHandle);

    return NULL;
}

void CFileSystem::MarkPathIDByRequestOnly(const char* pPathID, bool bRequestOnly)
{
    LogFunction(pPathID);
}

fsasync_t CFileSystem::AsyncFileSetPriority(int asyncID, int newPriority)
{
    LogFunction("");
    return FSASYNC_OK;
}

bool CFileSystem::AddPackFile(const char* fullpath, const char* pathID)
{
    char suffix[10];
    if (CPath::GetCompleteSuffix(suffix, sizeof(suffix), fullpath) && _stricmp(suffix, ".bsp") == 0)
    {
        CFile* file = new CFile();
        if (file)
        {
            if (CPath::IsAbsolute(fullpath))
            {
                file->Open(fullpath, IFile::ReadOnly);
            }
            else
            {
                char path[CPATH_LIMIT + 1];

                int offset = 0;
                while (true)
                {
                    if (_paths.Resolve(fullpath, NULL, path, &offset) == 0)
                        break;

                    offset++;

                    if (file->Open(path, IFile::ReadOnly))
                        break;
                }
            }

            if (file->IsOpen() && _map.SetFile(file))
                return true;

            delete file;
        }
    }

    return false;
}

fsasync_t CFileSystem::AsyncFileWrite(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory)
{
    LogFunction("");
    return FSASYNC_OK;
}

fsasync_t CFileSystem::AsyncFileAppendFile(const char* pDestFileName, const char* pSrcFileName)
{
    LogFunction("");
    return FSASYNC_OK;
}

void CFileSystem::AsyncFileFinishAllWrites()
{
    LogFunction("");
}

