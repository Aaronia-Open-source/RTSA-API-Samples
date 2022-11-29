#include "AARTSAAPIWrapper.h"

#include <fmt/core.h>
#include <fmt/xchar.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <thread>

#ifndef NDEBUG
#define LOG_DEBUG( f, ... ) fmt::print( stderr, f, ##__VA_ARGS__ )
#else
#define LOG_DEBUG( f, ... )
#endif

namespace AARTSAAPI
{

namespace
{

std::string WStringToString( const std::wstring &in )
{
    return std::string( in.cbegin(), in.cend() );
}

std::wstring StringToWString( const std::string &in )
{
    return std::wstring( in.cbegin(), in.cend() );
}

}; // namespace

static const char *const RETURN_MESSAGES[] = {
    "Operation performed successfully.",
    "No data available.",
    "Retry operation!" };

static const char *const STATUS_NAMES[] = {
    "Idle",
    "Connecting",
    "Connected",
    "Starting",
    "Running",
    "Stopping",
    "Disconnecting" };

static const char *const WARNING_MESSAGES[] = {
    "Warning:",
    "Warning: Value adjusted!",
    "Warning: Value disabled!" };

static const char *const ERROR_MESSAGES[] = {
    "Error: ?",
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

static const wchar_t *const DEVICE_TYPE_IDs[] = {
    L"spectranv6" };

static const wchar_t *const DEVICE_MODE_IDs[] = {
    L"raw",
    L"rtsa",
    L"sweepsa",
    L"iqreceiver",
    L"iqtransmitter",
    L"iqtransceiver" };

static const char *const TYPE_NAMES[] = {
    "Other",
    "Group",
    "blob",
    "Number",
    "Bool",
    "Enum",
    "String" };

const char *ResultToString( AARTSAAPI_Result result )
{
    const char *const *target;
    size_t len = 0;

    if ( ( result & AARTSAAPI_IDLE ) == AARTSAAPI_IDLE )
    {
        target = STATUS_NAMES;
        len = sizeof( STATUS_NAMES ) / sizeof( *STATUS_NAMES );
    }
    else if ( ( result & AARTSAAPI_WARNING ) == AARTSAAPI_WARNING )
    {
        target = WARNING_MESSAGES;
        len = sizeof( WARNING_MESSAGES ) / sizeof( *WARNING_MESSAGES );
    }
    else if ( ( result & AARTSAAPI_ERROR ) == AARTSAAPI_ERROR )
    {
        target = ERROR_MESSAGES;
        len = sizeof( ERROR_MESSAGES ) / sizeof( *ERROR_MESSAGES );
    }
    else
    {
        target = RETURN_MESSAGES;
        len = sizeof( RETURN_MESSAGES ) / sizeof( *RETURN_MESSAGES );
    }

    if ( ( result & 0xFF ) >= len )
        return "INTERNAL ERROR: There is no translation available for this error code!";

    return target[result & 0xFF];
}

ConfigNode::ConfigNode( std::shared_ptr<DeviceWrapper> device, AARTSAAPI_Config config, const std::string &path )
    : mDevice( device ),
      mConfigNode( config ),
      mPath( path )
{
    auto &info = mDevice->mConfigInfo;
    AARTSAAPI_Result res = AARTSAAPI_ConfigGetInfo( &mDevice->mDeviceHandle, &mConfigNode, &info );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to obtain information about config item \"{}\": {}", mName, ResultToString( res ) ) );

    mNameW = std::wstring( info.name );
    mName = WStringToString( mNameW );
    mFullName = path + mName;

    mTitleW = std::wstring( info.title );
    mTitle = WStringToString( mTitleW );

    mUnitW = std::wstring( info.unit );
    mUnit = WStringToString( mUnitW );

    mOptionsW = std::wstring( info.options );
    mOptions = WStringToString( mOptionsW );

    mDisabledOptions = info.disabledOptions;

    mType = static_cast<ConfigNodeType>( info.type );

