#include "AddPasswordDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>

enum {
    ID_GENERATE_BTN = 2000,
    ID_SAVE_BTN
};

wxBEGIN_EVENT_TABLE(AddPasswordDialog, wxDialog)
EVT_BUTTON(ID_GENERATE_BTN, AddPasswordDialog::OnGenerate)
EVT_BUTTON(ID_SAVE_BTN, AddPasswordDialog::OnSave)
wxEND_EVENT_TABLE()

AddPasswordDialog::AddPasswordDialog(wxWindow* parent, PasswordManager* pm)
    : wxDialog(parent, wxID_ANY, "Add Password", wxDefaultPosition, wxSize(400, 400)),
    pm_(pm) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Service
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Service:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtService_ = new wxTextCtrl(this, wxID_ANY);
    mainSizer->Add(txtService_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Username
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Username:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtUsername_ = new wxTextCtrl(this, wxID_ANY);
    mainSizer->Add(txtUsername_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Password
    wxBoxSizer* passSizer = new wxBoxSizer(wxHORIZONTAL);
    passSizer->Add(new wxStaticText(this, wxID_ANY, "Password:"), 1, wxALIGN_CENTER_VERTICAL);
    wxButton* btnGen = new wxButton(this, ID_GENERATE_BTN, "Generate");
    passSizer->Add(btnGen, 0);
    mainSizer->Add(passSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    txtPassword_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
    mainSizer->Add(txtPassword_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // URL
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "URL (optional):"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtUrl_ = new wxTextCtrl(this, wxID_ANY);
    mainSizer->Add(txtUrl_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Notes
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Notes (optional):"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtNotes_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 60),
        wxTE_MULTILINE);
    mainSizer->Add(txtNotes_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
    wxButton* btnSave = new wxButton(this, ID_SAVE_BTN, "Save");
    btnSave->SetDefault();
    btnSizer->Add(btnSave, 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

    SetSizer(mainSizer);
}

void AddPasswordDialog::OnGenerate(wxCommandEvent& event) {
    std::string pass = pm_->generatePassword(16, true, true, true, true);
    txtPassword_->SetValue(pass);
}

void AddPasswordDialog::OnSave(wxCommandEvent& event) {
    wxString service = txtService_->GetValue();
    wxString username = txtUsername_->GetValue();
    wxString password = txtPassword_->GetValue();

    if (service.IsEmpty() || username.IsEmpty() || password.IsEmpty()) {
        wxMessageBox("Please fill in all required fields", "Error", wxOK | wxICON_ERROR);
        return;
    }

    pm_->addPassword(
        service.ToStdString(),
        username.ToStdString(),
        password.ToStdString(),
        txtUrl_->GetValue().ToStdString(),
        txtNotes_->GetValue().ToStdString()
    );

    EndModal(wxID_OK);
}