#include "CFinder.h"

CFinder::CFinder(std::vector<CPath>& list)
    : _counter(1)
    , _list(list)
{
}

const char* CFinder::Begin(const char* name, const char* group, int* queryIndex)
{
    if (queryIndex)
        *queryIndex = -1;

    if (!name)
        return NULL;

    size_t nameLength = strlen(name);

    if (nameLength < 1 || nameLength > CPATH_LIMIT)
        return 0;

    if (_counter < 0)
        _counter = 1;

    int index = _counter++;

    Query& query = _queries[index];
    query.Clear();

    strcpy(query.name, name);
    CPath::Normalize(query.name, nameLength);

    if (group)
    {
        size_t length = strlen(group);

        if (length > CPATH_GROUP_LIMIT)
            length = CPATH_GROUP_LIMIT;

        strncpy(query.group, group, length);
        query.group[length] = '\0';
        _strupr(query.group);
    }

    for (size_t i = 0; i < _list.size(); ++i)
    {
        if (query.group[0] == '\0' || (query.group[0] != '\0' && _list[i].EqualGroup(query.group)))
        {
            bool found = false;
            for (size_t j = 0; j < query.list.size(); ++j)
            {
                if (query.list[j].EqualPath(_list[i].Get()))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                query.list.push_back(_list[i]);
        }
    }

    char path[CPATH_LIMIT + 1];
    path[CPATH_LIMIT] = '\0';

    while (query.offset < query.list.size())
    {
        if (query.list[query.offset].Size() + nameLength <= CPATH_LIMIT)
        {
            strcpy(path, query.list[query.offset].Get());
            strcat(path, query.name);

            query.handle = FindFirstFileA(path, &query.data);
            if (query.handle != INVALID_HANDLE_VALUE)
                break;
        }
        query.offset++;
    }

    if (query.handle != INVALID_HANDLE_VALUE)
    {
        if (queryIndex)
            *queryIndex = index;

        return query.data.cFileName;
    }

    return NULL;
}

const char* CFinder::Next(int index)
{
    if (index < 0)
        return NULL;

    Query& query = _queries[index];

    if (query.handle != INVALID_HANDLE_VALUE)
    {
        if (FindNextFileA(query.handle, &query.data))
        {
            return query.data.cFileName;
        }
        else
        {
            FindClose(query.handle);
            query.handle = INVALID_HANDLE_VALUE;
            query.offset++;
        }
    }

    size_t nameLength = strlen(query.name);

    char path[CPATH_LIMIT + 1];
    path[CPATH_LIMIT] = '\0';

    while (query.offset < query.list.size())
    {
        if (query.group[0] == '\0' || (query.group[0] != '\0' && query.list[query.offset].EqualGroup(query.group)))
        {
            if (query.list[query.offset].Size() + nameLength <= CPATH_LIMIT)
            {
                strcpy(path, query.list[query.offset].Get());
                strcat(path, query.name);

                query.handle = FindFirstFileA(path, &query.data);
                if (query.handle != INVALID_HANDLE_VALUE)
                    return query.data.cFileName;
            }
        }

        query.offset++;
    }

    return NULL;
}

void CFinder::End(int index)
{
    if (index < 0)
        return;

    Query& query = _queries[index];

    if (query.handle != INVALID_HANDLE_VALUE)
    {
        FindClose(query.handle);
        query.handle = INVALID_HANDLE_VALUE;
    }

    _queries.erase(index);
}

bool CFinder::IsDirectory(int index)
{
    if (index < 0)
        return false;

    Query& query = _queries[index];

    return (query.handle != INVALID_HANDLE_VALUE) && (query.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}
