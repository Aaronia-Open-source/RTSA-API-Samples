#include <aaroniartsaapi.h>

#include <iostream>
#include <string>

void TreeConfig(AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config group);

void ItemConfig(AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config config)
{
	AARTSAAPI_ConfigInfo	cinfo = { sizeof(AARTSAAPI_ConfigInfo) };
	wchar_t					str[1024];

	AARTSAAPI_ConfigGetInfo(&d, &config, &cinfo);
	int64_t	ssize = sizeof(str);
	AARTSAAPI_ConfigGetString(&d, &config, str, &ssize);

	std::wcout << prefix << cinfo.name << L"(" << cinfo.title << L", " << cinfo.unit << L", " << cinfo.options << L"), " << L" : \"" << str << "\"" << std::endl;

	if (cinfo.type == AARTSAAPI_CONFIG_TYPE_GROUP)
	{
		TreeConfig(d, prefix + L". ", config);
	}
}

void TreeConfig(AARTSAAPI_Device d, std::wstring prefix, AARTSAAPI_Config group)
{
	AARTSAAPI_Config	config;

	if (AARTSAAPI_ConfigFirst(&d, &group, &config) == AARTSAAPI_OK)
	{
		do {
			ItemConfig(d, prefix, config);
		} while (AARTSAAPI_ConfigNext(&d, &group, &config) == AARTSAAPI_OK);
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
						AARTSAAPI_Config	root;

						// Begin configuration, get root of configuration tree
						std::wcout << "CONFIG:" << std::endl;
						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							TreeConfig(d, L"", root);
						}
						
						// Also display "health" config tree as it contains important data
						std::wcout << std::endl << "STATUS:" << std::endl;
						if (AARTSAAPI_ConfigHealth(&d, &root) == AARTSAAPI_OK)
						{
							TreeConfig(d, L"", root);
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
