#include "PasswordGeneratorDialog.h"
#include <wx/clipbrd.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>  // ADD THIS INCLUDE

// Use unique IDs, not wxID_ANY
enum {
    ID_GENERATE = 4000,
    ID_COPY,
    ID_SPIN_LENGTH  // ADD THIS for the spin control
};

wxBEGIN_EVENT_TABLE(PasswordGeneratorDialog, wxDialog)
EVT_BUTTON(ID_GENERATE, PasswordGeneratorDialog::OnGenerate)
EVT_BUTTON(ID_COPY, PasswordGeneratorDialog::OnCopy)
wxEND_EVENT_TABLE()

PasswordGeneratorDialog::PasswordGeneratorDialog(wxWindow* parent, PasswordManager* pm)
    : wxDialog(parent, wxID_ANY, "Password Generator", wxDefaultPosition, wxSize(350, 300)),
    pm_(pm) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Length
    wxBoxSizer* lenSizer = new wxBoxSizer(wxHORIZONTAL);
    lenSizer->Add(new wxStaticText(this, wxID_ANY, "Length:"), 0, wxALIGN_CENTER_VERTICAL);

    // FIX: Use proper constructor for wxSpinCtrl
    spinLength_ = new wxSpinCtrl(this, ID_SPIN_LENGTH);
    spinLength_->SetRange(4, 64);
    spinLength_->SetValue(16);  // Set default value separately

    lenSizer->Add(spinLength_, 0, wxLEFT, 5);
    mainSizer->Add(lenSizer, 0, wxEXPAND | wxALL, 10);

    // Options
    chkUpper_ = new wxCheckBox(this, wxID_ANY, "Uppercase (A-Z)");
    chkUpper_->SetValue(true);
    mainSizer->Add(chkUpper_, 0, wxLEFT | wxRIGHT, 10);

    chkLower_ = new wxCheckBox(this, wxID_ANY, "Lowercase (a-z)");
    chkLower_->SetValue(true);
    mainSizer->Add(chkLower_, 0, wxLEFT | wxRIGHT, 10);

    chkDigits_ = new wxCheckBox(this, wxID_ANY, "Digits (0-9)");
    chkDigits_->SetValue(true);
    mainSizer->Add(chkDigits_, 0, wxLEFT | wxRIGHT, 10);

    chkSpecial_ = new wxCheckBox(this, wxID_ANY, "Special (!@#$...)");  // Fixed quote
    chkSpecial_->SetValue(true);
    mainSizer->Add(chkSpecial_, 0, wxLEFT | wxRIGHT, 10);

    // Result
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Generated Password:"), 0, wxLEFT | wxRIGHT | wxTOP, 10);
    txtResult_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    mainSizer->Add(txtResult_, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, ID_GENERATE, "Generate"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, ID_COPY, "Copy to Clipboard"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, wxID_CLOSE, "Close"), 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
}

void PasswordGeneratorDialog::OnGenerate(wxCommandEvent& event) {
    std::string pass = pm_->generatePassword(
        spinLength_->GetValue(),
        chkUpper_->IsChecked(),
        chkLower_->IsChecked(),
        chkDigits_->IsChecked(),
        chkSpecial_->IsChecked()
    );
    txtResult_->SetValue(pass);
}

void PasswordGeneratorDialog::OnCopy(wxCommandEvent& event) {
    wxString text = txtResult_->GetValue();
    if (!text.IsEmpty() && wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(text));
        wxTheClipboard->Close();
        wxMessageBox("Password copied to clipboard!", "Success", wxOK | wxICON_INFORMATION);
    }
}