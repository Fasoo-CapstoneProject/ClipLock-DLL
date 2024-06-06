#ifndef CRYPTO_H
#define CRYPTO_H

#include <vector>
#include <string>
#include "Windows.h"

using namespace std;

// AES 대칭키 생성 함수
vector<BYTE> GenerateAESKey();

// // IV(초기화 벡터) 생성 함수
vector<BYTE> GenerateIV();

// AES 암호화 함수 (CBC 모드)
vector<BYTE> EncryptAES(const vector<BYTE>& plaintext, const vector<BYTE>& key, const vector<BYTE>& iv);

// AES 복호화 함수 (CBC 모드)
vector<BYTE> DecryptAES(const vector<BYTE>& ciphertext, const vector<BYTE>& key, const vector<BYTE>& iv);

// 유니코드 문자열을 바이트 배열로 변환
vector<BYTE> UnicodeToUtf8(const wstring& unicode_text);

// 바이트 배열을 유니코드 문자열로 변환
wstring Utf8ToUnicode(const std::vector<BYTE>& utf8_text);

#endif // CRYPTO_H