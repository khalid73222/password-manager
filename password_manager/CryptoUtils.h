#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

class CryptoUtils {
public:
    // Password generation
    static std::string generatePassword(size_t length = 16,
        bool useUpper = true,
        bool useLower = true,
        bool useDigits = true,
        bool useSpecial = true);

    // Stronger hashing (SHA-256 inspired, no external lib)
    static std::string hashStrong(const std::string& input, const std::string& salt = "");
    static std::string generateSalt(size_t length = 16);

    // XOR + key-stretching encryption for vault data
    static std::string encryptField(const std::string& plaintext, const std::string& key);
    static std::string decryptField(const std::string& ciphertext, const std::string& key);

    // Base64 encode/decode for safe JSON storage
    static std::string base64Encode(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> base64Decode(const std::string& encoded);

    // FIX: Made public so it can be called from outside the class
    static std::vector<uint8_t> sha256Raw(const std::string& input);

private:
    static const std::string UPPER;
    static const std::string LOWER;
    static const std::string DIGITS;
    static const std::string SPECIAL;

    // Internal SHA-256 helpers (no external lib)
    static std::vector<uint32_t> sha256Block(const std::vector<uint8_t>& data);
    static std::string toHex(const std::vector<uint8_t>& data);

    // Key derivation (PBKDF2-like stretching)
    static std::vector<uint8_t> deriveKey(const std::string& password,
        const std::string& salt,
        int iterations = 10000,
        size_t keyLen = 32);
};

#endif