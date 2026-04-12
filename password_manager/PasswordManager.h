#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include "StorageManager.h"
#include "PasswordEntry.h"
#include <memory>

class PasswordManager {
public:
    PasswordManager(const std::string& vaultPath);
    ~PasswordManager();

    bool initialize();
    bool isInitialized() const;

    bool addPassword(const std::string& service,
        const std::string& username,
        const std::string& password,
        const std::string& url = "",
        const std::string& notes = "");
    bool updatePassword(const std::string& id,
        const std::string& service,
        const std::string& username,
        const std::string& password,
        const std::string& url = "",
        const std::string& notes = "");
    bool removePassword(const std::string& id);
    PasswordEntry* getPassword(const std::string& id);
    std::vector<PasswordEntry> listPasswords();
    std::vector<PasswordEntry> searchPasswords(const std::string& query);

    std::string generatePassword(size_t length = 16,
        bool useUpper = true,
        bool useLower = true,
        bool useDigits = true,
        bool useSpecial = true);

private:
    std::unique_ptr<StorageManager> storage_;
    bool initialized_;
    std::string generateId();
};

#endif