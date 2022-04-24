#include "CFile.h"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


CFile::CFile()
    : _handle(INVALID_HANDLE_VALUE)
    , _mode(NotOpen)
{
    _name[0] = '\0';
    _name[ARRAYSIZE(_name) - 1] = '\0';
}

CFile::~CFile()
{
    Close();
}

IFile::FileType CFile::Type() const
{
    return IFile::FileSystem;
}

bool CFile::Open(const char* fileName, unsigned int mode)
{
    if (_handle != INVALID_HANDLE_VALUE)
        return false;

    size_t length = strlen(fileName);
    if (length > MAX_PATH)
        return false;

    if (mode & (Append | NewOnly))
        mode |= WriteOnly;

    if ((mode & (ReadOnly | WriteOnly)) == 0)
        return false;

    DWORD dwDesiredAccess = 0;

    if (mode & ReadOnly)
        dwDesiredAccess |= GENERIC_READ;
    if (mode & WriteOnly)
        dwDesiredAccess |= GENERIC_WRITE;

    DWORD dwCreationDisposition = 0;

    if (mode & NewOnly)
    {
        dwCreationDisposition = CREATE_NEW;
    }
    else
    {
        if ((mode & WriteOnly) && !(mode & ExistingOnly))
        {
            dwCreationDisposition = OPEN_ALWAYS;
        }
        else
        {
            dwCreationDisposition = OPEN_EXISTING;
        }
    }


    _handle = CreateFileA(
        fileName,
        dwDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (_handle == INVALID_HANDLE_VALUE)
        return false;

    _mode = mode;
    memcpy(_name, fileName, length);
    _name[length] = '\0';

    if (mode & Truncate)
    {
        SetFilePointer(_handle, 0, NULL, FILE_BEGIN);
        SetEndOfFile(_handle);
    }

    if (mode & Append)
    {
        SetFilePointer(_handle, 0, NULL, FILE_END);
    }

    return true;
}

void CFile::Close()
{
    CloseHandle(_handle);
    _handle = INVALID_HANDLE_VALUE;
    _mode = NotOpen;
    _name[0] = '\0';
}

const char* CFile::FileName() const
{
    return _name;
}

bool CFile::IsOpen() const
{
    return _handle != INVALID_HANDLE_VALUE;
}

unsigned long CFile::Seek(long position, FileSystemSeek_t origin)
{
    DWORD result = SetFilePointer(_handle, position, NULL, static_cast<DWORD>(origin));
    if (result == INVALID_SET_FILE_POINTER)
        return 0;

    return result;
}

unsigned long CFile::Size()
{
    LARGE_INTEGER result;
    memset(&result, 0, sizeof(result));
    GetFileSizeEx(_handle, &result);
    return result.LowPart;
}

void CFile::Flush()
{
    FlushFileBuffers(_handle);
}

unsigned long CFile::Read(void* data, unsigned long size)
{
    DWORD result = 0;

    if (_mode & IFile::Text)
    {
        unsigned long offset = Seek(0, FILESYSTEM_SEEK_CURRENT);
        char* text = static_cast<char*>(data);

        while (size > 0)
        {
            Seek(offset, FILESYSTEM_SEEK_HEAD);

            DWORD subresult = 0;
            if (!ReadFile(_handle, text, size, &subresult, NULL) || subresult == 0)
                return result;

            unsigned long i;
            for (i = 0; i < subresult; ++i)
            {
                if (i > 0 && text[i] == '\n' && text[i - 1] == '\r')
                {
                    text[i - 1] = '\n';
                    text[i] = '\0';
                    break;
                }
            }

            size = (i < subresult) ? (size - i) : 0;
            result += i;
            text += i;
            offset += i + 1;
        }
    }
    else
    {
        if (!ReadFile(_handle, data, size, &result, NULL))
            return 0;
    }

    return result;
}

unsigned long CFile::Write(const void* data, unsigned long size)
{
    DWORD result = 0;

    if (_mode & IFile::Text)
    {
        const char* text = static_cast<const char*>(data);

        while (size > 0)
        {
            unsigned long i;
            for (i = 0; i < size; ++i)
            {
                if (text[i] == '\n')
                    break;
            }

            DWORD subresult = 0;
            if (!WriteFile(_handle, text, i, &subresult, NULL))
                return result;

            result += subresult;

            if (i < size)
            {
                size -= (i + 1);
                text += (i + 1);

                if (!WriteFile(_handle, "\r\n", 2, &subresult, NULL))
                    return result;

                result += subresult;
            }
            else
            {
                size -= i;
                text += i;
            }

        }
    }
    else
    {
        if (!WriteFile(_handle, data, size, &result, NULL))
            return 0;
    }


    return result;
}

bool CFile::GetFileTime(unsigned __int64* creationTime, unsigned __int64* lastAccessTime, unsigned __int64* lastWriteTime)
{
    FILETIME creation;
    FILETIME access;
    FILETIME write;

    if (::GetFileTime(_handle, &creation, &access, &write))
    {
        if (creationTime)
            *creationTime = (((unsigned __int64) creation.dwHighDateTime << 4) | creation.dwLowDateTime) / 10000000ULL - 11644473600ULL;

        if (lastAccessTime)
            *lastAccessTime = (((unsigned __int64) access.dwHighDateTime << 4) | access.dwLowDateTime) / 10000000ULL - 11644473600ULL;

        if (lastWriteTime)
            *lastWriteTime = (((unsigned __int64) write.dwHighDateTime << 4) | write.dwLowDateTime) / 10000000ULL - 11644473600ULL;

        return true;
    }

    return false;
}
