#pragma once
#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>

#define code_read_physical CTL_CODE(FILE_DEVICE_UNKNOWN, 0x316, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_read_virtual CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_write CTL_CODE(FILE_DEVICE_UNKNOWN, 0x461, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_get_base_address CTL_CODE(FILE_DEVICE_UNKNOWN, 0x135, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_decrypt_cr3 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x136, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_move_mouse CTL_CODE(FILE_DEVICE_UNKNOWN, 0x619, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_hide_process CTL_CODE(FILE_DEVICE_UNKNOWN, 0x618, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_restart_driver CTL_CODE(FILE_DEVICE_UNKNOWN, 0x361, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _rw {
	INT32 process_id;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
} rw, * WriteStruct;


typedef struct _r {
	INT32 process_id;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
} rwr, * ReadStruct;


typedef struct _ba {
	INT32 process_id;
	ULONGLONG* address;
} ba, * BaseAddressStruct;

typedef struct _cr3 {
	INT32 process_id;
} cr3, * DTBStruct;

typedef struct _restart {
	bool bResetCr3;
} restart, * RestartStruct;

typedef struct _movemouse {
	long x;
	long y;
	unsigned short button_flags;
} movemouse, * MouseMovementStruct;

typedef struct _hf {
	INT32 process_id;
} hidefile, * HideFileStruct;

inline namespace mem {
	inline HANDLE driver_handle;
	inline INT32 process_id;

	inline bool VirtualMode;

	inline bool SetupDriver();

	inline bool read(PVOID address, PVOID buffer, DWORD size);

	inline bool write_physical(PVOID address, PVOID buffer, DWORD size);

	inline uintptr_t get_base_address(const char* process_name);

	inline uintptr_t Attach(LPCTSTR process_name);

	inline void bShouldUseVirtualMode(bool mode);

	inline bool GetDTB();

	inline bool RestartBuffers();

	inline bool KernelMouseMovement(int x, int y);

	inline bool HideProcess(INT32 pid);

	inline BOOL __stdcall DeviceIoControl_Spoofed(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);

	inline BOOL __stdcall CloseHandleA_Spoofed(HANDLE hObject);
}

class Kern
{
public:
	template <typename T>
	inline T read(uint64_t address) {
		T buffer{ };
		mem::read((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}

	template <typename T>
	inline T write(uint64_t address, T buffer) {
		mem::write_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}

	inline bool read_array(const std::uintptr_t address, void* buffer, const std::size_t size)
	{
		if (buffer == nullptr || size == 0) {
			return false;
		}

		mem::read(reinterpret_cast<PVOID>(address), buffer, static_cast<DWORD>(size));

	}
}; inline Kern* request;

inline bool mem::HideProcess(INT32 pid)
{
	_hf arguments = { NULL };

	arguments.process_id = pid;

	return DeviceIoControl_Spoofed(driver_handle, code_hide_process, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

inline void mem::bShouldUseVirtualMode(bool mode)
{
	VirtualMode = mode;
}


inline uintptr_t mem::get_base_address(const char* process_name) {
	uintptr_t image_address = { NULL };
	_ba arguments = { NULL };

	arguments.process_id = process_id;
	arguments.address = (ULONGLONG*)&image_address;

	DeviceIoControl_Spoofed(driver_handle, code_get_base_address, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

	return image_address;
}

inline bool mem::write_physical(PVOID address, PVOID buffer, DWORD size) {
	_rw arguments = { 0 };

	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = process_id;

	return DeviceIoControl_Spoofed(driver_handle, code_write, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

inline bool mem::KernelMouseMovement(int x, int y) {
	_movemouse arguments = { 0 };

	arguments.x = (long)y;
	arguments.y = (long)x;

	return DeviceIoControl_Spoofed(driver_handle, code_move_mouse, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

inline bool mem::read(PVOID address, PVOID buffer, DWORD size) {
	_r arguments = { 0 };
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = process_id;


	if (VirtualMode)
	{
		return DeviceIoControl_Spoofed(driver_handle, code_read_virtual, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}
	else {
		return DeviceIoControl_Spoofed(driver_handle, code_read_physical, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}
}

inline uintptr_t mem::Attach(LPCTSTR process_name) {
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, process_name)) {
				CloseHandle(hsnap);
				process_id = pt.th32ProcessID;
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);

	return { NULL };
}

inline bool mem::SetupDriver() {
	auto DriverName = ("\\\\.\\\{1351351511513-3151351351135-13566351-3612351335}");

	driver_handle = CreateFileA(DriverName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);


	if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
		return false;

	return true;
}

inline bool mem::GetDTB()
{
	_cr3 arguments = { NULL };

	arguments.process_id = process_id;

	return DeviceIoControl_Spoofed(driver_handle, code_decrypt_cr3, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

inline bool mem::RestartBuffers() {
	_restart arguments = { NULL };

	arguments.bResetCr3 = true;

	return DeviceIoControl_Spoofed(driver_handle, code_restart_driver, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}