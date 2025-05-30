#ifndef AARONIARTSAWRAPPER_H
#define AARONIARTSAWRAPPER_H

#include "helper.h"
#include <aaroniartsaapi.h>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <locale>
#include <codecvt>

#if defined(_WIN32)
#include <windows.h>
typedef HMODULE LibHandleType;
#else 
#include <dlfcn.h>
typedef void* LibHandleType;
#endif

namespace AarRtsaSdkWrapper
{

    class AaroniaRtsaSdkWrapper;

    struct Wrapper_DeviceInfo
    {
        std::string serialNumber;
        bool ready;
        bool boost;
        bool superspeed;
        bool active;

        Wrapper_DeviceInfo() : ready(false), boost(false), superspeed(false), active(false) {}


        Wrapper_DeviceInfo(const AARTSAAPI_DeviceInfo& orig, AaroniaRtsaSdkWrapper* wrapperInstance);
        Wrapper_DeviceInfo(const Wrapper_DeviceInfo&) = default;
        Wrapper_DeviceInfo& operator=(const Wrapper_DeviceInfo&) = default;
    };

    struct Wrapper_ConfigInfo
    {
        std::string name;
        std::string title;
        AARTSAAPI_ConfigType type;
        double minValue, maxValue, stepValue;
        std::string unit;
        std::string options;
        uint64_t disabledOptions;

        Wrapper_ConfigInfo() : type(AARTSAAPI_CONFIG_TYPE_OTHER), minValue(0), maxValue(0), stepValue(0), disabledOptions(0) {}


        Wrapper_ConfigInfo(const AARTSAAPI_ConfigInfo& orig, AaroniaRtsaSdkWrapper* wrapperInstance);
        Wrapper_ConfigInfo(const Wrapper_ConfigInfo&) = default;
        Wrapper_ConfigInfo& operator=(const Wrapper_ConfigInfo&) = default;
    };

    class AaroniaRtsaSdkWrapper
    {
    public:
        explicit AaroniaRtsaSdkWrapper(const std::string& libPathOverride = "");
        ~AaroniaRtsaSdkWrapper();

        bool isSuccessfullyLoaded() const;

        std::string getErrorString(AARTSAAPI_Result result) const;

        static std::wstring string_to_wstring(const std::string& str);
        static std::string wstring_to_string(const std::wstring& wstr);
        static std::string wchar_array_to_string(const wchar_t* wc_array, size_t array_capacity);

        AARTSAAPI_Result Init(uint32_t memory);
        AARTSAAPI_Result Init_With_Path(uint32_t memory, const std::string& pathXmlLocation);
        AARTSAAPI_Result Shutdown(void);
        uint32_t Version(void);

        AARTSAAPI_Result Open(AARTSAAPI_Handle* handle);
        AARTSAAPI_Result Close(AARTSAAPI_Handle* handle);

        AARTSAAPI_Result RescanDevices(AARTSAAPI_Handle* handle, int timeout);
        AARTSAAPI_Result ResetDevices(AARTSAAPI_Handle* handle);

        AARTSAAPI_Result EnumDevice(AARTSAAPI_Handle* handle, const std::string& type, int32_t index, Wrapper_DeviceInfo* dinfo);
        AARTSAAPI_Result OpenDevice(AARTSAAPI_Handle* handle, AARTSAAPI_Device* dhandle, const std::string& type, const std::string& serialNumber);
        AARTSAAPI_Result CloseDevice(AARTSAAPI_Handle* handle, AARTSAAPI_Device* dhandle);
        AARTSAAPI_Result ConnectDevice(AARTSAAPI_Device* dhandle);
        AARTSAAPI_Result DisconnectDevice(AARTSAAPI_Device* dhandle);
        AARTSAAPI_Result StartDevice(AARTSAAPI_Device* dhandle);
        AARTSAAPI_Result StopDevice(AARTSAAPI_Device* dhandle);
        AARTSAAPI_Result GetDeviceState(AARTSAAPI_Device* dhandle);

        AARTSAAPI_Result AvailPackets(AARTSAAPI_Device* dhandle, int32_t channel, int32_t* num);
        AARTSAAPI_Result GetPacket(AARTSAAPI_Device* dhandle, int32_t channel, int32_t index, AARTSAAPI_Packet* packet);
        AARTSAAPI_Result ConsumePackets(AARTSAAPI_Device* dhandle, int32_t channel, int32_t num);
        AARTSAAPI_Result GetMasterStreamTime(AARTSAAPI_Device* dhandle, double& stime);
        AARTSAAPI_Result SendPacket(AARTSAAPI_Device* dhandle, int32_t channel, const AARTSAAPI_Packet* packet);

