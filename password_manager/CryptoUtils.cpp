#include "CryptoUtils.h"
#include <random>
#include <algorithm>

const std::string CryptoUtils::UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string CryptoUtils::LOWER = "abcdefghijklmnopqrstuvwxyz";
const std::string CryptoUtils::DIGITS = "0123456789";
const std::string CryptoUtils::SPECIAL = "!@#$%^&*()_+-=[]{}|;:,.<>?";

std::string CryptoUtils::generatePassword(size_t length,
    bool useUpper,
    bool useLower,
    bool useDigits,
    bool useSpecial) {
    std::string chars;
    if (useLower) chars += LOWER;
    if (useUpper) chars += UPPER;
    if (useDigits) chars += DIGITS;
    if (useSpecial) chars += SPECIAL;

    if (chars.empty()) chars = LOWER;

    std::random_device rd;
    std::mt19937 gen(rd());
    //std::uniform_int_distribution<> dis(0, chars.size() - 1);
    std::uniform_int_distribution<size_t> dis(0, chars.size() - 1);


    std::string password;
    for (size_t i = 0; i < length; ++i) {
        password += chars[dis(gen)];
    }

    return password;
}