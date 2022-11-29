#include <AARTSAAPIWrapper.h>

#include <fmt/core.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <complex>

volatile bool shutdown = false;

#ifdef __linux__
#include <signal.h>
extern "C"
{

    void signalHandler( int )
    {
        shutdown = true;
    }
}
#endif

namespace AAR = AARTSAAPI;

int main( int argc, char *argv[] )
{
#ifdef __linux__
    // Register signal handler
    signal( SIGINT, signalHandler );
    signal( SIGHUP, signalHandler );
    signal( SIGTERM, signalHandler );
#endif

    size_t packetCount = 0xFFFFFFFFFFFF;
    if ( argc > 1 )
        sscanf( argv[1], "%lu", &packetCount );

    auto api = AAR::RTSAWrapper::create( AAR::MemoryMode::MEDIUM );
    auto device = api->getDevice( AAR::DeviceType::SPECTRANV6 );
    device->open( AAR::DeviceMode::RAW );

    auto config = device->getConfigRoot();

    auto dev = config["device"];
    dev["receiverchannel"] = "Rx1";
    dev["outputformat"] = "iq";
    dev["receiverclock"] = "92MHz";

    auto main = config["main"];
    main["centerfreq"] = 2.44e9;
    main["decimation"] = "Full";
    main["reflevel"] = -10.0;

    auto calib = config["calibration"];
    calib["rffilter"] = "Auto Extended";
    calib["preamp"] = "Auto";

    device->start();

    AARTSAAPI_Packet packet;

    // Request first packet with a large timeout to allow for the FPGA to start up. The data is ignored
    device->getPacket( &packet, 0, 0, true, 10000 );

    fmt::print( "\nReceiving packets:\n" );
    fmt::print( "|    i    | streamID |    flags   |     startTime    |      endTime     | startFrequency |  stepFrequency | spanFrequency | rbwFrequency |   num   |  size  | signalEnergy |\n" );
    fmt::print( "|---------|----------|------------|------------------|------------------|----------------|----------------|---------------|--------------|---------|--------|--------------|\n" );
    for ( size_t i = 0; i < packetCount && !shutdown; i++ )
    {
        if ( !device->getPacket( &packet, 0, 0, true, 100 ) )
            continue;

        std::complex<float> *data = reinterpret_cast<std::complex<float> *>(packet.fp32);
        size_t n = packet.num;

        float energy = 0;
        for (size_t j = 0; j < n; j++)
            energy += std::norm(data[j]);

        fmt::print( "|{:8d} | {:8d} | 0x{:08x} | {:12.5f} | {:12.5f} | {:14.0f} | {:14.0f} | {:13.0f} | {:12.0f} | {:7d} | {:6d} | {:1.6e} |\n",
                    i, packet.streamID, packet.flags, packet.startTime, packet.endTime, packet.startFrequency, packet.stepFrequency,
                    packet.spanFrequency, packet.rbwFrequency, packet.num, packet.size, energy );

        device->consumePackets( 0, 1 );
    }

    device->close();

    return EXIT_SUCCESS;
}
