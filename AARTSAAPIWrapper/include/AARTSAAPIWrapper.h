#pragma once

#include <aaroniartsaapi.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace AARTSAAPI
{

const char *ResultToString( AARTSAAPI_Result result );

enum class DeviceType
{
    SPECTRANV6 = 0
};

enum class DeviceMode
{
    RAW = 0,
    RTSA,
    SWEEPSA,
    IQRECEIVER,
    IQTRANSMITTER,
    IQTRANSCEIVER
};

enum class MemoryMode
{
    SMALL = AARTSAAPI_MEMORY_SMALL,
    MEDIUM = AARTSAAPI_MEMORY_MEDIUM,
    LARGE = AARTSAAPI_MEMORY_LARGE,
    LUDICRIOUS = AARTSAAPI_MEMORY_LUDICROUS
};

class DeviceWrapper;

enum class ConfigNodeType
{
    OTHER = AARTSAAPI_CONFIG_TYPE_OTHER,
    GROUP = AARTSAAPI_CONFIG_TYPE_GROUP,
    BLOB = AARTSAAPI_CONFIG_TYPE_BLOB,
    NUMBER = AARTSAAPI_CONFIG_TYPE_NUMBER,
    BOOL = AARTSAAPI_CONFIG_TYPE_BOOL,
    ENUM = AARTSAAPI_CONFIG_TYPE_ENUM,
    STRING = AARTSAAPI_CONFIG_TYPE_STRING
};

enum class DeviceState
{
    IDLE = AARTSAAPI_IDLE,
    CONNECTING = AARTSAAPI_CONNECTING,
    CONNECTED = AARTSAAPI_CONNECTED,
    STARTING = AARTSAAPI_STARTING,
    RUNNING = AARTSAAPI_RUNNING,
    STOPPING = AARTSAAPI_STOPPING,
    DISCONNECTING = AARTSAAPI_DISCONNECTING
};

class ConfigNode
{
    friend class DeviceWrapper;

    std::shared_ptr<DeviceWrapper> mDevice;

    ConfigNode( std::shared_ptr<DeviceWrapper> device, AARTSAAPI_Config config, const std::string &path );

    AARTSAAPI_Config mConfigNode;

    std::string mPath;
    std::string mFullName;

    std::string mName;
    std::wstring mNameW;

    std::string mTitle;
    std::wstring mTitleW;

    std::string mUnit;
    std::wstring mUnitW;

    std::string mOptions;
    std::wstring mOptionsW;

    uint64_t mDisabledOptions;

    // Parameters of numeric config items
    double mValueMax = 0;
    double mValueMin = 0;
    double mValueStep = 0;

    ConfigNodeType mType;

    std::vector<ConfigNode> mChildren;

public:
    ~ConfigNode();

    ConfigNode operator[]( const std::string &path );

    std::string getName() const
    {
        return mName;
    }

    std::wstring getNameW() const
    {
        return mNameW;
    }

    std::string getFullName() const
    {
        return mFullName;
    }

    std::string getPath() const
    {
        return mPath;
    }

    std::string getTitle() const
    {
        return mTitle;
    }

    std::wstring getTitleW() const
    {
        return mTitleW;
    }

    std::string getUnit() const
    {
        return mUnit;
    }

    std::wstring getUnitW() const
    {
        return mUnitW;
    }

    std::string getOptions() const
    {
        return mOptions;
    }

    std::wstring getOptionsW() const
    {
        return mOptionsW;
    }

    ConfigNodeType getType() const
    {
        return mType;
    }

    double getValueMax() const
    {
        return mValueMax;
    }

    double getValueMin() const
    {
        return mValueMin;
    }

    double getValueStep() const
    {
        return mValueStep;
    }

    std::string getString();
    void setString( const std::string &str );
    ConfigNode &operator=( const std::string &str );
    ConfigNode &operator=( const char *str );

    int64_t getInt();
    void setInt( int64_t v );
    ConfigNode &operator=( int64_t v );

    double getFloat();
    void setFloat( double v );
    ConfigNode &operator=( double v );

    bool getBool();
    void setBool( bool v );
    ConfigNode &operator=( bool v );

    std::vector<ConfigNode> &getChildren( bool refresh = false );
};

class RTSAWrapper;

class DeviceWrapper : public std::enable_shared_from_this<DeviceWrapper>
{
    friend class RTSAWrapper;
    friend class ConfigNode;

private:
    std::shared_ptr<RTSAWrapper> mParent;
    DeviceType mDeviceType;
    std::wstring mSerialNumberW;
    std::string mSerialNumber;

    // This object is only used by ConfigNodes which belog to this device object.
    // It is stored here to facilitate sharing a single ConfigInfo object between all
    // "child" ConfigNodes.
    AARTSAAPI_ConfigInfo mConfigInfo;

    bool mOpened = false;
    bool mConnected = false;
    bool mStarted = false;
    AARTSAAPI_Device mDeviceHandle{};

    bool mReady;
    bool mBoost;
    bool mSuperSpeed;
    bool mActive;

    static std::shared_ptr<DeviceWrapper> create( std::shared_ptr<RTSAWrapper> parent, DeviceType deviceType, const AARTSAAPI_DeviceInfo &dinfo )
    {
        return std::shared_ptr<DeviceWrapper>( new DeviceWrapper( parent, deviceType, dinfo ) );
    }

    DeviceWrapper( std::shared_ptr<RTSAWrapper> parent, DeviceType deviceType, const AARTSAAPI_DeviceInfo &dinfo );

public:
    ~DeviceWrapper();

    void open( DeviceMode mode );

    void close();

    void connect();

    void disconnect();

    void start();

    void stop();

    ConfigNode getConfigRoot();

    ConfigNode getHealthRoot();

    DeviceState getState();

    int32_t getAvailablePacketCount( int32_t channel );

    bool getPacket( AARTSAAPI_Packet *packet, int32_t channel, int32_t index, bool blocking = true, int timeout = 100 );

    void sendPacket( AARTSAAPI_Packet *packet, int32_t channel );

    void consumePackets( int32_t channel, int32_t count );

    double getMasterStreamTime();

    const std::shared_ptr<RTSAWrapper> getRTSAWrapper() const
    {
        return mParent;
    }

    const std::wstring &getSerialNumberW() const
    {
        return mSerialNumberW;
    }

    const std::string getSerialNumber() const
    {
        return mSerialNumber;
    }

    bool isReady() const
    {
        return mReady;
    }

    bool hasBoost() const
    {
        return mBoost;
    }

    bool isSuperSpeed() const
    {
        return mSuperSpeed;
    }

    bool isActive() const
    {
        return mActive;
    }

    bool isOpen() const
    {
        return mOpened;
    }
};

class RTSAWrapper : public std::enable_shared_from_this<RTSAWrapper>
{
    friend class DeviceWrapper;
    friend class ConfigNode;

private:
    AARTSAAPI_Handle mAPIHandle;

    std::vector<std::weak_ptr<DeviceWrapper>> mDevices;

    RTSAWrapper( MemoryMode memoryMode );

public:
    static std::shared_ptr<RTSAWrapper> create( MemoryMode mode )
    {
        return std::shared_ptr<RTSAWrapper>( new RTSAWrapper( mode ) );
    }
    ~RTSAWrapper();

    const AARTSAAPI_Handle &getAPIHandle()
    {
        return mAPIHandle;
    }

    std::vector<std::shared_ptr<DeviceWrapper>> getAllDevices( DeviceType deviceType, bool rescan = true, int timeout = 30000 );

    std::shared_ptr<DeviceWrapper> getDevice( DeviceType deviceType, const std::string &serialNumber = "", int timeout = 30000 );
};

}; // namespace AARTSAAPI
