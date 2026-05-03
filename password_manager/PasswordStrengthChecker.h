#pragma once
#ifndef PASSWORD_STRENGTH_CHECKER_H
#define PASSWORD_STRENGTH_CHECKER_H

#include <wx/wx.h>
#include <string>
#include <vector>

enum class StrengthLevel { WEAK, MODERATE, STRONG, EXCELLENT };

struct StrengthResult {
    StrengthLevel level;
    int score;          // 0-100
    wxString label;
    wxString description;
    wxColour color;
    std::vector<std::string> suggestions;
};

class PasswordStrengthChecker {
public:
    static StrengthResult check(const std::string& password);

private:
    static int  calculateScore(const std::string& password);
    static bool hasCommonPatterns(const std::string& password);
};

#endif