    mValueMax = info.maxValue;
    mValueMin = info.minValue;
    mValueStep = info.stepValue;
}

ConfigNode::~ConfigNode()
{
}

ConfigNode ConfigNode::operator[]( const std::string &path )
{
    AARTSAAPI_Config cfg;
    AARTSAAPI_Result res = AARTSAAPI_ConfigFind( &mDevice->mDeviceHandle, &mConfigNode, &cfg, StringToWString( path ).c_str() );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Could not find path {} in node {}: {}", path, mFullName, ResultToString( res ) ) );

    return ConfigNode( mDevice, cfg, mFullName + "/" );
}

std::string ConfigNode::getString()
{
    if ( mType != ConfigNodeType::STRING && mType != ConfigNodeType::ENUM )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    ssize_t size = sizeof( mDevice->mConfigInfo.options );
    AARTSAAPI_Result res = AARTSAAPI_ConfigGetString( &mDevice->mDeviceHandle, &mConfigNode, mDevice->mConfigInfo.options, &size );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get string value of {}: {}", mFullName, ResultToString( res ) ) );

    std::wstring str( mDevice->mConfigInfo.options );
    return WStringToString( str );
}

void ConfigNode::setString( const std::string &str )
{
    if ( mType != ConfigNodeType::STRING && mType != ConfigNodeType::ENUM )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    std::wstring wstr = StringToWString( str );

    AARTSAAPI_Result res = AARTSAAPI_ConfigSetString( &mDevice->mDeviceHandle, &mConfigNode, wstr.c_str() );
    if ( res != AARTSAAPI_OK )
    {
        if ( mType == ConfigNodeType::ENUM && res == AARTSAAPI_ERROR_VALUE_INVALID )
            throw std::runtime_error( fmt::format( "Failed to set enum value of {} to \"{}\": {}. Valid options are: {}", mFullName, str, ResultToString( res ), mOptions ) );
        throw std::runtime_error( fmt::format( "Failed to set string value of {} to \"{}\": {}", mFullName, str, ResultToString( res ) ) );
    }
}

ConfigNode &ConfigNode::operator=( const std::string &str )
{
    setString( str );

    return *this;
}

ConfigNode &ConfigNode::operator=( const char *str )
{
    setString( std::string( str ) );

    return *this;
}

int64_t ConfigNode::getInt()
{
    if ( mType != ConfigNodeType::NUMBER && mType != ConfigNodeType::BOOL )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    int64_t value;
    AARTSAAPI_Result res = AARTSAAPI_ConfigGetInteger( &mDevice->mDeviceHandle, &mConfigNode, &value );

    // Config item might represent a button
    if ( res == AARTSAAPI_ERROR_INVALID_CONFIG )
        return 0;

    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get integer value of {}: {}", mFullName, ResultToString( res ) ) );

    return value;
}

void ConfigNode::setInt( int64_t v )
{
    if ( mType != ConfigNodeType::NUMBER && mType != ConfigNodeType::BOOL )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    AARTSAAPI_Result res = AARTSAAPI_ConfigSetInteger( &mDevice->mDeviceHandle, &mConfigNode, v );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to set integer value of {}: {}", mFullName, ResultToString( res ) ) );
}

ConfigNode &ConfigNode::operator=( int64_t v )
{
    setInt( v );
    return *this;
}

double ConfigNode::getFloat()
{
    if ( mType != ConfigNodeType::NUMBER )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    double value;
    AARTSAAPI_Result res = AARTSAAPI_ConfigGetFloat( &mDevice->mDeviceHandle, &mConfigNode, &value );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get float value of {}: {}", mFullName, ResultToString( res ) ) );

    return value;
}

void ConfigNode::setFloat( double v )
{
    if ( mType != ConfigNodeType::NUMBER )
        throw std::runtime_error( fmt::format( "Cannot interpret value of {} with type {} as Number!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    AARTSAAPI_Result res = AARTSAAPI_ConfigSetFloat( &mDevice->mDeviceHandle, &mConfigNode, v );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to set float value of {}: {}", mFullName, ResultToString( res ) ) );
}

