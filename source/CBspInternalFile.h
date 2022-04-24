#pragma once

#include "CFile.h"


class CBspInternalFile : public IFile
{
public:
    CBspInternalFile();
    virtual ~CBspInternalFile();

    virtual FileType Type();

    virtual bool Open(const char* fileName, unsigned int mode);
    virtual bool Open(const char* fileName, unsigned int hash, unsigned int offset, unsigned int size, const char* bspName, bool text = false);
    virtual void Close();

    virtual const char* FileName() const;
    virtual bool IsOpen() const;

    virtual unsigned long Seek(long position, FileSystemSeek_t origin);
    virtual unsigned long Size();
    virtual void Flush();

    virtual unsigned long Read(void* data, unsigned long size);
    virtual unsigned long Write(const void* data, unsigned long size);

    virtual bool GetFileTime(unsigned __int64* creationTime, unsigned __int64* lastAccessTime, unsigned __int64* lastWriteTime);

private:
    CFile _bsp;
    char _name[260];
    unsigned int _hash;
    unsigned int _offset;
    unsigned int _size;
};

