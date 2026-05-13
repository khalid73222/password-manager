#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include "PasswordManager.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxString& vaultPath);
    ~MainFrame();

private:
    PasswordManager* pm_;
    wxListCtrl* listCtrl_;
    wxTextCtrl* txtSearch_;
    wxTimer* inactivityTimer_;
    bool             locked_;
    wxPanel* filterPanel_;   // rebuilt when categories change
    wxBoxSizer* outerSizer_;    // kept so we can insert/remove filterPanel_

    wxStaticText* lblStatus_;
    static const int INACTIVITY_TIMEOUT;

    void SetupMenu();
    void SetupToolbar();
    void SetupList();
    void SetupStatusBar();
    void RebuildFilterBar();   // dynamic — call after category changes
    void LoadPasswords();
    void LoadPasswords(const std::vector<PasswordEntry>& entries);
    void UpdateStatusBar(int total, int shown);

    bool CheckPin();
    void LockApp();
    void UnlockApp();
    void SetupPin();
    void RecoverPin();

    void OnAdd(wxCommandEvent&);
    void OnEdit(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnGenerate(wxCommandEvent&);
    void OnSearch(wxCommandEvent&);
    void OnSearchText(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnItemActivated(wxListEvent&);
    void OnTimer(wxTimerEvent&);
    void OnAnyActivity(wxEvent&);
    void OnChangePin(wxCommandEvent&);
    void OnClearSearch(wxCommandEvent&);
    void OnManageCategories(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif