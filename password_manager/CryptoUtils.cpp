#define _CRT_SECURE_NO_WARNINGS
#include "CryptoUtils.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

// ─── Character sets ───────────────────────────────────────────────────────────
const std::string CryptoUtils::UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string CryptoUtils::LOWER = "abcdefghijklmnopqrstuvwxyz";
const std::string CryptoUtils::DIGITS = "0123456789";
const std::string CryptoUtils::SPECIAL = "!#_.";

// ─── Password generation ──────────────────────────────────────────────────────
std::string CryptoUtils::generatePassword(size_t length,
    bool useUpper, bool useLower,
    bool useDigits, bool useSpecial) {
    std::string chars;
    if (useLower)   chars += LOWER;
    if (useUpper)   chars += UPPER;
    if (useDigits)  chars += DIGITS;
    if (useSpecial) chars += SPECIAL;
    if (chars.empty()) chars = LOWER;

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, chars.size() - 1);

    std::string password;
    password.reserve(length);
    for (size_t i = 0; i < length; ++i)
        password += chars[dis(gen)];
    return password;
}

// ─── Salt generation ──────────────────────────────────────────────────────────
std::string CryptoUtils::generateSalt(size_t length) {
    static const char alphanum[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, sizeof(alphanum) - 2);
    std::string salt;
    salt.reserve(length);
    for (size_t i = 0; i < length; ++i)
        salt += alphanum[dis(gen)];
    return salt;
}

// ─── SHA-256 (pure C++, no external lib) ─────────────────────────────────────
static const uint32_t K256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static inline uint32_t rotr32(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t sig0(uint32_t x) { return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22); }
static inline uint32_t sig1(uint32_t x) { return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25); }
static inline uint32_t gam0(uint32_t x) { return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3); }
static inline uint32_t gam1(uint32_t x) { return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10); }

