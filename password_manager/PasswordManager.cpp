#define _CRT_SECURE_NO_WARNINGS
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
    if (storage_->loadVault()) { initialized_ = true; return true; }
    return false;
}

bool PasswordManager::isInitialized() const { return initialized_; }

bool PasswordManager::addPassword(const std::string& service,
    const std::string& username, const std::string& password,
    const std::string& url, const std::string& notes, PasswordTag tag) {
    if (!initialized_) return false;
    PasswordEntry entry;
    entry.id = generateId();
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;
    entry.tag = tag;
    entry.lastChanged = std::time(nullptr);
    return storage_->addEntry(entry);
}

bool PasswordManager::updatePassword(const std::string& id,
    const std::string& service, const std::string& username,
    const std::string& password, const std::string& url,
    const std::string& notes, PasswordTag tag) {
    if (!initialized_) return false;
    PasswordEntry entry;
    entry.id = id;
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;
    entry.tag = tag;
    entry.created = 0;
    entry.lastChanged = std::time(nullptr);
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

std::vector<PasswordEntry> PasswordManager::searchByTag(PasswordTag tag) {
    if (!initialized_) return {};
    return storage_->searchByTag(tag);
}

std::string PasswordManager::generatePassword(size_t length,
    bool useUpper, bool useLower, bool useDigits, bool useSpecial) {
    return CryptoUtils::generatePassword(length, useUpper, useLower, useDigits, useSpecial);
}

StrengthResult PasswordManager::checkStrength(const std::string& password) {
    return PasswordStrengthChecker::check(password);
}

bool     PasswordManager::hasPin() const { return storage_->hasPin(); }
bool     PasswordManager::verifyPin(const std::string& pin) { return storage_->verifyPin(pin); }
bool     PasswordManager::setPin(const std::string& p, const std::string& q,
    const std::string& a) {
    return storage_->setPin(p, q, a);
}
bool     PasswordManager::recoverPin(const std::string& a, const std::string& n) { return storage_->recoverPin(a, n); }
wxString PasswordManager::getSecurityQuestion() const { return storage_->getSecurityQuestion(); }
bool     PasswordManager::changePin(const std::string& o, const std::string& n) { return storage_->changePin(o, n); }

std::string PasswordManager::generateId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0, 15);
    std::ostringstream ss;
    for (int i = 0; i < 16; ++i) ss << std::hex << dis(gen);
    return ss.str();
}