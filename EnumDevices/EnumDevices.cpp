#include <aaroniartsaapi.h>

#include <iostream>
#include <string>

int main()
{
    AARTSAAPI_Result res;

    // Initialize library for medium memory usage
    if ( ( res = AARTSAAPI_Init( AARTSAAPI_MEMORY_MEDIUM ) ) != AARTSAAPI_OK )
    {
        std::cerr << "AARTSAAPI_Init failed : " << std::hex << res << std::endl;
        return EXIT_FAILURE;
    }

    // Open a library handle for use by this application
    AARTSAAPI_Handle h;

    if ( ( res = AARTSAAPI_Open( &h ) ) == AARTSAAPI_OK )
    {
        // Rescan all devices controlled by the aaronia library and update
        // the firmware if required.

        while ( true )
        {
            res = AARTSAAPI_RescanDevices( &h, 2000 );
            if ( res == AARTSAAPI_RETRY )
                continue;

            if ( res != AARTSAAPI_OK )
            {
                std::cerr << "AARTSAAPI_RescanDevices failed : " << std::hex << res << std::endl;
                break;
            }

            // Initialize device info structure with the structure size

            AARTSAAPI_DeviceInfo dinfo = { sizeof( AARTSAAPI_DeviceInfo ) };

            // Loop over all devices, starting from zero until an error occurs
            // or we run out of devices

            int i = 0;
            while ( AARTSAAPI_EnumDevice( &h, L"spectranv6", i, &dinfo ) == AARTSAAPI_OK )
            {
                // Print the device serial number

                std::wcout << i << " : " << dinfo.serialNumber << std::endl;
                i++;
            }

            break;
        }

        // Close the library handle

        AARTSAAPI_Close( &h );
    }
    else
        std::wcerr << "AARTSAAPI_Open failed : " << std::hex << res << std::endl;

    // Shutdown library, release resources
    AARTSAAPI_Shutdown();

    return EXIT_SUCCESS;
}
