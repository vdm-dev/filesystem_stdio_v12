#pragma once

#include <vector>

#include "CPath.h"


class CPathList
{
public:
    int Find(const char* path, const char* group = NULL);
    int FindGroup(const char* group);

    bool Append(const char* path, const char* group = NULL, bool prepend = false);

    int Remove(const char* path, const char* group = NULL);

    void Clear();

    size_t Resolve(const char* fileName, const char* group, char* path, int* offset = NULL);

    std::vector<CPath>& List();

private:
    std::vector<CPath> _list;
};


inline std::vector<CPath>& CPathList::List()
{
    return _list;
}
