#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include "StorageManager.h"
#include "PasswordEntry.h"
#include "PasswordStrengthChecker.h"
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
        const std::string& notes = "",
        PasswordTag tag = PasswordTag::NONE);

    bool updatePassword(const std::string& id,
        const std::string& service,
        const std::string& username,
        const std::string& password,
        const std::string& url = "",
        const std::string& notes = "",
        PasswordTag tag = PasswordTag::NONE);

    bool removePassword(const std::string& id);
    PasswordEntry* getPassword(const std::string& id);
    std::vector<PasswordEntry> listPasswords();
    std::vector<PasswordEntry> searchPasswords(const std::string& query);
    std::vector<PasswordEntry> searchByTag(PasswordTag tag);

    std::string generatePassword(size_t length = 16,
        bool useUpper = true,
        bool useLower = true,
        bool useDigits = true,
        bool useSpecial = true);

    StrengthResult checkStrength(const std::string& password);

    // PIN Management
    bool     hasPin() const;
    bool     verifyPin(const std::string& pin);
    bool     setPin(const std::string& pin,
        const std::string& question,
        const std::string& answer);
    bool     recoverPin(const std::string& answer, const std::string& newPin);
    wxString getSecurityQuestion() const;
    bool     changePin(const std::string& oldPin, const std::string& newPin);

private:
    std::unique_ptr<StorageManager> storage_;
    bool initialized_;
    std::string generateId();
};

#endif