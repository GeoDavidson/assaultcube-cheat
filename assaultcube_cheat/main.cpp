#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>

DWORD getModuleBaseAddress(const wchar_t* lpszModuleName, DWORD pID) {
    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID); // make snapshot of all modules within process
    MODULEENTRY32 ModuleEntry32 = { 0 };
    ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &ModuleEntry32)) // store first module in ModuleEntry32
    {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if found fodule matches fodule we look for -> done!
            {
                dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in snapshot and store in ModuleEntry32
    }
    CloseHandle(hSnapshot);
    return dwModuleBaseAddress;
}

DWORD getPointerAddress(HWND hwnd, DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets)
{
    DWORD pID = NULL; // game process ID
    GetWindowThreadProcessId(hwnd, &pID);
    HANDLE phandle = NULL;
    phandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (phandle == INVALID_HANDLE_VALUE || phandle == NULL);

    DWORD offset_null = NULL;
    ReadProcessMemory(phandle, (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
    DWORD pointeraddress = offset_null; // the address we need
    for (int i = 0; i < offsets.size() - 1; i++) // we dont want to change the last offset value so we do -1
    {
        ReadProcessMemory(phandle, (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
    }
    return pointeraddress += offsets.at(offsets.size() - 1); // adding the last offset
}

int main()
{
	HWND windowHandle = FindWindowA(NULL, ("AssaultCube"));
	if (windowHandle == NULL)
	{
		std::cout << "error: windowHandle returned NULL";
		return EXIT_FAILURE;
	}

    DWORD processID;
    GetWindowThreadProcessId(windowHandle, &processID);

	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (processHandle == 0)
	{
		std::cout << "error: processHandle returned 0\n";
        return EXIT_FAILURE;
	}

    uintptr_t baseAddress = getModuleBaseAddress(L"ac_client.exe", processID);

    DWORD ammoAddress = 0x00183828;
    std::vector<DWORD> ammoOffsets{0x8, 0x748, 0x30, 0x8F4};

    DWORD ammoPointerAddress = getPointerAddress(windowHandle, baseAddress, ammoAddress, ammoOffsets);

    std::cout << "windowHandle: " << windowHandle << "\n";
    std::cout << "processID: " << processID << "\n";
    std::cout << "processHandle: " << processHandle << "\n";
    std::cout << "baseAddress: " << baseAddress << "\n";
    std::cout << "ammoAddress: " << ammoAddress << "\n";
    std::cout << "ammoPointerAddress: " << ammoPointerAddress << "\n";

    while (true)
    {
        int ammo = 1337;
        WriteProcessMemory(processHandle, (LPVOID*)(ammoPointerAddress), &ammo, 4, 0);
    }

	return EXIT_SUCCESS;
}
