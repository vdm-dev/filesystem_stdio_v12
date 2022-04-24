#pragma once

#include <map>

#include "CFile.h"


class CBspArchive
{
public:
    struct InternalFile
    {
        char name[260];
        unsigned int hash;
        unsigned int offset;
        unsigned int size;
    };

public:
    CBspArchive();
    ~CBspArchive();


    bool IsOpen() const;
    bool SetFile(CFile* file);
    void Clear();

    const char* FileName() const;

    bool FileExists(const char* fileName, const char* group) const;
    IFile* OpenInternal(const char* fileName, const char* group, bool text = false) const;

private:
    CFile* _file;
    std::multimap<unsigned int, InternalFile> _table;
};

