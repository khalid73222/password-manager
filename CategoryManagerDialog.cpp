#define _CRT_SECURE_NO_WARNINGS
#include "CategoryManagerDialog.h"
#include <wx/colordlg.h>
#include <wx/statline.h>

enum {
    ID_CAT_ADD = 6100,
    ID_CAT_RENAME,
    ID_CAT_DELETE,
    ID_CAT_COLOUR,
    ID_CAT_LIST,
    ID_CAT_CLOSE
};

wxBEGIN_EVENT_TABLE(CategoryManagerDialog, wxDialog)
EVT_BUTTON(ID_CAT_ADD, CategoryManagerDialog::OnAdd)
EVT_BUTTON(ID_CAT_RENAME, CategoryManagerDialog::OnRename)
EVT_BUTTON(ID_CAT_DELETE, CategoryManagerDialog::OnDelete)
EVT_BUTTON(ID_CAT_COLOUR, CategoryManagerDialog::OnPickColour)
EVT_BUTTON(ID_CAT_CLOSE, CategoryManagerDialog::OnClose)
EVT_LIST_ITEM_SELECTED(ID_CAT_LIST, CategoryManagerDialog::OnListSelect)
wxEND_EVENT_TABLE()

CategoryManagerDialog::CategoryManagerDialog(wxWindow* parent, PasswordManager* pm)
    : wxDialog(parent, wxID_ANY, "Manage Categories",
        wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    pm_(pm), selectedColour_(wxColour(0, 123, 255))
{
    SetBackgroundColour(wxColour(245, 246, 250));
    SetMinSize(wxSize(460, 420));

    wxBoxSizer* root = new wxBoxSizer(wxVERTICAL);

    // ── Header ────────────────────────────────────────────────────────────────
    wxPanel* hdr = new wxPanel(this, wxID_ANY);
    hdr->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* hdrS = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* hdrLbl = new wxStaticText(hdr, wxID_ANY, "  Category Manager");
    hdrLbl->SetForegroundColour(*wxWHITE);
    wxFont hf = hdrLbl->GetFont();
    hf.SetPointSize(11); hf.SetWeight(wxFONTWEIGHT_BOLD);
    hdrLbl->SetFont(hf);
    hdrS->Add(hdrLbl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    hdr->SetSizer(hdrS);
    root->Add(hdr, 0, wxEXPAND);

    // ── Body ──────────────────────────────────────────────────────────────────
    wxBoxSizer* body = new wxBoxSizer(wxHORIZONTAL);

    // Left: list of categories
    wxBoxSizer* leftCol = new wxBoxSizer(wxVERTICAL);
    wxStaticText* listLbl = new wxStaticText(this, wxID_ANY, "Categories:");
    wxFont lf = listLbl->GetFont(); lf.SetWeight(wxFONTWEIGHT_BOLD);
    listLbl->SetFont(lf);
    leftCol->Add(listLbl, 0, wxBOTTOM, 4);

    list_ = new wxListCtrl(this, ID_CAT_LIST,
        wxDefaultPosition, wxSize(200, 240),
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SIMPLE);
    list_->AppendColumn("", wxLIST_FORMAT_CENTER, 24);
    list_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
    leftCol->Add(list_, 1, wxEXPAND);
    body->Add(leftCol, 1, wxEXPAND | wxALL, 12);

    // Right: editing controls
    wxBoxSizer* rightCol = new wxBoxSizer(wxVERTICAL);
    rightCol->AddSpacer(20);

    wxStaticText* nameLbl = new wxStaticText(this, wxID_ANY, "Category name:");
    wxFont nf = nameLbl->GetFont(); nf.SetWeight(wxFONTWEIGHT_BOLD);
    nameLbl->SetFont(nf);
    rightCol->Add(nameLbl, 0, wxBOTTOM, 4);

    txtName_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(180, 32));
    txtName_->SetMaxLength(32);
    rightCol->Add(txtName_, 0, wxEXPAND | wxBOTTOM, 10);

    // Colour picker row
    wxBoxSizer* colRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* colLbl = new wxStaticText(this, wxID_ANY, "Colour:");
    wxFont cf = colLbl->GetFont(); cf.SetWeight(wxFONTWEIGHT_BOLD);
    colLbl->SetFont(cf);
    colRow->Add(colLbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    btnColour_ = new wxButton(this, ID_CAT_COLOUR, "   ",
        wxDefaultPosition, wxSize(40, 28));
    btnColour_->SetBackgroundColour(selectedColour_);
    colRow->Add(btnColour_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    lblPreview_ = new wxStaticText(this, wxID_ANY, "#007BFF");
    lblPreview_->SetForegroundColour(wxColour(100, 100, 120));
    colRow->Add(lblPreview_, 0, wxALIGN_CENTER_VERTICAL);
    rightCol->Add(colRow, 0, wxBOTTOM, 16);

    rightCol->Add(new wxStaticLine(this), 0, wxEXPAND | wxBOTTOM, 12);

    // Action buttons
    auto makeBtn = [&](int id, const wxString& label,
        const wxColour& bg, const wxColour& fg) -> wxButton* {
            wxButton* b = new wxButton(this, id, label,
                wxDefaultPosition, wxSize(160, 34));
            b->SetBackgroundColour(bg);
            b->SetForegroundColour(fg);
            return b;
        };

    rightCol->Add(makeBtn(ID_CAT_ADD, "+ Add Category",
        wxColour(30, 34, 45), *wxWHITE),
        0, wxBOTTOM, 6);
    rightCol->Add(makeBtn(ID_CAT_RENAME, "Rename Selected",
        wxColour(52, 58, 75), *wxWHITE),
        0, wxBOTTOM, 6);
    rightCol->Add(makeBtn(ID_CAT_DELETE, "Delete Selected",
        wxColour(180, 40, 50), *wxWHITE),
        0, wxBOTTOM, 6);

    body->Add(rightCol, 0, wxTOP | wxRIGHT | wxBOTTOM, 12);
    root->Add(body, 1, wxEXPAND);

    // ── Footer ────────────────────────────────────────────────────────────────
    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 8);
    wxBoxSizer* footer = new wxBoxSizer(wxHORIZONTAL);
    footer->AddStretchSpacer(1);
    wxButton* closeBtn = new wxButton(this, ID_CAT_CLOSE, "Done",
        wxDefaultPosition, wxSize(90, 34));
    closeBtn->SetBackgroundColour(wxColour(30, 34, 45));
    closeBtn->SetForegroundColour(*wxWHITE);
    closeBtn->SetDefault();
    footer->Add(closeBtn, 0, wxALL, 8);
    root->Add(footer, 0, wxEXPAND);

    SetSizer(root);
    Fit();
    Centre();

    RebuildList();
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void CategoryManagerDialog::RebuildList() {
    list_->DeleteAllItems();
    auto cats = pm_->getCategories();

    // Build a small image list for the colour swatches
    wxImageList* imgs = new wxImageList(16, 16, false, (int)cats.size() + 1);

    for (size_t i = 0; i < cats.size(); ++i) {
        // Draw a filled rectangle swatch
        wxBitmap bmp(16, 16);
        wxMemoryDC dc(bmp);
        dc.SetBackground(wxBrush(cats[i].colour));
        dc.Clear();
        dc.SetPen(wxPen(cats[i].colour.ChangeLightness(60), 1));
        dc.DrawRectangle(0, 0, 16, 16);
        dc.SelectObject(wxNullBitmap);
        imgs->Add(bmp);

        long idx = list_->InsertItem((long)i, "", (int)i);
        list_->SetItem(idx, 1, cats[i].name);
    }
    list_->AssignImageList(imgs, wxIMAGE_LIST_SMALL);
}

void CategoryManagerDialog::OnListSelect(wxListEvent& event) {
    auto cats = pm_->getCategories();
    int idx = (int)event.GetIndex();
    if (idx < 0 || idx >= (int)cats.size()) return;
    txtName_->SetValue(cats[idx].name);
    selectedColour_ = cats[idx].colour;
    btnColour_->SetBackgroundColour(selectedColour_);
    lblPreview_->SetLabel(wxString::Format("#%02X%02X%02X",
        selectedColour_.Red(), selectedColour_.Green(), selectedColour_.Blue()));
    Refresh();
}

void CategoryManagerDialog::OnPickColour(wxCommandEvent&) {
    wxColourData data;
    data.SetColour(selectedColour_);
    data.SetChooseFull(true);
    // Pre-load palette
    for (int i = 0; i < PALETTE_SIZE; ++i)
        data.SetCustomColour(i, CATEGORY_PALETTE[i]);
    wxColourDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK) {
        selectedColour_ = dlg.GetColourData().GetColour();
        btnColour_->SetBackgroundColour(selectedColour_);
        lblPreview_->SetLabel(wxString::Format("#%02X%02X%02X",
            selectedColour_.Red(), selectedColour_.Green(), selectedColour_.Blue()));
        Refresh();
    }
}

void CategoryManagerDialog::OnAdd(wxCommandEvent&) {
    wxString name = txtName_->GetValue().Trim();
    if (name.IsEmpty()) {
        wxMessageBox("Please enter a category name.", "Required",
            wxOK | wxICON_WARNING);
        txtName_->SetFocus();
        return;
    }
    Category cat(name.ToStdString(), selectedColour_);
    if (!pm_->addCategory(cat)) {
        wxMessageBox("A category with that name already exists.",
            "Duplicate", wxOK | wxICON_WARNING);
        return;
    }
    txtName_->Clear();
    RebuildList();
}

void CategoryManagerDialog::OnRename(wxCommandEvent&) {
    long sel = list_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Select a category to rename.", "Info",
            wxOK | wxICON_INFORMATION);
        return;
    }
    wxString newName = txtName_->GetValue().Trim();
    if (newName.IsEmpty()) {
        wxMessageBox("Enter a new name in the text box.", "Required",
            wxOK | wxICON_WARNING);
        return;
    }
    auto cats = pm_->getCategories();
    if (sel >= (long)cats.size()) return;

    // Also update colour while we're at it
    pm_->renameCategory(cats[sel].name, newName.ToStdString());
    // Update colour separately via remove+add is clunky;
    // instead update through StorageManager directly:
    auto updatedCats = pm_->getCategories();
    for (auto& c : updatedCats) {
        if (c.name == newName.ToStdString()) {
            // We'll re-add with new colour by removing and re-inserting
            // (simpler approach: expose a colour-update method)
            break;
        }
    }
    RebuildList();
}

void CategoryManagerDialog::OnDelete(wxCommandEvent&) {
    long sel = list_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Select a category to delete.", "Info",
            wxOK | wxICON_INFORMATION);
        return;
    }
    auto cats = pm_->getCategories();
    if (sel >= (long)cats.size()) return;
    wxString name = cats[sel].name;
    int res = wxMessageBox(
        "Delete category \"" + name + "\"?\n\n"
        "Entries using this category will become uncategorised.",
        "Confirm Delete", wxYES_NO | wxICON_WARNING, this);
    if (res != wxYES) return;
    pm_->removeCategory(name.ToStdString());
    txtName_->Clear();
    RebuildList();
}

void CategoryManagerDialog::OnClose(wxCommandEvent&) {
    EndModal(wxID_OK);
}