ConfigNode &ConfigNode::operator=( double v )
{
    setFloat( v );
    return *this;
}

bool ConfigNode::getBool()
{
    return getInt();
}

void ConfigNode::setBool( bool v )
{
    setInt( v );
}

ConfigNode &ConfigNode::operator=( bool v )
{
    setBool( v );
    return *this;
}

std::vector<ConfigNode> &ConfigNode::getChildren( bool refresh )
{
    if ( mType != ConfigNodeType::GROUP )
        throw std::runtime_error( fmt::format( "Only group nodes can have children, {} is of type {}!", mFullName, TYPE_NAMES[static_cast<int>( mType )] ) );

    if ( !refresh && mChildren.size() )
        return mChildren;

    mChildren.clear();

    AARTSAAPI_Config config;
    AARTSAAPI_Result res = AARTSAAPI_ConfigFirst( &mDevice->mDeviceHandle, &mConfigNode, &config );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to descend into children of {}: {}", mFullName, ResultToString( res ) ) );

    do
    {
        mChildren.push_back( ConfigNode( mDevice, config, mFullName + "/" ) );
    } while ( AARTSAAPI_ConfigNext( &mDevice->mDeviceHandle, &mConfigNode, &config ) == AARTSAAPI_OK );

    return mChildren;
}

void DeviceWrapper::open( DeviceMode mode )
{
    if ( mOpened )
        throw std::runtime_error( "Device already open!" );

    std::wstring deviceString = fmt::format( L"{}/{}", DEVICE_TYPE_IDs[static_cast<int>( mDeviceType )], DEVICE_MODE_IDs[static_cast<int>( mode )] );
    AARTSAAPI_Result res = AARTSAAPI_OpenDevice( &mParent->mAPIHandle, &mDeviceHandle,
                                                 deviceString.c_str(),
                                                 mSerialNumberW.c_str() );

    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to open device: {}\n", ResultToString( res ) ) );

    mOpened = true;
}

void DeviceWrapper::close()
{
    if ( !mOpened )
    {
        LOG_DEBUG( "Tried to close already closed device!\n" );
        return;
    }

    if ( mConnected )
        disconnect();

    AARTSAAPI_Result res = AARTSAAPI_CloseDevice( &mParent->mAPIHandle, &mDeviceHandle );
    if ( res != AARTSAAPI_OK )
        LOG_DEBUG( "Failed to close device ({}): {}\n", getSerialNumber(), ResultToString( res ) );

    mOpened = false;
}

void DeviceWrapper::connect()
{
    if ( !mOpened )
        throw std::runtime_error( fmt::format( "Cannot connect to device ({}) which has not been opened yet!", mSerialNumber ) );

    AARTSAAPI_Result res = AARTSAAPI_ConnectDevice( &mDeviceHandle );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to connect to device: {}", ResultToString( res ) ) );

    mConnected = true;
}

void DeviceWrapper::disconnect()
{
    if ( !mConnected )
    {
        LOG_DEBUG( "Tried to disconnect from already disconnected device!\n" );
        return;
    }

    if ( mStarted )
        stop();

    AARTSAAPI_Result res = AARTSAAPI_DisconnectDevice( &mDeviceHandle );
    if ( res != AARTSAAPI_OK )
        LOG_DEBUG( "Failed to disconnect from device: {}\n", ResultToString( res ) );

    mConnected = false;
}

void DeviceWrapper::start()
{
    if ( !mConnected )
        connect();

    AARTSAAPI_Result res = AARTSAAPI_StartDevice( &mDeviceHandle );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to start device {}: {}", mSerialNumber, ResultToString( res ) ) );

    mStarted = true;
}

