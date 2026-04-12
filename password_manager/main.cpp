#include <wx/wx.h>
#include "MainFrame.h"

class PasswordManagerApp : public wxApp {
public:
    virtual bool OnInit() {
        MainFrame* frame = new MainFrame("Password Manager");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(PasswordManagerApp);