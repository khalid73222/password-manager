#define _CRT_SECURE_NO_WARNINGS
#include "PasswordDialog.h"
#include "PinDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>

enum {
    ID_SHOW_CHK = 3000,
    ID_SAVE,
    ID_TAG_CHOICE,
    ID_TXT_PASSWORD
};

wxBEGIN_EVENT_TABLE(PasswordDialog, wxDialog)
EVT_CHECKBOX(ID_SHOW_CHK, PasswordDialog::OnShowPassword)
EVT_BUTTON(ID_SAVE, PasswordDialog::OnSave)
EVT_TEXT(ID_TXT_PASSWORD, PasswordDialog::OnPasswordChanged)
wxEND_EVENT_TABLE()

PasswordDialog::PasswordDialog(wxWindow* parent, PasswordManager* pm, PasswordEntry* entry)
    : wxDialog(parent, wxID_ANY, "Edit Password",
        wxDefaultPosition, wxSize(470, 580),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    pm_(pm), entry_(entry), showingPassword_(false)
{
    SetBackgroundColour(wxColour(250, 250, 252));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title bar strip
    wxPanel* titlePanel = new wxPanel(this, wxID_ANY);
    titlePanel->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* titleSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* titleLbl = new wxStaticText(titlePanel, wxID_ANY, "  Edit Password Entry");
    titleLbl->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleLbl->GetFont();
    titleFont.SetPointSize(11);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLbl->SetFont(titleFont);
    titleSizer->Add(titleLbl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);
    titlePanel->SetSizer(titleSizer);
    mainSizer->Add(titlePanel, 0, wxEXPAND);

    // Form fields
    wxPanel* formPanel = new wxPanel(this, wxID_ANY);
    formPanel->SetBackgroundColour(wxColour(250, 250, 252));
    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);

    auto addField = [&](const wxString& label, wxTextCtrl*& ctrl,
        const wxString& value, long style = 0) {
            wxStaticText* lbl = new wxStaticText(formPanel, wxID_ANY, label);
            wxFont f = lbl->GetFont();
            f.SetWeight(wxFONTWEIGHT_BOLD);
            lbl->SetFont(f);
            lbl->SetForegroundColour(wxColour(70, 70, 90));
            formSizer->Add(lbl, 0, wxLEFT | wxRIGHT | wxTOP, 12);
            ctrl = new wxTextCtrl(formPanel, wxID_ANY, value,
                wxDefaultPosition, wxDefaultSize, style);
            formSizer->Add(ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
        };

    addField("Service", txtService_, entry->service);
    addField("Username", txtUsername_, entry->username);

    // Password field with specific ID for EVT_TEXT binding
    {
        wxStaticText* lbl = new wxStaticText(formPanel, wxID_ANY, "Password");
        wxFont f = lbl->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        lbl->SetFont(f); lbl->SetForegroundColour(wxColour(70, 70, 90));
        formSizer->Add(lbl, 0, wxLEFT | wxRIGHT | wxTOP, 12);

        txtPassword_ = new wxTextCtrl(formPanel, ID_TXT_PASSWORD, entry->password,
            wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        formSizer->Add(txtPassword_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    // Show password checkbox
    chkShow_ = new wxCheckBox(formPanel, ID_SHOW_CHK, "Show password (requires PIN)");
    chkShow_->SetForegroundColour(wxColour(60, 60, 80));
    formSizer->Add(chkShow_, 0, wxLEFT | wxTOP, 12);

    // Strength meter
    formSizer->AddSpacer(8);
    wxBoxSizer* strengthSizer = new wxBoxSizer(wxHORIZONTAL);
    lblStrength_ = new wxStaticText(formPanel, wxID_ANY, "Strength: --");
    lblStrength_->SetForegroundColour(wxColour(100, 100, 120));
    strengthSizer->Add(lblStrength_, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    gaugeStrength_ = new wxGauge(formPanel, wxID_ANY, 100,
        wxDefaultPosition, wxSize(160, 16));
    strengthSizer->Add(gaugeStrength_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    formSizer->Add(strengthSizer, 0, wxEXPAND);

    // Age warning
    lblWarning_ = new wxStaticText(formPanel, wxID_ANY, "");
    lblWarning_->SetForegroundColour(wxColour(230, 100, 0));
    formSizer->Add(lblWarning_, 0, wxLEFT | wxRIGHT | wxTOP, 12);

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
    choiceTag_->SetSelection(static_cast<int>(entry->tag));
    formSizer->Add(choiceTag_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    addField("URL (optional)", txtUrl_, entry->url);
    addField("Notes (optional)", txtNotes_, entry->notes, wxTE_MULTILINE);
    if (txtNotes_) {
        formSizer->GetItem(formSizer->GetItemCount() - 1)->SetMinSize(wxSize(-1, 60));
    }

    formPanel->SetSizer(formSizer);
    mainSizer->Add(formPanel, 1, wxEXPAND);

    // Button bar
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer(1);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 6);
    wxButton* btnSave = new wxButton(this, ID_SAVE, "Save Changes");
    btnSave->SetBackgroundColour(wxColour(30, 34, 45));
    btnSave->SetForegroundColour(*wxWHITE);
    btnSave->SetDefault();
    btnSizer->Add(btnSave, 0, wxALL, 6);
    mainSizer->Add(btnSizer, 0, wxEXPAND);

    SetSizer(mainSizer);

    updateStrengthMeter(entry->password);
    checkAgeWarning();
}

void PasswordDialog::OnShowPassword(wxCommandEvent&) {
    if (chkShow_->IsChecked()) {
        // Ask for PIN before revealing
        PinDialog pinDlg(this, PinMode::VERIFY);
        if (pinDlg.ShowModal() != wxID_OK ||
            !pm_->verifyPin(pinDlg.GetPin().ToStdString())) {
            wxMessageBox("Incorrect PIN.", "Access Denied", wxOK | wxICON_ERROR);
            chkShow_->SetValue(false);
            return;
        }
        showingPassword_ = true;
    }
    else {
        showingPassword_ = false;
    }

    // Recreate the text control with the correct style
    wxString currentValue = txtPassword_->GetValue();
    wxSizer* parentSizer = txtPassword_->GetContainingSizer();
    int      sizerPos = -1;

    if (parentSizer) {
        for (size_t i = 0; i < parentSizer->GetItemCount(); ++i) {
            if (parentSizer->GetItem(i)->GetWindow() == txtPassword_) {
                sizerPos = static_cast<int>(i);
                break;
            }
        }
    }

    long newStyle = showingPassword_ ? 0 : wxTE_PASSWORD;
    wxTextCtrl* newCtrl = new wxTextCtrl(
        txtPassword_->GetParent(), ID_TXT_PASSWORD,
        currentValue, wxDefaultPosition, wxDefaultSize, newStyle);
    newCtrl->Bind(wxEVT_TEXT, &PasswordDialog::OnPasswordChanged, this);

    if (parentSizer && sizerPos >= 0) {
        parentSizer->Remove(sizerPos);
        parentSizer->Insert(sizerPos, newCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    txtPassword_->Destroy();
    txtPassword_ = newCtrl;

    Layout();
    Refresh();
}

void PasswordDialog::OnSave(wxCommandEvent&) {
    PasswordTag tag = static_cast<PasswordTag>(choiceTag_->GetSelection());
    pm_->updatePassword(
        entry_->id,
        txtService_->GetValue().ToStdString(),
        txtUsername_->GetValue().ToStdString(),
        txtPassword_->GetValue().ToStdString(),
        txtUrl_->GetValue().ToStdString(),
        txtNotes_->GetValue().ToStdString(),
        tag
    );
    EndModal(wxID_OK);
}

void PasswordDialog::OnPasswordChanged(wxCommandEvent&) {
    updateStrengthMeter(txtPassword_->GetValue().ToStdString());
}

void PasswordDialog::updateStrengthMeter(const std::string& password) {
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

// FIX: Replaced Unicode characters with ASCII equivalents
void PasswordDialog::checkAgeWarning() {
    std::time_t now = std::time(nullptr);
    double days = std::difftime(now, entry_->lastChanged) / (60.0 * 60.0 * 24.0);

    if (days > 180) {
        lblWarning_->SetLabel(
            wxString::Format("! Password is %.0f days old -- please change it!", days));
        lblWarning_->SetForegroundColour(wxColour(220, 80, 0));
    }
    else if (days > 150) {
        lblWarning_->SetLabel(
            wxString::Format("! Password is %.0f days old -- consider updating.", days));
        lblWarning_->SetForegroundColour(wxColour(200, 130, 0));
    }
    else {
        lblWarning_->SetLabel(
            wxString::Format("OK Password is %.0f days old -- looks good.", days));
        lblWarning_->SetForegroundColour(wxColour(40, 167, 69));
    }
}