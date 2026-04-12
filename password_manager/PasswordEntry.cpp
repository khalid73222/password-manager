#include "PasswordEntry.h"

json PasswordEntry::toJson() const {
    return json{
        {"id", id},
        {"service", service},
        {"username", username},
        {"password", password},
        {"url", url},
        {"notes", notes},
        {"created", created},
        {"modified", modified}
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
    entry.created = j.value("created", std::time(nullptr));
    entry.modified = j.value("modified", std::time(nullptr));
    return entry;
}