#define _CRT_SECURE_NO_WARNINGS
#include "StorageManager.h"
#include "CryptoUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>

StorageManager::StorageManager(const std::string& filename)
    : filename_(filename), isLoaded_(false) {
}

StorageManager::~StorageManager() {}

bool StorageManager::loadVault() {
    if (!vaultExists()) {
        isLoaded_ = true;
        return true;
    }
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Cannot open vault: " << filename_ << "\n";
        return false;
    }
    try {
        json j; file >> j;
        pinHash_ = j.value("pinHash", "");
        securityQuestion_ = j.value("securityQuestion", "");
        securityAnswerHash_ = j.value("securityAnswerHash", "");
        if (j.contains("entries") && j["entries"].is_array())
            entriesFromJson(j["entries"]);
        if (j.contains("categories") && j["categories"].is_array())
            categoriesFromJson(j["categories"]);
        isLoaded_ = true;
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Vault parse error: " << ex.what() << "\n";
        return false;
    }
}

bool StorageManager::saveVault() {
    std::ofstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Cannot write vault: " << filename_ << "\n";
        return false;
    }
    try {
        json j;
        j["pinHash"] = pinHash_;
        j["securityQuestion"] = securityQuestion_;
        j["securityAnswerHash"] = securityAnswerHash_;
        j["entries"] = entriesToJson();
        j["categories"] = categoriesToJson();
        file << j.dump(4);
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Vault save error: " << ex.what() << "\n";
        return false;
    }
}

bool StorageManager::vaultExists() const {
    return std::ifstream(filename_).good();
}

// ── Entries ───────────────────────────────────────────────────────────────────
bool StorageManager::addEntry(const PasswordEntry& entry) {
    entries_.push_back(entry);
    return saveVault();
}

bool StorageManager::updateEntry(const std::string& id, const PasswordEntry& updated) {
    for (auto& e : entries_) {
        if (e.id == id) {
            std::time_t orig = e.created;
            e = updated;
            e.created = orig;
            e.modified = std::time(nullptr);
            return saveVault();
        }
    }
    return false;
}

bool StorageManager::deleteEntry(const std::string& id) {
    auto it = std::remove_if(entries_.begin(), entries_.end(),
        [&](const PasswordEntry& e) { return e.id == id; });
    if (it == entries_.end()) return false;
    entries_.erase(it, entries_.end());
    return saveVault();
}

PasswordEntry* StorageManager::getEntry(const std::string& id) {
    for (auto& e : entries_) if (e.id == id) return &e;
    return nullptr;
}

std::vector<PasswordEntry> StorageManager::getAllEntries() { return entries_; }

std::vector<PasswordEntry> StorageManager::searchEntries(const std::string& query) {
    if (query.empty()) return entries_;
    std::string lq = query;
    std::transform(lq.begin(), lq.end(), lq.begin(), ::tolower);
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
    std::vector<PasswordEntry> res;
    for (const auto& e : entries_)
        if (lower(e.service).find(lq) != std::string::npos ||
            lower(e.username).find(lq) != std::string::npos ||
            lower(e.url).find(lq) != std::string::npos ||
            lower(e.notes).find(lq) != std::string::npos ||
            lower(e.category).find(lq) != std::string::npos)
            res.push_back(e);
    return res;
}

std::vector<PasswordEntry> StorageManager::searchByCategory(const std::string& cat) {
    std::vector<PasswordEntry> res;
    for (const auto& e : entries_)
        if (e.category == cat) res.push_back(e);
    return res;
}

// ── Categories ────────────────────────────────────────────────────────────────
std::vector<Category> StorageManager::getCategories() const { return categories_; }

bool StorageManager::addCategory(const Category& cat) {
    // No duplicates (case-insensitive)
    for (const auto& c : categories_)
        if (wxString(c.name).Lower() == wxString(cat.name).Lower()) return false;
    categories_.push_back(cat);
    return saveVault();
}

bool StorageManager::removeCategory(const std::string& name) {
    auto it = std::remove_if(categories_.begin(), categories_.end(),
        [&](const Category& c) { return c.name == name; });
    if (it == categories_.end()) return false;
    categories_.erase(it, categories_.end());
    // Clear category from entries that used it
    for (auto& e : entries_)
        if (e.category == name) e.category = "";
    return saveVault();
}

bool StorageManager::renameCategory(const std::string& oldName,
    const std::string& newName) {
    for (auto& c : categories_) {
        if (c.name == oldName) {
            c.name = newName;
            for (auto& e : entries_)
                if (e.category == oldName) e.category = newName;
            return saveVault();
        }
    }
    return false;
}

const Category* StorageManager::findCategory(const std::string& name) const {
    for (const auto& c : categories_)
        if (c.name == name) return &c;
    return nullptr;
}

// ── PIN ───────────────────────────────────────────────────────────────────────
bool StorageManager::hasPin() const { return !pinHash_.empty(); }

std::string StorageManager::hashString(const std::string& input,
    const std::string& existingSalt) {
    return CryptoUtils::hashStrong(input, existingSalt);
}

bool StorageManager::verifyHash(const std::string& input,
    const std::string& storedHash) {
    if (storedHash.empty()) return false;
    auto pos = storedHash.find('$');
    if (pos == std::string::npos) return false;
    return CryptoUtils::hashStrong(input, storedHash.substr(0, pos)) == storedHash;
}

bool StorageManager::verifyPin(const std::string& pin) {
    return verifyHash(pin, pinHash_);
}

bool StorageManager::setPin(const std::string& pin,
    const std::string& q, const std::string& a) {
    pinHash_ = hashString(pin);
    securityQuestion_ = q;
    securityAnswerHash_ = hashString(a);
    return saveVault();
}

bool StorageManager::recoverPin(const std::string& answer,
    const std::string& newPin) {
    if (!verifyHash(answer, securityAnswerHash_)) return false;
    pinHash_ = hashString(newPin);
    return saveVault();
}

wxString StorageManager::getSecurityQuestion() const {
    return wxString::FromUTF8(securityQuestion_.c_str());
}

bool StorageManager::changePin(const std::string& oldPin,
    const std::string& newPin) {
    if (!verifyPin(oldPin)) return false;
    pinHash_ = hashString(newPin);
    return saveVault();
}

// ── JSON ──────────────────────────────────────────────────────────────────────
json StorageManager::entriesToJson() const {
    json arr = json::array();
    for (const auto& e : entries_) arr.push_back(e.toJson());
    return arr;
}

json StorageManager::categoriesToJson() const {
    json arr = json::array();
    for (const auto& c : categories_) arr.push_back(c.toJson());
    return arr;
}

void StorageManager::entriesFromJson(const json& j) {
    entries_.clear();
    for (const auto& item : j)
        entries_.push_back(PasswordEntry::fromJson(item));
}

void StorageManager::categoriesFromJson(const json& j) {
    categories_.clear();
    for (const auto& item : j)
        categories_.push_back(Category::fromJson(item));
}