#include "CFileSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


bool CFileSystem::Connect(CreateInterfaceFn factory)
{
    return true;
}

void CFileSystem::Disconnect()
{
}

void* CFileSystem::QueryInterface(const char* pInterfaceName)
{
    LogFunction("");
    return NULL;
}

InitReturnVal_t CFileSystem::Init()
{
    char applicationPath[MAX_PATH + 1];
    applicationPath[MAX_PATH] = '\0';

    DWORD length = GetModuleFileNameA(NULL, applicationPath, MAX_PATH);
    for (DWORD i = 0; i < length; ++i)
    {
        if (applicationPath[length - i - 1] == '\\' || applicationPath[length - i - 1] == '/')
        {
            applicationPath[length - i] = '\0';
            break;
        }
    }

    AddSearchPath(applicationPath, "EXECUTABLE_PATH");

    return INIT_OK;
}

void CFileSystem::Shutdown()
{
}