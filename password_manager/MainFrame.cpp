#include "MainFrame.h"
#include "AddPasswordDialog.h"
#include "PasswordDialog.h"
#include "PasswordGeneratorDialog.h"
#include <wx/msgdlg.h>
#include <wx/artprov.h>

enum {
    ID_ADD = 1000,
    ID_EDIT,
    ID_DELETE,
    ID_GENERATE,
    ID_SEARCH,
    ID_LIST
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_ADD, MainFrame::OnAdd)
EVT_MENU(ID_EDIT, MainFrame::OnEdit)
EVT_MENU(ID_DELETE, MainFrame::OnDelete)
EVT_MENU(ID_GENERATE, MainFrame::OnGenerate)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_LIST_ITEM_ACTIVATED(ID_LIST, MainFrame::OnItemActivated)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {

    // Initialize password manager
    pm_ = new PasswordManager("vault.json");
    if (!pm_->initialize()) {
        wxMessageBox("Failed to initialize vault!", "Error", wxOK | wxICON_ERROR);
    }

    SetupMenu();
    SetupToolbar();
    SetupList();
    LoadPasswords();

    Centre();
}

MainFrame::~MainFrame() {
    delete pm_;
}

void MainFrame::SetupMenu() {
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_ADD, "&Add Password\tCtrl+A");
    fileMenu->Append(ID_GENERATE, "&Generate Password\tCtrl+G");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+X");

    wxMenu* editMenu = new wxMenu();
    editMenu->Append(ID_EDIT, "&Edit\tCtrl+E");
    editMenu->Append(ID_DELETE, "&Delete\tCtrl+D");

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About");

    menuBar->Append(fileMenu, "&File");
    menuBar->Append(editMenu, "&Edit");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
}

void MainFrame::SetupToolbar() {
    wxToolBar* toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL);

    // Use stock art IDs (built into wxWidgets)
    toolbar->AddTool(ID_ADD, "Add", wxArtProvider::GetBitmap(wxART_NEW), "Add new password");
    toolbar->AddTool(ID_EDIT, "Edit", wxArtProvider::GetBitmap(wxART_EDIT), "Edit password");
    toolbar->AddTool(ID_DELETE, "Delete", wxArtProvider::GetBitmap(wxART_DELETE), "Delete password");
    toolbar->AddSeparator();
    toolbar->AddTool(ID_GENERATE, "Generate", wxArtProvider::GetBitmap(wxART_REDO), "Generate password");

    toolbar->Realize();
}

void MainFrame::SetupList() {
    listCtrl_ = new wxListCtrl(this, ID_LIST, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL);

    listCtrl_->AppendColumn("Service", wxLIST_FORMAT_LEFT, 200);
    listCtrl_->AppendColumn("Username", wxLIST_FORMAT_LEFT, 200);
    listCtrl_->AppendColumn("ID", wxLIST_FORMAT_LEFT, 150);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(listCtrl_, 1, wxEXPAND | wxALL, 5);
    SetSizer(sizer);
}

void MainFrame::LoadPasswords() {
    listCtrl_->DeleteAllItems();

    auto entries = pm_->listPasswords();
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        long idx = listCtrl_->InsertItem(i, entry.service);
        listCtrl_->SetItem(idx, 1, entry.username);
        listCtrl_->SetItem(idx, 2, entry.id);
        listCtrl_->SetItemData(idx, (long)i);
    }
}

void MainFrame::OnAdd(wxCommandEvent& event) {
    AddPasswordDialog dlg(this, pm_);
    if (dlg.ShowModal() == wxID_OK) {
        LoadPasswords();
    }
}

void MainFrame::OnEdit(wxCommandEvent& event) {
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Please select an item to edit", "Info");
        return;
    }

    wxString id = listCtrl_->GetItemText(sel, 2);
    PasswordEntry* entry = pm_->getPassword(id.ToStdString());
    if (entry) {
        PasswordDialog dlg(this, pm_, entry);
        if (dlg.ShowModal() == wxID_OK) {
            LoadPasswords();
        }
    }
}

void MainFrame::OnDelete(wxCommandEvent& event) {
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Please select an item to delete", "Info");
        return;
    }

    wxString id = listCtrl_->GetItemText(sel, 2);
    wxString service = listCtrl_->GetItemText(sel, 0);

    int res = wxMessageBox("Delete password for " + service + "?", "Confirm",
        wxYES_NO | wxICON_QUESTION);
    if (res == wxYES) {
        pm_->removePassword(id.ToStdString());
        LoadPasswords();
    }
}

void MainFrame::OnGenerate(wxCommandEvent& event) {
    PasswordGeneratorDialog dlg(this, pm_);
    dlg.ShowModal();
}

void MainFrame::OnSearch(wxCommandEvent& event) {
    // TODO: Implement search dialog
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("Password Manager\nUsing wxWidgets and JSON", "About", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnItemActivated(wxListEvent& event) {
    OnEdit(event);
}