void DeviceWrapper::stop()
{
    if ( !mStarted )
    {
        LOG_DEBUG( "Tried to stop already stopped device!\n" );
        return;
    }

    AARTSAAPI_Result res = AARTSAAPI_StopDevice( &mDeviceHandle );
    if ( res != AARTSAAPI_OK )
        LOG_DEBUG( "Failed to stop device {}: {}\n", mSerialNumber, ResultToString( res ) );

    mStarted = false;
}

DeviceWrapper::DeviceWrapper( std::shared_ptr<RTSAWrapper> parent, DeviceType deviceType, const AARTSAAPI_DeviceInfo &dinfo )
    : mParent( parent ),
      mDeviceType( deviceType ),
      mSerialNumberW( dinfo.serialNumber ),
      mSerialNumber( mSerialNumberW.cbegin(), mSerialNumberW.cend() ),
      mReady( dinfo.ready ),
      mBoost( dinfo.boost ),
      mSuperSpeed( dinfo.superspeed ),
      mActive( dinfo.active )
{
}

DeviceWrapper::~DeviceWrapper()
{
    if ( mOpened )
        close();
}

ConfigNode DeviceWrapper::getConfigRoot()
{
    if ( !mOpened )
        throw std::runtime_error( "Can only read config of open devices!" );

    AARTSAAPI_Config cfg;

    AARTSAAPI_Result res = AARTSAAPI_ConfigRoot( &mDeviceHandle, &cfg );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get config root element of {}: {}", mSerialNumber, ResultToString( res ) ) );

    return ConfigNode( this->shared_from_this(), cfg, "/" );
}

ConfigNode DeviceWrapper::getHealthRoot()
{
    if ( !mOpened )
        throw std::runtime_error( "Can only read config of open devices!" );

    AARTSAAPI_Config cfg;

    AARTSAAPI_Result res = AARTSAAPI_ConfigHealth( &mDeviceHandle, &cfg );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get health root element of {}: {}", mSerialNumber, ResultToString( res ) ) );

    return ConfigNode( this->shared_from_this(), cfg, "/" );
}

DeviceState DeviceWrapper::getState()
{
    AARTSAAPI_Result res = AARTSAAPI_GetDeviceState( &mDeviceHandle );
    if ( !( res & AARTSAAPI_IDLE ) )
        throw std::runtime_error( fmt::format( "Failed to get device state: {}", ResultToString( res ) ) );

    return static_cast<DeviceState>( res & 0xFF );
}

int32_t DeviceWrapper::getAvailablePacketCount( int32_t channel )
{
    int32_t v;
    AARTSAAPI_Result res = AARTSAAPI_AvailPackets( &mDeviceHandle, channel, &v );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to query available packet count of device {} on channel {}: {}", mSerialNumber, channel, ResultToString( res ) ) );

    return v;
}

bool DeviceWrapper::getPacket( AARTSAAPI_Packet *packet, int32_t channel, int32_t index, bool blocking, int timeout )
{
    packet->cbsize = sizeof( AARTSAAPI_Packet );

    for ( int i = 0; ( i == 0 || blocking ) && i < timeout / 5; i++ )
    {
        AARTSAAPI_Result res = AARTSAAPI_GetPacket( &mDeviceHandle, channel, index, packet );
        if ( res == AARTSAAPI_OK )
            return true;

        if ( blocking && res == AARTSAAPI_EMPTY )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
            continue;
        }

        throw std::runtime_error( fmt::format( "An error occurred while trying to get a new packet: {}", res ) );
    }

    return false;
}

void DeviceWrapper::sendPacket( AARTSAAPI_Packet *packet, int32_t channel )
{
    packet->cbsize = sizeof( AARTSAAPI_Packet );

    AARTSAAPI_Result res = AARTSAAPI_SendPacket( &mDeviceHandle, channel, packet );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to send packet to channel {}: {}", channel, ResultToString( res ) ) );
}

void DeviceWrapper::consumePackets( int32_t channel, int32_t count )
{
    AARTSAAPI_Result res = AARTSAAPI_ConsumePackets( &mDeviceHandle, channel, count );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to consume {} samples on channel {}: {}", count, channel, ResultToString( res ) ) );
}

