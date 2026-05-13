#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <wx/wx.h>

enum class PinMode { SETUP, VERIFY, RECOVER, CHANGE };

class PinDialog : public wxDialog {
public:
    PinDialog(wxWindow* parent, PinMode mode);

    void SetSecurityQuestion(const wxString& question);
    bool DidRequestRecovery() const { return requestedRecovery_; }

    wxString GetPin()              const { return pin_; }
    wxString GetSecurityQuestion() const { return securityQuestion_; }
    wxString GetSecurityAnswer()   const { return securityAnswer_; }

private:
    PinMode  mode_;
    wxString pin_;
    wxString securityQuestion_;
    wxString securityAnswer_;
    bool     requestedRecovery_;

    wxTextCtrl* txtPin_;
    wxTextCtrl* txtConfirm_;
    wxTextCtrl* txtQuestion_;
    wxTextCtrl* txtAnswer_;
    wxStaticText* lblPrompt_;
    wxButton* btnRecover_;

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnRecover(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif