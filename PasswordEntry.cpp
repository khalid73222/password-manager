#define _CRT_SECURE_NO_WARNINGS
#include "PasswordEntry.h"

json PasswordEntry::toJson() const {
    return json{
        {"id",          id},
        {"service",     service},
        {"username",    username},
        {"password",    password},
        {"url",         url},
        {"notes",       notes},
        {"created",     created},
        {"modified",    modified},
        {"lastChanged", lastChanged},
        {"category",    category}
    };
}

PasswordEntry PasswordEntry::fromJson(const json& j) {
    PasswordEntry e;
    e.id = j.value("id", "");
    e.service = j.value("service", "");
    e.username = j.value("username", "");
    e.password = j.value("password", "");
    e.url = j.value("url", "");
    e.notes = j.value("notes", "");
    e.created = j.value("created", (std::time_t)std::time(nullptr));
    e.modified = j.value("modified", (std::time_t)std::time(nullptr));
    e.lastChanged = j.value("lastChanged", (std::time_t)std::time(nullptr));

    // Support old enum-based vaults (int tag → category name)
    if (j.contains("category") && j["category"].is_string()) {
        e.category = j["category"].get<std::string>();
    }
    else if (j.contains("tag") && j["tag"].is_number()) {
        switch (j["tag"].get<int>()) {
        case 1: e.category = "Work";     break;
        case 2: e.category = "Personal"; break;
        case 3: e.category = "Finance";  break;
        case 4: e.category = "Social";   break;
        default: e.category = "";        break;
        }
    }
    return e;
}