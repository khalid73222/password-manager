#define _CRT_SECURE_NO_WARNINGS
#include "PasswordDialog.h"
#include "PinDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>

enum {
    ID_SHOW_CHK = 3000,
    ID_SAVE = 3001,
    ID_CAT_CHOICE_PD = 3002,
    ID_TXT_PASSWORD = 3003
};

wxBEGIN_EVENT_TABLE(PasswordDialog, wxDialog)
EVT_CHECKBOX(ID_SHOW_CHK, PasswordDialog::OnShowPassword)
EVT_BUTTON(ID_SAVE, PasswordDialog::OnSave)
EVT_TEXT(ID_TXT_PASSWORD, PasswordDialog::OnPasswordChanged)
wxEND_EVENT_TABLE()

PasswordDialog::PasswordDialog(wxWindow* parent, PasswordManager* pm, PasswordEntry* entry)
    : wxDialog(parent, wxID_ANY, "Edit Password",
        wxDefaultPosition, wxSize(480, 600),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    pm_(pm), entry_(entry), showingPassword_(false)
{
    SetBackgroundColour(wxColour(250, 250, 252));
    wxBoxSizer* root = new wxBoxSizer(wxVERTICAL);

    // Header
    wxPanel* hdr = new wxPanel(this, wxID_ANY);
    hdr->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* hdrS = new wxBoxSizer(wxHORIZONTAL);
    auto* hdrLbl = new wxStaticText(hdr, wxID_ANY, "  Edit Password Entry");
    hdrLbl->SetForegroundColour(*wxWHITE);
    wxFont hf = hdrLbl->GetFont(); hf.SetPointSize(11); hf.SetWeight(wxFONTWEIGHT_BOLD);
    hdrLbl->SetFont(hf);
    hdrS->Add(hdrLbl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);
    hdr->SetSizer(hdrS);
    root->Add(hdr, 0, wxEXPAND);

    // Form
    wxPanel* form = new wxPanel(this, wxID_ANY);
    form->SetBackgroundColour(wxColour(250, 250, 252));
    wxBoxSizer* fs = new wxBoxSizer(wxVERTICAL);

    auto addField = [&](const wxString& lbl, wxTextCtrl*& ctrl,
        const wxString& val, long style = 0) {
            auto* l = new wxStaticText(form, wxID_ANY, lbl);
            wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
            l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
            fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
            ctrl = new wxTextCtrl(form, wxID_ANY, val,
                wxDefaultPosition, wxDefaultSize, style);
            fs->Add(ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
        };

    addField("Service", txtService_, entry->service);
    addField("Username", txtUsername_, entry->username);

    // Password
    {
        auto* l = new wxStaticText(form, wxID_ANY, "Password");
        wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
        fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
        txtPassword_ = new wxTextCtrl(form, ID_TXT_PASSWORD, entry->password,
            wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        fs->Add(txtPassword_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    chkShow_ = new wxCheckBox(form, ID_SHOW_CHK, "Show password (requires PIN)");
    chkShow_->SetForegroundColour(wxColour(60, 60, 80));
    fs->Add(chkShow_, 0, wxLEFT | wxTOP, 12);

    // Strength
    fs->AddSpacer(8);
    wxBoxSizer* strRow = new wxBoxSizer(wxHORIZONTAL);
    lblStrength_ = new wxStaticText(form, wxID_ANY, "Strength: --");
    lblStrength_->SetForegroundColour(wxColour(120, 120, 140));
    strRow->Add(lblStrength_, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    gaugeStrength_ = new wxGauge(form, wxID_ANY, 100,
        wxDefaultPosition, wxSize(160, 14));
    strRow->Add(gaugeStrength_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    fs->Add(strRow, 0, wxEXPAND);

    // Age warning
    lblWarning_ = new wxStaticText(form, wxID_ANY, "");
    lblWarning_->SetForegroundColour(wxColour(200, 80, 0));
    fs->Add(lblWarning_, 0, wxLEFT | wxRIGHT | wxTOP, 12);

    // Category
    {
        auto* l = new wxStaticText(form, wxID_ANY, "Category");
        wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
        fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    }
    choiceCategory_ = new wxChoice(form, ID_CAT_CHOICE_PD);
    PopulateCategories();
    fs->Add(choiceCategory_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    addField("URL (optional)", txtUrl_, entry->url);
    addField("Notes (optional)", txtNotes_, entry->notes, wxTE_MULTILINE);
    if (auto* item = fs->GetItem(fs->GetItemCount() - 1))
        item->SetMinSize(wxSize(-1, 60));

    form->SetSizer(fs);
    root->Add(form, 1, wxEXPAND);

    // Footer
    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);
    wxBoxSizer* btnRow = new wxBoxSizer(wxHORIZONTAL);
    btnRow->AddStretchSpacer(1);
    btnRow->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 6);
    auto* btnSave = new wxButton(this, ID_SAVE, "Save Changes");
    btnSave->SetBackgroundColour(wxColour(30, 34, 45));
    btnSave->SetForegroundColour(*wxWHITE);
    btnSave->SetDefault();
    btnRow->Add(btnSave, 0, wxALL, 6);
    root->Add(btnRow, 0, wxEXPAND);

    SetSizer(root);
    updateStrengthMeter(entry->password);
    checkAgeWarning();
}

void PasswordDialog::PopulateCategories() {
    choiceCategory_->Clear();
    choiceCategory_->Append("(none)");
    auto cats = pm_->getCategories();
    int sel = 0;
    for (int i = 0; i < (int)cats.size(); ++i) {
        choiceCategory_->Append(cats[i].name);
        if (cats[i].name == entry_->category) sel = i + 1;
    }
    choiceCategory_->SetSelection(sel);
}

void PasswordDialog::OnShowPassword(wxCommandEvent&) {
    if (chkShow_->IsChecked()) {
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
    wxString val = txtPassword_->GetValue();
    wxSizer* sizer = txtPassword_->GetContainingSizer();
    int pos = -1;
    if (sizer) {
        for (size_t i = 0; i < sizer->GetItemCount(); ++i)
            if (sizer->GetItem(i)->GetWindow() == txtPassword_) { pos = (int)i; break; }
    }
    long style = showingPassword_ ? 0 : wxTE_PASSWORD;
    auto* nc = new wxTextCtrl(txtPassword_->GetParent(), ID_TXT_PASSWORD,
        val, wxDefaultPosition, wxDefaultSize, style);
    nc->Bind(wxEVT_TEXT, &PasswordDialog::OnPasswordChanged, this);
    if (sizer && pos >= 0) {
        sizer->Remove(pos);
        sizer->Insert(pos, nc, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }
    txtPassword_->Destroy();
    txtPassword_ = nc;
    Layout(); Refresh();
}

void PasswordDialog::OnSave(wxCommandEvent&) {
    std::string cat = "";
    int sel = choiceCategory_->GetSelection();
    if (sel > 0) {
        auto cats = pm_->getCategories();
        if (sel - 1 < (int)cats.size()) cat = cats[sel - 1].name;
    }
    pm_->updatePassword(
        entry_->id,
        txtService_->GetValue().ToStdString(),
        txtUsername_->GetValue().ToStdString(),
        txtPassword_->GetValue().ToStdString(),
        txtUrl_->GetValue().ToStdString(),
        txtNotes_->GetValue().ToStdString(),
        cat);
    EndModal(wxID_OK);
}

void PasswordDialog::OnPasswordChanged(wxCommandEvent&) {
    updateStrengthMeter(txtPassword_->GetValue().ToStdString());
}

void PasswordDialog::updateStrengthMeter(const std::string& pw) {
    if (pw.empty()) {
        gaugeStrength_->SetValue(0);
        lblStrength_->SetLabel("Strength: --");
        lblStrength_->SetForegroundColour(wxColour(150, 150, 150));
        return;
    }
    auto r = pm_->checkStrength(pw);
    gaugeStrength_->SetValue(r.score);
    lblStrength_->SetLabel(wxString::Format("Strength: %s (%d%%)", r.label, r.score));
    lblStrength_->SetForegroundColour(r.color);
    Refresh();
}

void PasswordDialog::checkAgeWarning() {
    double days = std::difftime(std::time(nullptr), entry_->lastChanged) / 86400.0;
    if (days > 180)
        lblWarning_->SetLabel(wxString::Format(
            "! Password is %.0f days old - please change it!", days));
    else if (days > 150)
        lblWarning_->SetLabel(wxString::Format(
            "~ Password is %.0f days old - consider updating.", days));
    else
        lblWarning_->SetLabel(wxString::Format(
            "OK Password age: %.0f days", days));
    lblWarning_->SetForegroundColour(
        days > 180 ? wxColour(210, 60, 0) :
        days > 150 ? wxColour(190, 130, 0) : wxColour(40, 167, 69));
}