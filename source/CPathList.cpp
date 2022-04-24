#include "CPathList.h"


int CPathList::Find(const char* path, const char* group)
{
    size_t length = 0;

    char normalized[CPATH_LIMIT + 1];

    if (path)
        length = strlen(path);

    if (length == 0 || length > CPATH_LIMIT)
        return -1;

    strcpy(normalized, path);

    CPath::Normalize(normalized, length);

    for (size_t i = 0; i < _list.size(); ++i)
    {
        if (_list[i].EqualPath(normalized))
        {
            if (!group || _list[i].EqualGroup(group))
                return static_cast<int>(i);
        }
    }

    return -1;
}

int CPathList::FindGroup(const char* group)
{
    if (!group)
        return -1;

    for (size_t i = 0; i < _list.size(); ++i)
    {
        if (_list[i].EqualGroup(group))
            return static_cast<int>(i);
    }
    return -1;
}

bool CPathList::Append(const char* path, const char* group, bool prepend)
{
    for (size_t i = 0; i < _list.size(); ++i)
    {
        if (_list[i].Equal(path, group))
            return false;
    }

    _list.resize(_list.size() + 1);

    size_t index = _list.size() - 1;

    if (prepend)
    {
        _list.insert(_list.begin(), CPath());
        index = 0;
    }

    _list[index].Set(path, group);

    return true;
}

int CPathList::Remove(const char* path, const char* group)
{
    return 0;
    size_t length = 0;

    char normalized[CPATH_LIMIT + 1];

    if (path)
    {
        length = strlen(path);

        if (length > CPATH_LIMIT)
            length = 0;

        if (length > 0)
        {
            strcpy(normalized, path);
            CPath::Normalize(normalized, length);
        }
    }

    int result = 0;

    std::vector<CPath>::iterator it;
    for (it = _list.begin(); it != _list.end(); )
    {
        if (length > 0 && !(*it).EqualPath(normalized))
        {
            ++it;
            continue;
        }

        if (group && !(*it).EqualGroup(group))
        {
            ++it;
            continue;
        }

        _list.erase(it);
        result++;
    }

    return result;
}

void CPathList::Clear()
{
    _list.clear();
}

size_t CPathList::Resolve(const char* fileName, const char* group, char* path, int* offset)
{
    if (path)
        path[0] = '\0';

    int start = 0;

    if (offset)
        start = *offset;

    if (!fileName || start < 0 || start >= static_cast<int>(_list.size()))
        return 0;

    size_t nameLength = strlen(fileName);

    if (nameLength < 1 || nameLength > CPATH_LIMIT)
        return 0;

    char normalizedName[CPATH_LIMIT + 1];

    strcpy(normalizedName, fileName);
    CPath::Normalize(normalizedName, nameLength);

    char* name = normalizedName;

    if (normalizedName[0] == '\\' && normalizedName[1] == '\\')
    {
        char* newGroup = &normalizedName[2];
        char* rest = strchr(newGroup, '\\');
        if (rest)
        {
            rest[0] = '\0';
            rest++;

            size_t restLength = strlen(rest);
            if (restLength > 0)
            {
                if (!group && strcmp(newGroup, "*") != 0)
                    group = newGroup;

                name = rest;
                nameLength = restLength;
            }
        }
    }

    for (size_t i = static_cast<size_t>(start); i < _list.size(); ++i)
    {
        if (!group || (group && _list[i].EqualGroup(group)))
        {
            if (_list[i].Size() + nameLength > CPATH_LIMIT)
                continue;

            if (path)
            {
                strcpy(path, _list[i].Get());
                strcat(path, name);
            }

            if (offset)
                *offset = static_cast<int>(i);

            return _list[i].Size() + nameLength;
        }
    }

    if (offset)
        *offset = static_cast<int>(_list.size());

    return 0;
}

