#ifndef PASSWORD_ENTRY_H
#define PASSWORD_ENTRY_H

#include <string>
#include <ctime>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct PasswordEntry {
    std::string id;
    std::string service;
    std::string username;
    std::string password;
    std::string url;
    std::string notes;
    std::time_t created;
    std::time_t modified;

    PasswordEntry() : created(std::time(nullptr)), modified(std::time(nullptr)) {}

    // Just declarations here, no implementations
    json toJson() const;
    static PasswordEntry fromJson(const json& j);
};

#endif