#include "CFileSystem.h"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


void CFileSystem::Log(int severity, const char* format, ...) const
{
    if (severity < _severity)
        return;

    va_list args;
    va_start(args, format);
    if (format)
    {
        const size_t limit = 4096;
        char text[limit];

        _vsnprintf(text, limit - 1, format, args);
        text[limit - 1] = '\0';

        if (text[0] != '\0')
            strcat(text, "\n");
            OutputDebugStringA(text);
    }
    va_end(args);
}

void CFileSystem::Log(int severity, int line, const char* fileName, const char* function, const char* message) const
{
    if (severity < _severity)
        return;

    const size_t limit = 4096;
    char text[limit];

    text[0] = '\0';

    if (fileName)
    {
        size_t offset = 0;
        size_t length = strlen(fileName);
        for (size_t i = 0; i < length; ++i)
        {
            if (fileName[length - i - 1] == '\\' || fileName[length - i - 1] == '/')
            {
                offset = length - i;
                break;
            }
        }
        if (length - offset > 0)
        {
            strcat(text, &fileName[offset]);
        }
        else
        {
            strcat(text, "(none)");
        }
    }

    strcat(text, ":");
    _itoa(line, &text[strlen(text)], 10);
    strcat(text, " ");

    if (function)
    {
        strcat(text, "<");
        strcat(text, function);
        strcat(text, "> ");
    }

    if (message)
        strcat(text, message);

    strcat(text, "\n");
    OutputDebugStringA(text);
}
