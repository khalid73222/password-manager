#include "PasswordDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>

enum {
    ID_SHOW_CHK = 3000,
    ID_SAVE
};

wxBEGIN_EVENT_TABLE(PasswordDialog, wxDialog)
EVT_CHECKBOX(ID_SHOW_CHK, PasswordDialog::OnShowPassword)
EVT_BUTTON(ID_SAVE, PasswordDialog::OnSave)
wxEND_EVENT_TABLE()

PasswordDialog::PasswordDialog(wxWindow* parent, PasswordManager* pm, PasswordEntry* entry)
    : wxDialog(parent, wxID_ANY, "Edit Password", wxDefaultPosition, wxSize(400, 400)),
    pm_(pm), entry_(entry) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Service
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Service:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtService_ = new wxTextCtrl(this, wxID_ANY, entry->service);
    mainSizer->Add(txtService_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Username
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Username:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtUsername_ = new wxTextCtrl(this, wxID_ANY, entry->username);
    mainSizer->Add(txtUsername_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Password with show checkbox
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Password:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtPassword_ = new wxTextCtrl(this, wxID_ANY, entry->password, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
    mainSizer->Add(txtPassword_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    chkShow_ = new wxCheckBox(this, ID_SHOW_CHK, "Show password");
    mainSizer->Add(chkShow_, 0, wxLEFT | wxRIGHT, 10);

    // URL
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "URL:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtUrl_ = new wxTextCtrl(this, wxID_ANY, entry->url);
    mainSizer->Add(txtUrl_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Notes
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Notes:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtNotes_ = new wxTextCtrl(this, wxID_ANY, entry->notes, wxDefaultPosition, wxSize(-1, 60),
        wxTE_MULTILINE);
    mainSizer->Add(txtNotes_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
    wxButton* btnSave = new wxButton(this, ID_SAVE, "Save");
    btnSave->SetDefault();
    btnSizer->Add(btnSave, 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

    SetSizer(mainSizer);
}

void PasswordDialog::OnShowPassword(wxCommandEvent& event) {
    long style = txtPassword_->GetWindowStyle();
    if (chkShow_->IsChecked()) {
        style &= ~wxTE_PASSWORD;
    }
    else {
        style |= wxTE_PASSWORD;
    }
    txtPassword_->SetWindowStyle(style);
    txtPassword_->Refresh();
}

void PasswordDialog::OnSave(wxCommandEvent& event) {
    pm_->updatePassword(
        entry_->id,
        txtService_->GetValue().ToStdString(),
        txtUsername_->GetValue().ToStdString(),
        txtPassword_->GetValue().ToStdString(),
        txtUrl_->GetValue().ToStdString(),
        txtNotes_->GetValue().ToStdString()
    );
    EndModal(wxID_OK);
}