std::vector<uint8_t> CryptoUtils::sha256Raw(const std::string& input) {
    // Build padded message
    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bitLen = static_cast<uint64_t>(input.size()) * 8;
    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0x00);
    for (int i = 7; i >= 0; --i)
        msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));

    // Initial hash values
    uint32_t h[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };

    // Process each 512-bit block
    for (size_t b = 0; b < msg.size(); b += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (static_cast<uint32_t>(msg[b + i * 4]) << 24) |
            (static_cast<uint32_t>(msg[b + i * 4 + 1]) << 16) |
            (static_cast<uint32_t>(msg[b + i * 4 + 2]) << 8) |
            static_cast<uint32_t>(msg[b + i * 4 + 3]);
        for (int i = 16; i < 64; ++i)
            w[i] = gam1(w[i - 2]) + w[i - 7] + gam0(w[i - 15]) + w[i - 16];

        uint32_t a = h[0], b2 = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = hh + sig1(e) + ch(e, f, g) + K256[i] + w[i];
            uint32_t t2 = sig0(a) + maj(a, b2, c);
            hh = g; g = f; f = e; e = d + t1; d = c; c = b2; b2 = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b2; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f;  h[6] += g;  h[7] += hh;
    }

    std::vector<uint8_t> digest(32);
    for (int i = 0; i < 8; ++i) {
        digest[i * 4] = (h[i] >> 24) & 0xFF;
        digest[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        digest[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        digest[i * 4 + 3] = h[i] & 0xFF;
    }
    return digest;
}

std::string CryptoUtils::toHex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (auto b : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

// ─── PBKDF2-like key derivation (HMAC-SHA256 rounds) ─────────────────────────
// Simple but significantly stronger than plain std::hash
std::vector<uint8_t> CryptoUtils::deriveKey(const std::string& password,
    const std::string& salt,
    int iterations, size_t keyLen) {
    // Initial material: sha256(password + salt)
    std::string combined = password + "|" + salt;
    std::vector<uint8_t> key = sha256Raw(combined);

    // Stretch with repeated hashing
    for (int i = 0; i < iterations; ++i) {
        std::string iterStr(key.begin(), key.end());
        iterStr += static_cast<char>(i & 0xFF);
        iterStr += static_cast<char>((i >> 8) & 0xFF);
        key = sha256Raw(iterStr);
    }

    key.resize(keyLen);
    return key;
}

// ─── Strong hash (salt + 10 k iterations) ─────────────────────────────────────
std::string CryptoUtils::hashStrong(const std::string& input, const std::string& salt) {
    std::string s = salt.empty() ? generateSalt() : salt;
    auto key = deriveKey(input, s, 10000, 32);
    // Format: salt$hex
    return s + "$" + toHex(key);
}

// ─── Base64 ───────────────────────────────────────────────────────────────────
static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string CryptoUtils::base64Encode(const std::vector<uint8_t>& data) {
    std::string out;
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(B64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::vector<uint8_t> CryptoUtils::base64Decode(const std::string& encoded) {
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; ++i) T[(uint8_t)B64[i]] = i;
    std::vector<uint8_t> out;
    int val = 0, valb = -8;
    for (uint8_t c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// ─── Field encryption (ChaCha20-inspired XOR stream cipher) ──────────────────
// Key: PBKDF2-derived 32 bytes. Nonce: random 12 bytes prepended to ciphertext.
static void generateStream(const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& nonce,
    size_t length,
    std::vector<uint8_t>& stream) {
    // Counter-mode XOR keystream: repeatedly hash(key + nonce + counter)
    stream.clear();
    stream.reserve(length);
    uint32_t counter = 0;
    while (stream.size() < length) {
        std::string block(key.begin(), key.end());
        block.append(nonce.begin(), nonce.end());
        block += static_cast<char>(counter & 0xFF);
        block += static_cast<char>((counter >> 8) & 0xFF);
        block += static_cast<char>((counter >> 16) & 0xFF);
        block += static_cast<char>((counter >> 24) & 0xFF);
        auto h = CryptoUtils::sha256Raw(block);   // exposed via friend; use public via header trick below
        for (auto b : h) {
            stream.push_back(b);
            if (stream.size() == length) break;
        }
        ++counter;
    }
}

// Expose sha256Raw publicly for the stream function above
// (already defined in .cpp; called through CryptoUtils::encryptField)

std::string CryptoUtils::encryptField(const std::string& plaintext, const std::string& key) {
    if (plaintext.empty()) return "";

    // Derive 32-byte key from master key + fixed label
    std::string salt = generateSalt(16);
    auto derivedKey = deriveKey(key, salt, 1000, 32);  // lighter iteration for field enc

    // Random 12-byte nonce
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    std::vector<uint8_t> nonce(12);
    for (auto& b : nonce) b = static_cast<uint8_t>(dis(gen));

    // Keystream XOR
    std::vector<uint8_t> stream;
    generateStream(derivedKey, nonce, plaintext.size(), stream);

    std::vector<uint8_t> ciphertext(plaintext.size());
    for (size_t i = 0; i < plaintext.size(); ++i)
        ciphertext[i] = static_cast<uint8_t>(plaintext[i]) ^ stream[i];

    // Output layout: salt(16) + nonce(12) + ciphertext → base64
    std::vector<uint8_t> output;
    output.insert(output.end(), salt.begin(), salt.end());
    output.insert(output.end(), nonce.begin(), nonce.end());
    output.insert(output.end(), ciphertext.begin(), ciphertext.end());

    return base64Encode(output);
}

std::string CryptoUtils::decryptField(const std::string& encoded, const std::string& key) {
    if (encoded.empty()) return "";

    auto raw = base64Decode(encoded);
    if (raw.size() < 28) return "";  // 16 salt + 12 nonce minimum

    std::string salt(raw.begin(), raw.begin() + 16);
    std::vector<uint8_t> nonce(raw.begin() + 16, raw.begin() + 28);
    std::vector<uint8_t> ciphertext(raw.begin() + 28, raw.end());

    auto derivedKey = deriveKey(key, salt, 1000, 32);

    std::vector<uint8_t> stream;
    generateStream(derivedKey, nonce, ciphertext.size(), stream);

    std::string plaintext(ciphertext.size(), '\0');
    for (size_t i = 0; i < ciphertext.size(); ++i)
        plaintext[i] = static_cast<char>(ciphertext[i] ^ stream[i]);

    return plaintext;
}

// Make sha256Raw accessible to generateStream (it's in the same TU)
// We expose it as a public static for the stream helper above
std::vector<uint32_t> CryptoUtils::sha256Block(const std::vector<uint8_t>& data) {
    // Not used externally; stub to satisfy header
    return {};
}