#include <aaroniartsaapi.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#pragma comment( lib, "aaroniartsaapi.lib" )

AARTSAAPI_Result testRange( AARTSAAPI_Device d, double startFrequency, double stopFrequency, double rbwFrequency )
{
    AARTSAAPI_Result res;
    int32_t num;

    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );

    // Consume all pending packets from previous test

    res = AARTSAAPI_AvailPackets( &d, 0, &num );
    if ( res == AARTSAAPI_OK )
    {
        if ( num > 0 )
            AARTSAAPI_ConsumePackets( &d, 0, num );

        // Configure for new test

        AARTSAAPI_Config config, root;

        if ( ( res = AARTSAAPI_ConfigRoot( &d, &root ) ) == AARTSAAPI_OK )
        {
            // Set start frequency

            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/startfreq" ) == AARTSAAPI_OK )
                AARTSAAPI_ConfigSetFloat( &d, &config, startFrequency );

            // Set stop frequency

            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/stopfreq" ) == AARTSAAPI_OK )
                AARTSAAPI_ConfigSetFloat( &d, &config, stopFrequency );

            // Set RBW frequency

            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/rbwfreq" ) == AARTSAAPI_OK )
                AARTSAAPI_ConfigSetFloat( &d, &config, rbwFrequency );

            // Start capturing

            if ( ( res = AARTSAAPI_StartDevice( &d ) ) == AARTSAAPI_OK )
            {
                // Prepare data packet

                AARTSAAPI_Packet packet = { sizeof( AARTSAAPI_Packet ) };
                AARTSAAPI_Result res;

                // Wait for the first new packet

                do
                {
                    // Wait for the next packet

                    while ( ( res = AARTSAAPI_GetPacket( &d, 0, 0, &packet ) ) == AARTSAAPI_EMPTY )
                        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );

                    // Loop until stream start, drop all pending packets from previous test

                } while ( res == AARTSAAPI_OK && !( packet.flags & AARTSAAPI_PACKET_STREAM_START ) );

                if ( res == AARTSAAPI_OK )
                {
                    // Dump packet information

                    std::wcout << L"I: " << startFrequency << L", " << stopFrequency << L", " << rbwFrequency << L" O: " << packet.startFrequency << L", " << packet.startFrequency + packet.spanFrequency << L", " << packet.rbwFrequency << " S: " << packet.size << std::endl;
                }

                // Stop the device

                AARTSAAPI_StopDevice( &d );
            }
        }
    }

    return res;
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
                    // Try to open the first V6 in the system in sweep mode

                    AARTSAAPI_Device d;

                    if ( ( res = AARTSAAPI_OpenDevice( &h, &d, L"spectranv6/sweepsa", dinfo.serialNumber ) ) == AARTSAAPI_OK )
                    {
                        // Begin configuration, get root of configuration tree

                        AARTSAAPI_Config config, root;

                        if ( AARTSAAPI_ConfigRoot( &d, &root ) == AARTSAAPI_OK )
                        {
                            // Select the first receiver channel

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/receiverchannel" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetString( &d, &config, L"Rx1" );

                            // Use fast receiver clock

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/receiverclock" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetString( &d, &config, L"245MHz" );

                            // Reference level at -20dBm

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/reflevel" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, -20.0 );

                            // Connect to the physical device

                            if ( ( res = AARTSAAPI_ConnectDevice( &d ) ) == AARTSAAPI_OK )
                            {
                                // Iterate over 50 different RBW ranges

#if 1
                                for ( int i = 0; i < 10000; i += 1 )
                                {
                                    testRange( d, 800.0e6, 1000.0e6, 100.0e3 );
                                    testRange( d, 900.0e6, 920.0e6, 20.0e3 );
                                    testRange( d, 1000.0e6, 1300.0e6, 1000.0e3 );
                                    testRange( d, 2000.0e6, 2400.0e6, 1000.0e3 );
                                }

#else
                                for ( int i = 0; i <= 50; i += 1 )
                                {
                                    testRange( d, 1.0e9, 1.1e9, 1.0e6 * pow( 10, -double( i ) / 10 ) );
                                }
#endif

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
