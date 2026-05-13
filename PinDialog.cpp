#define _CRT_SECURE_NO_WARNINGS
#include "PinDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statline.h>

enum { ID_PIN_OK = 5000, ID_PIN_CANCEL, ID_PIN_RECOVER };

wxBEGIN_EVENT_TABLE(PinDialog, wxDialog)
EVT_BUTTON(ID_PIN_OK, PinDialog::OnOk)
EVT_BUTTON(ID_PIN_CANCEL, PinDialog::OnCancel)
EVT_BUTTON(ID_PIN_RECOVER, PinDialog::OnRecover)
wxEND_EVENT_TABLE()

PinDialog::PinDialog(wxWindow* parent, PinMode mode)
    : wxDialog(parent, wxID_ANY, "PIN Authentication",
        wxDefaultPosition, wxSize(360, 360),
        wxDEFAULT_DIALOG_STYLE),
    mode_(mode), requestedRecovery_(false),
    txtPin_(nullptr), txtConfirm_(nullptr),
    txtQuestion_(nullptr), txtAnswer_(nullptr),
    btnRecover_(nullptr)
{
    SetBackgroundColour(wxColour(245, 246, 250));

    wxString title;
    switch (mode_) {
    case PinMode::SETUP:   title = "Set Up PIN";    break;
    case PinMode::VERIFY:  title = "Enter PIN";     break;
    case PinMode::RECOVER: title = "Recover PIN";   break;
    case PinMode::CHANGE:  title = "Change PIN";    break;
    }
    SetTitle(title);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header
    wxPanel* hdr = new wxPanel(this, wxID_ANY);
    hdr->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* hdrS = new wxBoxSizer(wxHORIZONTAL);

    // FIX: Replaced \U0001F512 (lock emoji) with [PIN] ASCII text
    wxStaticText* hdrLbl = new wxStaticText(hdr, wxID_ANY, "  [PIN]  " + title);

    hdrLbl->SetForegroundColour(*wxWHITE);
    wxFont hf = hdrLbl->GetFont(); hf.SetPointSize(11); hf.SetWeight(wxFONTWEIGHT_BOLD);
    hdrLbl->SetFont(hf);
    hdrS->Add(hdrLbl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);
    hdr->SetSizer(hdrS);
    mainSizer->Add(hdr, 0, wxEXPAND);

    lblPrompt_ = new wxStaticText(this, wxID_ANY, "");
    lblPrompt_->SetForegroundColour(wxColour(60, 60, 80));
    mainSizer->Add(lblPrompt_, 0, wxALL, 12);

    if (mode_ == PinMode::SETUP || mode_ == PinMode::CHANGE) {
        lblPrompt_->SetLabel("Create a 4-6 digit PIN to secure your vault:");
        mainSizer->Add(new wxStaticText(this, wxID_ANY, "PIN:"), 0, wxLEFT | wxRIGHT, 12);
        txtPin_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        mainSizer->Add(txtPin_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Confirm PIN:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtConfirm_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        mainSizer->Add(txtConfirm_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Security Question:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtQuestion_ = new wxTextCtrl(this, wxID_ANY);
        mainSizer->Add(txtQuestion_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Answer:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtAnswer_ = new wxTextCtrl(this, wxID_ANY);
        mainSizer->Add(txtAnswer_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    }
    else if (mode_ == PinMode::VERIFY) {
        lblPrompt_->SetLabel("Enter your PIN to access the vault:");
        mainSizer->Add(new wxStaticText(this, wxID_ANY, "PIN:"), 0, wxLEFT | wxRIGHT, 12);
        txtPin_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        mainSizer->Add(txtPin_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        btnRecover_ = new wxButton(this, ID_PIN_RECOVER, "Forgot PIN?");
        btnRecover_->SetForegroundColour(wxColour(0, 100, 200));
        mainSizer->Add(btnRecover_, 0, wxALIGN_CENTER | wxTOP, 12);

    }
    else if (mode_ == PinMode::RECOVER) {
        lblPrompt_->SetLabel("Answer your security question to reset PIN:");
        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Security Question:"), 0, wxLEFT | wxRIGHT, 12);
        txtQuestion_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        mainSizer->Add(txtQuestion_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Your Answer:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtAnswer_ = new wxTextCtrl(this, wxID_ANY);
        mainSizer->Add(txtAnswer_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "New PIN:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtPin_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        mainSizer->Add(txtPin_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

        mainSizer->Add(new wxStaticText(this, wxID_ANY, "Confirm New PIN:"), 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtConfirm_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        mainSizer->Add(txtConfirm_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    mainSizer->AddStretchSpacer(1);
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer(1);
    wxButton* btnOk = new wxButton(this, ID_PIN_OK,
        mode_ == PinMode::VERIFY ? "Unlock" : "OK");
    btnOk->SetBackgroundColour(wxColour(30, 34, 45));
    btnOk->SetForegroundColour(*wxWHITE);
    btnOk->SetDefault();
    btnSizer->Add(btnOk, 0, wxALL, 6);
    btnSizer->Add(new wxButton(this, ID_PIN_CANCEL, "Cancel"), 0, wxALL, 6);
    mainSizer->Add(btnSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
    Centre();
}

void PinDialog::SetSecurityQuestion(const wxString& question) {
    if (txtQuestion_) txtQuestion_->SetValue(question);
}

void PinDialog::OnRecover(wxCommandEvent&) {
    requestedRecovery_ = true;
    EndModal(wxID_CANCEL);
}

void PinDialog::OnOk(wxCommandEvent&) {
    if (mode_ == PinMode::SETUP || mode_ == PinMode::CHANGE) {
        wxString pin = txtPin_->GetValue();
        wxString confirm = txtConfirm_->GetValue();
        wxString q = txtQuestion_->GetValue();
        wxString a = txtAnswer_->GetValue();
        if (pin.length() < 4 || pin.length() > 6) {
            wxMessageBox("PIN must be 4-6 digits.", "Error", wxOK | wxICON_ERROR); return;
        }
        if (pin != confirm) {
            wxMessageBox("PINs do not match.", "Error", wxOK | wxICON_ERROR); return;
        }
        if (q.IsEmpty() || a.IsEmpty()) {
            wxMessageBox("Security question and answer are required.", "Error", wxOK | wxICON_ERROR); return;
        }
        pin_ = pin; securityQuestion_ = q; securityAnswer_ = a;
        EndModal(wxID_OK);

    }
    else if (mode_ == PinMode::VERIFY) {
        pin_ = txtPin_->GetValue();
        if (pin_.IsEmpty()) {
            wxMessageBox("Please enter your PIN.", "Error", wxOK | wxICON_ERROR); return;
        }
        EndModal(wxID_OK);

    }
    else if (mode_ == PinMode::RECOVER) {
        wxString a = txtAnswer_->GetValue();
        wxString pin = txtPin_->GetValue();
        wxString confirm = txtConfirm_->GetValue();
        if (a.IsEmpty()) {
            wxMessageBox("Please enter an answer.", "Error", wxOK | wxICON_ERROR); return;
        }
        if (pin.length() < 4 || pin.length() > 6) {
            wxMessageBox("New PIN must be 4-6 digits.", "Error", wxOK | wxICON_ERROR); return;
        }
        if (pin != confirm) {
            wxMessageBox("PINs do not match.", "Error", wxOK | wxICON_ERROR); return;
        }
        securityAnswer_ = a; pin_ = pin;
        EndModal(wxID_OK);
    }
}

void PinDialog::OnCancel(wxCommandEvent&) { EndModal(wxID_CANCEL); }