#include <iostream>
#include <vector>
#include <string>
#include "Windows.h"
#include "detours.h"
#include "crypto.h"
#include "formatDebug.h"
#include "pngcrypt.h"
#include "util.h"

#define DLLBASIC_API extern "C" __declspec(dllexport)

using namespace std;

// Header 구조체
struct Header {
    BYTE key[32];           // 256bit 대칭키
    BYTE iv[16];            // IV(Initialize Vector)
    DWORD encryptedSize;    // 암호화된 크기(Byte 단위)
    DWORD width;
    DWORD height;
    DWORD leftOverSize;
};

struct PNGINFO {
    vector<BYTE> key;           // 256bit 대칭키
    vector<BYTE> iv;            // IV(Initialize Vector)
    DWORD encryptedSize;    // 암호화된 크기(Byte 단위)
    DWORD width;
    DWORD height;
    DWORD leftOverSize;
};

// Registered Clipboard Format
const char* RegisteredName = "MYFORMAT";
UINT CF_MYFORMAT;

// Header 메모리 공간 할당 및 초기화 함수
HGLOBAL AllocateHeader(const vector<BYTE>& key, const vector<BYTE>& iv, DWORD encryptedSize, DWORD width, DWORD height, DWORD leftOverSize);

// Header의 데이터를 가져오는 함수
bool LoadHeader(HANDLE hHeaderMem, Header& header);

// 암호화한 png를 새롭게 allocate하는 함수
HGLOBAL AllocEncPng(HGLOBAL hMem, PNGINFO& info);

// 암호화된 png를 복호화하는 함수
HGLOBAL DecPng(HGLOBAL hMem, Header header);

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
                vector<BYTE> key = cGenerateAESKey();
                vector<BYTE> iv = cGenerateIV();

                // AES 암호화
                vector<BYTE> encryptedText = cEncryptAES(plaintext, key, iv);

                // Header를 위한 공간 Allocate
                HGLOBAL pHeader = AllocateHeader(key, iv, (DWORD)encryptedText.size(), 0, 0, 0);
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
                //OutputDebugStringA(e.what());
            }
        } // if (pUniText != NULL)
    }
    // png 처리
    else if (uFormat == RegisterClipboardFormatA("PNG")) {
        if (hMem != NULL) {
            PNGINFO info;

            // 암호화한 png를 새롭게 allocate하는 함수
            HGLOBAL hEncPng = AllocEncPng(hMem, info);

            HGLOBAL pHeader = AllocateHeader(info.key, info.iv, info.encryptedSize, info.width, info.height, info.leftOverSize);

            // 클립보드에 헤더 넣기
            TrueSetClipboardData(CF_MYFORMAT, pHeader);

            // 클립보드에 원래 형식으로 암호화된 데이터 넣기
            return TrueSetClipboardData(uFormat, hEncPng);
        }

        // 클립보드에 NULL 헤더 넣기
        TrueSetClipboardData(CF_MYFORMAT, NULL);
    }

    return TrueSetClipboardData(uFormat, hMem);
}

DLLBASIC_API HANDLE WINAPI MyGetClipboardData(UINT uFormat)
{
    // 유니코드 형식이면서, 클립보드에 CF_MYFORMAT 형식이 있는 경우
    if (IsClipboardFormatAvailable(CF_MYFORMAT)) {

        Header header = { 0 };
        if (uFormat == CF_UNICODETEXT) {

            // CF_MYFORMAT 형식의 클립보드 데이터 가져오기
            HANDLE hHeaderMem = TrueGetClipboardData(CF_MYFORMAT);
            if (LoadHeader(hHeaderMem, header)) {

                // 유니코드 형식의 클립보드 데이터 가져오기
                HANDLE hMem = TrueGetClipboardData(CF_UNICODETEXT);

                if (hMem != NULL) {
                    BYTE* pEncryptedText = (BYTE*)GlobalLock(hMem);

                    if (pEncryptedText != NULL) {
                        vector<BYTE> encryptedText(pEncryptedText, pEncryptedText + header.encryptedSize);
                        GlobalUnlock(hMem);

                        try {
                            // header에서 대칭키, IV 불러오기
                            vector<byte> key(header.key, header.key + sizeof(header.key));
                            vector<byte> iv(header.iv, header.iv + sizeof(header.iv));

                            // 대칭키와 IV를 사용해서 복호화
                            vector<BYTE> decryptedText = cDecryptAES(encryptedText, key, iv);

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
                            //OutputDebugStringA(e.what());
                        }
                    }
                }
            }
        }
        // png 처리
        else if (uFormat == RegisterClipboardFormatA("PNG")) {

            // PNG 포맷의 클립보드 데이터 가져오기
            HANDLE hMem = TrueGetClipboardData(RegisterClipboardFormatA("PNG"));
            if (hMem != NULL) {

                // CF_MYFORMAT 형식의 클립보드 데이터 가져오기
                HANDLE hHeaderMem = TrueGetClipboardData(CF_MYFORMAT);
                if (LoadHeader(hHeaderMem, header)) {
                    return DecPng(hMem, header);
                }
            }
        }
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
        // sprintf_s(buffer, sizeof(buffer), "My Format : %d\n", CF_MYFORMAT);
        // OutputDebugStringA(buffer);

        DisableThreadLibraryCalls(hModule);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueSetClipboardData, MySetClipboardData);
        DetourAttach(&(PVOID&)TrueGetClipboardData, MyGetClipboardData);
        lError = DetourTransactionCommit();
        if (lError != NO_ERROR) {
            // MessageBox(HWND_DESKTOP, L"Could not add detour", L"Detour Error", MB_OK);
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
HGLOBAL AllocateHeader(const vector<BYTE>& key, const vector<BYTE>& iv, DWORD encryptedSize, DWORD width, DWORD height, DWORD leftOverSize)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(Header));
    if (hMem != NULL) {

        Header* pHeader = (Header*)GlobalLock(hMem);
        if (pHeader != NULL) {
            // 구조체의 key, iv, encryptedSize 값 할당해주기
            memcpy(pHeader->key, key.data(), key.size());
            memcpy(pHeader->iv, iv.data(), iv.size());
            pHeader->encryptedSize = encryptedSize;
            pHeader->width = width;
            pHeader->height = height;
            pHeader->leftOverSize = leftOverSize;
            GlobalUnlock(pHeader);
            return hMem;
        }
    }

    return NULL;
}

