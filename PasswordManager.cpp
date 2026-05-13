#define _CRT_SECURE_NO_WARNINGS
#include "PasswordManager.h"
#include "CryptoUtils.h"
#include <random>
#include <sstream>

PasswordManager::PasswordManager(const std::string& vaultPath)
    : storage_(std::make_unique<StorageManager>(vaultPath)), initialized_(false) {
}

PasswordManager::~PasswordManager() {}

bool PasswordManager::initialize() {
    initialized_ = storage_->loadVault();
    return initialized_;
}

bool PasswordManager::isInitialized() const { return initialized_; }

bool PasswordManager::addPassword(const std::string& service,
    const std::string& username, const std::string& password,
    const std::string& url, const std::string& notes,
    const std::string& category) {
    if (!initialized_) return false;
    PasswordEntry entry;
    entry.id = generateId();
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;
    entry.category = category;
    entry.lastChanged = std::time(nullptr);
    return storage_->addEntry(entry);
}

bool PasswordManager::updatePassword(const std::string& id,
    const std::string& service, const std::string& username,
    const std::string& password, const std::string& url,
    const std::string& notes, const std::string& category) {
    if (!initialized_) return false;
    PasswordEntry entry;
    entry.id = id;
    entry.service = service;
    entry.username = username;
    entry.password = password;
    entry.url = url;
    entry.notes = notes;
    entry.category = category;
    entry.created = 0;   // preserved by StorageManager::updateEntry
    entry.lastChanged = std::time(nullptr);
    return storage_->updateEntry(id, entry);
}

bool PasswordManager::removePassword(const std::string& id) {
    return initialized_ ? storage_->deleteEntry(id) : false;
}

PasswordEntry* PasswordManager::getPassword(const std::string& id) {
    return initialized_ ? storage_->getEntry(id) : nullptr;
}

std::vector<PasswordEntry> PasswordManager::listPasswords() {
    return initialized_ ? storage_->getAllEntries() : std::vector<PasswordEntry>{};
}

std::vector<PasswordEntry> PasswordManager::searchPasswords(const std::string& q) {
    return initialized_ ? storage_->searchEntries(q) : std::vector<PasswordEntry>{};
}

std::vector<PasswordEntry> PasswordManager::searchByCategory(const std::string& cat) {
    return initialized_ ? storage_->searchByCategory(cat) : std::vector<PasswordEntry>{};
}

// ── Categories ────────────────────────────────────────────────────────────────
std::vector<Category> PasswordManager::getCategories() const {
    return initialized_ ? storage_->getCategories() : std::vector<Category>{};
}

bool PasswordManager::addCategory(const Category& cat) {
    return initialized_ ? storage_->addCategory(cat) : false;
}

bool PasswordManager::removeCategory(const std::string& name) {
    return initialized_ ? storage_->removeCategory(name) : false;
}

bool PasswordManager::renameCategory(const std::string& o, const std::string& n) {
    return initialized_ ? storage_->renameCategory(o, n) : false;
}

const Category* PasswordManager::findCategory(const std::string& name) const {
    return initialized_ ? storage_->findCategory(name) : nullptr;
}

// ── Tools ─────────────────────────────────────────────────────────────────────
std::string PasswordManager::generatePassword(size_t length,
    bool useUpper, bool useLower, bool useDigits, bool useSpecial) {
    return CryptoUtils::generatePassword(length, useUpper, useLower, useDigits, useSpecial);
}

StrengthResult PasswordManager::checkStrength(const std::string& pw) {
    return PasswordStrengthChecker::check(pw);
}

// ── PIN ───────────────────────────────────────────────────────────────────────
bool     PasswordManager::hasPin()    const { return storage_->hasPin(); }
bool     PasswordManager::verifyPin(const std::string& p) { return storage_->verifyPin(p); }
bool     PasswordManager::setPin(const std::string& p,
    const std::string& q, const std::string& a)
{
    return storage_->setPin(p, q, a);
}
bool     PasswordManager::recoverPin(const std::string& a, const std::string& n)
{
    return storage_->recoverPin(a, n);
}
wxString PasswordManager::getSecurityQuestion() const { return storage_->getSecurityQuestion(); }
bool     PasswordManager::changePin(const std::string& o, const std::string& n)
{
    return storage_->changePin(o, n);
}

std::string PasswordManager::generateId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0, 15);
    std::ostringstream ss;
    for (int i = 0; i < 16; ++i) ss << std::hex << dis(gen);
    return ss.str();
}