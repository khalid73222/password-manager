#include "PasswordManager.h"
#include "CryptoUtils.h"
#include <random>
#include <sstream>
#include <iomanip>

PasswordManager::PasswordManager(const std::string& vaultPath)
    : storage_(std::make_unique<StorageManager>(vaultPath)), initialized_(false) {
}

PasswordManager::~PasswordManager() {}

bool PasswordManager::initialize() {
    if (storage_->loadVault()) {
        initialized_ = true;
        return true;
    }
    return false;
}

bool PasswordManager::isInitialized() const {
    return initialized_;
}

bool PasswordManager::addPassword(const std::string& service,
    const std::string& username,
    const std::string& password,
    const std::string& url,
    const std::string& notes) {
    if (!initialized_) return false;

    PasswordEntry entry;
    entry.id = generateId();
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;

    return storage_->addEntry(entry);
}

bool PasswordManager::updatePassword(const std::string& id,
    const std::string& service,
    const std::string& username,
    const std::string& password,
    const std::string& url,
    const std::string& notes) {
    if (!initialized_) return false;

    PasswordEntry entry;
    entry.id = id;
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;
    entry.created = 0;

    return storage_->updateEntry(id, entry);
}

bool PasswordManager::removePassword(const std::string& id) {
    if (!initialized_) return false;
    return storage_->deleteEntry(id);
}

PasswordEntry* PasswordManager::getPassword(const std::string& id) {
    if (!initialized_) return nullptr;
    return storage_->getEntry(id);
}

std::vector<PasswordEntry> PasswordManager::listPasswords() {
    if (!initialized_) return {};
    return storage_->getAllEntries();
}

std::vector<PasswordEntry> PasswordManager::searchPasswords(const std::string& query) {
    if (!initialized_) return {};
    return storage_->searchEntries(query);
}

std::string PasswordManager::generatePassword(size_t length,
    bool useUpper,
    bool useLower,
    bool useDigits,
    bool useSpecial) {
    return CryptoUtils::generatePassword(length, useUpper, useLower, useDigits, useSpecial);
}

std::string PasswordManager::generateId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << dis(gen);
    }

    return ss.str();
}