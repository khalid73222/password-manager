#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "PasswordManager.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

private:
    PasswordManager* pm_;
    wxListCtrl* listCtrl_;

    void SetupMenu();
    void SetupToolbar();
    void SetupList();
    void LoadPasswords();

    // Event handlers
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnItemActivated(wxListEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif