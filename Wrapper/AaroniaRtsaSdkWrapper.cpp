#include "AaroniaRtsaSdkWrapper.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

namespace AarRtsaSdkWrapper
{

    std::wstring AaroniaRtsaSdkWrapper::string_to_wstring(const std::string& str)
    {
        try
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(str);
        }
        catch (const std::range_error& e)
        {
            std::cerr << "Warning: string_to_wstring conversion failed for: " << str << " Error: " << e.what() << std::endl;
            return L"";
        }
    }

    std::string AaroniaRtsaSdkWrapper::wstring_to_string(const std::wstring& wstr)
    {
        try
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.to_bytes(wstr);
        }
        catch (const std::range_error& e)
        {
            std::cerr << "Warning: wstring_to_string conversion failed. Error: " << e.what() << std::endl;
            return "";
        }
    }

    std::string AaroniaRtsaSdkWrapper::wchar_array_to_string(const wchar_t* wc_array, size_t array_capacity)
    {
        if (!wc_array || array_capacity == 0)
        {
            return "";
        }

        size_t len = 0;
        while (len < array_capacity && wc_array[len] != L'\0')
        {
            len++;
        }

        std::wstring wstr_val(wc_array, len);
        return wstring_to_string(wstr_val);
    }

#if defined(_WIN32)
#include <windows.h>
#include <string>

    // Helper to print errors
    void PrintLastError(const std::string& context)
    {
        DWORD errorCode = GetLastError();
        LPSTR errorMsg = nullptr;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&errorMsg,
            0, NULL);

        std::cerr << context << ". Error code: " << errorCode
            << ". Message: " << (errorMsg ? errorMsg : "unknown") << std::endl;

        if (errorMsg)
            LocalFree(errorMsg);
    }

    LibHandleType LoadLibraryWithDependencies(const std::string& libPathToUse, const std::vector<std::string>& dependencyDirs)
    {
        HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
        auto pSetDefaultDllDirectories = (decltype(SetDefaultDllDirectories)*)GetProcAddress(kernel32, "SetDefaultDllDirectories");
        auto pAddDllDirectory = (decltype(AddDllDirectory)*)GetProcAddress(kernel32, "AddDllDirectory");

        if (!pSetDefaultDllDirectories || !pAddDllDirectory)
        {
            std::cerr << "AddDllDirectory not supported on this system. Requires Windows 7+ with KB2533623." << std::endl;
            return nullptr;
        }

        if (!pSetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS))
        {
            PrintLastError("SetDefaultDllDirectories failed");
            return nullptr;
        }

        std::vector<DLL_DIRECTORY_COOKIE> cookies;
        for (const auto& dir : dependencyDirs)
        {
            DLL_DIRECTORY_COOKIE cookie = pAddDllDirectory(std::wstring(dir.begin(), dir.end()).c_str());
            if (!cookie)
            {
                PrintLastError("AddDllDirectory failed for: " + dir);
                return nullptr;
            }
            cookies.push_back(cookie);
        }

        HMODULE hLib = LoadLibraryExA(libPathToUse.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!hLib)
        {
            PrintLastError("LoadLibraryExA failed for: " + libPathToUse);
        }

        // for (auto cookie : cookies) {
        //     RemoveDllDirectory(cookie);
        // }

        return hLib;
    }
