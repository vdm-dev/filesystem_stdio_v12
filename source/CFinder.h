#pragma once

#include <string.h>

#include <map>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "CPath.h"

struct Query
{
    Query()
    {
        Clear();
    }

    void Clear()
    {
        list.clear();
        memset(name, 0, sizeof(name));
        memset(group, 0, sizeof(group));
        memset(&data, 0, sizeof(data));
        offset = 0;
        handle = INVALID_HANDLE_VALUE;
    }

    std::vector<CPath> list;
    char name[CPATH_LIMIT + 1];
    char group[CPATH_GROUP_LIMIT + 1];
    size_t offset;
    WIN32_FIND_DATAA data;
    HANDLE handle;
};

class CFinder
{
public:
    CFinder(std::vector<CPath>& list);

    const char* Begin(const char* name, const char* group, int* queryIndex);
    const char* Next(int index);
    void End(int index);
    bool IsDirectory(int index);


private:
    int _counter;
    std::vector<CPath>& _list;
    std::map<int, Query> _queries;
};

