#include "../helper.h"

#include <chrono>
#include <cstdint>
#include <iomanip>
uint64_t get_tick_count_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void streamIQ(AARTSAAPI_Device d)
{
	// Prepare a line buffer

	wchar_t	buff[102];
	for (int i = 0; i < 101; i++)
		buff[i] = ' ';
	buff[101] = 0;

	int64_t			numSamples = 0;
	uint64_t		startTicks = get_tick_count_ms();
	int				numPackets = 0;
	double			prevTime;


	// Receive up to 10 packets

	for (;;)
	{
		// Prepare data packet

		AARTSAAPI_Packet	packet = { sizeof(AARTSAAPI_Packet) };
		AARTSAAPI_Result	res;

		// Get the next data packet, sleep for some milliseconds, if none
		// available yet.

		while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &packet)) == AARTSAAPI_EMPTY)
			std::this_thread::sleep_for( std::chrono::milliseconds(1));

		// If we actually got a packet

		if (res == AARTSAAPI_OK)
		{
			if (numSamples == 0)
				startTicks = get_tick_count_ms();
			else
			{
				if (prevTime != packet.startTime)
					std::wcout << "Drop " << packet.startTime - prevTime << std::endl;
			}

			prevTime = packet.endTime;
			numPackets++;

			// Remove the first packet from the packet queue

			AARTSAAPI_ConsumePackets(&d, 0, 1);

			if (numPackets % 1000 == 0)
			{
				uint64_t		timeTicks = get_tick_count_ms();
				double	rate = double(numSamples) / double(timeTicks - startTicks) * 1000;
				std::wcout << "Samples : " << numSamples << " Millis : " << timeTicks - startTicks << " Rate " << std::setprecision(12) << rate << " err " << std::setprecision(6) << (rate - 92.16e6) / 92.16e4 << "%" << " (" << 2000.0 / (timeTicks - startTicks) << "%)" << std::endl;
			}

			numSamples += packet.num;
		}
	}

}

int main()
{
	if (LoadRTSAAPI_with_searchpath() != 0)
	{
		std::wcerr << "Load RTSSAPI failed";
		return - 1; 
	}

	AARTSAAPI_Result	res;

	// Initialize library for medium memory usage

	if ((res = AARTSAAPI_Init_With_Path(AARTSAAPI_MEMORY_MEDIUM, CFG_AARONIA_XML_LOOKUP_DIRECTORY)) == AARTSAAPI_OK)
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
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in raw mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/raw", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Select IQ as output format

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/outputformat") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"iq");

							// Use slow receiver clock

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"92MHz");
#if 0
							// Set decimation to 1/64

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/decimation") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"1 / 64");

							// could have also used
							// AARTSAAPI_ConfigSetInteger(&d, &config, 6);
#endif
							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Receive some spectra

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