#endif


    AaroniaRtsaSdkWrapper::AaroniaRtsaSdkWrapper(const std::string& libPathOverride)
        : m_libHandle(nullptr), m_loadedSuccessfully(false)
    {
        std::string libPathToUse = libPathOverride;
        if (libPathToUse.empty())
        {
           std::cerr << "Library Path not provided";
        }


#if defined(_WIN32)

        std::string libPath = (std::filesystem::path(CFG_AARONIA_SDK_DIRECTORY) / CFG_CPP_RTSAAPI_DLIB).string();

        std::vector<std::string> depDirs = {
            CFG_AARONIA_RTSA_INSTALL_DIRECTORY
        };

        m_libHandle = LoadLibraryWithDependencies(libPath, depDirs);

#else
        libPathToUse = (std::filesystem::path(CFG_AARONIA_RTSA_INSTALL_DIRECTORY) / CFG_CPP_RTSAAPI_DLIB).string();

        m_libHandle = dlopen(libPathToUse.c_str(), RTLD_LAZY);
        if (!m_libHandle)
        {
            const char* error = dlerror();
            std::cerr << "Failed to load library: " << libPathToUse << ". Error: " << (error ? error : "unknown") << std::endl;
        }
#endif

        if (!m_libHandle)
        {
            return;
        }


        bool allFunctionsLoaded = true;
        m_Init = loadFunction<Ptr_AARTSAAPI_Init>("AARTSAAPI_Init");
        if (!m_Init)
            allFunctionsLoaded = false;

        m_Init_With_Path = loadFunction<Ptr_AARTSAAPI_Init_With_Path>("AARTSAAPI_Init_With_Path");
        if (!m_Init_With_Path)
            allFunctionsLoaded = false;

        m_Shutdown = loadFunction<Ptr_AARTSAAPI_Shutdown>("AARTSAAPI_Shutdown");
        if (!m_Shutdown)
            allFunctionsLoaded = false;

        m_Version = loadFunction<Ptr_AARTSAAPI_Version>("AARTSAAPI_Version");
        if (!m_Version)
            allFunctionsLoaded = false;

        m_Open = loadFunction<Ptr_AARTSAAPI_Open>("AARTSAAPI_Open");
        if (!m_Open)
            allFunctionsLoaded = false;

        m_Close = loadFunction<Ptr_AARTSAAPI_Close>("AARTSAAPI_Close");
        if (!m_Close)
            allFunctionsLoaded = false;

        m_RescanDevices = loadFunction<Ptr_AARTSAAPI_RescanDevices>("AARTSAAPI_RescanDevices");
        if (!m_RescanDevices)
            allFunctionsLoaded = false;

        m_ResetDevices = loadFunction<Ptr_AARTSAAPI_ResetDevices>("AARTSAAPI_ResetDevices");
        if (!m_ResetDevices)
            allFunctionsLoaded = false;

        m_EnumDevice = loadFunction<Ptr_AARTSAAPI_EnumDevice>("AARTSAAPI_EnumDevice");
        if (!m_EnumDevice)
            allFunctionsLoaded = false;

        m_OpenDevice = loadFunction<Ptr_AARTSAAPI_OpenDevice>("AARTSAAPI_OpenDevice");
        if (!m_OpenDevice)
            allFunctionsLoaded = false;

        m_CloseDevice = loadFunction<Ptr_AARTSAAPI_CloseDevice>("AARTSAAPI_CloseDevice");
        if (!m_CloseDevice)
            allFunctionsLoaded = false;

        m_ConnectDevice = loadFunction<Ptr_AARTSAAPI_ConnectDevice>("AARTSAAPI_ConnectDevice");
        if (!m_ConnectDevice)
            allFunctionsLoaded = false;

        m_DisconnectDevice = loadFunction<Ptr_AARTSAAPI_DisconnectDevice>("AARTSAAPI_DisconnectDevice");
        if (!m_DisconnectDevice)
            allFunctionsLoaded = false;

        m_StartDevice = loadFunction<Ptr_AARTSAAPI_StartDevice>("AARTSAAPI_StartDevice");
        if (!m_StartDevice)
            allFunctionsLoaded = false;

        m_StopDevice = loadFunction<Ptr_AARTSAAPI_StopDevice>("AARTSAAPI_StopDevice");
        if (!m_StopDevice)
            allFunctionsLoaded = false;

        m_GetDeviceState = loadFunction<Ptr_AARTSAAPI_GetDeviceState>("AARTSAAPI_GetDeviceState");
        if (!m_GetDeviceState)
            allFunctionsLoaded = false;

        m_AvailPackets = loadFunction<Ptr_AARTSAAPI_AvailPackets>("AARTSAAPI_AvailPackets");
        if (!m_AvailPackets)
            allFunctionsLoaded = false;

        m_GetPacket = loadFunction<Ptr_AARTSAAPI_GetPacket>("AARTSAAPI_GetPacket");
        if (!m_GetPacket)
            allFunctionsLoaded = false;

        m_ConsumePackets = loadFunction<Ptr_AARTSAAPI_ConsumePackets>("AARTSAAPI_ConsumePackets");
        if (!m_ConsumePackets)
            allFunctionsLoaded = false;

        m_GetMasterStreamTime = loadFunction<Ptr_AARTSAAPI_GetMasterStreamTime>("AARTSAAPI_GetMasterStreamTime");
        if (!m_GetMasterStreamTime)
            allFunctionsLoaded = false;

        m_SendPacket = loadFunction<Ptr_AARTSAAPI_SendPacket>("AARTSAAPI_SendPacket");
        if (!m_SendPacket)
            allFunctionsLoaded = false;

        m_ConfigRoot = loadFunction<Ptr_AARTSAAPI_ConfigRoot>("AARTSAAPI_ConfigRoot");
        if (!m_ConfigRoot)
            allFunctionsLoaded = false;

        m_ConfigHealth = loadFunction<Ptr_AARTSAAPI_ConfigHealth>("AARTSAAPI_ConfigHealth");
        if (!m_ConfigHealth)
            allFunctionsLoaded = false;

        m_ConfigFirst = loadFunction<Ptr_AARTSAAPI_ConfigFirst>("AARTSAAPI_ConfigFirst");
        if (!m_ConfigFirst)
            allFunctionsLoaded = false;

        m_ConfigNext = loadFunction<Ptr_AARTSAAPI_ConfigNext>("AARTSAAPI_ConfigNext");
        if (!m_ConfigNext)
            allFunctionsLoaded = false;

        m_ConfigFind = loadFunction<Ptr_AARTSAAPI_ConfigFind>("AARTSAAPI_ConfigFind");
        if (!m_ConfigFind)
            allFunctionsLoaded = false;

        m_ConfigGetName = loadFunction<Ptr_AARTSAAPI_ConfigGetName>("AARTSAAPI_ConfigGetName");
        if (!m_ConfigGetName)
            allFunctionsLoaded = false;

        m_ConfigGetInfo = loadFunction<Ptr_AARTSAAPI_ConfigGetInfo>("AARTSAAPI_ConfigGetInfo");
        if (!m_ConfigGetInfo)
            allFunctionsLoaded = false;

        m_ConfigSetFloat = loadFunction<Ptr_AARTSAAPI_ConfigSetFloat>("AARTSAAPI_ConfigSetFloat");
        if (!m_ConfigSetFloat)
            allFunctionsLoaded = false;

        m_ConfigGetFloat = loadFunction<Ptr_AARTSAAPI_ConfigGetFloat>("AARTSAAPI_ConfigGetFloat");
        if (!m_ConfigGetFloat)
            allFunctionsLoaded = false;

        m_ConfigSetString = loadFunction<Ptr_AARTSAAPI_ConfigSetString>("AARTSAAPI_ConfigSetString");
        if (!m_ConfigSetString)
            allFunctionsLoaded = false;

        m_ConfigGetString = loadFunction<Ptr_AARTSAAPI_ConfigGetString>("AARTSAAPI_ConfigGetString");
        if (!m_ConfigGetString)
            allFunctionsLoaded = false;

        m_ConfigSetInteger = loadFunction<Ptr_AARTSAAPI_ConfigSetInteger>("AARTSAAPI_ConfigSetInteger");
        if (!m_ConfigSetInteger)
            allFunctionsLoaded = false;

        m_ConfigGetInteger = loadFunction<Ptr_AARTSAAPI_ConfigGetInteger>("AARTSAAPI_ConfigGetInteger");
        if (!m_ConfigGetInteger)
            allFunctionsLoaded = false;

        if (!allFunctionsLoaded)
        {
            std::cerr << "Failed to load one or more functions from the library." << std::endl;

#if defined(_WIN32)
            FreeLibrary(m_libHandle);
#else
            dlclose(m_libHandle);
#endif
            m_libHandle = nullptr;
            return;
        }

        m_loadedSuccessfully = true;
        initializeErrorMessages();
    }

    AaroniaRtsaSdkWrapper::~AaroniaRtsaSdkWrapper()
    {
        if (m_libHandle)
        {
#if defined(_WIN32)
            FreeLibrary(m_libHandle);
#else
            dlclose(m_libHandle);
#endif
            m_libHandle = nullptr;
        }
    }

    bool AaroniaRtsaSdkWrapper::isSuccessfullyLoaded() const
    {
        return m_loadedSuccessfully;
    }

    template <typename T>
    T AaroniaRtsaSdkWrapper::loadFunction(const char* funcName)
    {
        if (!m_libHandle)
            return nullptr;
#if defined(_WIN32)
        return reinterpret_cast<T>(GetProcAddress(m_libHandle, funcName));
#else
        return reinterpret_cast<T>(dlsym(m_libHandle, funcName));
#endif
    }

    void AaroniaRtsaSdkWrapper::initializeErrorMessages()
    {
        m_errorMessages[AARTSAAPI_OK] = "OK";
        m_errorMessages[AARTSAAPI_EMPTY] = "Empty";
        m_errorMessages[AARTSAAPI_RETRY] = "Retry";
        m_errorMessages[AARTSAAPI_IDLE] = "Idle";
        m_errorMessages[AARTSAAPI_CONNECTING] = "Connecting";
        m_errorMessages[AARTSAAPI_CONNECTED] = "Connected";
        m_errorMessages[AARTSAAPI_STARTING] = "Starting";
        m_errorMessages[AARTSAAPI_RUNNING] = "Running";
        m_errorMessages[AARTSAAPI_STOPPING] = "Stopping";
        m_errorMessages[AARTSAAPI_DISCONNECTING] = "Disconnecting";
        m_errorMessages[AARTSAAPI_WARNING] = "Warning";
        m_errorMessages[AARTSAAPI_WARNING_VALUE_ADJUSTED] = "Warning: Value adjusted";
        m_errorMessages[AARTSAAPI_WARNING_VALUE_DISABLED] = "Warning: Value disabled";
        m_errorMessages[AARTSAAPI_ERROR] = "Error";
        m_errorMessages[AARTSAAPI_ERROR_NOT_INITIALIZED] = "Error: Not initialized";
        m_errorMessages[AARTSAAPI_ERROR_NOT_FOUND] = "Error: Not found";
        m_errorMessages[AARTSAAPI_ERROR_BUSY] = "Error: Busy";
        m_errorMessages[AARTSAAPI_ERROR_NOT_OPEN] = "Error: Not open";
        m_errorMessages[AARTSAAPI_ERROR_NOT_CONNECTED] = "Error: Not connected";
        m_errorMessages[AARTSAAPI_ERROR_INVALID_CONFIG] = "Error: Invalid configuration";
        m_errorMessages[AARTSAAPI_ERROR_BUFFER_SIZE] = "Error: Buffer size";
        m_errorMessages[AARTSAAPI_ERROR_INVALID_CHANNEL] = "Error: Invalid channel";
        m_errorMessages[AARTSAAPI_ERROR_INVALID_PARAMETR] = "Error: Invalid parameter";
        m_errorMessages[AARTSAAPI_ERROR_INVALID_SIZE] = "Error: Invalid size";
        m_errorMessages[AARTSAAPI_ERROR_MISSING_PATHS_FILE] = "Error: Missing paths file";
        m_errorMessages[AARTSAAPI_ERROR_VALUE_INVALID] = "Error: Value invalid";
        m_errorMessages[AARTSAAPI_ERROR_VALUE_MALFORMED] = "Error: Value malformed";
    }

    std::string AaroniaRtsaSdkWrapper::getErrorString(AARTSAAPI_Result result) const
    {
        auto it = m_errorMessages.find(result);
        if (it != m_errorMessages.end())
        {
            return it->second;
        }
        if ((result & AARTSAAPI_ERROR) == AARTSAAPI_ERROR)
            return "Generic Error (unknown code)";
        if ((result & AARTSAAPI_WARNING) == AARTSAAPI_WARNING)
            return "Generic Warning (unknown code)";

        return "Unknown result code";
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::Init(uint32_t memory)
    {
        if (!m_Init)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_Init(memory);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::Init_With_Path(uint32_t memory, const std::string& pathXmlLocation)
    {
        if (!m_Init_With_Path)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        std::wstring wPath = string_to_wstring(pathXmlLocation);
        return m_Init_With_Path(memory, wPath.c_str());
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::Shutdown(void)
    {
        if (!m_Shutdown)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_Shutdown();
    }

    uint32_t AaroniaRtsaSdkWrapper::Version(void)
    {
        if (!m_Version)
            return 0;
        return m_Version();
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::Open(AARTSAAPI_Handle* handle)
    {
        if (!m_Open)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_Open(handle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::Close(AARTSAAPI_Handle* handle)
    {
        if (!m_Close)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_Close(handle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::RescanDevices(AARTSAAPI_Handle* handle, int timeout)
    {
        if (!m_RescanDevices)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_RescanDevices(handle, timeout);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ResetDevices(AARTSAAPI_Handle* handle)
    {
        if (!m_ResetDevices)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ResetDevices(handle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::EnumDevice(AARTSAAPI_Handle* handle, const std::string& type, int32_t index, Wrapper_DeviceInfo* dinfo_wrapper)
    {
        if (!m_EnumDevice || !dinfo_wrapper)
            return AARTSAAPI_ERROR_INVALID_PARAMETR;

        AARTSAAPI_DeviceInfo orig_dinfo;
        orig_dinfo.cbsize = sizeof(AARTSAAPI_DeviceInfo);

        std::wstring wType = string_to_wstring(type);
        AARTSAAPI_Result res = m_EnumDevice(handle, wType.c_str(), index, &orig_dinfo);

        if (res == AARTSAAPI_OK)
        {
            *dinfo_wrapper = Wrapper_DeviceInfo(orig_dinfo, this);
        }
        return res;
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::OpenDevice(AARTSAAPI_Handle* handle, AARTSAAPI_Device* dhandle, const std::string& type, const std::string& serialNumber)
    {
        if (!m_OpenDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        std::wstring wType = string_to_wstring(type);
        std::wstring wSerial = string_to_wstring(serialNumber);
        return m_OpenDevice(handle, dhandle, wType.c_str(), wSerial.c_str());
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::CloseDevice(AARTSAAPI_Handle* handle, AARTSAAPI_Device* dhandle)
    {
        if (!m_CloseDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_CloseDevice(handle, dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConnectDevice(AARTSAAPI_Device* dhandle)
    {
        if (!m_ConnectDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConnectDevice(dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::DisconnectDevice(AARTSAAPI_Device* dhandle)
    {
        if (!m_DisconnectDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_DisconnectDevice(dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::StartDevice(AARTSAAPI_Device* dhandle)
    {
        if (!m_StartDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_StartDevice(dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::StopDevice(AARTSAAPI_Device* dhandle)
    {
        if (!m_StopDevice)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_StopDevice(dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::GetDeviceState(AARTSAAPI_Device* dhandle)
    {
        if (!m_GetDeviceState)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_GetDeviceState(dhandle);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::AvailPackets(AARTSAAPI_Device* dhandle, int32_t channel, int32_t* num)
    {
        if (!m_AvailPackets)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_AvailPackets(dhandle, channel, num);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::GetPacket(AARTSAAPI_Device* dhandle, int32_t channel, int32_t index, AARTSAAPI_Packet* packet)
    {
        if (!m_GetPacket)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        if (packet)
            packet->cbsize = sizeof(AARTSAAPI_Packet);
        return m_GetPacket(dhandle, channel, index, packet);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConsumePackets(AARTSAAPI_Device* dhandle, int32_t channel, int32_t num)
    {
        if (!m_ConsumePackets)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConsumePackets(dhandle, channel, num);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::GetMasterStreamTime(AARTSAAPI_Device* dhandle, double& stime)
    {
        if (!m_GetMasterStreamTime)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_GetMasterStreamTime(dhandle, &stime);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::SendPacket(AARTSAAPI_Device* dhandle, int32_t channel, const AARTSAAPI_Packet* packet)
    {
        if (!m_SendPacket)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_SendPacket(dhandle, channel, packet);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigRoot(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config)
    {
        if (!m_ConfigRoot)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigRoot(dhandle, config);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigHealth(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config)
    {
        if (!m_ConfigHealth)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigHealth(dhandle, config);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigFirst(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config)
    {
        if (!m_ConfigFirst)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigFirst(dhandle, group, config);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigNext(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config)
    {
        if (!m_ConfigNext)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigNext(dhandle, group, config);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigFind(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config, const std::string& name)
    {
        if (!m_ConfigFind)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        std::wstring wName = string_to_wstring(name);
        return m_ConfigFind(dhandle, group, config, wName.c_str());
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigGetName(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, std::string& name_out)
    {
        if (!m_ConfigGetName)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        wchar_t temp_name_buffer[256];
        AARTSAAPI_Result res = m_ConfigGetName(dhandle, config, temp_name_buffer);

        if (res == AARTSAAPI_OK)
        {
            name_out = wchar_array_to_string(temp_name_buffer, sizeof(temp_name_buffer) / sizeof(wchar_t));
        }
        else
        {
            name_out.clear();
        }
        return res;
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigGetInfo(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, Wrapper_ConfigInfo* cinfo_wrapper)
    {
        if (!m_ConfigGetInfo || !cinfo_wrapper)
            return AARTSAAPI_ERROR_INVALID_PARAMETR;

        AARTSAAPI_ConfigInfo orig_cinfo;
        orig_cinfo.cbsize = sizeof(AARTSAAPI_ConfigInfo);

        AARTSAAPI_Result res = m_ConfigGetInfo(dhandle, config, &orig_cinfo);

        if (res == AARTSAAPI_OK)
        {
            *cinfo_wrapper = Wrapper_ConfigInfo(orig_cinfo, this);
        }
        return res;
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigSetFloat(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, double value)
    {
        if (!m_ConfigSetFloat)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigSetFloat(dhandle, config, value);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigGetFloat(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, double* value)
    {
        if (!m_ConfigGetFloat)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigGetFloat(dhandle, config, value);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigSetString(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, const std::string& value)
    {
        if (!m_ConfigSetString)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        std::wstring wValue = string_to_wstring(value);
        return m_ConfigSetString(dhandle, config, wValue.c_str());
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigGetString(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, std::string& value_out)
    {
        if (!m_ConfigGetString)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;

        value_out.clear();
        int64_t required_wchars = 0;

        AARTSAAPI_Result res = m_ConfigGetString(dhandle, config, nullptr, &required_wchars);

        if (res == AARTSAAPI_OK && required_wchars == 0)
        {
            return AARTSAAPI_OK;
        }

        if (res != AARTSAAPI_ERROR_BUFFER_SIZE && res != AARTSAAPI_OK)
        {
            return res;
        }

        if (required_wchars <= 0)
        {
            return AARTSAAPI_OK;
        }

        std::vector<wchar_t> buffer(static_cast<size_t>(required_wchars) + 1);
        int64_t buffer_capacity_for_api = required_wchars;

        res = m_ConfigGetString(dhandle, config, buffer.data(), &buffer_capacity_for_api);

        if (res == AARTSAAPI_OK)
        {
            value_out = wchar_array_to_string(buffer.data(), static_cast<size_t>(buffer_capacity_for_api));
        }

        return res;
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigSetInteger(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, int64_t value)
    {
        if (!m_ConfigSetInteger)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigSetInteger(dhandle, config, value);
    }

    AARTSAAPI_Result AaroniaRtsaSdkWrapper::ConfigGetInteger(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, int64_t* value)
    {
        if (!m_ConfigGetInteger)
            return AARTSAAPI_ERROR_NOT_INITIALIZED;
        return m_ConfigGetInteger(dhandle, config, value);
    }
}