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
        {"tag",         static_cast<int>(tag)}
    };
}

PasswordEntry PasswordEntry::fromJson(const json& j) {
    PasswordEntry entry;
    entry.id = j.value("id", "");
    entry.service = j.value("service", "");
    entry.username = j.value("username", "");
    entry.password = j.value("password", "");
    entry.url = j.value("url", "");
    entry.notes = j.value("notes", "");
    entry.created = j.value("created", static_cast<std::time_t>(std::time(nullptr)));
    entry.modified = j.value("modified", static_cast<std::time_t>(std::time(nullptr)));
    entry.lastChanged = j.value("lastChanged", static_cast<std::time_t>(std::time(nullptr)));
    entry.tag = static_cast<PasswordTag>(j.value("tag", 0));
    return entry;
}

wxColour PasswordEntry::GetTagColor(PasswordTag tag) {
    switch (tag) {
    case PasswordTag::WORK:     return wxColour(220, 53, 69);   // Red
    case PasswordTag::PERSONAL: return wxColour(0, 123, 255);  // Blue
    case PasswordTag::FINANCE:  return wxColour(40, 167, 69);   // Green
    case PasswordTag::SOCIAL:   return wxColour(255, 193, 7);    // Amber
    default:                    return wxColour(150, 150, 150);   // Gray
    }
}

wxString PasswordEntry::GetTagName(PasswordTag tag) {
    switch (tag) {
    case PasswordTag::WORK:     return "Work";
    case PasswordTag::PERSONAL: return "Personal";
    case PasswordTag::FINANCE:  return "Finance";
    case PasswordTag::SOCIAL:   return "Social";
    default:                    return "";
    }
}