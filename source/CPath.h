#pragma once


#define CPATH_LIMIT       260
#define CPATH_GROUP_LIMIT 50

class CPath
{
public:
    static bool IsAbsolute(const char* path);
    static void Normalize(char* path);
    static void Normalize(char* path, size_t length);
    static bool IncludeTrailingBackslash(char* path, size_t length);
    static size_t GetCompleteSuffix(char* output, size_t osize, const char* spath);
    static size_t KeepPath(char* path);

public:
    CPath();

    void Set(const char* path, const char* group = 0);

    bool Equal(const char* path, const char* group = 0);
    bool EqualPath(const char* path);
    bool EqualGroup(const char* group);
    bool EqualWeak(const char* path, const char* group = 0);

    const char* Get() const;
    const char* Group() const;
    size_t Size() const;

private:
    size_t _pathSize;
    size_t _groupSize;
    char _path[CPATH_LIMIT + 1];
    char _group[CPATH_GROUP_LIMIT + 1];
};

