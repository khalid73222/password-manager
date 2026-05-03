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
    MainFrame(const wxString& title);
    ~MainFrame();

private:
    PasswordManager* pm_;
    wxListCtrl* listCtrl_;
    wxTextCtrl* txtSearch_;
    wxTimer* inactivityTimer_;
    bool             locked_;

    // Status bar panels
    wxStaticText* lblStatus_;

    static const int INACTIVITY_TIMEOUT;  // ms

    void SetupMenu();
    void SetupToolbar();
    void SetupList();
    void SetupStatusBar();
    void LoadPasswords();
    void LoadPasswords(const std::vector<PasswordEntry>& entries);
    void UpdateStatusBar(int total, int shown);

    bool CheckPin();
    void LockApp();
    void UnlockApp();
    void SetupPin();
    void RecoverPin();

    // Event handlers
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    void OnSearchText(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnAnyActivity(wxEvent& event);
    void OnChangePin(wxCommandEvent& event);
    void OnClearSearch(wxCommandEvent& event);

    // Filter buttons
    void OnFilterAll(wxCommandEvent& event);
    void OnFilterWork(wxCommandEvent& event);
    void OnFilterPersonal(wxCommandEvent& event);
    void OnFilterFinance(wxCommandEvent& event);
    void OnFilterSocial(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif