#include <iostream>
#include <vector>
#include <string>
#include "Windows.h"
#include "detours.h"
#include "crypto.h"
#include "formatDebug.h"

#define DLLBASIC_API extern "C" __declspec(dllexport)

using namespace std;

// Header 구조체
struct Header {
    BYTE key[32];           // 256bit 대칭키
    BYTE iv[16];            // IV(Initialize Vector)
    DWORD encryptedSize;    // 암호화된 크기(Byte 단위)
};

// Registered Clipboard Format
const char* RegisteredName = "CF_MYFORMAT";
UINT CF_MYFORMAT;

// Header 메모리 공간 할당 및 초기화 함수
HGLOBAL AllocateHeader(const vector<BYTE>& key, const vector<BYTE>& iv, DWORD encryptedSize);

// Target Pointer
static HANDLE(WINAPI* TrueSetClipboardData)(UINT uFormat, HANDLE hMem) = SetClipboardData;
static HANDLE(WINAPI* TrueGetClipboardData)(UINT uFormat) = GetClipboardData;

DLLBASIC_API HANDLE WINAPI MySetClipboardData(UINT uFormat, HANDLE hMem)
{
    // 유니코드 형식인 경우
    if (uFormat == CF_UNICODETEXT) {
        // 유니코드 데이터 가져오기
        wchar_t* pUniText = (wchar_t*)GlobalLock(hMem);

        if (pUniText != NULL) {
            // 유티코드를 BYTE로 변환하기
            vector<BYTE> plaintext = UnicodeToUtf8(pUniText);
            GlobalUnlock(hMem);

            try {
                // AES 대칭키 및 IV 생성
                vector<BYTE> key = GenerateAESKey();
                vector<BYTE> iv = GenerateIV();

                // AES 암호화
                vector<BYTE> encryptedText = EncryptAES(plaintext, key, iv);

                // Header를 위한 공간 Allocate
                HGLOBAL pHeader = AllocateHeader(key, iv, (DWORD)encryptedText.size());
                if (pHeader != NULL) {
                    // 메모장에서는 클립보드에 CF_UNICODETEXT 형식으로 데이터를 저장할 때, 마지막 2byte는 0으로 초기화를 무조건 하는 것 같음
                    HGLOBAL hEncryptedMem = GlobalAlloc(GMEM_MOVEABLE, encryptedText.size() + sizeof(wchar_t));
                    if (hEncryptedMem != NULL) {
                        BYTE* encryptedBuffer = (BYTE*)GlobalLock(hEncryptedMem);
                        if (encryptedBuffer != NULL) {
                            memcpy(encryptedBuffer, encryptedText.data(), encryptedText.size());
                            GlobalUnlock(hEncryptedMem);

                            // 유니코드 형식, 등록된 형식 2개 다 클립보드에 넣기
                            TrueSetClipboardData(CF_MYFORMAT, pHeader);

                            return TrueSetClipboardData(uFormat, hEncryptedMem);
                        }
                    }
                }
            }
            catch (const exception& e) {
                // 오류 메세지 출력
                OutputDebugStringA(e.what());
            }
        } // if (pUniText != NULL)
    }

    return TrueSetClipboardData(uFormat, hMem);
}

DLLBASIC_API HANDLE WINAPI MyGetClipboardData(UINT uFormat)
{
    // 유니코드 형식이면서, 클립보드에 CF_MYFORMAT 형식이 있는 경우
    if (uFormat == CF_UNICODETEXT && IsClipboardFormatAvailable(CF_MYFORMAT)) {
        vector<BYTE> key(32);
        vector<BYTE> iv(16);
        DWORD encryptedSize = 0;

        // CF_MYFORMAT 형식의 클립보드 데이터 가져오기
        HANDLE hHeaderMem = TrueGetClipboardData(CF_MYFORMAT);
        if (hHeaderMem != NULL) {
            Header* pHeader = (Header*)GlobalLock(hHeaderMem);

            if (pHeader != NULL) {
                // Key값과 IV값 가져오기
                memcpy(key.data(), pHeader->key, key.size());
                memcpy(iv.data(), pHeader->iv, iv.size());
                encryptedSize = pHeader->encryptedSize;
                GlobalUnlock(pHeader);
            }
        }

        if (!key.empty() && !iv.empty() && encryptedSize > 0) {
            // 유니코드 형식의 클립보드 데이터 가져오기
            HANDLE hMem = TrueGetClipboardData(CF_UNICODETEXT);

            if (hMem != NULL) {
                BYTE* pEncryptedText = (BYTE*)GlobalLock(hMem);

                if (pEncryptedText != NULL) {
                    vector<BYTE> encryptedText(pEncryptedText, pEncryptedText + encryptedSize);
                    GlobalUnlock(hMem);

                    try {
                        // 대칭키와 IV를 사용해서 복호화
                        vector<BYTE> decryptedText = DecryptAES(encryptedText, key, iv);

                        // BYTE를 유니코드로 변환하기
                        wstring decryptedTextWstr = Utf8ToUnicode(decryptedText);

                        HGLOBAL hDecryptedMem = GlobalAlloc(GMEM_MOVEABLE, (decryptedTextWstr.size() + 1) * sizeof(wchar_t));
                        if (hDecryptedMem != NULL) {
                            wchar_t* decryptedBuffer = (wchar_t*)GlobalLock(hDecryptedMem);

                            if (decryptedBuffer != NULL) {
                                wcscpy_s(decryptedBuffer, decryptedTextWstr.size() + 1, decryptedTextWstr.c_str());
                                GlobalUnlock(hDecryptedMem);

                                return hDecryptedMem;
                            }
                        }
                    }
                    catch (const exception& e) {
                        // 오류 메시지 출력
                        OutputDebugStringA(e.what());
                    }
                }
            }
        } // if(!key.empty() && !iv.empty() && encryptedSize > 0)
    }

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
        CF_MYFORMAT = RegisterClipboardFormatA(RegisteredName);
        sprintf_s(buffer, sizeof(buffer), "My Format : %d\n", CF_MYFORMAT);
        OutputDebugStringA(buffer);

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

// Header 메모리 공간 할당 및 초기화 함수
HGLOBAL AllocateHeader(const vector<BYTE>& key, const vector<BYTE>& iv, DWORD encryptedSize)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(Header));
    if (hMem != NULL) {

        Header* pHeader = (Header*)GlobalLock(hMem);
        if (pHeader != NULL) {
            // 구조체의 key, iv, encryptedSize 값 할당해주기
            memcpy(pHeader->key, key.data(), key.size());
            memcpy(pHeader->iv, iv.data(), iv.size());
            pHeader->encryptedSize = encryptedSize;
            GlobalUnlock(pHeader);
            return hMem;
        }
    }

    return NULL;
}