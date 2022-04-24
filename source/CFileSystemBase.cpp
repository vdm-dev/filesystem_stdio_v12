#include "CFileSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "CFile.h"


int CFileSystem::Read(void* pOutput, int size, FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->Read(pOutput, size);
    return 0;
}

int CFileSystem::Write(void const* pInput, int size, FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->Write(pInput, size);
    return 0;
}

FileHandle_t CFileSystem::Open(const char* pFileName, const char* pOptions, const char* pathID)
{
    unsigned int mode = IFile::NotOpen;

    bool binary = false;

    if (pOptions)
    {
        for (size_t i = 0; i < strlen(pOptions); ++i)
        {
            switch (pOptions[i])
            {
            case 'r':
                mode |= IFile::ReadOnly;
                break;
            case 'w':
                mode |= IFile::WriteOnly;
                break;
            case 'a':
                mode |= IFile::Append;
                break;
            case '+':
                if (mode & IFile::ReadOnly)
                {
                    mode |= IFile::WriteOnly;
                }
                else if (mode & IFile::WriteOnly)
                {
                    mode |= IFile::Truncate;
                }
                break;
            case 'b':
                binary = true;
                break;
            case 't':
                mode |= IFile::Text;
                break;
            default:
                Log(2, "Open(%s, %s, %s) -> WRONG OPTIONS", pFileName, pOptions, pathID);
                break;
            }
        }
    }

    if (binary)
    {
        mode &= ~IFile::Text;
    }
    else
    {
        mode |= IFile::Text;
    }

    IFile* file = NULL;

    if (CPath::IsAbsolute(pFileName))
    {
        file = new CFile();
        if (file)
            file->Open(pFileName, mode);
    }
    else
    {
        if ((mode & (IFile::WriteOnly | IFile::Append | IFile::Truncate)) == 0 && (mode & IFile::ReadOnly) != 0)
            file = _map.OpenInternal(pFileName, pathID, !binary);

        if (!file)
        {
            file = new CFile();

            char path[CPATH_LIMIT + 1];

            int offset = 0;
            while (file)
            {
                if ((mode & (IFile::WriteOnly | IFile::Append)) && !pathID)
                {
                    if (_paths.FindGroup("DEFAULT_WRITE_PATH") > -1)
                        pathID = "DEFAULT_WRITE_PATH";
                }

                if (_paths.Resolve(pFileName, pathID, path, &offset) == 0)
                    break;

                offset++;

                if (file->Open(path, mode))
                {
                    if (mode & (IFile::WriteOnly | IFile::Append))
                        Log(2, "Open(%s, %s, %s) -> %s", pFileName, pOptions, pathID, file->FileName());

                    break;
                }
            }
        }
    }

    if (file && file->IsOpen())
        return file;

    if (file)
        delete file;

    return NULL;
}

void CFileSystem::Close(FileHandle_t file)
{
    if (file)
    {
        IFile* cfile = reinterpret_cast<IFile*>(file);
        delete cfile;
    }
}

void CFileSystem::Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType)
{
    if (file)
        reinterpret_cast<IFile*>(file)->Seek(pos, seekType);
}

unsigned int CFileSystem::Tell(FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->Seek(0, FILESYSTEM_SEEK_CURRENT);
    return 0;
}

unsigned int CFileSystem::Size(FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->Size();
    return 0;
}

unsigned int CFileSystem::Size(const char* pFileName, const char* pPathID)
{
    IFile* file = reinterpret_cast<IFile*>(Open(pFileName, "r", pPathID));
    if (file)
    {
        unsigned int result = file->Size();
        delete file;
        return result;
    }
    return 0;
}

void CFileSystem::Flush(FileHandle_t file)
{
    if (file)
        return reinterpret_cast<IFile*>(file)->Flush();
}

bool CFileSystem::Precache(const char* pFileName, const char* pPathID)
{
    return false;
}

bool CFileSystem::FileExists(const char* pFileName, const char* pPathID)
{
    if (!pFileName)
        return false;

    DWORD attributes = INVALID_FILE_ATTRIBUTES;

    if (CPath::IsAbsolute(pFileName))
    {
        attributes = GetFileAttributesA(pFileName);
    }
    else
    {
        if (_map.FileExists(pFileName, pPathID))
            return true;

        char path[CPATH_LIMIT + 1];

        int offset = 0;
        while (true)
        {
            attributes = INVALID_FILE_ATTRIBUTES;

            if (_paths.Resolve(pFileName, pPathID, path, &offset) == 0)
                break;

            offset++;

            attributes = GetFileAttributesA(path);

            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                break;
        }
    }

    return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

bool CFileSystem::IsFileWritable(char const* pFileName, const char* pPathID)
{
    if (!pFileName)
        return false;

    DWORD attributes = INVALID_FILE_ATTRIBUTES;

    if (CPath::IsAbsolute(pFileName))
    {
        attributes = GetFileAttributesA(pFileName);
    }
    else
    {
        if (_map.FileExists(pFileName, pPathID))
            return true;

        char path[CPATH_LIMIT + 1];

        int offset = 0;
        while (true)
        {
            attributes = INVALID_FILE_ATTRIBUTES;

            if (_paths.Resolve(pFileName, pPathID, path, &offset) == 0)
                break;

            offset++;

            attributes = GetFileAttributesA(path);

            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                break;
        }
    }

    return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 && (attributes & FILE_ATTRIBUTE_READONLY) == 0);
}

bool CFileSystem::SetFileWritable(char const* pFileName, bool writable, const char* pPathID)
{
    if (!pFileName)
        return false;

    DWORD attributes = INVALID_FILE_ATTRIBUTES;

    if (CPath::IsAbsolute(pFileName))
    {
        attributes = GetFileAttributesA(pFileName);
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            if (writable)
            {
                attributes &= ~FILE_ATTRIBUTE_READONLY;
            }
            else
            {
                attributes |= FILE_ATTRIBUTE_READONLY;
            }

            if (SetFileAttributesA(pFileName, attributes))
                return true;
        }
    }
    else
    {
        if (_map.FileExists(pFileName, pPathID))
            return true;

        char path[CPATH_LIMIT + 1];

        int offset = 0;
        while (true)
        {
            attributes = INVALID_FILE_ATTRIBUTES;

            if (_paths.Resolve(pFileName, pPathID, path, &offset) == 0)
                break;

            offset++;

            attributes = GetFileAttributesA(path);
            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                if (writable)
                {
                    attributes &= ~FILE_ATTRIBUTE_READONLY;
                }
                else
                {
                    attributes |= FILE_ATTRIBUTE_READONLY;
                }

                if (SetFileAttributesA(path, attributes))
                    return true;
            }
        }
    }

    return false;
}

long CFileSystem::GetFileTime(const char* pFileName, const char* pPathID)
{
    IFile* file = reinterpret_cast<IFile*>(Open(pFileName, "r", pPathID));
    if (file)
    {
        unsigned __int64 timestamp = 0;
        file->GetFileTime(NULL, NULL, &timestamp);
        delete file;
        return static_cast<long>(timestamp);
    }
    return 0;
}
