#include <AARTSAAPIWrapper.h>

#include <fmt/core.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace AAR = AARTSAAPI;

int main()
{
    auto apiWrapper = AAR::RTSAWrapper::create( AAR::MemoryMode::MEDIUM );

    std::cout << "Devices:" << std::endl;
    for ( auto &device : apiWrapper->getAllDevices( AAR::DeviceType::SPECTRANV6 ) )
        fmt::print( " - Serial Number: {}\n   ready: {}\n   boost: {}\n   superSpeed: {}\n   active: {}\n",
                    device->getSerialNumber(),
                    device->isReady(),
                    device->hasBoost(),
                    device->isSuperSpeed(),
                    device->isActive() );

    return EXIT_SUCCESS;
}