double DeviceWrapper::getMasterStreamTime()
{
    double v;

    AARTSAAPI_Result res = AARTSAAPI_GetMasterStreamTime( &mDeviceHandle, v );
    if ( res != AARTSAAPI_OK )
        throw std::runtime_error( fmt::format( "Failed to get master stream time: {}", ResultToString( res ) ) );

    return v;
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

std::shared_ptr<DeviceWrapper> RTSAWrapper::getDevice( DeviceType deviceType, const std::string &serialNumber, int timeout )
{
    for ( auto &dev : mDevices )
    {
        auto ptr = dev.lock();
        if ( !ptr )
            continue;

        if ( ptr->mDeviceType != deviceType )
            continue;

        if ( serialNumber.size() > 0 && ptr->mSerialNumber != serialNumber )
            continue;

        if ( ptr->mOpened )
            LOG_DEBUG( "Warning: RTSAWrapper::getDevice was used to get a handle on a device which was already opened!\n" );

        return ptr;
    }

    AARTSAAPI_Result res;

    while ( true )
    {
        res = AARTSAAPI_RescanDevices( &mAPIHandle, timeout );
        if ( res == AARTSAAPI_RETRY )
            continue;

        if ( res != AARTSAAPI_OK )
            throw std::runtime_error( fmt::format( "Failed to scan for devices: {}", ResultToString( res ) ) );

        break;
    }

    AARTSAAPI_DeviceInfo dinfo{};
    dinfo.cbsize = sizeof( AARTSAAPI_DeviceInfo );

    for ( int i = 0; AARTSAAPI_EnumDevice( &mAPIHandle, DEVICE_TYPE_IDs[static_cast<int>( deviceType )], i, &dinfo ) == AARTSAAPI_OK; i++ )
    {
        if ( serialNumber.size() )
        {
            std::wstring devSerialW( dinfo.serialNumber );
            std::string devSerial = WStringToString( devSerialW );

            if ( serialNumber != devSerial )
                continue;
        }

        if ( dinfo.active )
            LOG_DEBUG( "Warning: RTSAWrapper::getDevice was used to get a handle on a device which was already opened!\n" );

        auto ptr = DeviceWrapper::create( this->shared_from_this(), deviceType, dinfo );
        mDevices.push_back( std::weak_ptr( ptr ) );
        return ptr;
    }

    throw std::runtime_error( "Failed to find available device!" );
}

std::vector<std::shared_ptr<DeviceWrapper>> RTSAWrapper::getAllDevices( const DeviceType deviceType, bool rescan, int timeout )
{
    std::vector<std::shared_ptr<DeviceWrapper>> devices;

    mDevices.clear();

    if ( rescan )
    {
        AARTSAAPI_Result res;

        while ( true )
        {
            res = AARTSAAPI_RescanDevices( &mAPIHandle, timeout );
            if ( res == AARTSAAPI_RETRY )
                continue;

            if ( res != AARTSAAPI_OK )
                throw std::runtime_error( fmt::format( "Failed to scan for devices: {}", ResultToString( res ) ) );

            AARTSAAPI_DeviceInfo dinfo{};
            dinfo.cbsize = sizeof( AARTSAAPI_DeviceInfo );

            for ( int i = 0; AARTSAAPI_EnumDevice( &mAPIHandle, DEVICE_TYPE_IDs[static_cast<int>( deviceType )], i, &dinfo ) == AARTSAAPI_OK; i++ )
            {
                auto ptr = DeviceWrapper::create( this->shared_from_this(), deviceType, dinfo );
                mDevices.push_back( std::weak_ptr( ptr ) );
                devices.push_back( ptr );
            }

            break;
        }
    }
    else
    {
        for ( auto &weakPtr : mDevices )
        {
            auto ptr = weakPtr.lock();
            if ( ptr )
                devices.push_back( ptr );
        }
    }

    return devices;
}

}; // namespace AARTSAAPI
