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

	// Prepare data packet
	AARTSAAPI_Packet	packets[2] = { { sizeof(AARTSAAPI_Packet) }, { sizeof(AARTSAAPI_Packet) } };
	int					pi[2] = { 0, 0 };

	AARTSAAPI_Result	res;

	// Get the first data packets for both streams, sleep for some milliseconds, if none
	// available yet.

	for (int j = 0; j < 2; j++)
	{
		while ((res = AARTSAAPI_GetPacket(&d, j, 0, packets + j)) == AARTSAAPI_EMPTY)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

		if (res != AARTSAAPI_OK)
			return;
	}

	// Stream for 10 total packets (5 each)

	for (int i = 0; i < 10; i++)
	{
		// Synchronize streams by time stamp

		if (packets[0].endTime < packets[1].startTime)
			pi[0] = packets[0].num;
		else if (packets[1].endTime < packets[0].startTime)
			pi[1] = packets[1].num;
		else if (packets[0].startTime < packets[1].startTime)
		{
			pi[1] = 0;
			pi[0] = (packets[1].startTime - packets[0].startTime) * packets[0].stepFrequency;
		}
		else
		{
			pi[0] = 0;
			pi[1] = (packets[0].startTime - packets[1].startTime) * packets[0].stepFrequency;
		}

		while (pi[0] < packets[0].num && pi[1] < packets[1].num)
		{
			// Prepare grid lines

			buff[0] = '|';
			buff[20] = '|';
			buff[10] = '.';
			buff[30] = '.';
			buff[40] = '|';

			buff[60] = '|';
			buff[50] = '.';
			buff[70] = '.';
			buff[80] = '|';

			int	ki[2], kq[2];
			for (int k = 0; k < 2; k++)
			{

				// Use 1mV for full size

				ki[k] = int(packets[k].fp32[2 * pi[k] + 0] * 20 * 1000);
				kq[k] = int(packets[k].fp32[2 * pi[k] + 1] * 20 * 1000);

				if (ki[k] >= -20 && ki[k] <= 20)
					buff[ki[k] + 20 + 40 * k] = 'I';
				if (kq[k] >= -20 && kq[k] <= 20)
					buff[kq[k] + 20 + 40 * k] = 'Q';
			}

			std::wcout << buff << std::endl;

			for (int k = 0; k < 2; k++)
			{
				if (ki[k] >= -20 && ki[k] <= 20)
					buff[ki[k] + 20 + 40 * k] = ' ';
				if (kq[k] >= -20 && kq[k] <= 20)
					buff[kq[k] + 20 + 40 * k] = ' ';

				pi[k]++;
			}
		}

		// Get fresh packet

		if (pi[0] >= packets[0].num)
		{
			// Remove the first packet from the packet queue
			AARTSAAPI_ConsumePackets(&d, 0, 1);

			// Get next packet
			while ((res = AARTSAAPI_GetPacket(&d, 0, 0, packets + 0)) == AARTSAAPI_EMPTY)
                std::this_thread::sleep_for(std::chrono::milliseconds(5));

			pi[0] = 0;

			if (res != AARTSAAPI_OK)
				return;
		}
		
		if (pi[1] >= packets[1].num)
		{
			// Remove the first packet from the packet queue
			AARTSAAPI_ConsumePackets(&d, 1, 1);

			// Get next packet
			while ((res = AARTSAAPI_GetPacket(&d, 1, 0, packets + 1)) == AARTSAAPI_EMPTY)
                std::this_thread::sleep_for(std::chrono::milliseconds(5));

			pi[1] = 0;

			if (res != AARTSAAPI_OK)
				return;
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
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1+Rx2");

							// Select IQ as output format

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/outputformat") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"iq");

							// Use slow receiver clock

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"92MHz");

							// Set decimation to 1/64

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/decimation") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"1 / 64");

							// could have also used
							// AARTSAAPI_ConfigSetInteger(&d, &config, 6);

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
