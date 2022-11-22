#include <aaroniartsaapi.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

void streamSpectra(AARTSAAPI_Device d)
{
	// ASCII art brightness levels

	static const wchar_t* hlevels = L"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

	// Test 1k spectra packets

	for (int i = 0; i < 1000; i++)
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
			const float* fp = packet.fp32;

			for (int s = 0; s < packet.num; s++)
			{
				wchar_t	buff[129];

				int	k = 0;
				for (int j = 0; j < 128; j++)
				{
					int	l = int(packet.size) * (j + 1) / 128;
					float	mv = -200.0;
					for (int n = k; n < l; n++)
						if (fp[n] > mv)
							mv = fp[n];
					k = l;

					int	 mi = -int(mv);
					if (mi < 0)
						mi = 0;
					else if (mi > 68)
						mi = 68;
					buff[j] = hlevels[mi];
				}

				buff[128] = 0;
				std::wcout << buff << "|" << std::endl;

				// Advance to next sample

				fp += packet.stride;
			}

			// Remove the first packet from the packet queue

			AARTSAAPI_ConsumePackets(&d, 0, 1);
		}
		else
			break;
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
					// Try to open the first V6 in the system in sweep mode

					AARTSAAPI_Device	d;

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/sweepsa", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Use fast receiver clock

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"245MHz");

							// Start frequency at 75MHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/startfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 75.0e6);

							// Stop frequency at 6000MHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/stopfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 6000.0e6);

							// Set RBW frequency to 1MHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/rbwfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 1.0e6);

							// Reference level at -20dBm

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/reflevel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, -20.0);

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Receive some spectra

									streamSpectra(d);

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
