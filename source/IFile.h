#pragma once


#include <filesystem.h>


class IFile
{
public:
    enum OpenMode
    {
        NotOpen = 0,
        ReadOnly = 1,
        WriteOnly = 2,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 4,
        Truncate = 8,
        Text = 16,
        Unbuffered = 32,
        NewOnly = 64,
        ExistingOnly = 128
    };

    enum FileType
    {
        Unknown     = 0,
        FileSystem  = 1,
        BspInternal = 2
    };

public:
    IFile() { };
    virtual ~IFile() { };

    virtual FileType Type() const { return Unknown; }

    virtual bool Open(const char* fileName, unsigned int mode) = 0;
    virtual void Close() = 0;

    virtual const char* FileName() const = 0;
    virtual bool IsOpen() const = 0;

    virtual unsigned long Seek(long position, FileSystemSeek_t origin) = 0;
    virtual unsigned long Size() = 0;
    virtual void Flush() = 0;

    virtual unsigned long Read(void* data, unsigned long size) = 0;
    virtual unsigned long Write(const void* data, unsigned long size) = 0;

    virtual bool GetFileTime(unsigned __int64* creationTime, unsigned __int64* lastAccessTime, unsigned __int64* lastWriteTime) = 0;
};