// Header의 데이터를 가져오는 함수
bool LoadHeader(HANDLE hHeaderMem, Header& header)
{
    if (hHeaderMem != NULL) {
        Header* pHeader = (Header*)GlobalLock(hHeaderMem);

        if (pHeader != NULL) {
            // 구조체 복사
            memcpy(&header, pHeader, sizeof(Header));
            GlobalUnlock(pHeader);

            return true;
        }
    }

    return false;
}

HGLOBAL AllocEncPng(HGLOBAL hMem, PNGINFO& info) {

    BYTE* pData = (BYTE*)GlobalLock(hMem);
    SIZE_T dataSize = GlobalSize(hMem);

    // 압축 해제, 디필터링
    int width, height;
    std::vector<unsigned char> imageData = read_png_from_memory(pData, dataSize, width, height);

    GlobalUnlock(hMem);

    // AES 대칭키 및 IV 생성
    vector<BYTE> key = cGenerateAESKey();
    vector<BYTE> iv = cGenerateIV();

    // AES 암호화
    vector<BYTE> encryptedRGB = cEncryptAES(imageData, key, iv);

    std::pair<int, int> pair = find_factors((int)encryptedRGB.size() / 4);

    // rgb 사이즈를 정하고 부족한 부분을 더미 픽셀로 채운다.
    int leftOverSize = pair.first * pair.second - (int)encryptedRGB.size() / 4;
    for (int i = 0; i < leftOverSize * 4; i++) {
        encryptedRGB.push_back(0xff);
    }

    // info 채우기 (header alloc을 위한)
    info.key = key;
    info.iv = iv;
    info.width = width;
    info.height = height;
    info.encryptedSize = encryptedRGB.size();
    info.leftOverSize = leftOverSize;


    // 필터링, 압축
    SIZE_T pngSize;
    char* encryptedPng = create_png_from_rgb(encryptedRGB, pair.first, pair.second, pngSize); // rgb 잘 추출 이미지 잘 나옴 -> 빠진 거 (보조청크)

    // 글로벌 메모리 alloc
    HGLOBAL hNewMem = GlobalAlloc(GMEM_MOVEABLE, (pngSize) * sizeof(char));
    if (hNewMem != NULL) {
        char* newBuffer = (char*)GlobalLock(hNewMem);

        if (newBuffer != NULL) {
            memcpy(newBuffer, encryptedPng, pngSize);
            GlobalUnlock(hNewMem);
        }
    }

    return hNewMem;
}

HGLOBAL DecPng(HGLOBAL hMem, Header header) {
    BYTE* pData = (BYTE*)GlobalLock(hMem);

    if (pData != NULL) {
        // 메모리에서 png 읽기  
        SIZE_T dataSize = GlobalSize(hMem);

        int _width, _height;
        std::vector<unsigned char> imageData = read_png_from_memory(pData, dataSize, _width, _height);

        // 더미 픽셀 제거
        imageData.erase(imageData.end() - 4 * header.leftOverSize, imageData.end());

        GlobalUnlock(hMem);

        try {
            // 대칭키와 IV를 사용해서 복호화
            vector<byte> vKey(header.key, header.key + sizeof(header.key));
            vector<byte> vIv(header.iv, header.iv + sizeof(header.iv));
            vector<BYTE> decryptedPng = cDecryptAES(imageData, vKey, vIv);

            //png에서 메모리에 쓸 수 있도록 byte로 변환
            SIZE_T pngSize;
            char* originalPng = create_png_from_rgb(decryptedPng, header.width, header.height, pngSize);

            HGLOBAL hOriginalPng = GlobalAlloc(GMEM_MOVEABLE, (pngSize) * sizeof(char));
            if (hOriginalPng != NULL) {
                char* newBuffer = (char*)GlobalLock(hOriginalPng);

                if (newBuffer != NULL) {
                    memcpy(newBuffer, originalPng, pngSize);
                    GlobalUnlock(hOriginalPng);
                    return hOriginalPng;
                }
            }
        }
        catch (const exception& e) {
            // 오류 메시지 출력
            /*OutputDebugStringA("복호화 실패");
            OutputDebugStringA(e.what());*/
        }
    }

    return hMem;
}