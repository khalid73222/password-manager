#define _CRT_SECURE_NO_WARNINGS
#include "MainFrame.h"
#include "AddPasswordDialog.h"
#include "PasswordDialog.h"
#include "PasswordGeneratorDialog.h"
#include "CategoryManagerDialog.h"
#include "PinDialog.h"
#include <wx/msgdlg.h>
#include <wx/artprov.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/dc.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

enum {
    ID_ADD = 1000, ID_EDIT, ID_DELETE, ID_GENERATE,
    ID_SEARCH, ID_LIST, ID_TIMER,
    ID_CHANGE_PIN, ID_CLEAR_SEARCH,
    ID_MANAGE_CATEGORIES,
    // Dynamic filter buttons start here
    ID_FILTER_BASE = 2000   // ID_FILTER_BASE+0 = All, +1..N = categories
};

const int MainFrame::INACTIVITY_TIMEOUT = 5 * 60 * 1000;

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_ADD, MainFrame::OnAdd)
EVT_MENU(ID_EDIT, MainFrame::OnEdit)
EVT_MENU(ID_DELETE, MainFrame::OnDelete)
EVT_MENU(ID_GENERATE, MainFrame::OnGenerate)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(ID_CHANGE_PIN, MainFrame::OnChangePin)
EVT_MENU(ID_MANAGE_CATEGORIES, MainFrame::OnManageCategories)
EVT_LIST_ITEM_ACTIVATED(ID_LIST, MainFrame::OnItemActivated)
EVT_TIMER(ID_TIMER, MainFrame::OnTimer)
EVT_BUTTON(ID_ADD, MainFrame::OnAdd)
EVT_BUTTON(ID_EDIT, MainFrame::OnEdit)
EVT_BUTTON(ID_DELETE, MainFrame::OnDelete)
EVT_BUTTON(ID_GENERATE, MainFrame::OnGenerate)
EVT_BUTTON(ID_CLEAR_SEARCH, MainFrame::OnClearSearch)
EVT_BUTTON(ID_MANAGE_CATEGORIES, MainFrame::OnManageCategories)
wxEND_EVENT_TABLE()

// ── Circle bitmap helper ──────────────────────────────────────────────────────
// Replace MakeCircleBitmap at the top of MainFrame.cpp
static wxBitmap MakeCircleBitmap(const wxColour& col, int size = 14) {
    wxBitmap bmp(size, size);
    {
        wxMemoryDC dc(bmp);
        // Fill entire bitmap with the list background colour first
        dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
        dc.Clear();
        // Draw circle with NO border pen — same colour as fill
        dc.SetBrush(wxBrush(col));
        dc.SetPen(wxPen(col, 1));   // pen = same colour as brush → invisible border
        dc.DrawCircle(size / 2, size / 2, size / 2 - 1);
    }
    return bmp;
}

// ── Constructor ───────────────────────────────────────────────────────────────
MainFrame::MainFrame(const wxString& title, const wxString& vaultPath)
    : wxFrame(nullptr, wxID_ANY, title,
        wxDefaultPosition, wxSize(960, 700)),
    locked_(false), filterPanel_(nullptr), outerSizer_(nullptr)
{
    SetBackgroundColour(wxColour(245, 246, 250));

    pm_ = new PasswordManager(vaultPath.ToStdString());
    if (!pm_->initialize())
        wxMessageBox("Failed to initialize vault!\nPath: " + vaultPath,
            "Error", wxOK | wxICON_ERROR);

    if (!pm_->hasPin())
        SetupPin();
    else if (!CheckPin()) { Close(true); return; }

    SetupMenu();
    SetupToolbar();     // creates outerSizer_
    SetupList();
    SetupStatusBar();
    RebuildFilterBar(); // builds filter panel from categories
    LoadPasswords();

    inactivityTimer_ = new wxTimer(this, ID_TIMER);
    inactivityTimer_->Start(INACTIVITY_TIMEOUT);

    Bind(wxEVT_MOTION, &MainFrame::OnAnyActivity, this);
    Bind(wxEVT_KEY_DOWN, &MainFrame::OnAnyActivity, this);
    Bind(wxEVT_LEFT_DOWN, &MainFrame::OnAnyActivity, this);

    Centre();
}

MainFrame::~MainFrame() { delete inactivityTimer_; delete pm_; }

// ── PIN ───────────────────────────────────────────────────────────────────────
bool MainFrame::CheckPin() {
    while (true) {
        PinDialog dlg(this, PinMode::VERIFY);
        if (dlg.ShowModal() == wxID_OK) {
            if (pm_->verifyPin(dlg.GetPin().ToStdString())) return true;
            wxMessageBox("Incorrect PIN. Try again.", "Access Denied",
                wxOK | wxICON_ERROR);
        }
        else if (dlg.DidRequestRecovery()) {
            RecoverPin();
        }
        else {
            return false;
        }
    }
}

void MainFrame::SetupPin() {
    wxMessageBox("Welcome!\n\nPlease create a PIN and security question.",
        "First Time Setup", wxOK | wxICON_INFORMATION);
    PinDialog dlg(this, PinMode::SETUP);
    if (dlg.ShowModal() == wxID_OK)
        pm_->setPin(dlg.GetPin().ToStdString(),
            dlg.GetSecurityQuestion().ToStdString(),
            dlg.GetSecurityAnswer().ToStdString());
}

void MainFrame::RecoverPin() {
    wxString q = pm_->getSecurityQuestion();
    if (q.IsEmpty()) {
        wxMessageBox("No security question set.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    PinDialog dlg(this, PinMode::RECOVER);
    dlg.SetSecurityQuestion(q);
    if (dlg.ShowModal() == wxID_OK) {
        bool ok = pm_->recoverPin(dlg.GetSecurityAnswer().ToStdString(),
            dlg.GetPin().ToStdString());
        wxMessageBox(ok ? "PIN reset! Log in with your new PIN."
            : "Wrong answer. PIN reset failed.",
            ok ? "Success" : "Error",
            wxOK | (ok ? wxICON_INFORMATION : wxICON_ERROR));
    }
}

void MainFrame::LockApp() {
    if (locked_) return;
    locked_ = true;
    listCtrl_->Disable(); txtSearch_->Disable();
    while (locked_) {
        PinDialog dlg(this, PinMode::VERIFY);
        dlg.SetTitle("Vault Locked");
        int res = dlg.ShowModal();
        if (res == wxID_OK && pm_->verifyPin(dlg.GetPin().ToStdString())) {
            locked_ = false;
            listCtrl_->Enable(); txtSearch_->Enable();
            inactivityTimer_->Start(INACTIVITY_TIMEOUT);
            SetStatusText("Vault unlocked.", 0);
        }
        else if (dlg.DidRequestRecovery()) {
            RecoverPin();
        }
        else if (res != wxID_OK) {
            Close(true); return;
        }
        else {
            wxMessageBox("Incorrect PIN.", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::UnlockApp() { locked_ = false; }
void MainFrame::OnTimer(wxTimerEvent&) { if (!locked_) LockApp(); }
void MainFrame::OnAnyActivity(wxEvent& e) {
    if (!locked_ && inactivityTimer_->IsRunning())
        inactivityTimer_->Start(INACTIVITY_TIMEOUT);
    e.Skip();
}

// ── Menu ──────────────────────────────────────────────────────────────────────
void MainFrame::SetupMenu() {
    wxMenuBar* mb = new wxMenuBar();
    wxMenu* file = new wxMenu();
    file->Append(ID_ADD, "&Add Password\tCtrl+A");
    file->Append(ID_GENERATE, "&Generate Password\tCtrl+G");
    file->AppendSeparator();
    file->Append(wxID_EXIT, "E&xit\tAlt+X");

    wxMenu* edit = new wxMenu();
    edit->Append(ID_EDIT, "&Edit Selected\tCtrl+E");
    edit->Append(ID_DELETE, "&Delete Selected\tCtrl+D");
    edit->AppendSeparator();
    edit->Append(ID_MANAGE_CATEGORIES, "&Manage Categories\tCtrl+M");
    edit->AppendSeparator();
    edit->Append(ID_CHANGE_PIN, "Change &PIN\tCtrl+P");

    wxMenu* help = new wxMenu();
    help->Append(wxID_ABOUT, "&About");

    mb->Append(file, "&File");
    mb->Append(edit, "&Edit");
    mb->Append(help, "&Help");
    SetMenuBar(mb);
}

// ── Toolbar ───────────────────────────────────────────────────────────────────
void MainFrame::SetupToolbar() {
    wxPanel* tb = new wxPanel(this, wxID_ANY);
    tb->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* ts = new wxBoxSizer(wxHORIZONTAL);

    auto mkBtn = [&](int id, const wxString& lbl, const wxString& tip) {
        auto* b = new wxButton(tb, id, lbl, wxDefaultPosition, wxSize(-1, 38));
        b->SetBackgroundColour(wxColour(52, 58, 75));
        b->SetForegroundColour(*wxWHITE);
        b->SetToolTip(tip);
        return b;
        };
    ts->Add(mkBtn(ID_ADD, "+ Add", "Add password"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    ts->Add(mkBtn(ID_EDIT, "Edit", "Edit selected"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    ts->Add(mkBtn(ID_DELETE, "Delete", "Delete selected"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    ts->AddSpacer(6);
    ts->Add(new wxStaticLine(tb, wxID_ANY, wxDefaultPosition, wxSize(1, -1), wxLI_VERTICAL),
        0, wxEXPAND | wxTOP | wxBOTTOM, 8);
    ts->AddSpacer(6);
    ts->Add(mkBtn(ID_GENERATE, "Generate", "Password generator"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    ts->Add(mkBtn(ID_MANAGE_CATEGORIES, "Categories", "Manage categories"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    ts->AddStretchSpacer(1);

    auto* sl = new wxStaticText(tb, wxID_ANY, "Search:");
    sl->SetForegroundColour(*wxWHITE);
    ts->Add(sl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    txtSearch_ = new wxTextCtrl(tb, ID_SEARCH, "",
        wxDefaultPosition, wxSize(230, 30), wxTE_PROCESS_ENTER);
    txtSearch_->SetHint("Service, username, category...");
    txtSearch_->Bind(wxEVT_TEXT, &MainFrame::OnSearchText, this);
    ts->Add(txtSearch_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    auto* clr = new wxButton(tb, ID_CLEAR_SEARCH, "x", wxDefaultPosition, wxSize(28, 28));
    clr->SetBackgroundColour(wxColour(80, 86, 105));
    clr->SetForegroundColour(*wxWHITE);
    ts->Add(clr, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    tb->SetSizer(ts);

    outerSizer_ = new wxBoxSizer(wxVERTICAL);
    outerSizer_->Add(tb, 0, wxEXPAND);
    // filterPanel_ placeholder — filled in RebuildFilterBar()
    SetSizer(outerSizer_);
}

// ── Dynamic filter bar ────────────────────────────────────────────────────────
void MainFrame::RebuildFilterBar() {
    if (filterPanel_) {
        outerSizer_->Detach(filterPanel_);
        filterPanel_->Destroy();
        filterPanel_ = nullptr;
    }

    filterPanel_ = new wxPanel(this, wxID_ANY);
    filterPanel_->SetBackgroundColour(wxColour(235, 236, 244));
    wxBoxSizer* fs = new wxBoxSizer(wxHORIZONTAL);

    auto* lbl = new wxStaticText(filterPanel_, wxID_ANY, "  Filter:");
    lbl->SetForegroundColour(wxColour(80, 80, 100));
    fs->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // "All" button
    auto* btnAll = new wxButton(filterPanel_, ID_FILTER_BASE, "All",
        wxDefaultPosition, wxSize(-1, 26));
    btnAll->SetBackgroundColour(wxColour(80, 86, 110));
    btnAll->SetForegroundColour(*wxWHITE);
    btnAll->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        txtSearch_->Clear(); LoadPasswords();
        });
    fs->Add(btnAll, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // One button per category
    auto cats = pm_->getCategories();
    for (int i = 0; i < (int)cats.size(); ++i) {
        std::string catName = cats[i].name;
        wxColour    catCol = cats[i].colour;

        auto* btn = new wxButton(filterPanel_, ID_FILTER_BASE + 1 + i,
            catName, wxDefaultPosition, wxSize(-1, 26));
        btn->SetBackgroundColour(catCol);
        btn->SetForegroundColour(*wxWHITE);
        btn->Bind(wxEVT_BUTTON, [this, catName](wxCommandEvent&) {
            LoadPasswords(pm_->searchByCategory(catName));
            });
        fs->Add(btn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    }

    filterPanel_->SetSizer(fs);

    // Insert after toolbar (index 1), before the list panel (index 2)
    outerSizer_->Insert(1, filterPanel_, 0, wxEXPAND);
    outerSizer_->Layout();
}

// ── List ──────────────────────────────────────────────────────────────────────
// Replace SetupList() in MainFrame.cpp
void MainFrame::SetupList() {
    wxPanel* lp = new wxPanel(this, wxID_ANY);
    lp->SetBackgroundColour(*wxWHITE);
    wxBoxSizer* ls = new wxBoxSizer(wxVERTICAL);

    listCtrl_ = new wxListCtrl(lp, ID_LIST,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE);
    listCtrl_->SetBackgroundColour(*wxWHITE);

    listCtrl_->AppendColumn("", wxLIST_FORMAT_CENTER, 38);
    listCtrl_->AppendColumn("Service", wxLIST_FORMAT_LEFT, 180);
    listCtrl_->AppendColumn("Username", wxLIST_FORMAT_LEFT, 180);
    listCtrl_->AppendColumn("Category", wxLIST_FORMAT_LEFT, 100);
    listCtrl_->AppendColumn("Age", wxLIST_FORMAT_LEFT, 80);
    listCtrl_->AppendColumn("Notes", wxLIST_FORMAT_LEFT, 300);  // NEW

    ls->Add(listCtrl_, 1, wxEXPAND | wxALL, 6);
    lp->SetSizer(ls);
    outerSizer_->Add(lp, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
}

void MainFrame::SetupStatusBar() {
    CreateStatusBar(2);
    SetStatusText("Ready", 0);
}

void MainFrame::UpdateStatusBar(int total, int shown) {
    SetStatusText(wxString::Format("Showing %d of %d entries", shown, total), 1);
}

// ── Load passwords ────────────────────────────────────────────────────────────
// Replace LoadPasswords(const std::vector<PasswordEntry>&) in MainFrame.cpp
void MainFrame::LoadPasswords(const std::vector<PasswordEntry>& entries) {
    listCtrl_->DeleteAllItems();

    // Rebuild image list so circles match current categories
    auto cats = pm_->getCategories();
    wxImageList* imgs = new wxImageList(14, 14, false, (int)cats.size() + 1);
    // Index 0: grey circle for "no category"
    imgs->Add(MakeCircleBitmap(wxColour(180, 180, 190)));
    for (const auto& c : cats)
        imgs->Add(MakeCircleBitmap(c.colour));
    listCtrl_->AssignImageList(imgs, wxIMAGE_LIST_SMALL);

    std::time_t now = std::time(nullptr);
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];

        // Find image index for this entry's category
        int imgIdx = 0;
        for (int ci = 0; ci < (int)cats.size(); ++ci)
            if (cats[ci].name == e.category) { imgIdx = ci + 1; break; }

        long idx = listCtrl_->InsertItem((long)i, "", imgIdx);
        listCtrl_->SetItem(idx, 1, e.service);
        listCtrl_->SetItem(idx, 2, e.username);
        listCtrl_->SetItem(idx, 3, e.category.empty() ? wxString("—")
            : wxString(e.category));

        double days = std::difftime(now, e.lastChanged) / 86400.0;
        listCtrl_->SetItem(idx, 4,
            days > 180 ? wxString::Format("%.0f days (!)", days)
            : wxString::Format("%.0f days", days));

        // Notes: show first line only, trimmed to keep the list clean
        wxString notes = wxString::FromUTF8(e.notes.c_str());
        notes = notes.BeforeFirst('\n').Trim();   // first line only
        if (notes.length() > 80)
            notes = notes.Left(77) + "...";       // cap length
        listCtrl_->SetItem(idx, 5, notes);

        listCtrl_->SetItemTextColour(idx,
            days > 180 ? wxColour(210, 80, 0) : wxColour(33, 37, 41));
    }

    UpdateStatusBar((int)pm_->listPasswords().size(), (int)entries.size());
}

void MainFrame::LoadPasswords() { LoadPasswords(pm_->listPasswords()); }

// ── CRUD ──────────────────────────────────────────────────────────────────────
void MainFrame::OnAdd(wxCommandEvent&) {
    if (locked_) return;
    AddPasswordDialog dlg(this, pm_);
    if (dlg.ShowModal() == wxID_OK) LoadPasswords();
}

void MainFrame::OnEdit(wxCommandEvent&) {
    if (locked_) return;
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) { wxMessageBox("Select an entry first.", "Info"); return; }
    wxString svc = listCtrl_->GetItemText(sel, 1);
    wxString user = listCtrl_->GetItemText(sel, 2);
    for (auto& e : pm_->listPasswords())
        if (e.service == svc.ToStdString() && e.username == user.ToStdString()) {
            PasswordDialog dlg(this, pm_, &e);
            if (dlg.ShowModal() == wxID_OK) LoadPasswords();
            return;
        }
}

void MainFrame::OnDelete(wxCommandEvent&) {
    if (locked_) return;
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) { wxMessageBox("Select an entry first.", "Info"); return; }
    wxString svc = listCtrl_->GetItemText(sel, 1);
    wxString user = listCtrl_->GetItemText(sel, 2);
    for (const auto& e : pm_->listPasswords())
        if (e.service == svc.ToStdString() && e.username == user.ToStdString()) {
            if (wxMessageBox("Delete \"" + svc + "\"?",
                "Confirm", wxYES_NO | wxICON_WARNING) == wxYES)
            {
                pm_->removePassword(e.id); LoadPasswords();
            }
            return;
        }
}

void MainFrame::OnGenerate(wxCommandEvent&) {
    if (locked_) return;
    PasswordGeneratorDialog dlg(this, pm_); dlg.ShowModal();
}

void MainFrame::OnManageCategories(wxCommandEvent&) {
    if (locked_) return;
    CategoryManagerDialog dlg(this, pm_);
    dlg.ShowModal();
    RebuildFilterBar();   // refresh filter buttons to match new categories
    LoadPasswords();      // refresh list (colours / names may have changed)
}

void MainFrame::OnSearchText(wxCommandEvent&) {
    wxString q = txtSearch_->GetValue();
    q.IsEmpty() ? LoadPasswords() : LoadPasswords(pm_->searchPasswords(q.ToStdString()));
}

void MainFrame::OnClearSearch(wxCommandEvent&) { txtSearch_->Clear(); LoadPasswords(); }
void MainFrame::OnSearch(wxCommandEvent&) {}

void MainFrame::OnChangePin(wxCommandEvent&) {
    PinDialog vDlg(this, PinMode::VERIFY);
    if (vDlg.ShowModal() != wxID_OK) return;
    if (!pm_->verifyPin(vDlg.GetPin().ToStdString())) {
        wxMessageBox("Incorrect PIN.", "Error", wxOK | wxICON_ERROR); return;
    }
    PinDialog cDlg(this, PinMode::CHANGE);
    if (cDlg.ShowModal() == wxID_OK) {
        pm_->changePin(vDlg.GetPin().ToStdString(), cDlg.GetPin().ToStdString());
        wxMessageBox("PIN changed!", "Success", wxOK | wxICON_INFORMATION);
    }
}

void MainFrame::OnExit(wxCommandEvent&) { Close(true); }
void MainFrame::OnAbout(wxCommandEvent&) {
    wxMessageBox(
        "Password Manager v3.0\n\n"
        "  + PIN auth with PBKDF2-SHA256\n"
        "  + Custom user-defined categories\n"
        "  + Dynamic filter bar\n"
        "  + Real-time search\n"
        "  + Password strength meter\n"
        "  + Age warnings & auto-lock\n\n"
        "Built with wxWidgets & nlohmann/json",
        "About", wxOK | wxICON_INFORMATION);
}
void MainFrame::OnItemActivated(wxListEvent&) {
    wxCommandEvent d; OnEdit(d);
}