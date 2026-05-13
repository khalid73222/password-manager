#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "PasswordEntry.h"
#include <vector>
#include <string>
#include <wx/wx.h>
#include "json.hpp"

using json = nlohmann::json;

class StorageManager {
public:
    StorageManager(const std::string& filename);
    ~StorageManager();

    bool loadVault();
    bool saveVault();
    bool vaultExists() const;

    // Entries
    bool addEntry(const PasswordEntry& entry);
    bool updateEntry(const std::string& id, const PasswordEntry& entry);
    bool deleteEntry(const std::string& id);
    PasswordEntry* getEntry(const std::string& id);
    std::vector<PasswordEntry> getAllEntries();
    std::vector<PasswordEntry> searchEntries(const std::string& query);
    std::vector<PasswordEntry> searchByCategory(const std::string& category);

    // Categories
    std::vector<Category> getCategories() const;
    bool addCategory(const Category& cat);
    bool removeCategory(const std::string& name);
    bool renameCategory(const std::string& oldName, const std::string& newName);
    const Category* findCategory(const std::string& name) const;

    // PIN
    bool     hasPin() const;
    bool     verifyPin(const std::string& pin);
    bool     setPin(const std::string& pin,
        const std::string& question,
        const std::string& answer);
    bool     recoverPin(const std::string& answer, const std::string& newPin);
    wxString getSecurityQuestion() const;
    bool     changePin(const std::string& oldPin, const std::string& newPin);

private:
    std::string filename_;
    std::vector<PasswordEntry> entries_;
    std::vector<Category>      categories_;
    bool isLoaded_;

    std::string pinHash_;
    std::string securityQuestion_;
    std::string securityAnswerHash_;

    json entriesToJson()    const;
    json categoriesToJson() const;
    void entriesFromJson(const json& j);
    void categoriesFromJson(const json& j);

    std::string hashString(const std::string& input,
        const std::string& existingSalt = "");
    bool verifyHash(const std::string& input, const std::string& storedHash);
};

#endif