        AARTSAAPI_Result ConfigRoot(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config);
        AARTSAAPI_Result ConfigHealth(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config);
        AARTSAAPI_Result ConfigFirst(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config);
        AARTSAAPI_Result ConfigNext(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config);
        AARTSAAPI_Result ConfigFind(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* group, AARTSAAPI_Config* config, const std::string& name);

        AARTSAAPI_Result ConfigGetName(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, std::string& name);
        AARTSAAPI_Result ConfigGetInfo(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, Wrapper_ConfigInfo* cinfo);

        AARTSAAPI_Result ConfigSetFloat(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, double value);
        AARTSAAPI_Result ConfigGetFloat(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, double* value);

        AARTSAAPI_Result ConfigSetString(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, const std::string& value);
        AARTSAAPI_Result ConfigGetString(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, std::string& value);

        AARTSAAPI_Result ConfigSetInteger(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, int64_t value);
        AARTSAAPI_Result ConfigGetInteger(AARTSAAPI_Device* dhandle, AARTSAAPI_Config* config, int64_t* value);

    private:
        LibHandleType m_libHandle;
        bool m_loadedSuccessfully;
        std::map<AARTSAAPI_Result, std::string> m_errorMessages;

        template <typename T>
        T loadFunction(const char* funcName);

        void initializeErrorMessages();

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_Init)(uint32_t);
        Ptr_AARTSAAPI_Init m_Init;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_Init_With_Path)(uint32_t, const wchar_t*);
        Ptr_AARTSAAPI_Init_With_Path m_Init_With_Path;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_Shutdown)(void);
        Ptr_AARTSAAPI_Shutdown m_Shutdown;

        typedef uint32_t(*Ptr_AARTSAAPI_Version)(void);
        Ptr_AARTSAAPI_Version m_Version;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_Open)(AARTSAAPI_Handle*);
        Ptr_AARTSAAPI_Open m_Open;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_Close)(AARTSAAPI_Handle*);
        Ptr_AARTSAAPI_Close m_Close;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_RescanDevices)(AARTSAAPI_Handle*, int);
        Ptr_AARTSAAPI_RescanDevices m_RescanDevices;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ResetDevices)(AARTSAAPI_Handle*);
        Ptr_AARTSAAPI_ResetDevices m_ResetDevices;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_EnumDevice)(AARTSAAPI_Handle*, const wchar_t*, int32_t, AARTSAAPI_DeviceInfo*);
        Ptr_AARTSAAPI_EnumDevice m_EnumDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_OpenDevice)(AARTSAAPI_Handle*, AARTSAAPI_Device*, const wchar_t*, const wchar_t*);
        Ptr_AARTSAAPI_OpenDevice m_OpenDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_CloseDevice)(AARTSAAPI_Handle*, AARTSAAPI_Device*);
        Ptr_AARTSAAPI_CloseDevice m_CloseDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConnectDevice)(AARTSAAPI_Device*);
        Ptr_AARTSAAPI_ConnectDevice m_ConnectDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_DisconnectDevice)(AARTSAAPI_Device*);
        Ptr_AARTSAAPI_DisconnectDevice m_DisconnectDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_StartDevice)(AARTSAAPI_Device*);
        Ptr_AARTSAAPI_StartDevice m_StartDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_StopDevice)(AARTSAAPI_Device*);
        Ptr_AARTSAAPI_StopDevice m_StopDevice;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_GetDeviceState)(AARTSAAPI_Device*);
        Ptr_AARTSAAPI_GetDeviceState m_GetDeviceState;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_AvailPackets)(AARTSAAPI_Device*, int32_t, int32_t*);
        Ptr_AARTSAAPI_AvailPackets m_AvailPackets;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_GetPacket)(AARTSAAPI_Device*, int32_t, int32_t, AARTSAAPI_Packet*);
        Ptr_AARTSAAPI_GetPacket m_GetPacket;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConsumePackets)(AARTSAAPI_Device*, int32_t, int32_t);
        Ptr_AARTSAAPI_ConsumePackets m_ConsumePackets;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_GetMasterStreamTime)(AARTSAAPI_Device*, double*);
        Ptr_AARTSAAPI_GetMasterStreamTime m_GetMasterStreamTime;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_SendPacket)(AARTSAAPI_Device*, int32_t, const AARTSAAPI_Packet*);
        Ptr_AARTSAAPI_SendPacket m_SendPacket;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigRoot)(AARTSAAPI_Device*, AARTSAAPI_Config*);
        Ptr_AARTSAAPI_ConfigRoot m_ConfigRoot;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigHealth)(AARTSAAPI_Device*, AARTSAAPI_Config*);
        Ptr_AARTSAAPI_ConfigHealth m_ConfigHealth;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigFirst)(AARTSAAPI_Device*, AARTSAAPI_Config*, AARTSAAPI_Config*);
        Ptr_AARTSAAPI_ConfigFirst m_ConfigFirst;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigNext)(AARTSAAPI_Device*, AARTSAAPI_Config*, AARTSAAPI_Config*);
        Ptr_AARTSAAPI_ConfigNext m_ConfigNext;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigFind)(AARTSAAPI_Device*, AARTSAAPI_Config*, AARTSAAPI_Config*, const wchar_t*);
        Ptr_AARTSAAPI_ConfigFind m_ConfigFind;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigGetName)(AARTSAAPI_Device*, AARTSAAPI_Config*, wchar_t*);
        Ptr_AARTSAAPI_ConfigGetName m_ConfigGetName;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigGetInfo)(AARTSAAPI_Device*, AARTSAAPI_Config*, AARTSAAPI_ConfigInfo*);
        Ptr_AARTSAAPI_ConfigGetInfo m_ConfigGetInfo;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigSetFloat)(AARTSAAPI_Device*, AARTSAAPI_Config*, double);
        Ptr_AARTSAAPI_ConfigSetFloat m_ConfigSetFloat;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigGetFloat)(AARTSAAPI_Device*, AARTSAAPI_Config*, double*);
        Ptr_AARTSAAPI_ConfigGetFloat m_ConfigGetFloat;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigSetString)(AARTSAAPI_Device*, AARTSAAPI_Config*, const wchar_t*);
        Ptr_AARTSAAPI_ConfigSetString m_ConfigSetString;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigGetString)(AARTSAAPI_Device*, AARTSAAPI_Config*, wchar_t*, int64_t*);
        Ptr_AARTSAAPI_ConfigGetString m_ConfigGetString;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigSetInteger)(AARTSAAPI_Device*, AARTSAAPI_Config*, int64_t);
        Ptr_AARTSAAPI_ConfigSetInteger m_ConfigSetInteger;

        typedef AARTSAAPI_Result(*Ptr_AARTSAAPI_ConfigGetInteger)(AARTSAAPI_Device*, AARTSAAPI_Config*, int64_t*);
        Ptr_AARTSAAPI_ConfigGetInteger m_ConfigGetInteger;
    };

    inline Wrapper_DeviceInfo::Wrapper_DeviceInfo(const AARTSAAPI_DeviceInfo& orig, AaroniaRtsaSdkWrapper* wrapperInstance)
    {
        if (!wrapperInstance)
            throw std::invalid_argument("Wrapper instance cannot be null for conversion.");
        serialNumber = AaroniaRtsaSdkWrapper::wchar_array_to_string(orig.serialNumber, sizeof(orig.serialNumber) / sizeof(wchar_t));
        ready = orig.ready;
        boost = orig.boost;
        superspeed = orig.superspeed;
        active = orig.active;
    }

    inline Wrapper_ConfigInfo::Wrapper_ConfigInfo(const AARTSAAPI_ConfigInfo& orig, AaroniaRtsaSdkWrapper* wrapperInstance)
    {
        if (!wrapperInstance)
            throw std::invalid_argument("Wrapper instance cannot be null for conversion.");
        name = AaroniaRtsaSdkWrapper::wchar_array_to_string(orig.name, sizeof(orig.name) / sizeof(wchar_t));
        title = AaroniaRtsaSdkWrapper::wchar_array_to_string(orig.title, sizeof(orig.title) / sizeof(wchar_t));
        type = orig.type;
        minValue = orig.minValue;
        maxValue = orig.maxValue;
        stepValue = orig.stepValue;
        unit = AaroniaRtsaSdkWrapper::wchar_array_to_string(orig.unit, sizeof(orig.unit) / sizeof(wchar_t));
        options = AaroniaRtsaSdkWrapper::wchar_array_to_string(orig.options, sizeof(orig.options) / sizeof(wchar_t));
        disabledOptions = orig.disabledOptions;
    }
}

#endif