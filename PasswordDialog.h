#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <wx/wx.h>
#include <wx/choice.h>
#include <wx/gauge.h>
#include "PasswordManager.h"
#include "PasswordEntry.h"

class PasswordDialog : public wxDialog {
public:
    PasswordDialog(wxWindow* parent, PasswordManager* pm, PasswordEntry* entry);

private:
    PasswordManager* pm_;
    PasswordEntry* entry_;
    bool             showingPassword_;

    wxTextCtrl* txtService_;
    wxTextCtrl* txtUsername_;
    wxTextCtrl* txtPassword_;
    wxTextCtrl* txtUrl_;
    wxTextCtrl* txtNotes_;
    wxCheckBox* chkShow_;
    wxChoice* choiceCategory_;
    wxGauge* gaugeStrength_;
    wxStaticText* lblStrength_;
    wxStaticText* lblWarning_;

    void PopulateCategories();
    void OnShowPassword(wxCommandEvent&);
    void OnSave(wxCommandEvent&);
    void OnPasswordChanged(wxCommandEvent&);
    void updateStrengthMeter(const std::string& pw);
    void checkAgeWarning();

    wxDECLARE_EVENT_TABLE();
};

#endif