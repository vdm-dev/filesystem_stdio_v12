#include "CPath.h"


#include <cstring>
#include <cctype>

#include "ExLib.h"


bool CPath::IsAbsolute(const char* path)
{
    return strstr(path, ":\\") || strstr(path, ":/");
}

void CPath::Normalize(char* path)
{
    Normalize(path, strlen(path));
}

void CPath::Normalize(char* path, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        if (path[i] == '/')
        {
            path[i] = '\\';
        }
        else
        {
            path[i] = static_cast<char>(tolower(path[i]));
        }
    }
}

bool CPath::IncludeTrailingBackslash(char* path, size_t length)
{
    if (length > 0 && path[length - 1] != '\\')
    {
        if (path[length - 1] == '/')
        {
            path[length - 1] = '\\';
            return true;
        }

        if ((length + 1) > CPATH_LIMIT)
            return false;

        path[length++] = '\\';
        path[length] = '\0';
    }

    return true;
}

size_t CPath::GetCompleteSuffix(char* output, size_t osize, const char* spath)
{
    size_t dot = 0;
    size_t size = ex_strlen(spath);
    while (size > 0)
    {
        size--;
        if (spath[size] == '.')
        {
            dot = size;
        }
        else if (spath[size] == '\\' || spath[size] == '/')
        {
            size++;
            break;
        }
    }

    if (dot >= size)
    {
        exs_strcpy(output, osize, &spath[dot]);
        return ex_strlen(output);
    }

    if (output != NULL && osize > 0)
        output[0] = '\0';

    return 0;
}

size_t CPath::KeepPath(char* path)
{
    size_t size = ex_strlen(path);
    while (size > 0)
    {
        size--;
        if (path[size] == '\\' || path[size] == '/')
        {
            size++;
            break;
        }
    }
    if (path)
        path[size] = '\0';

    return size;
}

CPath::CPath()
    : _pathSize(0)
    , _groupSize(0)
{
    _path[0] = '\0';
    _path[sizeof(_path) - 1] = '\0';

    _group[0] = '\0';
    _group[sizeof(_group) - 1] = '\0';
}

void CPath::Set(const char* path, const char* group)
{
    if (path)
    {
        size_t length = strlen(path);

        if (length > CPATH_LIMIT)
            length = CPATH_LIMIT;

        strncpy(_path, path, length);
        _path[length] = '\0';
        _pathSize = length;
    }
    else
    {
        _path[0] = '\0';
        _pathSize = 0;
    }

    if (group)
    {
        size_t length = strlen(group);

        if (length > CPATH_GROUP_LIMIT)
            length = CPATH_GROUP_LIMIT;

        strncpy(_group, group, length);
        _group[length] = '\0';
        _strlwr(_group);
        _groupSize = length;
    }
    else
    {
        _group[0] = '\0';
        _groupSize = 0;
    }
}

bool CPath::Equal(const char* path, const char* group)
{
    return EqualGroup(group) && EqualPath(path);
}

bool CPath::EqualPath(const char* path)
{
    if (!path)
        return false;

    size_t leftLength = _pathSize;
    size_t rightLength = strlen(path);

    if (leftLength > 0 && _path[leftLength - 1] == '\\')
        leftLength--;

    if (rightLength > 0 && _path[rightLength - 1] == '\\')
        rightLength--;

    if (leftLength != rightLength)
        return false;

    return strncmp(_path, path, rightLength) == 0;
}

bool CPath::EqualGroup(const char* group)
{
    if (!group)
        return false;

    return _stricmp(_group, group) == 0;
}

bool CPath::EqualWeak(const char* path, const char* group)
{
    if (group && _stricmp(_group, group) != 0)
        return false;

    return EqualPath(path);
}

const char* CPath::Get() const
{
    return _path;
}

const char* CPath::Group() const
{
    return _group;
}

size_t CPath::Size() const
{
    return _pathSize;
}
