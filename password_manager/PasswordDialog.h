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
    bool             showingPassword_;   // tracks current visibility state

    wxTextCtrl* txtService_;
    wxTextCtrl* txtUsername_;
    wxTextCtrl* txtPassword_;
    wxTextCtrl* txtUrl_;
    wxTextCtrl* txtNotes_;
    wxCheckBox* chkShow_;
    wxChoice* choiceTag_;
    wxGauge* gaugeStrength_;
    wxStaticText* lblStrength_;
    wxStaticText* lblWarning_;

    void OnShowPassword(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnPasswordChanged(wxCommandEvent& event);
    void updateStrengthMeter(const std::string& password);
    void checkAgeWarning();

    wxDECLARE_EVENT_TABLE();
};

#endif