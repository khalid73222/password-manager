#pragma once
#ifndef CATEGORYMANAGERDIALOG_H
#define CATEGORYMANAGERDIALOG_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "PasswordManager.h"

class CategoryManagerDialog : public wxDialog {
public:
    CategoryManagerDialog(wxWindow* parent, PasswordManager* pm);

private:
    PasswordManager* pm_;
    wxListCtrl* list_;
    wxTextCtrl* txtName_;
    wxButton* btnColour_;
    wxColour         selectedColour_;
    wxStaticText* lblPreview_;

    void RebuildList();
    wxColour GetColourForIndex(int idx) const;

    void OnAdd(wxCommandEvent&);
    void OnRename(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnPickColour(wxCommandEvent&);
    void OnListSelect(wxListEvent&);
    void OnClose(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif