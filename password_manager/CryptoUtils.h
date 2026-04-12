#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>

class CryptoUtils {
public:
    static std::string generatePassword(size_t length = 16,
        bool useUpper = true,
        bool useLower = true,
        bool useDigits = true,
        bool useSpecial = true);

private:
    static const std::string UPPER;
    static const std::string LOWER;
    static const std::string DIGITS;
    static const std::string SPECIAL;
};

#endif