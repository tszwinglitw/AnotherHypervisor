#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <string>
#include <intrin.h>
#include <Windows.h>


std::string getVendorName()
{
	std::array<int, 4> cpui;
	// Calling __cpuid with 0x0 as the function_id argument
	// gets the number of the highest valid function ID.

	// cpui[0] = eax, cpui[1] = ebx, cpui[2] = ecx, cpui[3] = edx
	__cpuid(cpui.data(), 0);
	std::string vendor_name;
	// What an interesting way to get the vendor name
	vendor_name += std::string((const char*)&cpui[1], 4);
	vendor_name += std::string((const char*)&cpui[3], 4);
	vendor_name += std::string((const char*)&cpui[2], 4);
	return vendor_name;
}

bool checkVirtualizationSupport()
{
	std::array<int, 4> cpui;
	// EAX = 1 to request the feature information
	__cpuid(cpui.data(), 1);
	// Check if the CPU has the virtualization: cpui[2] = ECX, bit 5
	return (cpui[2] & (1 << 5)) != 0;
}

int
main()
{
	if (getVendorName() == "GenuineIntel")
	{
		std::cout << "[*] Intel CPU detected" << std::endl;
	}
	else
	{
		std::cout << "[!] Only Intel CPUs are supported" << std::endl;
		return 1;
	}

	if (!checkVirtualizationSupport())
	{
		std::cout << "[!] Virtualization is not supported" << std::endl;
		return 1;
	}
	else
	{
		std::cout << "[*] Virtualization is supported" << std::endl;
	}



	HANDLE hHypervisor = CreateFile(L"\\\\.\\AnotherHypervisor",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,
		NULL, /// lpSecurityAttirbutes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL); /// lpTemplateFile

	if (hHypervisor == INVALID_HANDLE_VALUE)
	{
		std::cout << "[!] Failed to open the device" << std::endl;
		return 1;
	}
	else
	{
		std::cout << "[*] Device opened successfully" << std::endl;
	}


	int _x = getchar();

	return 0;
}