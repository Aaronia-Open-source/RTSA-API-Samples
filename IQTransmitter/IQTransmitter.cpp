#include <aaroniartsaapi.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

void streamIQ( AARTSAAPI_Device d )
{
    static const double pi = 4.0 * atan( 1.0 );
    static const double zeroDBm = sqrt( 1.0 / 20.0 );
    float iqbuffer[32768];
    float riqbuffer[32768];

    // Prepare a small frequency sweep range

    double w = 0;
    for ( int i = 0; i < 16384; i++ )
    {
        double phi = ( double( i ) / 16384 * 2 - 1 ) * pi;
        w += phi;

        iqbuffer[2 * i + 1] = float( cos( w ) * zeroDBm );
        iqbuffer[2 * i + 0] = float( sin( w ) * zeroDBm );

        riqbuffer[2 * i + 1] = float( cos( -w ) * zeroDBm );
        riqbuffer[2 * i + 0] = float( sin( -w ) * zeroDBm );
    }

    // Prepare output packet
    AARTSAAPI_Packet packet = { sizeof( AARTSAAPI_Packet ) };

    // Frequency range

    packet.startFrequency = 2430.0e6;
    packet.stepFrequency = 1.0e6;

    double startTime;

    // Get the current system time

    AARTSAAPI_GetMasterStreamTime( &d, startTime );

    // Prepare the first packet to be played in 200ms

    packet.startTime = startTime + 0.2;
    packet.size = 2;
    packet.stride = 2;
    packet.fp32 = iqbuffer;
    packet.num = 16384;

    int NumPackets = 100;

    // Stream 100 packets

    for ( int i = 0; i < NumPackets; i++ )
    {
        // Calculate end time of packet base on number of samples
        // and sample rate

        packet.endTime = packet.startTime + packet.num / packet.stepFrequency;

        // Alternate direction

        if ( i & 1 )
            packet.fp32 = riqbuffer;
        else
            packet.fp32 = iqbuffer;

        // Wait for a max queue fill level of 45ms

        AARTSAAPI_GetMasterStreamTime( &d, startTime );
        while ( startTime + 0.05 < packet.startTime )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( int( 1000 * ( packet.startTime - startTime - 0.045 ) ) ) );
            AARTSAAPI_GetMasterStreamTime( &d, startTime );
        }

        // Set start and end flags

        if ( i == 0 )
            packet.flags = AARTSAAPI_PACKET_SEGMENT_START | AARTSAAPI_PACKET_STREAM_START;
        else if ( i + 1 == NumPackets )
            packet.flags = AARTSAAPI_PACKET_SEGMENT_END | AARTSAAPI_PACKET_STREAM_END;
        else
            packet.flags = 0;

        // Send the packet

        AARTSAAPI_SendPacket( &d, 0, &packet );

        // Advance packet time

        packet.startTime = packet.endTime;
    }

    // Wait for the last packet to finish

    AARTSAAPI_GetMasterStreamTime( &d, startTime );
    while ( startTime < packet.startTime )
    {

        std::this_thread::sleep_for( std::chrono::milliseconds( int( 1000 * ( packet.startTime - startTime ) ) ) );
        AARTSAAPI_GetMasterStreamTime( &d, startTime );
    }
}

int main()
{
    AARTSAAPI_Result res;

    // Initialize library for large memory usage

    if ( ( res = AARTSAAPI_Init( AARTSAAPI_MEMORY_LARGE ) ) == AARTSAAPI_OK )
    {

        // Open a library handle for use by this application

        AARTSAAPI_Handle h;

        if ( ( res = AARTSAAPI_Open( &h ) ) == AARTSAAPI_OK )
        {
            // Rescan all devices controlled by the aaronia library and update
            // the firmware if required.

            if ( ( res = AARTSAAPI_RescanDevices( &h, 2000 ) ) == AARTSAAPI_OK )
            {
                AARTSAAPI_DeviceInfo dinfo = { sizeof( AARTSAAPI_DeviceInfo ) };

                // Get the serial number of the first V6 in the system

                if ( ( res = AARTSAAPI_EnumDevice( &h, L"spectranv6", 0, &dinfo ) ) == AARTSAAPI_OK )
                {
                    AARTSAAPI_Device d;

                    // Try to open the first V6 in the system in transmitter mode

                    if ( ( res = AARTSAAPI_OpenDevice( &h, &d, L"spectranv6/iqtransmitter", dinfo.serialNumber ) ) == AARTSAAPI_OK )
                    {
                        AARTSAAPI_Config config, root;

                        if ( AARTSAAPI_ConfigRoot( &d, &root ) == AARTSAAPI_OK )
                        {
                            // Select the center frequency of the transmitter

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/centerfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 2440.0e6 );

                            // Select the frequency range of the transmitter

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/spanfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 50.0e6 );

                            // Select the transmitter gain

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/transgain" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 0.0 );

                            // Connect to the physical device

                            if ( ( res = AARTSAAPI_ConnectDevice( &d ) ) == AARTSAAPI_OK )
                            {
                                // Start the receiver

                                if ( AARTSAAPI_StartDevice( &d ) == AARTSAAPI_OK )
                                {
                                    // Wait for the transmitter running

                                    while ( AARTSAAPI_GetDeviceState( &d ) != AARTSAAPI_RUNNING )
                                    {
                                        std::wcout << ".";
                                        std::wcout.flush();
                                        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                                    }
                                    std::wcout << std::endl;

                                    // Send data to the transmitter

                                    streamIQ( d );
                                }

                                // Release the hardware

                                AARTSAAPI_DisconnectDevice( &d );
                            }
                            else
                                std::wcerr << "AARTSAAPI_ConnectDevice failed : " << std::hex << res << std::endl;
                        }

                        // Close the device handle

                        AARTSAAPI_CloseDevice( &h, &d );
                    }
                    else
                        std::wcerr << "AARTSAAPI_OpenDevice failed : " << std::hex << res << std::endl;
                }
                else
                    std::wcerr << "AARTSAAPI_EnumDevice failed : " << std::hex << res << std::endl;
            }
            else
                std::wcerr << "AARTSAAPI_RescanDevices failed : " << std::hex << res << std::endl;

            // Close the library handle

            AARTSAAPI_Close( &h );
        }
        else
            std::wcerr << "AARTSAAPI_Open failed : " << std::hex << res << std::endl;

        // Shutdown library, release resources

        AARTSAAPI_Shutdown();
    }
    else
        std::wcerr << "AARTSAAPI_Init failed : " << std::hex << res << std::endl;

    return 0;
}
