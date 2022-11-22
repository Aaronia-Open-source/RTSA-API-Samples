#include <aaroniartsaapi.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

void streamIQ(AARTSAAPI_Device d)
{
	// Prepare output packet
	AARTSAAPI_Packet	packet = { sizeof(AARTSAAPI_Packet) };

	// Stream 1000 packets

	int NumPackets = 1000;

	for (int i = 0; i < NumPackets; i++)
	{
		AARTSAAPI_Result	res;

		// Get the next data packet, sleep for some milliseconds, if none
		// available yet.

		while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &packet)) == AARTSAAPI_EMPTY)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

		// If we actually got a packet

		if (res == AARTSAAPI_OK)
		{
			// Get the current system time

			double	streamTime;
			AARTSAAPI_GetMasterStreamTime(&d, streamTime);

			// Check if we are still close to the capture stream

			if (packet.startTime > streamTime - 0.1)
			{
				// Push packet into future for buffering

				packet.startTime += 0.2;
				packet.endTime += 0.2;

				// New center frequency

				packet.startFrequency = 2450.0e6 - 0.5 * packet.stepFrequency;

				// Send packet to transmitter queue

				AARTSAAPI_SendPacket(&d, 0, &packet);
			}
		}

		// Consume the packet from the receiver queue

		AARTSAAPI_ConsumePackets(&d, 0, 1);
	}
}

int main()
{
	AARTSAAPI_Result	res;

	// Initialize library for large memory usage

	if ((res = AARTSAAPI_Init(AARTSAAPI_MEMORY_LARGE)) == AARTSAAPI_OK)
	{

		// Open a library handle for use by this application

		AARTSAAPI_Handle	h;

		if ((res = AARTSAAPI_Open(&h)) == AARTSAAPI_OK)
		{
			// Rescan all devices controlled by the aaronia library and update
			// the firmware if required.

			if ((res = AARTSAAPI_RescanDevices(&h, 2000)) == AARTSAAPI_OK)
			{
				AARTSAAPI_DeviceInfo	dinfo = { sizeof(AARTSAAPI_DeviceInfo) };

				// Get the serial number of the first V6 in the system

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in transmitter mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/iqtransceiver", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Select the center frequency of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2440.0e6);

							// Select the frequency range of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 50.0e6);

							// Select the frequency range of the receiver demodulator to pick up a
							// frequency range from the input stream

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodcenterfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2430.0e6);

							// Select the frequency span of the receiver demodulator

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodspanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2.0e6);

							// Select the transmitter gain

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/transgain") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 0.0);

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Wait for the transceiver running

									while (AARTSAAPI_GetDeviceState(&d) != AARTSAAPI_RUNNING)
									{
										std::wcout << ".";
										std::wcout.flush();
                                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
									}
									std::wcout << std::endl;

									// Send data to the transceiver

									streamIQ(d);
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
