#include "Windows.h"
#include "detours.h"
#include "formatDebug.h"
#include "pngcrypt.h"

#define DLLBASIC_API extern "C" __declspec(dllexport)

// Target Pointer
static HANDLE(WINAPI* TrueSetClipboardData)(UINT uFormat, HANDLE hMem) = SetClipboardData;
static HANDLE(WINAPI* TrueGetClipboardData)(UINT uFormat) = GetClipboardData;

DLLBASIC_API HANDLE WINAPI MySetClipboardData(UINT uFormat, HANDLE hMem)
{
    GetClipboardFormat(uFormat, formatName, sizeof(formatName));
    sprintf_s(buffer, sizeof(buffer), "Set Format num : 0x%x, name : %s\n", uFormat, formatName);
    OutputDebugStringA(buffer);

    return TrueSetClipboardData(uFormat, hMem);
}

DLLBASIC_API HANDLE WINAPI MyGetClipboardData(UINT uFormat)
{
    GetClipboardFormat(uFormat, formatName, sizeof(formatName));
    sprintf_s(buffer, sizeof(buffer), "Get Format num : 0x%x, name : %s\n", uFormat, formatName);
    OutputDebugStringA(buffer);

    return TrueGetClipboardData(uFormat);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    LONG lError = 0;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // png reading test
        PngParseTest();

        DisableThreadLibraryCalls(hModule);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueSetClipboardData, MySetClipboardData);
        DetourAttach(&(PVOID&)TrueGetClipboardData, MyGetClipboardData);
        lError = DetourTransactionCommit();
        if (lError != NO_ERROR) {
            MessageBox(HWND_DESKTOP, L"Could not add detour", L"Detour Error", MB_OK);
            return FALSE;
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueSetClipboardData, MySetClipboardData);
        DetourDetach(&(PVOID&)TrueGetClipboardData, MyGetClipboardData);
        DetourTransactionCommit();
        break;
    }

    return TRUE;
}