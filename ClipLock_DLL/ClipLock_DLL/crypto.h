#ifndef CRYPTO_H
#define CRYPTO_H

#include <vector>
#include <string>
#include "Windows.h"

using namespace std;

// AES ��ĪŰ ���� �Լ�
vector<BYTE> GenerateAESKey();

// // IV(�ʱ�ȭ ����) ���� �Լ�
vector<BYTE> GenerateIV();

// AES ��ȣȭ �Լ� (CBC ���)
vector<BYTE> EncryptAES(const vector<BYTE>& plaintext, const vector<BYTE>& key, const vector<BYTE>& iv);

// AES ��ȣȭ �Լ� (CBC ���)
vector<BYTE> DecryptAES(const vector<BYTE>& ciphertext, const vector<BYTE>& key, const vector<BYTE>& iv);

// �����ڵ� ���ڿ��� ����Ʈ �迭�� ��ȯ
vector<BYTE> UnicodeToUtf8(const wstring& unicode_text);

// ����Ʈ �迭�� �����ڵ� ���ڿ��� ��ȯ
wstring Utf8ToUnicode(const std::vector<BYTE>& utf8_text);

#endif // CRYPTO_H