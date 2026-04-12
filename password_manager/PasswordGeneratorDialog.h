#ifndef PASSWORDGENERATORDIALOG_H
#define PASSWORDGENERATORDIALOG_H

#include <wx/wx.h>
#include <wx/spinctrl.h>  // ADD THIS
#include "PasswordManager.h"

class PasswordGeneratorDialog : public wxDialog {
public:
    PasswordGeneratorDialog(wxWindow* parent, PasswordManager* pm);

private:
    PasswordManager* pm_;
    wxSpinCtrl* spinLength_;        // Make sure these are declared
    wxCheckBox* chkUpper_;
    wxCheckBox* chkLower_;
    wxCheckBox* chkDigits_;
    wxCheckBox* chkSpecial_;
    wxTextCtrl* txtResult_;

    void OnGenerate(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif