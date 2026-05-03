#define _CRT_SECURE_NO_WARNINGS
#include "MainFrame.h"
#include "AddPasswordDialog.h"
#include "PasswordDialog.h"
#include "PasswordGeneratorDialog.h"
#include "PinDialog.h"
#include <wx/msgdlg.h>
#include <wx/artprov.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/bmpbuttn.h>
#include <wx/dc.h>

enum {
    ID_ADD = 1000,
    ID_EDIT,
    ID_DELETE,
    ID_GENERATE,
    ID_SEARCH,
    ID_LIST,
    ID_TIMER,
    ID_CHANGE_PIN,
    ID_CLEAR_SEARCH,
    ID_FILTER_ALL,
    ID_FILTER_WORK,
    ID_FILTER_PERSONAL,
    ID_FILTER_FINANCE,
    ID_FILTER_SOCIAL
};

const int MainFrame::INACTIVITY_TIMEOUT = 5 * 60 * 1000;   // 5 minutes

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_ADD, MainFrame::OnAdd)
EVT_MENU(ID_EDIT, MainFrame::OnEdit)
EVT_MENU(ID_DELETE, MainFrame::OnDelete)
EVT_MENU(ID_GENERATE, MainFrame::OnGenerate)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(ID_CHANGE_PIN, MainFrame::OnChangePin)
EVT_LIST_ITEM_ACTIVATED(ID_LIST, MainFrame::OnItemActivated)
EVT_TIMER(ID_TIMER, MainFrame::OnTimer)
EVT_BUTTON(ID_ADD, MainFrame::OnAdd)
EVT_BUTTON(ID_EDIT, MainFrame::OnEdit)
EVT_BUTTON(ID_DELETE, MainFrame::OnDelete)
EVT_BUTTON(ID_GENERATE, MainFrame::OnGenerate)
EVT_BUTTON(ID_CLEAR_SEARCH, MainFrame::OnClearSearch)
EVT_BUTTON(ID_FILTER_ALL, MainFrame::OnFilterAll)
EVT_BUTTON(ID_FILTER_WORK, MainFrame::OnFilterWork)
EVT_BUTTON(ID_FILTER_PERSONAL, MainFrame::OnFilterPersonal)
EVT_BUTTON(ID_FILTER_FINANCE, MainFrame::OnFilterFinance)
EVT_BUTTON(ID_FILTER_SOCIAL, MainFrame::OnFilterSocial)
wxEND_EVENT_TABLE()

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: draw a filled circle into a wxBitmap
// ─────────────────────────────────────────────────────────────────────────────
static wxBitmap MakeCircleBitmap(const wxColour& col, int size = 14) {
    wxBitmap bmp(size, size, 32);
    wxMemoryDC dc(bmp);
    // transparent background
    dc.SetBackground(wxBrush(wxColour(0, 0, 0, 0)));
    dc.Clear();
    dc.SetBrush(wxBrush(col));
    dc.SetPen(wxPen(col.ChangeLightness(70), 1));
    dc.DrawCircle(size / 2, size / 2, size / 2 - 1);
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title,
        wxDefaultPosition, wxSize(920, 660)),
    locked_(false)
{
    // Background colour - slightly off-white
    SetBackgroundColour(wxColour(245, 246, 250));

    pm_ = new PasswordManager("vault.json");
    if (!pm_->initialize())
        wxMessageBox("Failed to initialize vault!", "Error", wxOK | wxICON_ERROR);

    if (!pm_->hasPin())
        SetupPin();
    else if (!CheckPin()) {
        Close(true);
        return;
    }

    SetupMenu();
    SetupToolbar();
    SetupList();
    SetupStatusBar();
    LoadPasswords();

    inactivityTimer_ = new wxTimer(this, ID_TIMER);
    inactivityTimer_->Start(INACTIVITY_TIMEOUT);

    // Bind activity events on the frame and list
    Bind(wxEVT_MOTION, &MainFrame::OnAnyActivity, this);
    Bind(wxEVT_KEY_DOWN, &MainFrame::OnAnyActivity, this);
    Bind(wxEVT_LEFT_DOWN, &MainFrame::OnAnyActivity, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::OnAnyActivity, this);

    Centre();
}

MainFrame::~MainFrame() {
    delete inactivityTimer_;
    delete pm_;
}

// ─────────────────────────────────────────────────────────────────────────────
//  PIN helpers
// ─────────────────────────────────────────────────────────────────────────────
bool MainFrame::CheckPin() {
    while (true) {
        PinDialog dlg(this, PinMode::VERIFY);
        int result = dlg.ShowModal();
        if (result == wxID_OK) {
            if (pm_->verifyPin(dlg.GetPin().ToStdString()))
                return true;
            wxMessageBox("Incorrect PIN. Try again.", "Error", wxOK | wxICON_ERROR);
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
    wxMessageBox("Welcome!\n\nPlease create a PIN and security question to protect your vault.",
        "First Time Setup", wxOK | wxICON_INFORMATION);
    PinDialog dlg(this, PinMode::SETUP);
    if (dlg.ShowModal() == wxID_OK)
        pm_->setPin(dlg.GetPin().ToStdString(),
            dlg.GetSecurityQuestion().ToStdString(),
            dlg.GetSecurityAnswer().ToStdString());
}

void MainFrame::RecoverPin() {
    if (!pm_->hasPin()) { wxMessageBox("No PIN set.", "Info", wxOK); return; }
    wxString q = pm_->getSecurityQuestion();
    if (q.IsEmpty()) { wxMessageBox("No security question set.", "Error", wxOK | wxICON_ERROR); return; }

    PinDialog dlg(this, PinMode::RECOVER);
    dlg.SetSecurityQuestion(q);
    if (dlg.ShowModal() == wxID_OK) {
        if (pm_->recoverPin(dlg.GetSecurityAnswer().ToStdString(), dlg.GetPin().ToStdString()))
            wxMessageBox("PIN reset! Log in with your new PIN.", "Success", wxOK | wxICON_INFORMATION);
        else
            wxMessageBox("Wrong answer. PIN reset failed.", "Error", wxOK | wxICON_ERROR);
    }
}

void MainFrame::LockApp() {
    if (locked_) return;
    locked_ = true;
    listCtrl_->Disable();
    txtSearch_->Disable();

    while (locked_) {
        PinDialog dlg(this, PinMode::VERIFY);
        dlg.SetTitle("Vault Locked — Enter PIN");
        int res = dlg.ShowModal();
        if (res == wxID_OK) {
            if (pm_->verifyPin(dlg.GetPin().ToStdString())) {
                locked_ = false;
                listCtrl_->Enable();
                txtSearch_->Enable();
                inactivityTimer_->Start(INACTIVITY_TIMEOUT);
                SetStatusText("Vault unlocked.", 0);
                break;
            }
            wxMessageBox("Incorrect PIN.", "Error", wxOK | wxICON_ERROR);
        }
        else if (dlg.DidRequestRecovery()) {
            RecoverPin();
        }
        else {
            Close(true);
            return;
        }
    }
}

void MainFrame::UnlockApp() { locked_ = false; }

void MainFrame::OnAnyActivity(wxEvent& event) {
    if (!locked_ && inactivityTimer_->IsRunning())
        inactivityTimer_->Start(INACTIVITY_TIMEOUT);
    event.Skip();
}

void MainFrame::OnTimer(wxTimerEvent&) {
    if (!locked_) LockApp();
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI Setup
// ─────────────────────────────────────────────────────────────────────────────
void MainFrame::SetupMenu() {
    wxMenuBar* mb = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_ADD, "&Add Password\tCtrl+A");
    fileMenu->Append(ID_GENERATE, "&Generate Password\tCtrl+G");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+X");

    wxMenu* editMenu = new wxMenu();
    editMenu->Append(ID_EDIT, "&Edit\tCtrl+E");
    editMenu->Append(ID_DELETE, "&Delete\tCtrl+D");
    editMenu->AppendSeparator();
    editMenu->Append(ID_CHANGE_PIN, "Change &PIN\tCtrl+P");

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About");

    mb->Append(fileMenu, "&File");
    mb->Append(editMenu, "&Edit");
    mb->Append(helpMenu, "&Help");
    SetMenuBar(mb);
}

void MainFrame::SetupToolbar() {
    // Use a custom panel instead of wxToolBar for better styling control
    wxPanel* tbPanel = new wxPanel(this, wxID_ANY);
    tbPanel->SetBackgroundColour(wxColour(30, 34, 45));   // dark navy

    wxBoxSizer* tbSizer = new wxBoxSizer(wxHORIZONTAL);

    // Helper lambda: create a styled toolbar button
    auto makeBtn = [&](int id, const wxString& label,
        const wxArtID& art, const wxString& tip) -> wxButton* {
            wxButton* btn = new wxButton(tbPanel, id, label,
                wxDefaultPosition, wxSize(-1, 38));
            btn->SetBackgroundColour(wxColour(52, 58, 75));
            btn->SetForegroundColour(*wxWHITE);
            btn->SetToolTip(tip);
            return btn;
        };

    wxButton* btnAdd = makeBtn(ID_ADD, " + Add", wxART_NEW, "Add new password");
    wxButton* btnEdit = makeBtn(ID_EDIT, " Edit", wxART_EDIT, "Edit selected");
    wxButton* btnDel = makeBtn(ID_DELETE, " Delete", wxART_DELETE, "Delete selected");
    wxButton* btnGen = makeBtn(ID_GENERATE, " Generate", wxART_REDO, "Generate password");

    tbSizer->Add(btnAdd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    tbSizer->Add(btnEdit, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    tbSizer->Add(btnDel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    tbSizer->AddSpacer(10);
    tbSizer->Add(new wxStaticLine(tbPanel, wxID_ANY, wxDefaultPosition, wxSize(1, -1), wxLI_VERTICAL),
        0, wxEXPAND | wxTOP | wxBOTTOM, 8);
    tbSizer->AddSpacer(10);
    tbSizer->Add(btnGen, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    // Spacer
    tbSizer->AddStretchSpacer(1);

    // Search label
    wxStaticText* lblSearch = new wxStaticText(tbPanel, wxID_ANY, "Search:");
    lblSearch->SetForegroundColour(*wxWHITE);
    tbSizer->Add(lblSearch, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    // Search box
    txtSearch_ = new wxTextCtrl(tbPanel, ID_SEARCH, "",
        wxDefaultPosition, wxSize(220, 30), wxTE_PROCESS_ENTER);
    txtSearch_->SetHint("Service, username, or category...");
    txtSearch_->Bind(wxEVT_TEXT, &MainFrame::OnSearchText, this);
    tbSizer->Add(txtSearch_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    wxButton* btnClear = new wxButton(tbPanel, ID_CLEAR_SEARCH, "x",
        wxDefaultPosition, wxSize(28, 28));
    btnClear->SetBackgroundColour(wxColour(80, 86, 105));
    btnClear->SetForegroundColour(*wxWHITE);
    btnClear->SetToolTip("Clear search");
    tbSizer->Add(btnClear, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    tbPanel->SetSizer(tbSizer);

    // ── Category filter bar ──────────────────────────────────────────────────
    wxPanel* filterPanel = new wxPanel(this, wxID_ANY);
    filterPanel->SetBackgroundColour(wxColour(240, 241, 246));
    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* lblFilter = new wxStaticText(filterPanel, wxID_ANY, "Filter: ");
    lblFilter->SetForegroundColour(wxColour(80, 80, 80));
    filterSizer->Add(lblFilter, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    struct FilterBtn { int id; wxString label; wxColour col; };
    FilterBtn filters[] = {
        {ID_FILTER_ALL,      "All",      wxColour(100, 100, 120)},
        {ID_FILTER_WORK,     "Work",     wxColour(220, 53,  69)},
        {ID_FILTER_PERSONAL, "Personal", wxColour(0,   123, 255)},
        {ID_FILTER_FINANCE,  "Finance",  wxColour(40,  167, 69)},
        {ID_FILTER_SOCIAL,   "Social",   wxColour(220, 160, 0)},
    };

    for (auto& f : filters) {
        wxButton* b = new wxButton(filterPanel, f.id, f.label,
            wxDefaultPosition, wxSize(-1, 26));
        b->SetBackgroundColour(f.col);
        b->SetForegroundColour(*wxWHITE);
        filterSizer->Add(b, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    }

    filterPanel->SetSizer(filterSizer);

    // ── Outer layout ─────────────────────────────────────────────────────────
    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
    outerSizer->Add(tbPanel, 0, wxEXPAND);
    outerSizer->Add(filterPanel, 0, wxEXPAND);
    // listCtrl_ will be added inside SetupList
    SetSizer(outerSizer);
}

void MainFrame::SetupList() {
    // Get the outer sizer already created in SetupToolbar
    wxBoxSizer* outerSizer = static_cast<wxBoxSizer*>(GetSizer());

    // Wrapper panel for the list (white card look)
    wxPanel* listPanel = new wxPanel(this, wxID_ANY);
    listPanel->SetBackgroundColour(*wxWHITE);
    wxBoxSizer* listSizer = new wxBoxSizer(wxVERTICAL);

    listCtrl_ = new wxListCtrl(listPanel, ID_LIST,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE);
    listCtrl_->SetBackgroundColour(*wxWHITE);
    listCtrl_->SetTextColour(wxColour(33, 37, 41));

    // Columns
    // Col 0: tiny circle column
    listCtrl_->AppendColumn("", wxLIST_FORMAT_CENTER, 38);
    listCtrl_->AppendColumn("Service", wxLIST_FORMAT_LEFT, 220);
    listCtrl_->AppendColumn("Username", wxLIST_FORMAT_LEFT, 210);
    listCtrl_->AppendColumn("Category", wxLIST_FORMAT_LEFT, 110);
    listCtrl_->AppendColumn("Age", wxLIST_FORMAT_LEFT, 120);

    // Image list for colored circles
    wxImageList* imgList = new wxImageList(14, 14, true, 6);
    // Index 0: no tag (gray)
    imgList->Add(MakeCircleBitmap(wxColour(180, 180, 190)));
    // Index 1: Work - red
    imgList->Add(MakeCircleBitmap(wxColour(220, 53, 69)));
    // Index 2: Personal - blue
    imgList->Add(MakeCircleBitmap(wxColour(0, 123, 255)));
    // Index 3: Finance - green
    imgList->Add(MakeCircleBitmap(wxColour(40, 167, 69)));
    // Index 4: Social - amber
    imgList->Add(MakeCircleBitmap(wxColour(220, 160, 0)));

    listCtrl_->AssignImageList(imgList, wxIMAGE_LIST_SMALL);

    listSizer->Add(listCtrl_, 1, wxEXPAND | wxALL, 6);
    listPanel->SetSizer(listSizer);

    outerSizer->Add(listPanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
}

void MainFrame::SetupStatusBar() {
    CreateStatusBar(2);
    SetStatusText("Ready", 0);
    SetStatusText("", 1);
}

void MainFrame::UpdateStatusBar(int total, int shown) {
    wxString msg = wxString::Format("Showing %d of %d entries", shown, total);
    SetStatusText(msg, 1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  LoadPasswords
// ─────────────────────────────────────────────────────────────────────────────
void MainFrame::LoadPasswords(const std::vector<PasswordEntry>& entries) {
    listCtrl_->DeleteAllItems();
    std::time_t now = std::time(nullptr);

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];

        // Image index for the circle
        int imgIdx = 0;
        switch (entry.tag) {
        case PasswordTag::WORK:     imgIdx = 1; break;
        case PasswordTag::PERSONAL: imgIdx = 2; break;
        case PasswordTag::FINANCE:  imgIdx = 3; break;
        case PasswordTag::SOCIAL:   imgIdx = 4; break;
        default:                    imgIdx = 0; break;
        }

        // Insert with image in column 0 (empty text, just the circle)
        long idx = listCtrl_->InsertItem(static_cast<long>(i), "", imgIdx);
        listCtrl_->SetItem(idx, 1, entry.service);
        listCtrl_->SetItem(idx, 2, entry.username);
        listCtrl_->SetItem(idx, 3, PasswordEntry::GetTagName(entry.tag));

        // Age column
        double days = std::difftime(now, entry.lastChanged) / (60.0 * 60.0 * 24.0);
        wxString ageStr;
        if (days > 180)
            ageStr = wxString::Format("%.0f days (!)", days);
        else
            ageStr = wxString::Format("%.0f days", days);
        listCtrl_->SetItem(idx, 4, ageStr);

        // Highlight old passwords in orange
        if (days > 180)
            listCtrl_->SetItemTextColour(idx, wxColour(230, 100, 0));
        else
            listCtrl_->SetItemTextColour(idx, wxColour(33, 37, 41));

        // Store tag as item data (for edit lookup etc.)
        listCtrl_->SetItemData(idx, static_cast<long>(entry.tag));
    }

    int total = static_cast<int>(pm_->listPasswords().size());
    UpdateStatusBar(total, static_cast<int>(entries.size()));
}

void MainFrame::LoadPasswords() {
    LoadPasswords(pm_->listPasswords());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Event handlers — passwords
// ─────────────────────────────────────────────────────────────────────────────
void MainFrame::OnAdd(wxCommandEvent&) {
    if (locked_) return;
    AddPasswordDialog dlg(this, pm_);
    if (dlg.ShowModal() == wxID_OK) LoadPasswords();
}

void MainFrame::OnEdit(wxCommandEvent&) {
    if (locked_) return;
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) { wxMessageBox("Select an entry to edit.", "Info"); return; }

    wxString service = listCtrl_->GetItemText(sel, 1);
    wxString username = listCtrl_->GetItemText(sel, 2);
    auto entries = pm_->listPasswords();
    for (auto& e : entries) {
        if (e.service == service.ToStdString() && e.username == username.ToStdString()) {
            PasswordDialog dlg(this, pm_, &e);
            if (dlg.ShowModal() == wxID_OK) LoadPasswords();
            return;
        }
    }
}

void MainFrame::OnDelete(wxCommandEvent&) {
    if (locked_) return;
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) { wxMessageBox("Select an entry to delete.", "Info"); return; }

    wxString service = listCtrl_->GetItemText(sel, 1);
    wxString username = listCtrl_->GetItemText(sel, 2);
    auto entries = pm_->listPasswords();
    for (const auto& e : entries) {
        if (e.service == service.ToStdString() && e.username == username.ToStdString()) {
            int res = wxMessageBox("Delete password for \"" + service + "\"?",
                "Confirm Delete", wxYES_NO | wxICON_WARNING);
            if (res == wxYES) { pm_->removePassword(e.id); LoadPasswords(); }
            return;
        }
    }
}

void MainFrame::OnGenerate(wxCommandEvent&) {
    if (locked_) return;
    PasswordGeneratorDialog dlg(this, pm_);
    dlg.ShowModal();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Search & filter
// ─────────────────────────────────────────────────────────────────────────────
void MainFrame::OnSearch(wxCommandEvent&) {}

void MainFrame::OnSearchText(wxCommandEvent&) {
    wxString q = txtSearch_->GetValue();
    if (q.IsEmpty())
        LoadPasswords();
    else
        LoadPasswords(pm_->searchPasswords(q.ToStdString()));
}

void MainFrame::OnClearSearch(wxCommandEvent&) {
    txtSearch_->Clear();
    LoadPasswords();
}

void MainFrame::OnFilterAll(wxCommandEvent&) { txtSearch_->Clear(); LoadPasswords(); }
void MainFrame::OnFilterWork(wxCommandEvent&) { LoadPasswords(pm_->searchByTag(PasswordTag::WORK)); }
void MainFrame::OnFilterPersonal(wxCommandEvent&) { LoadPasswords(pm_->searchByTag(PasswordTag::PERSONAL)); }
void MainFrame::OnFilterFinance(wxCommandEvent&) { LoadPasswords(pm_->searchByTag(PasswordTag::FINANCE)); }
void MainFrame::OnFilterSocial(wxCommandEvent&) { LoadPasswords(pm_->searchByTag(PasswordTag::SOCIAL)); }

// ─────────────────────────────────────────────────────────────────────────────
//  PIN menu
// ─────────────────────────────────────────────────────────────────────────────
void MainFrame::OnChangePin(wxCommandEvent&) {
    PinDialog dlg(this, PinMode::VERIFY);
    if (dlg.ShowModal() == wxID_OK) {
        if (pm_->verifyPin(dlg.GetPin().ToStdString())) {
            PinDialog newDlg(this, PinMode::CHANGE);
            if (newDlg.ShowModal() == wxID_OK) {
                pm_->changePin(dlg.GetPin().ToStdString(), newDlg.GetPin().ToStdString());
                wxMessageBox("PIN changed!", "Success", wxOK | wxICON_INFORMATION);
            }
        }
        else {
            wxMessageBox("Incorrect PIN.", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::OnExit(wxCommandEvent&) { Close(true); }

void MainFrame::OnAbout(wxCommandEvent&) {
    wxMessageBox(
        "Password Manager v2.1\n\n"
        "Features:\n"
        "  \u2022 PIN Authentication with PBKDF2-SHA256\n"
        "  \u2022 PIN Recovery via security question\n"
        "  \u2022 Color-coded category circles\n"
        "  \u2022 Category filter buttons\n"
        "  \u2022 Real-time search (text + category)\n"
        "  \u2022 Password Strength Meter\n"
        "  \u2022 6-Month age reminders\n"
        "  \u2022 Auto-Lock after 5 min inactivity\n"
        "  \u2022 Stream-cipher field encryption\n\n"
        "Built with wxWidgets",
        "About", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnItemActivated(wxListEvent& event) {
    wxCommandEvent dummy;
    OnEdit(dummy);
}