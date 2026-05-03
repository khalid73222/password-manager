#ifndef PASSWORD_ENTRY_H
#define PASSWORD_ENTRY_H

#include <string>
#include <ctime>
#include <wx/wx.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class PasswordTag {
    NONE,
    WORK,       // Red
    PERSONAL,   // Blue
    FINANCE,    // Green
    SOCIAL      // Yellow
};

struct PasswordEntry {
    std::string id;
    std::string service;
    std::string username;
    std::string password;
    std::string url;
    std::string notes;
    std::time_t created;
    std::time_t modified;
    std::time_t lastChanged;
    PasswordTag tag;

    PasswordEntry()
        : created(std::time(nullptr)), modified(std::time(nullptr)),
        lastChanged(std::time(nullptr)), tag(PasswordTag::NONE) {
    }

    json toJson() const;
    static PasswordEntry fromJson(const json& j);

    static wxColour GetTagColor(PasswordTag tag);
    static wxString GetTagName(PasswordTag tag);
};

#endif