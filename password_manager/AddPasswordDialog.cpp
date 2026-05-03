#define _CRT_SECURE_NO_WARNINGS
#include "AddPasswordDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>

enum {
    ID_GENERATE_BTN = 2000,
    ID_SAVE_BTN,
    ID_TAG_CHOICE,
    ID_TXT_PASSWORD
};

wxBEGIN_EVENT_TABLE(AddPasswordDialog, wxDialog)
EVT_BUTTON(ID_GENERATE_BTN, AddPasswordDialog::OnGenerate)
EVT_BUTTON(ID_SAVE_BTN, AddPasswordDialog::OnSave)
EVT_TEXT(ID_TXT_PASSWORD, AddPasswordDialog::OnPasswordChanged)
wxEND_EVENT_TABLE()

AddPasswordDialog::AddPasswordDialog(wxWindow* parent, PasswordManager* pm)
    : wxDialog(parent, wxID_ANY, "Add New Password",
        wxDefaultPosition, wxSize(470, 560),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    pm_(pm)
{
    SetBackgroundColour(wxColour(250, 250, 252));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title strip
    wxPanel* titlePanel = new wxPanel(this, wxID_ANY);
    titlePanel->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* titleSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* titleLbl = new wxStaticText(titlePanel, wxID_ANY, "  Add New Password Entry");
    titleLbl->SetForegroundColour(*wxWHITE);
    wxFont tf = titleLbl->GetFont();
    tf.SetPointSize(11); tf.SetWeight(wxFONTWEIGHT_BOLD);
    titleLbl->SetFont(tf);
    titleSizer->Add(titleLbl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);
    titlePanel->SetSizer(titleSizer);
    mainSizer->Add(titlePanel, 0, wxEXPAND);

    // Form
    wxPanel* formPanel = new wxPanel(this, wxID_ANY);
    formPanel->SetBackgroundColour(wxColour(250, 250, 252));
    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);

    auto addField = [&](const wxString& label, wxTextCtrl*& ctrl,
        long style = 0) {
            wxStaticText* lbl = new wxStaticText(formPanel, wxID_ANY, label);
            wxFont f = lbl->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
            lbl->SetFont(f); lbl->SetForegroundColour(wxColour(70, 70, 90));
            formSizer->Add(lbl, 0, wxLEFT | wxRIGHT | wxTOP, 12);
            ctrl = new wxTextCtrl(formPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, style);
            formSizer->Add(ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
        };

    addField("Service *", txtService_);
    addField("Username *", txtUsername_);

    // Password row (with Generate button)
    {
        wxStaticText* lbl = new wxStaticText(formPanel, wxID_ANY, "Password *");
        wxFont f = lbl->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        lbl->SetFont(f); lbl->SetForegroundColour(wxColour(70, 70, 90));
        formSizer->Add(lbl, 0, wxLEFT | wxRIGHT | wxTOP, 12);

        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);
        txtPassword_ = new wxTextCtrl(formPanel, ID_TXT_PASSWORD, "",
            wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        rowSizer->Add(txtPassword_, 1, wxRIGHT, 6);

        wxButton* btnGen = new wxButton(formPanel, ID_GENERATE_BTN, "Generate",
            wxDefaultPosition, wxSize(90, -1));
        btnGen->SetBackgroundColour(wxColour(52, 58, 75));
        btnGen->SetForegroundColour(*wxWHITE);
        rowSizer->Add(btnGen, 0);
        formSizer->Add(rowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    // Strength meter
    formSizer->AddSpacer(6);
    wxBoxSizer* strSizer = new wxBoxSizer(wxHORIZONTAL);
    lblStrength_ = new wxStaticText(formPanel, wxID_ANY, "Strength: --");
    lblStrength_->SetForegroundColour(wxColour(100, 100, 120));
    strSizer->Add(lblStrength_, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    gaugeStrength_ = new wxGauge(formPanel, wxID_ANY, 100,
        wxDefaultPosition, wxSize(160, 16));
    strSizer->Add(gaugeStrength_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    formSizer->Add(strSizer, 0, wxEXPAND);

    // Category
    {
        wxStaticText* lbl = new wxStaticText(formPanel, wxID_ANY, "Category");
        wxFont f = lbl->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        lbl->SetFont(f); lbl->SetForegroundColour(wxColour(70, 70, 90));
        formSizer->Add(lbl, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    }
    choiceTag_ = new wxChoice(formPanel, ID_TAG_CHOICE);
    choiceTag_->Append("None");
    choiceTag_->Append("Work");
    choiceTag_->Append("Personal");
    choiceTag_->Append("Finance");
    choiceTag_->Append("Social");
    choiceTag_->SetSelection(0);
    formSizer->Add(choiceTag_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    addField("URL (optional)", txtUrl_);
    addField("Notes (optional)", txtNotes_, wxTE_MULTILINE);
    formSizer->GetItem(formSizer->GetItemCount() - 1)->SetMinSize(wxSize(-1, 55));

    formPanel->SetSizer(formSizer);
    mainSizer->Add(formPanel, 1, wxEXPAND);

    // Button bar
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer(1);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 6);
    wxButton* btnSave = new wxButton(this, ID_SAVE_BTN, "Save Password");
    btnSave->SetBackgroundColour(wxColour(30, 34, 45));
    btnSave->SetForegroundColour(*wxWHITE);
    btnSave->SetDefault();
    btnSizer->Add(btnSave, 0, wxALL, 6);
    mainSizer->Add(btnSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
}

void AddPasswordDialog::OnGenerate(wxCommandEvent&) {
    std::string pass = pm_->generatePassword(16, true, true, true, true);
    txtPassword_->SetValue(pass);
    updateStrengthMeter(pass);
}

void AddPasswordDialog::OnSave(wxCommandEvent&) {
    wxString service = txtService_->GetValue().Trim();
    wxString username = txtUsername_->GetValue().Trim();
    wxString password = txtPassword_->GetValue();

    if (service.IsEmpty() || username.IsEmpty() || password.IsEmpty()) {
        wxMessageBox("Please fill in Service, Username, and Password.", "Required Fields",
            wxOK | wxICON_WARNING);
        return;
    }

    PasswordTag tag = static_cast<PasswordTag>(choiceTag_->GetSelection());
    pm_->addPassword(service.ToStdString(), username.ToStdString(), password.ToStdString(),
        txtUrl_->GetValue().ToStdString(), txtNotes_->GetValue().ToStdString(), tag);
    EndModal(wxID_OK);
}

void AddPasswordDialog::OnPasswordChanged(wxCommandEvent&) {
    updateStrengthMeter(txtPassword_->GetValue().ToStdString());
}

void AddPasswordDialog::updateStrengthMeter(const std::string& password) {
    if (password.empty()) {
        gaugeStrength_->SetValue(0);
        lblStrength_->SetLabel("Strength: --");
        lblStrength_->SetForegroundColour(wxColour(150, 150, 150));
        return;
    }
    auto result = pm_->checkStrength(password);
    gaugeStrength_->SetValue(result.score);
    lblStrength_->SetLabel(
        wxString::Format("Strength: %s (%d%%)", result.label, result.score));
    lblStrength_->SetForegroundColour(result.color);
    Refresh();
}