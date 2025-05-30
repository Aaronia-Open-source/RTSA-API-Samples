#include "AaroniaRtsaSdkWrapper.h"
#include <iostream>
#include <string>
#include <iomanip>

void TreeConfig(AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper& sdkWrapper, AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config group);

void ItemConfig(AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper& sdkWrapper, AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config config)
{
	AarRtsaSdkWrapper::Wrapper_ConfigInfo	cinfo_wrapper;
	std::string value_str;

	sdkWrapper.ConfigGetInfo(&d, &config, &cinfo_wrapper);
	sdkWrapper.ConfigGetString(&d, &config, value_str);

	std::wcout << prefix 
               << AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper::string_to_wstring(cinfo_wrapper.name) 
               << L"(" << AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper::string_to_wstring(cinfo_wrapper.title) 
               << L", " << AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper::string_to_wstring(cinfo_wrapper.unit) 
               << L", " << AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper::string_to_wstring(cinfo_wrapper.options) 
               << L"), " << L" : \"" << AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper::string_to_wstring(value_str) << "\"" << std::endl;

	if (cinfo_wrapper.type == AARTSAAPI_CONFIG_TYPE_GROUP)
	{
		TreeConfig(sdkWrapper, d, prefix + L". ", config);
	}
}

void TreeConfig(AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper& sdkWrapper, AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config group)
{
	AARTSAAPI_Config	config_item;

	if (sdkWrapper.ConfigFirst(&d, &group, &config_item) == AARTSAAPI_OK)
	{
		do {
			ItemConfig(sdkWrapper, d, prefix, config_item);
		} while (sdkWrapper.ConfigNext(&d, &group, &config_item) == AARTSAAPI_OK);
	}
}



int main() {
    AarRtsaSdkWrapper::AaroniaRtsaSdkWrapper sdkWrapper(CFG_AARONIA_SDK_DIRECTORY);

    if (!sdkWrapper.isSuccessfullyLoaded()) {
        std::cerr << "Failed to load Aaronia RTSA SDK library." << std::endl;
        return -1;
    }
    std::cout << "Aaronia RTSA SDK Library loaded successfully. Version: "
        << sdkWrapper.Version() << std::endl;

    AARTSAAPI_Result res;

    res = sdkWrapper.Init_With_Path(AARTSAAPI_MEMORY_MEDIUM, CFG_AARONIA_XML_LOOKUP_DIRECTORY);
    if (res != AARTSAAPI_OK) {
        std::cerr << "AARTSAAPI_Init_With_Path failed: " << std::hex << res
            << " - " << sdkWrapper.getErrorString(res) << std::endl;
        return -1;
    }
    std::cout << "SDK Initialized." << std::endl;

    AARTSAAPI_Handle SdkHandle;

    res = sdkWrapper.Open(&SdkHandle);
    if (res != AARTSAAPI_OK) {
        std::cerr << "AARTSAAPI_Open failed: " << std::hex << res
            << " - " << sdkWrapper.getErrorString(res) << std::endl;
        sdkWrapper.Shutdown();
        return -1;
    }
    std::cout << "SDK Handle opened." << std::endl;


    res = sdkWrapper.RescanDevices(&SdkHandle, 2000);
    if (res != AARTSAAPI_OK && res != AARTSAAPI_RETRY) { 
        std::cerr << "AARTSAAPI_RescanDevices failed: " << std::hex << res
            << " - " << sdkWrapper.getErrorString(res) << std::endl;
        sdkWrapper.Close(&SdkHandle);
        sdkWrapper.Shutdown();
        return -1;
    }
    std::cout << "Device rescan complete." << std::endl;


    AarRtsaSdkWrapper::Wrapper_DeviceInfo dinfo;
    res = sdkWrapper.EnumDevice(&SdkHandle, "spectranv6", 0, &dinfo);
    if (res != AARTSAAPI_OK) {
        std::cerr << "AARTSAAPI_EnumDevice failed for 'spectranv6': " << std::hex << res
            << " - " << sdkWrapper.getErrorString(res) << std::endl;
        sdkWrapper.Close(&SdkHandle);
        sdkWrapper.Shutdown();
        return -1;
    }

    if (!dinfo.ready) {
        std::cerr << "Device 'spectranv6' with S/N " << dinfo.serialNumber << " is not ready." << std::endl;
        sdkWrapper.Close(&SdkHandle);
        sdkWrapper.Shutdown();
        return -1;
    }
    std::cout << "Found device: S/N " << dinfo.serialNumber
        << ", Ready: " << dinfo.ready
        << ", Active: " << dinfo.active << std::endl;

    AARTSAAPI_Device dhandle; 

    res = sdkWrapper.OpenDevice(&SdkHandle, &dhandle, "spectranv6/raw", dinfo.serialNumber);
    if (res != AARTSAAPI_OK) {
        std::cerr << "AARTSAAPI_OpenDevice failed for S/N " << dinfo.serialNumber
            << ": " << std::hex << res << " - " << sdkWrapper.getErrorString(res) << std::endl;
        sdkWrapper.Close(&SdkHandle);
        sdkWrapper.Shutdown();
        return -1;
    }
    std::cout << "Device S/N " << dinfo.serialNumber << " opened successfully." << std::endl;

    AARTSAAPI_Config root_config;

    std::wcout << "CONFIG:" << std::endl;
    if (sdkWrapper.ConfigRoot(&dhandle, &root_config) == AARTSAAPI_OK)
    {
        TreeConfig(sdkWrapper, dhandle, L"", root_config);
    }
    
    std::wcout << std::endl << "STATUS:" << std::endl;
    if (sdkWrapper.ConfigHealth(&dhandle, &root_config) == AARTSAAPI_OK)
    {
        TreeConfig(sdkWrapper, dhandle, L"", root_config);
    }

    sdkWrapper.CloseDevice(&SdkHandle, &dhandle);
    sdkWrapper.Close(&SdkHandle);
    sdkWrapper.Shutdown();  

    return 0;
}