#ifndef ADDPASSWORDDIALOG_H
#define ADDPASSWORDDIALOG_H

#include <wx/wx.h>
#include "PasswordManager.h"

class AddPasswordDialog : public wxDialog {
public:
    AddPasswordDialog(wxWindow* parent, PasswordManager* pm);

private:
    PasswordManager* pm_;
    wxTextCtrl* txtService_;
    wxTextCtrl* txtUsername_;
    wxTextCtrl* txtPassword_;
    wxTextCtrl* txtUrl_;
    wxTextCtrl* txtNotes_;

    void OnGenerate(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif