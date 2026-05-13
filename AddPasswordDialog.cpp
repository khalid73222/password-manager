#define _CRT_SECURE_NO_WARNINGS
#include "AddPasswordDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>

enum {
    ID_GENERATE_BTN = 2000,
    ID_SAVE_BTN,
    ID_CATEGORY_CHOICE,
    ID_TXT_PASSWORD
};

wxBEGIN_EVENT_TABLE(AddPasswordDialog, wxDialog)
EVT_BUTTON(ID_GENERATE_BTN, AddPasswordDialog::OnGenerate)
EVT_BUTTON(ID_SAVE_BTN, AddPasswordDialog::OnSave)
EVT_TEXT(ID_TXT_PASSWORD, AddPasswordDialog::OnPasswordChanged)
wxEND_EVENT_TABLE()

AddPasswordDialog::AddPasswordDialog(wxWindow* parent, PasswordManager* pm)
    : wxDialog(parent, wxID_ANY, "Add New Password",
        wxDefaultPosition, wxSize(480, 580),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    pm_(pm)
{
    SetBackgroundColour(wxColour(250, 250, 252));
    wxBoxSizer* root = new wxBoxSizer(wxVERTICAL);

    // Header
    wxPanel* hdr = new wxPanel(this, wxID_ANY);
    hdr->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* hdrS = new wxBoxSizer(wxHORIZONTAL);
    auto* hdrLbl = new wxStaticText(hdr, wxID_ANY, "  Add New Password Entry");
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

    auto addField = [&](const wxString& lbl, wxTextCtrl*& ctrl, long style = 0) {
        auto* l = new wxStaticText(form, wxID_ANY, lbl);
        wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
        fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
        ctrl = new wxTextCtrl(form, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, style);
        fs->Add(ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
        };

    addField("Service *", txtService_);
    addField("Username *", txtUsername_);

    // Password row
    {
        auto* l = new wxStaticText(form, wxID_ANY, "Password *");
        wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
        fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        txtPassword_ = new wxTextCtrl(form, ID_TXT_PASSWORD, "",
            wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        row->Add(txtPassword_, 1, wxRIGHT, 6);
        auto* btnGen = new wxButton(form, ID_GENERATE_BTN, "Generate",
            wxDefaultPosition, wxSize(100, -1));
        btnGen->SetBackgroundColour(wxColour(52, 58, 75));
        btnGen->SetForegroundColour(*wxWHITE);
        row->Add(btnGen, 0);
        fs->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    }

    // Strength
    fs->AddSpacer(6);
    wxBoxSizer* strRow = new wxBoxSizer(wxHORIZONTAL);
    lblStrength_ = new wxStaticText(form, wxID_ANY, "Strength: --");
    lblStrength_->SetForegroundColour(wxColour(120, 120, 140));
    strRow->Add(lblStrength_, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    gaugeStrength_ = new wxGauge(form, wxID_ANY, 100,
        wxDefaultPosition, wxSize(160, 14));
    strRow->Add(gaugeStrength_, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    fs->Add(strRow, 0, wxEXPAND);

    // Category dropdown
    {
        auto* l = new wxStaticText(form, wxID_ANY, "Category");
        wxFont f = l->GetFont(); f.SetWeight(wxFONTWEIGHT_BOLD);
        l->SetFont(f); l->SetForegroundColour(wxColour(60, 60, 80));
        fs->Add(l, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    }
    choiceCategory_ = new wxChoice(form, ID_CATEGORY_CHOICE);
    PopulateCategories();
    fs->Add(choiceCategory_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    addField("URL (optional)", txtUrl_);
    addField("Notes (optional)", txtNotes_, wxTE_MULTILINE);
    if (auto* item = fs->GetItem(fs->GetItemCount() - 1))
        item->SetMinSize(wxSize(-1, 55));

    form->SetSizer(fs);
    root->Add(form, 1, wxEXPAND);

    // Footer
    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);
    wxBoxSizer* btnRow = new wxBoxSizer(wxHORIZONTAL);
    btnRow->AddStretchSpacer(1);
    btnRow->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 6);
    auto* btnSave = new wxButton(this, ID_SAVE_BTN, "Save Password");
    btnSave->SetBackgroundColour(wxColour(30, 34, 45));
    btnSave->SetForegroundColour(*wxWHITE);
    btnSave->SetDefault();
    btnRow->Add(btnSave, 0, wxALL, 6);
    root->Add(btnRow, 0, wxEXPAND);

    SetSizer(root);
}

void AddPasswordDialog::PopulateCategories() {
    choiceCategory_->Clear();
    choiceCategory_->Append("(none)");
    for (const auto& cat : pm_->getCategories())
        choiceCategory_->Append(cat.name);
    choiceCategory_->SetSelection(0);
}

void AddPasswordDialog::OnGenerate(wxCommandEvent&) {
    std::string pw = pm_->generatePassword(16, true, true, true, true);
    txtPassword_->SetValue(pw);
    updateStrengthMeter(pw);
}

void AddPasswordDialog::OnSave(wxCommandEvent&) {
    wxString svc = txtService_->GetValue().Trim();
    wxString user = txtUsername_->GetValue().Trim();
    wxString pw = txtPassword_->GetValue();
    if (svc.IsEmpty() || user.IsEmpty() || pw.IsEmpty()) {
        wxMessageBox("Service, Username, and Password are required.",
            "Required Fields", wxOK | wxICON_WARNING);
        return;
    }
    std::string cat = "";
    int sel = choiceCategory_->GetSelection();
    if (sel > 0) {  // 0 = "(none)"
        auto cats = pm_->getCategories();
        if (sel - 1 < (int)cats.size())
            cat = cats[sel - 1].name;
    }
    pm_->addPassword(svc.ToStdString(), user.ToStdString(), pw.ToStdString(),
        txtUrl_->GetValue().ToStdString(),
        txtNotes_->GetValue().ToStdString(), cat);
    EndModal(wxID_OK);
}

void AddPasswordDialog::OnPasswordChanged(wxCommandEvent&) {
    updateStrengthMeter(txtPassword_->GetValue().ToStdString());
}

void AddPasswordDialog::updateStrengthMeter(const std::string& pw) {
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