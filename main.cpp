#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/dcbuffer.h>
#include <wx/notebook.h>
#include <memory>
#include <map>
#include <regex>
#include <fstream>
#include "multi_tape_turing_machine.h"
#include "Sequence.h"
#include "templates.h"
#include "MainFrame.h"

class TuringApp : public wxApp {
public:
    bool OnInit() override {
        wxInitAllImageHandlers();

        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(TuringApp);