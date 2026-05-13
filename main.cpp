#define _CRT_SECURE_NO_WARNINGS
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "MainFrame.h"

class PasswordManagerApp : public wxApp {
public:
    virtual bool OnInit() override {
        // Always store vault.json next to the executable,
        // regardless of the working directory the OS launches us from.
        wxFileName vaultPath(wxStandardPaths::Get().GetExecutablePath());
        vaultPath.SetFullName("vault.json");

        MainFrame* frame = new MainFrame("Password Manager",
            vaultPath.GetFullPath());
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(PasswordManagerApp);