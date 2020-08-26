#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ) {

    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            MessageBox(NULL, _T("WE ARE IN THE PROCESS"), _T("HOORAY"), MB_ICONQUESTION | MB_SYSTEMMODAL);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}