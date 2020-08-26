#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <conio.h>

#define UNUSED_RETURN_VALUE(x){ if(x){} }

#define USE_WIDECHAR_LOADLIBRARY

int main(int argc, char** argv) {

#ifdef _M_X64
	std::cout << "DLLInjection for amd64" << std::endl;
#else
	std::cout << "DLLInjection for ia32" << std::endl;
#endif
	std::cout << "**********************" << std::endl;

	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <PID> <DLLPath>" << std::endl;
		return -1;
	}

	{
		std::ifstream f(argv[2]);
		if (!f.good()) {
			std::cerr << "Unable to find \"" << argv[2] << "\" library" << std::endl;
			return -2;
		}
	}

	const unsigned int nProcessId = atoi(argv[1]);
	
#ifdef USE_WIDECHAR_LOADLIBRARY
	wchar_t szLibrary[MAX_PATH];
	{
		size_t numConverted;
		mbstowcs_s(&numConverted, szLibrary, argv[2], MAX_PATH);
	}
	const int nLibraryPathLength = (wcslen(szLibrary) + 1) * sizeof(WCHAR);
#else
	const char* szLibrary = argv[2];
	const int nLibraryPathLength = (strlen(szLibrary) + 1) * sizeof(char);
#endif

	int rc = 0;

	HANDLE hProcess = OpenProcess(
		PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION, FALSE, nProcessId);
	if (hProcess) {
		HMODULE hModKernel = GetModuleHandle(_T("kernel32.dll"));
		if (hModKernel) {
#ifdef USE_WIDECHAR_LOADLIBRARY
			FARPROC procLoadLibary = GetProcAddress(hModKernel, "LoadLibraryW");
#else
			FARPROC procLoadLibary = GetProcAddress(hModKernel, "LoadLibraryA");
#endif
			if (procLoadLibary) {

				LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL,
					nLibraryPathLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
				if (pRemoteMemory) {

					SIZE_T nBytesWritten;
					if (WriteProcessMemory(hProcess, pRemoteMemory, szLibrary,
						nLibraryPathLength, &nBytesWritten) &&
						(nBytesWritten == nLibraryPathLength)) {

						DWORD dwThreadId;
						HANDLE hThread = CreateRemoteThread(
							hProcess, NULL, 0,
							reinterpret_cast<LPTHREAD_START_ROUTINE>(procLoadLibary),
							pRemoteMemory, CREATE_SUSPENDED, &dwThreadId);

						if (hThread) {
							std::cout << "Thread ID " << dwThreadId << " ready to inject: Press any key to inject...";
							UNUSED_RETURN_VALUE(_getch());
							ResumeThread(hThread);

							WaitForSingleObject(hThread, INFINITE);
							DWORD dwExitCode = 0;
							if (GetExitCodeThread(hThread, &dwExitCode) && dwExitCode) {
								std::cout << "Injection routine was successful!" << std::endl;
							} else {
								std::cout << "Injection routine failed." << std::endl;
							}
							CloseHandle(hThread);

							std::cout << std::endl << "Finished injecting" << std::endl;
						} else {
							std::cerr << "Unable to create remote thread: " << GetLastError() << std::endl;
							rc = -8;
						}
					} else {
						std::cerr << "Unable to write process memory: " << GetLastError() << std::endl;
						rc = -7;
					}
					VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
				} else {
					std::cerr << "Unable to allocate virtual memory: " << GetLastError() << std::endl;
					rc = -6;
				}
			} else {
				std::cerr << "GetProcAddress failed for LoadLibrary: " << GetLastError() << std::endl;
				rc = -5;
			}
		} else {
			std::cerr << "Unable to load the kernel library: " << GetLastError() << std::endl;
			rc = -4;
		}
		CloseHandle(hProcess);
	} else {
		std::cerr << "Unable to open process ID " << nProcessId
			<< " for VM operations: " << GetLastError() << std::endl;
		rc = -3;
	}

	return rc;
}

