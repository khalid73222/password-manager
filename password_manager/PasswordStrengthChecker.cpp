#define _CRT_SECURE_NO_WARNINGS
#include "PasswordStrengthChecker.h"
#include <algorithm>
#include <cctype>
#include <set>

static inline bool safeIsUpper(unsigned char c) { return std::isupper(c) != 0; }
static inline bool safeIsLower(unsigned char c) { return std::islower(c) != 0; }
static inline bool safeIsDigit(unsigned char c) { return std::isdigit(c) != 0; }
static inline bool safeIsAlpha(unsigned char c) { return std::isalpha(c) != 0; }
static inline char safeToLower(unsigned char c) { return static_cast<char>(std::tolower(c)); }

StrengthResult PasswordStrengthChecker::check(const std::string& password) {
    StrengthResult result;
    result.score = calculateScore(password);

    if (result.score <= 40) {
        result.level = StrengthLevel::WEAK;
        result.label = "Weak";
        result.description = "Vulnerable to attacks";
        result.color = wxColour(220, 53, 69);
    }
    else if (result.score <= 70) {
        result.level = StrengthLevel::MODERATE;
        result.label = "Moderate";
        result.description = "Acceptable but improvable";
        result.color = wxColour(255, 193, 7);
    }
    else if (result.score <= 90) {
        result.level = StrengthLevel::STRONG;
        result.label = "Strong";
        result.description = "Good security level";
        result.color = wxColour(40, 167, 69);
    }
    else {
        result.level = StrengthLevel::EXCELLENT;
        result.label = "Excellent";
        result.description = "Maximum protection";
        result.color = wxColour(0, 123, 255);
    }

    if (password.length() < 12)
        result.suggestions.push_back("Use 12+ characters");
    if (!std::any_of(password.begin(), password.end(), safeIsUpper))
        result.suggestions.push_back("Add uppercase letters");
    if (!std::any_of(password.begin(), password.end(), safeIsLower))
        result.suggestions.push_back("Add lowercase letters");
    if (!std::any_of(password.begin(), password.end(), safeIsDigit))
        result.suggestions.push_back("Add numbers");
    if (password.find_first_of("!#_.") == std::string::npos)
        result.suggestions.push_back("Add special characters (!#_.)");
    if (hasCommonPatterns(password))
        result.suggestions.push_back("Avoid common patterns");

    return result;
}

int PasswordStrengthChecker::calculateScore(const std::string& password) {
    if (password.empty()) return 0;

    int score = 0;
    score += std::min(static_cast<int>(password.length()) * 3, 40);

    bool hasUpper = std::any_of(password.begin(), password.end(), safeIsUpper);
    bool hasLower = std::any_of(password.begin(), password.end(), safeIsLower);
    bool hasDigit = std::any_of(password.begin(), password.end(), safeIsDigit);
    bool hasSpecial = password.find_first_of("!#_.") != std::string::npos;

    if (hasUpper)   score += 10;
    if (hasLower)   score += 10;
    if (hasDigit)   score += 10;
    if (hasSpecial) score += 10;

    if (password.length() > 2) {
        int middleChars = 0;
        for (size_t i = 1; i < password.length() - 1; ++i)
            if (!safeIsAlpha(static_cast<unsigned char>(password[i]))) middleChars++;
        score += std::min(middleChars * 2, 10);
    }

    if (hasCommonPatterns(password)) score -= 20;
    if (std::all_of(password.begin(), password.end(), safeIsLower)) score -= 10;
    if (std::all_of(password.begin(), password.end(), safeIsDigit)) score -= 10;

    return std::max(0, std::min(100, score));
}

bool PasswordStrengthChecker::hasCommonPatterns(const std::string& password) {
    static const std::set<std::string> common = {
        "123","abc","qwe","asd","zxc","password","admin","login",
        "qwerty","letmein","welcome","monkey","dragon","master"
    };
    std::string lower;
    lower.reserve(password.size());
    for (unsigned char c : password) lower.push_back(safeToLower(c));
    for (const auto& p : common)
        if (lower.find(p) != std::string::npos) return true;
    return false;
}