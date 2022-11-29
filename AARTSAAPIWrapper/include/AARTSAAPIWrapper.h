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

enum class MemoryMode
{
    SMALL = AARTSAAPI_MEMORY_SMALL,
    MEDIUM = AARTSAAPI_MEMORY_MEDIUM,
    LARGE = AARTSAAPI_MEMORY_LARGE,
    LUDICRIOUS = AARTSAAPI_MEMORY_LUDICROUS
};

class RTSAWrapper;

class DeviceWrapper
{
private:
    std::shared_ptr<RTSAWrapper> mParent;
    std::wstring mSerialNumber;
    bool mReady;
    bool mBoost;
    bool mSuperSpeed;
    bool mActive;

public:
    DeviceWrapper( std::shared_ptr<RTSAWrapper> parent, const AARTSAAPI_DeviceInfo &dinfo )
        : mParent( parent ),
          mSerialNumber( dinfo.serialNumber ),
          mReady( dinfo.ready ),
          mBoost( dinfo.boost ),
          mSuperSpeed( dinfo.superspeed ),
          mActive( dinfo.active )
    {
    }

    const std::shared_ptr<RTSAWrapper> getRTSAWrapper()
    {
        return mParent;
    }

    const std::wstring &getSerialNumberW() const
    {
        return mSerialNumber;
    }

    const std::string getSerialNumber() const
    {
        std::string serialNumber( mSerialNumber.cbegin(), mSerialNumber.cend() );
        return serialNumber;
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
};

class RTSAWrapper : public std::enable_shared_from_this<RTSAWrapper>
{
private:
    AARTSAAPI_Handle mAPIHandle;

    RTSAWrapper( MemoryMode memoryMode );

public:
    static std::shared_ptr<RTSAWrapper> create( MemoryMode mode )
    {
        return std::shared_ptr<RTSAWrapper>( new RTSAWrapper( mode ) );
    }
    virtual ~RTSAWrapper();

    const AARTSAAPI_Handle &getAPIHandle()
    {
        return mAPIHandle;
    }

    std::vector<DeviceWrapper> getAllDevices( DeviceType deviceType, bool rescan = true, int timeout = 30000 );
};

}; // namespace AARTSAAPI
