#ifndef ADDPASSWORDDIALOG_H
#define ADDPASSWORDDIALOG_H

#include <wx/wx.h>
#include <wx/choice.h>
#include <wx/gauge.h>
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
    wxChoice* choiceTag_;
    wxGauge* gaugeStrength_;
    wxStaticText* lblStrength_;

    void OnGenerate(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnPasswordChanged(wxCommandEvent& event);
    void updateStrengthMeter(const std::string& password);

    wxDECLARE_EVENT_TABLE();
};

#endif