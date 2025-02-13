///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(_WIN32)

#pragma warning(disable : 4100)   // unreferenced formal parameter
#pragma warning(disable : 4127)   // constant conditional expression

#include "additional/windows/header.h"
#include <shellapi.h>
#include <string>
#include <vector>

using uIsWow64Process = BOOL (WINAPI *) (HANDLE, PBOOL);


// ****************************************************************
static bool FolderExists(const wchar_t* pcPath)
{
    // check if folder exists
    const DWORD iAttributes = GetFileAttributesW(pcPath);
    return ((iAttributes != INVALID_FILE_ATTRIBUTES) && ((iAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));
}


// ****************************************************************
static bool FolderScan(const wchar_t* pcPath, std::vector<std::wstring>* __restrict pasOutput)
{
    HANDLE pFolder;
    WIN32_FIND_DATAW oFile;

    // open folder
    pFolder = FindFirstFileExW(pcPath, FindExInfoBasic, &oFile, FindExSearchNameMatch, NULL, 0u);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        // check and add file path
        if(oFile.cFileName[0] != '.')
        {
            pasOutput->push_back(oFile.cFileName);
        }
    }
    while(FindNextFileW(pFolder, &oFile));

    // close folder
    FindClose(pFolder);
    return true;
}


// ****************************************************************
static bool IsWow64()
{
    int iStatus = 0;

    // check for pointer-size (compile-time)
    if(sizeof(void*) == 8u)
    {
        return true;
    }

    // get function pointer from kernel library
    const uIsWow64Process nIsWow64Process = reinterpret_cast<uIsWow64Process>(GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));
    if(nIsWow64Process)
    {
        // check for 64-bit operating system
        if(!nIsWow64Process(GetCurrentProcess(), &iStatus))
        {
            iStatus = 0;
        }
    }

    return (iStatus != 0);
}


// ****************************************************************
static bool IsWindows10OrGreater()
{
    // use only major version
    OSVERSIONINFOEXW oVersionInfo = {sizeof(oVersionInfo)};
    oVersionInfo.dwMajorVersion   = 10u;

    // check for Windows 10 or greater
    return (VerifyVersionInfoW(&oVersionInfo, VER_MAJORVERSION, VerSetConditionMask(0u, VER_MAJORVERSION, VER_GREATER_EQUAL)) != FALSE);
}


// ****************************************************************
extern int WINAPI wWinMain(_In_ HINSTANCE pInstance, _In_opt_ HINSTANCE pPrevInstance, _In_ LPWSTR pcCmdLine, _In_ int iCmdShow)
{
    // set working directory
    const wchar_t* pcDirectory = (IsWow64() && IsWindows10OrGreater()) ? L"bin\\windows_x86_64\\" : L"bin\\windows_x86_32\\";
    if(!FolderExists(pcDirectory))
    {
        MessageBoxW(NULL, L"Could not find binary directory!", L"Launcher", MB_OK | MB_ICONERROR);
        return -3;
    }
    if(!SetCurrentDirectoryW(pcDirectory))
    {
        MessageBoxW(NULL, L"Could not set working directory!", L"Launcher", MB_OK | MB_ICONERROR);
        return -2;
    }

    // find executable name
    std::vector<std::wstring> asFile;
    if(!FolderScan(L"*.exe", &asFile) || asFile.empty())
    {
        MessageBoxW(NULL, L"Could not find executable!", L"Launcher", MB_OK | MB_ICONERROR);
        return -1;
    }

    // start real application
    return int(ShellExecuteW(NULL, L"open", asFile[0].c_str(), pcCmdLine, NULL, SW_SHOWNORMAL));
}


#endif /* _WIN32 */