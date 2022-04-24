#pragma once


#include "IFile.h"


class CFile : public IFile
{
public:
    CFile();
    virtual ~CFile();

    virtual FileType Type() const;

    virtual bool Open(const char* fileName, unsigned int mode);
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
    void* _handle;
    unsigned int _mode;
    char  _name[261];
};
