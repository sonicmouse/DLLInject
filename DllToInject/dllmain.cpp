#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

DWORD CALLBACK ThreadProc(LPVOID param)
{
    TCHAR szPath[MAX_PATH], msg[MAX_PATH * 2];

    GetModuleFileName(NULL, szPath, sizeof(szPath) / sizeof(TCHAR));
    _stprintf_s(msg, sizeof(msg) / sizeof(TCHAR), _T("We have injected in to the process:\n%s"), szPath);

    return MessageBox(NULL, msg, _T("HOORAY"), MB_ICONINFORMATION | MB_SYSTEMMODAL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved) {

    switch (dwReasonForCall) {
        case DLL_PROCESS_ATTACH:
            {
                DisableThreadLibraryCalls(hModule);
                HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
                if (hThread) {
                    CloseHandle(hThread);
                }
            }
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}