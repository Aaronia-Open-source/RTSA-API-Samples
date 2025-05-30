#include "helper.h" 
#include <iostream>
#include <string> 

#ifndef HELPER_H 
#define HELPER_H
#ifdef _WIN32
#include <windows.h>
#endif
#endif

int LoadRTSAAPI_with_searchpath() {
#if !defined(RTSA_BUILDAPP_INTERNAL_SDK)

#ifdef _WIN32
    HMODULE mod = NULL;
    DLL_DIRECTORY_COOKIE cookie_install = NULL;

    cookie_install = ::AddDllDirectory(CFG_AARONIA_RTSA_INSTALL_DIRECTORY);
    if (!cookie_install) {
        DWORD error = GetLastError();
        wchar_t errBuf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       errBuf, (sizeof(errBuf) / sizeof(wchar_t)), NULL);

        std::wcerr << L"AddDllDirectory failed for install directory: " << CFG_AARONIA_RTSA_INSTALL_DIRECTORY
                   << L". Error " << error << L": " << errBuf << std::endl;
    }

    std::wstring fullDllPath = CFG_AARONIA_SDK_DIRECTORY;
    if (!fullDllPath.empty() && fullDllPath.back() != L'\\') {
        fullDllPath += L'\\';
    }
    fullDllPath += CFG_CPP_RTSAAPI_DLIB;
    
    DWORD loadFlags = LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
    if (cookie_install) {
        loadFlags |= LOAD_LIBRARY_SEARCH_USER_DIRS;
    } else {
        std::wcerr << L"Warning: Install directory was not added to dependency search path (AddDllDirectory failed)." << std::endl;
    }
    
    mod = ::LoadLibraryExW(fullDllPath.c_str(), NULL, loadFlags);

    if (!mod) {
        DWORD error = GetLastError();
        wchar_t errBuf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       errBuf, (sizeof(errBuf) / sizeof(wchar_t)), NULL);
        std::wcerr << L"LoadLibraryExW (full path) failed for " << fullDllPath
                   << L". Error " << error << L": " << errBuf << std::endl;
    } else {
        //std::wcout << L"LoadLibraryExW (full path) SUCCEEDED." << std::endl;
    }

    if (cookie_install) {
        ::RemoveDllDirectory(cookie_install);
    }

    if (!mod) {       
        return -1;
    }

    return 0; 
#else
    return 0; 
#endif // windows



#else
    return 0; 
#endif //RTSA_BUILDAPP_INTERNAL_SDK
}