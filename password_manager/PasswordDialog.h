#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <wx/wx.h>
#include "PasswordManager.h"
#include "PasswordEntry.h"

class PasswordDialog : public wxDialog {
public:
    PasswordDialog(wxWindow* parent, PasswordManager* pm, PasswordEntry* entry);

private:
    PasswordManager* pm_;
    PasswordEntry* entry_;
    wxTextCtrl* txtService_;
    wxTextCtrl* txtUsername_;
    wxTextCtrl* txtPassword_;
    wxTextCtrl* txtUrl_;
    wxTextCtrl* txtNotes_;
    wxCheckBox* chkShow_;

    void OnShowPassword(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif