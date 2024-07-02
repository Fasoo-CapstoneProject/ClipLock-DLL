#include "crypto.h"
#include <stdexcept>
#include <stdexcept>
#include <vector>
#include <string>

#include <cryptopp890/cryptlib.h>
#include <cryptopp890/secblock.h>
#include <cryptopp890/aes.h>
#include <cryptopp890/modes.h>
#include <cryptopp890/osrng.h>
#include <cryptopp890/filters.h>

using namespace std;
using namespace CryptoPP;

// AES 대칭키 생성 함수
vector<BYTE> cGenerateAESKey() {
    SecByteBlock key(AES::MAX_KEYLENGTH);
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(key, key.size());
    return vector<BYTE>(key.begin(), key.end());
}

// IV(초기화 벡터) 생성 함수
vector<BYTE> cGenerateIV() {
    SecByteBlock iv(AES::BLOCKSIZE);
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(iv, iv.size());
    return vector<BYTE>(iv.begin(), iv.end());
}

// AES 암호화 함수 (CBC 모드)
vector<BYTE> cEncryptAES(const vector<BYTE>& plaintext, const vector<BYTE>& key, const vector<BYTE>& iv) {
    SecByteBlock keyBlock = SecByteBlock(key.data(), key.size());
    SecByteBlock ivBlock = SecByteBlock(iv.data(), iv.size());

    vector<BYTE> ciphertext;
    CBC_Mode<AES>::Encryption encryption;
    encryption.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock);

    StringSource ss(plaintext.data(), plaintext.size(), true,
        new StreamTransformationFilter(encryption,
            new VectorSink(ciphertext),
            StreamTransformationFilter::PKCS_PADDING
        )
    );

    return ciphertext;
}

// AES 복호화 함수 (CBC 모드)
vector<BYTE> cDecryptAES(const vector<BYTE>& ciphertext, const vector<BYTE>& key, const vector<BYTE>& iv) {
    SecByteBlock keyBlock = SecByteBlock(key.data(), key.size());
    SecByteBlock ivBlock = SecByteBlock(iv.data(), iv.size());

    vector<BYTE> plaintext;
    CBC_Mode<AES>::Decryption decryption;
    decryption.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock);

    StringSource ss(ciphertext.data(), ciphertext.size(), true,
        new StreamTransformationFilter(decryption,
            new VectorSink(plaintext),
            StreamTransformationFilter::PKCS_PADDING
        )
    );

    return plaintext;
}

// 유니코드 문자열을 바이트 배열로 변환
vector<BYTE> UnicodeToUtf8(const wstring& unicode_text) {
    if (unicode_text.empty()) {
        return vector<BYTE>();
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, unicode_text.c_str(), (int)unicode_text.size(), NULL, 0, NULL, NULL);
    string utf8_text(size_needed, 0); // 널 문자를 포함한 크기로 초기화
    WideCharToMultiByte(CP_UTF8, 0, unicode_text.c_str(), (int)unicode_text.size(), &utf8_text[0], size_needed, NULL, NULL);

    return vector<BYTE>(utf8_text.begin(), utf8_text.end());
}

// 바이트 배열을 유니코드 문자열로 변환
wstring Utf8ToUnicode(const vector<BYTE>& utf8_text) {
    if (utf8_text.empty()) {
        return wstring();
    }

    string str(utf8_text.begin(), utf8_text.end());
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    wstring unicode_text(size_needed, 0); // 널 문자를 포함한 크기로 초기화
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &unicode_text[0], size_needed);

    return unicode_text;
}
