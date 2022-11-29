#include "AARTSAAPIWrapper.h"

#include <cstring>
#include <exception>
#include <fmt/core.h>
#include <iostream>

namespace AARTSAAPI
{

static const char *const RETURN_MESSAGES[] = {
    "Operation performed successfully.",
    "No data available.",
    "Retry operation!" };

static const char *const STATUS_NAMES[] = {
    "Status: Idle",
    "Status: Connecting",
    "Status: Connected",
    "Status: Starting",
    "Status: Running",
    "Status: Stopping",
    "Status: Disconnecting" };

static const char *const WARNING_MESSAGES[] = {
    "Warning: Value adjusted!",
    "Warning: Value disabled!" };

static const char *const ERROR_MESSAGES[] = {
    "Error: Target not initialized!",
    "Error: Target not found!",
    "Error: Target busy!",
    "Error: Target not open!",
    "Error: Target not connected!",
    "Error: Invalid configuration!",
    "Error: Bad buffer size!",
    "Error: Invalid channel selected!",
    "Error: Invalid parameter!",
    "Error: Invalid size!",
    "Error: Missing paths.xml file!",
    "Error: Value is invalid!",
    "Error: Value is malformed!" };

const char *ResultToString( AARTSAAPI_Result result )
{
    const char *const *target;
    size_t len;
    if ( ( result & AARTSAAPI_OK ) == AARTSAAPI_OK )
    {
        target = RETURN_MESSAGES;
        len = sizeof( RETURN_MESSAGES ) / sizeof( *RETURN_MESSAGES );
    }
    else if ( ( result & AARTSAAPI_IDLE ) == AARTSAAPI_OK )
    {
        target = STATUS_NAMES;
        len = sizeof( STATUS_NAMES ) / sizeof( *STATUS_NAMES );
    }
    else if ( ( result & AARTSAAPI_WARNING ) == AARTSAAPI_OK )
    {
        target = WARNING_MESSAGES;
        len = sizeof( WARNING_MESSAGES ) / sizeof( *WARNING_MESSAGES );
    }
    else if ( ( result & AARTSAAPI_ERROR ) == AARTSAAPI_OK )
    {
        target = ERROR_MESSAGES;
        len = sizeof( ERROR_MESSAGES ) / sizeof( *ERROR_MESSAGES );
    }

    if ( ( result & 0xFF ) >= len )
        return "Internal error: There is no translation for this error code available!";

    return target[result & 0xFF];
}

RTSAWrapper::RTSAWrapper( const MemoryMode memoryMode )
{
    AARTSAAPI_Result res = AARTSAAPI_Init( static_cast<uint32_t>( memoryMode ) );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to initialize RTSAAPI: {}", ResultToString( res ) ) );

    res = AARTSAAPI_Open( &mAPIHandle );
    if ( res != AARTSAAPI_OK )
    {
        AARTSAAPI_Shutdown();
        throw std::runtime_error( fmt::format( "Failed to open AARTSAAPI library handle: {}", ResultToString( res ) ) );
    }
}

RTSAWrapper::~RTSAWrapper()
{
    AARTSAAPI_Close( &mAPIHandle );
    AARTSAAPI_Shutdown();
}

static const wchar_t *const DEVICE_TYPE_IDs[] = {
    L"spectranv6" };

std::vector<DeviceWrapper> RTSAWrapper::getAllDevices( const DeviceType deviceType, bool rescan, int timeout )
{
    std::vector<DeviceWrapper> devices;

    AARTSAAPI_Result res;
    while ( rescan )
    {
        res = AARTSAAPI_RescanDevices( &mAPIHandle, timeout );
        if ( res == AARTSAAPI_RETRY )
            continue;

        if ( res != AARTSAAPI_OK )
            throw std::runtime_error( fmt::format( "Failed to scan for devices: {}", ResultToString( res ) ) );

        AARTSAAPI_DeviceInfo dinfo{};
        dinfo.cbsize = sizeof( AARTSAAPI_DeviceInfo );

        for ( int i = 0; AARTSAAPI_EnumDevice( &mAPIHandle, DEVICE_TYPE_IDs[static_cast<int>( deviceType )], i, &dinfo ) == AARTSAAPI_OK; i++ )
            devices.emplace_back( this->shared_from_this(), dinfo );

        break;
    }

    return devices;
}

}; // namespace AARTSAAPI
