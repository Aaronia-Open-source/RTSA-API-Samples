#include <aaroniartsaapi.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>


// Receive IQ samples and display on console
void streamIQ(AARTSAAPI_Device d)
{
	// Prepare a line buffer

	wchar_t	buff[102];
	for (int i = 0; i < 101; i++)
		buff[i] = ' ';
	buff[101] = 0;

	// Receive up to 10 packets

	for (int i = 0; i < 10; i++)
	{
		// Prepare data packet

		AARTSAAPI_Packet	packet = { sizeof(AARTSAAPI_Packet) };
		AARTSAAPI_Result	res;

		// Get the next data packet, sleep for some milliseconds, if none
		// available yet.

		while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &packet)) == AARTSAAPI_EMPTY)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

		// If we actually got a packet
		if (res == AARTSAAPI_OK)
		{
			for (int j = 0; j < packet.num; j++)
			{
				// Prepare grid lines

				buff[0] = '|';
				buff[50] = '|';
				buff[25] = '.';
				buff[75] = '.';
				buff[100] = '|';

				// Use 1mV for full size

				int ki = int(packet.fp32[2 * j + 0] * 50 * 1000), kq = int(packet.fp32[2 * j + 1] * 50 * 1000);

				if (ki >= -50 && ki <= 50)
					buff[ki + 50] = 'I';
				if (kq >= -50 && kq <= 50)
					buff[kq + 50] = 'Q';
				std::wcout << buff << std::endl;
				if (ki >= -50 && ki <= 50)
					buff[ki + 50] = ' ';
				if (kq >= -50 && kq <= 50)
					buff[kq + 50] = ' ';

			}

			// Remove the first packet from the packet queue

			AARTSAAPI_ConsumePackets(&d, 0, 1);
		}
	}

}

int main()
{
	AARTSAAPI_Result	res;

	// Initialize library for medium memory usage

	if ((res = AARTSAAPI_Init(AARTSAAPI_MEMORY_MEDIUM)) == AARTSAAPI_OK)
	{

		// Open a library handle for use by this application

		AARTSAAPI_Handle	h;

		if ((res = AARTSAAPI_Open(&h)) == AARTSAAPI_OK)
		{
			// Rescan all devices controlled by the aaronia library and update
			// the firmware if required.

			if ((res = AARTSAAPI_RescanDevices(&h, 2000)) == AARTSAAPI_OK)
			{
				// Get the serial number of the first V6 in the system

				AARTSAAPI_DeviceInfo	dinfo = { sizeof(AARTSAAPI_DeviceInfo) };

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6", 0, &dinfo)) == AARTSAAPI_OK)
				{
					// Try to open the first V6 in the system

					AARTSAAPI_Device	d;

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/iqreceiver", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Use slow receiver clock

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"92MHz");

							// Set the receiver center frequency

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2440.0e6);

							// Set required span frequency to 64kHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 64.0e3);

							// Set the reference level of the receiver

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/reflevel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, -20.0);

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Receive some IQ samples

									streamIQ(d);

									// Stop the receiver
									AARTSAAPI_StopDevice(&d);
								}

								// Release the hardware

								AARTSAAPI_DisconnectDevice(&d);
							}
							else
								std::wcerr << "AARTSAAPI_ConnectDevice failed : " << std::hex << res << std::endl;
						}

						// Close the device handle

						AARTSAAPI_CloseDevice(&h, &d);
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

			AARTSAAPI_Close(&h);
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
