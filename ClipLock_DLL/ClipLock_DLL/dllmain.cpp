#include "Windows.h"
#include "detours.h"
#include "formatDebug.h"
#include "pngcrypt.h"

#define DLLBASIC_API extern "C" __declspec(dllexport)

UINT costumFormat;

// Target Pointer
static HANDLE(WINAPI* TrueSetClipboardData)(UINT uFormat, HANDLE hMem) = SetClipboardData;
static HANDLE(WINAPI* TrueGetClipboardData)(UINT uFormat) = GetClipboardData;

DLLBASIC_API HANDLE WINAPI MySetClipboardData(UINT uFormat, HANDLE hMem)
{
    GetClipboardFormat(uFormat, formatName, sizeof(formatName));
    sprintf_s(buffer, sizeof(buffer), "Set Format num : 0x%x, name : %s\n", uFormat, formatName);
    OutputDebugStringA(buffer);

    // png 처리
    if (uFormat == 0xc17d) {
        if (hMem != NULL) {
            BYTE* pData = (BYTE*)GlobalLock(hMem);
            SIZE_T dataSize = GlobalSize(hMem);

            // 압축 해제, 디필터링
            int width, height;
            std::vector<unsigned char> imageData = read_png_from_memory(pData, dataSize, width, height);
            GlobalUnlock(hMem);

            // xor 암호화
            xorPng(imageData);

            // 필터링, 압축
            SIZE_T pngSize;
            char* encryptedPng = create_png_from_rgb(imageData, width, height, pngSize);

            HGLOBAL hNewMem = GlobalAlloc(GMEM_MOVEABLE, (pngSize + 1) * sizeof(char));
            if (hNewMem != NULL) {
                char* newBuffer = (char*)GlobalLock(hNewMem);
                if (newBuffer != NULL) {
                    memcpy(newBuffer, encryptedPng, pngSize + 1);
                    GlobalUnlock(hNewMem);
                    OutputDebugStringA("png encryption success");

                    // 클립보드에 넣기
                    TrueSetClipboardData(costumFormat, hNewMem);
                    return TrueSetClipboardData(uFormat, hNewMem);
                }
            }      
        }
    }

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
        // 새로운 format 등록
        costumFormat = RegisterClipboardFormatA("costumPng");

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