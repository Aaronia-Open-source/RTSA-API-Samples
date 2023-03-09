#include <aaroniartsaapi.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

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

                    if ( ( res = AARTSAAPI_OpenDevice( &h, &d, L"spectranv6/raw", dinfo.serialNumber ) ) == AARTSAAPI_OK )
                    {
                        AARTSAAPI_Config config, root;

                        if ( AARTSAAPI_ConfigRoot( &d, &root ) == AARTSAAPI_OK )
                        {
                            // Set generator config
                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/centerfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 2440.0e6 );                                                       
                            
                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"main/transgain" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, -30 );                            
                       
                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/transmittermode" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetString( &d, &config, L"Pattern Generator");
                             
                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/type" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetString( &d, &config, L"Sweep");
                            
                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/startfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 2450.0e6);

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/stopfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 2460.0e6);

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/stepfreq" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 100);                                

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/duration" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 0.01f);

                            if ( AARTSAAPI_ConfigFind( &d, &root, &config, L"device/generator/powerramp" ) == AARTSAAPI_OK )
                                AARTSAAPI_ConfigSetFloat( &d, &config, 0.0f);
                            
                            if ( ( res = AARTSAAPI_ConnectDevice( &d ) ) == AARTSAAPI_OK )
                            {                                
                                if ( AARTSAAPI_StartDevice( &d ) == AARTSAAPI_OK )
                                {                                    
                                    std::wcout << "\nbringing up device (SN: " << dinfo.serialNumber << ")\n";
                                    std::wcout.flush();

                                    while ( AARTSAAPI_GetDeviceState( &d ) != AARTSAAPI_RUNNING )
                                    {
                                        std::wcout << ".";
                                        std::wcout.flush();
                                        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
                                    }

                                    std::wcout << "\nstarting generator....\n";
                                    std::wcout.flush();

                                    std::wcout << std::endl;                                                                      
                                    do { std::wcout << '\n' << "Press a key to continue..."; std::wcout.flush(); } while (std::cin.get() != '\n');
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
