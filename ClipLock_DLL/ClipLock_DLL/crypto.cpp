#include "crypto.h"
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
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

/*
// AES 대칭키 생성 함수
vector<BYTE> GenerateAESKey() {
    vector<BYTE> key(32); // AES-256을 위한 32바이트 키
    if (!RAND_bytes(key.data(), key.size())) {
        throw runtime_error("Key generation failed");
    }
    return key;
}

// IV(초기화 벡터) 생성 함수
vector<BYTE> GenerateIV() {
    vector<BYTE> iv(EVP_MAX_IV_LENGTH); // IV는 블록 크기와 동일한 길이
    if (!RAND_bytes(iv.data(), iv.size())) {
        throw runtime_error("IV generation failed");
    }
    return iv;
}

// AES 암호화 함수 (CBC 모드)
vector<BYTE> EncryptAES(const vector<BYTE>& plaintext, const vector<BYTE>& key, const vector<BYTE>& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw runtime_error("EVP_CIPHER_CTX_new failed");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_EncryptInit_ex failed");
    }

    vector<BYTE> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_EncryptUpdate failed");
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// AES 복호화 함수 (CBC 모드)
vector<BYTE> DecryptAES(const vector<BYTE>& ciphertext, const vector<BYTE>& key, const vector<BYTE>& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw runtime_error("EVP_CIPHER_CTX_new failed");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_DecryptInit_ex failed");
    }

    vector<BYTE> plaintext(ciphertext.size());
    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_DecryptUpdate failed");
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("EVP_DecryptFinal_ex failed");
    }
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}
*/


/*************************************************************/
vector<BYTE> cGenerateAESKey() {
    SecByteBlock key(AES::MAX_KEYLENGTH);
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(key, key.size());
    return vector<BYTE>(key.begin(), key.end());
}

vector<BYTE> cGenerateIV() {
    SecByteBlock iv(AES::BLOCKSIZE);
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(iv, iv.size());
    return vector<BYTE>(iv.begin(), iv.end());
}

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


/*************************************************************/

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
