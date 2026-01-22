#pragma once

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
#include "Compiler.h"
#include "identifier.h"
#include "templates.h"

class TapeCanvas : public wxPanel {
    const BidirectionalLazyTape<char>* tape = nullptr;
    int headPos = 0;
    int tapeIndex = 0;

public:
    TapeCanvas(wxWindow* parent, int index) : wxPanel(parent), tapeIndex(index) {
        tape = nullptr;  
        headPos = 0;
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &TapeCanvas::OnPaint, this);
        Bind(wxEVT_SIZE, &TapeCanvas::OnResize, this);
    }

    ~TapeCanvas() {
        Unbind(wxEVT_PAINT, &TapeCanvas::OnPaint, this);
        Unbind(wxEVT_SIZE, &TapeCanvas::OnResize, this);
    }

    void UpdateData(const BidirectionalLazyTape<char>* t, int pos) {
        tape = t;
        headPos = pos;
        Refresh();
    }

    void OnResize(wxSizeEvent& evt) {
        Refresh();
    }

    void OnPaint(wxPaintEvent& evt) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(wxBrush(wxColour(30, 30, 30)));
        dc.Clear();

        if (!tape) {
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText("No Tape Data", 10, 10);
            return;
        }

        wxSize sz = GetClientSize();
        int cellW = 40;
        int cellH = 40;
        int half = (sz.GetWidth() / cellW) / 2 + 1;

        wxFont font = dc.GetFont();
        font.MakeBold();
        font.SetPointSize(12);
        dc.SetFont(font);

        for (int i = headPos - half; i <= headPos + half; ++i) {
            int x = (sz.GetWidth() / 2) - (cellW / 2) + (i - headPos) * cellW;
            int y = (sz.GetHeight() / 2) - (cellH / 2);

            if (i == headPos) {
                dc.SetBrush(wxBrush(wxColour(70, 120, 200)));
                dc.SetPen(wxPen(wxColour(255, 255, 255), 2));
            }
            else {
                dc.SetBrush(wxBrush(wxColour(60, 60, 60)));
                dc.SetPen(wxPen(wxColour(100, 100, 100)));
            }

            dc.DrawRectangle(x, y, cellW, cellH);

            char c = tape->Get(i);
            wxString sym = wxString::Format("%c", c);

            wxSize textSz = dc.GetTextExtent(sym);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText(sym, x + (cellW - textSz.x) / 2, y + (cellH - textSz.y) / 2);
        }

        int cx = sz.GetWidth() / 2;
        int cy = (sz.GetHeight() / 2) + (cellH / 2) + 5;
        wxPoint points[3] = {
            wxPoint(cx, cy),
            wxPoint(cx - 8, cy + 12),
            wxPoint(cx + 8, cy + 12)
        };
        dc.SetBrush(wxBrush(wxColour(255, 215, 0)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawPolygon(3, points);
    }
};

//  Главное окно
class MainFrame : public wxFrame {
    std::unique_ptr<MultiTapeTuringMachine> machine;
    wxTimer* timer;

    struct StoredTransition {
        std::string fromState;
        std::string toState;
        std::array<char, MultiTapeTuringMachine::MAX_TAPES> readSymbols;
        std::array<char, MultiTapeTuringMachine::MAX_TAPES> writeSymbols;
        std::array<int, MultiTapeTuringMachine::MAX_TAPES> moves;

        StoredTransition() {
            readSymbols.fill(' ');
            writeSymbols.fill(' ');
            moves.fill(0);
        }
    };

    Sequence<StoredTransition> storedTransitions;

    wxSpinCtrl* spinTapeCount;
    wxSlider* sliderSpeed;
    wxButton* btnRun;
    wxStaticText* lblState;
    wxStaticText* lblStep;
    wxStaticText* lblStatus;

    wxScrolledWindow* tapesScroll;
    wxBoxSizer* tapesSizer;
    Sequence<TapeCanvas*> tapeCanvases;
    Sequence<wxTextCtrl*> inputEdits;

    wxGrid* transitionsGrid;
    wxTextCtrl* txtFromState;
    wxTextCtrl* txtToState;
    Sequence<wxTextCtrl*> readEdits;
    Sequence<wxTextCtrl*> writeEdits;
    Sequence<wxChoice*> moveChoices;

    wxTextCtrl* programText;
    wxStaticText* compileStatus;

public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "Turing Machine Compiler & Simulator", wxDefaultPosition, wxSize(1400, 950)) {

        machine = nullptr; 
        timer = nullptr;

        // Создаем таймер после создания окна
        timer = new wxTimer(this, ID_TIMER);

        wxNotebook* notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

        // Вкладка 1: Программа (Компилятор)
        wxPanel* progPanel = new wxPanel(notebook);
        wxBoxSizer* progSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticBoxSizer* codeGroup = new wxStaticBoxSizer(wxVERTICAL, progPanel, "Turing Machine Program");

        programText = new wxTextCtrl(progPanel, ID_PROGRAM_TEXT, "", wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_WORDWRAP);
        wxFont monoFont = programText->GetFont();
        monoFont.SetFamily(wxFONTFAMILY_TELETYPE);
        monoFont.SetPointSize(10);
        programText->SetFont(monoFont);

        programText->SetValue(
            "# Format: FromState,Read1,Read2,Read3->ToState,Write1,Write2,Write3,Move1,Move2,Move3\n"
            "# Moves: R(Right), L(Left), S(Stay); Use space for blank\n"
            "# Example:\n"
            "q0,0, , ->q0,0,0, ,R,R,S\n"
            "q0,1, , ->q0,1,1, ,R,R,S\n"
            "q0, , , ->q1, , , ,S,S,S\n"
        );

        codeGroup->Add(programText, 1, wxEXPAND | wxALL, 5);

        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* btnCompile = new wxButton(progPanel, ID_COMPILE, "Compile");
        wxButton* btnLoadFile = new wxButton(progPanel, ID_LOAD_FILE, "Load File");
        wxButton* btnSaveFile = new wxButton(progPanel, ID_SAVE_FILE, "Save File");

        buttonSizer->Add(btnCompile, 0, wxALL, 5);
        buttonSizer->Add(btnLoadFile, 0, wxALL, 5);
        buttonSizer->Add(btnSaveFile, 0, wxALL, 5);

        codeGroup->Add(buttonSizer, 0, wxEXPAND);

        progSizer->Add(codeGroup, 1, wxEXPAND | wxALL, 5);

        // Кнопки для шаблонов
        wxStaticBoxSizer* templateGroup = new wxStaticBoxSizer(wxHORIZONTAL, progPanel, "Templates");
        wxButton* btnTemplateAlphabet = new wxButton(progPanel, ID_TEMPLATE_ALPHABET, "a-z Copy");
        wxButton* btnTemplateAlphanumeric = new wxButton(progPanel, ID_TEMPLATE_ALPHANUMERIC, "Copy Alphanumeric");
        wxButton* btnTemplateBinary = new wxButton(progPanel, ID_TEMPLATE_BINARY_INVERTER, "Invert (1 Tape)");
        wxButton* btnTemplateBinary2Tapes = new wxButton(progPanel, ID_TEMPLATE_BINARY_INVERTER_2TAPES, "Invert (2 Tapes)");
        wxButton* btnTemplateUnaryAddition = new wxButton(progPanel, ID_TEMPLATE_UNARY_ADDITION, "Unary Addition");

        templateGroup->Add(btnTemplateAlphabet, 0, wxALL, 3);
        templateGroup->Add(btnTemplateAlphanumeric, 0, wxALL, 3);
        templateGroup->Add(btnTemplateBinary, 0, wxALL, 3);
        templateGroup->Add(btnTemplateBinary2Tapes, 0, wxALL, 3);
        templateGroup->Add(btnTemplateUnaryAddition, 0, wxALL, 3);

        progSizer->Add(templateGroup, 0, wxEXPAND | wxALL, 5);

        compileStatus = new wxStaticText(progPanel, wxID_ANY, "Status: Ready");
        wxFont boldFont = compileStatus->GetFont(); boldFont.MakeBold(); compileStatus->SetFont(boldFont);
        progSizer->Add(compileStatus, 0, wxEXPAND | wxALL, 5);

        progPanel->SetSizer(progSizer);
        notebook->AddPage(progPanel, "Program");

        // Вкладка 2: Симуляция
        wxPanel* simPanel = new wxPanel(notebook);
        wxBoxSizer* simSizer = new wxBoxSizer(wxVERTICAL);

        wxPanel* topPanel = new wxPanel(simPanel);
        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

        topSizer->Add(new wxStaticText(topPanel, wxID_ANY, "Number of Tapes:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        spinTapeCount = new wxSpinCtrl(topPanel, ID_TAPE_COUNT, "1", wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 1, 3, 1);
        topSizer->Add(spinTapeCount, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        topPanel->SetSizer(topSizer);
        simSizer->Add(topPanel, 0, wxEXPAND | wxALL, 5);

        tapesScroll = new wxScrolledWindow(simPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
        tapesScroll->SetScrollRate(5, 5);
        tapesScroll->SetBackgroundColour(wxColour(40, 40, 40));

        tapesSizer = new wxBoxSizer(wxVERTICAL);
        tapesScroll->SetSizer(tapesSizer);

        simSizer->Add(tapesScroll, 1, wxEXPAND | wxALL, 5);

        wxPanel* controlPanel = new wxPanel(simPanel);
        controlPanel->SetMinSize(wxSize(-1, 60));
        wxBoxSizer* ctrlSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* btnStep = new wxButton(controlPanel, ID_STEP, "Step");
        btnRun = new wxButton(controlPanel, ID_RUN, "Run");
        wxButton* btnStop = new wxButton(controlPanel, ID_STOP, "Stop");
        wxButton* btnReset = new wxButton(controlPanel, ID_RESET, "Reset");

        ctrlSizer->Add(btnStep, 0, wxALL, 5);
        ctrlSizer->Add(btnRun, 0, wxALL, 5);
        ctrlSizer->Add(btnStop, 0, wxALL, 5);
        ctrlSizer->Add(btnReset, 0, wxALL, 5);

        ctrlSizer->Add(new wxStaticText(controlPanel, wxID_ANY, "Speed:"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
        sliderSpeed = new wxSlider(controlPanel, ID_SPEED_SLIDER, 100, 10, 1000, wxDefaultPosition, wxSize(150, -1));
        ctrlSizer->Add(sliderSpeed, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        lblState = new wxStaticText(controlPanel, wxID_ANY, "State: q0");
        wxFont stateBold = lblState->GetFont(); stateBold.MakeBold(); lblState->SetFont(stateBold);

        lblStep = new wxStaticText(controlPanel, wxID_ANY, "Step: 0");
        lblStatus = new wxStaticText(controlPanel, wxID_ANY, "Ready");

        ctrlSizer->Add(lblState, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
        ctrlSizer->Add(lblStep, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
        ctrlSizer->Add(lblStatus, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);

        controlPanel->SetSizer(ctrlSizer);
        simSizer->Add(controlPanel, 0, wxEXPAND | wxALL, 5);

        simPanel->SetSizer(simSizer);
        notebook->AddPage(simPanel, "Simulation");

        //  Вкладка 3: Переходы 
        wxPanel* transPanel = new wxPanel(notebook);
        wxBoxSizer* transSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticBoxSizer* addTransGroup = new wxStaticBoxSizer(wxVERTICAL, transPanel, "Add New Transition");
        wxFlexGridSizer* formGrid = new wxFlexGridSizer(0, 6, 5, 5);

        formGrid->Add(new wxStaticText(transPanel, wxID_ANY, "From State:"), 0, wxALIGN_CENTER_VERTICAL);
        txtFromState = new wxTextCtrl(transPanel, wxID_ANY, "q0", wxDefaultPosition, wxSize(70, -1));
        formGrid->Add(txtFromState, 0, wxEXPAND);

        formGrid->Add(new wxStaticText(transPanel, wxID_ANY, "To State:"), 0, wxALIGN_CENTER_VERTICAL);
        txtToState = new wxTextCtrl(transPanel, wxID_ANY, "q1", wxDefaultPosition, wxSize(70, -1));
        formGrid->Add(txtToState, 0, wxEXPAND);

        formGrid->Add(new wxStaticText(transPanel, wxID_ANY, ""), 0);
        formGrid->Add(new wxStaticText(transPanel, wxID_ANY, ""), 0);

        for (size_t i = 0; i < MultiTapeTuringMachine::MAX_TAPES; ++i) {
            formGrid->Add(new wxStaticText(transPanel, wxID_ANY, wxString::Format("Tape %zu Read:", i + 1)), 0, wxALIGN_CENTER_VERTICAL);

            auto* readEdit = new wxTextCtrl(transPanel, wxID_ANY, "", wxDefaultPosition, wxSize(50, -1));
            readEdit->SetMaxLength(1);
            readEdits.Append(readEdit);
            formGrid->Add(readEdit, 0, wxEXPAND);

            formGrid->Add(new wxStaticText(transPanel, wxID_ANY, "Write:"), 0, wxALIGN_CENTER_VERTICAL);

            auto* writeEdit = new wxTextCtrl(transPanel, wxID_ANY, "", wxDefaultPosition, wxSize(50, -1));
            writeEdit->SetMaxLength(1);
            writeEdits.Append(writeEdit);
            formGrid->Add(writeEdit, 0, wxEXPAND);

            formGrid->Add(new wxStaticText(transPanel, wxID_ANY, "Move:"), 0, wxALIGN_CENTER_VERTICAL);

            auto* moveChoice = new wxChoice(transPanel, wxID_ANY);
            moveChoice->Append("Stay (0)");
            moveChoice->Append("Left (-1)");
            moveChoice->Append("Right (+1)");
            moveChoice->SetSelection(0);
            moveChoices.Append(moveChoice);
            formGrid->Add(moveChoice, 0, wxEXPAND);
        }

        formGrid->AddGrowableCol(1, 1);
        formGrid->AddGrowableCol(3, 1);
        formGrid->AddGrowableCol(5, 1);

        addTransGroup->Add(formGrid, 0, wxEXPAND | wxALL, 5);

        wxButton* btnAddTransition = new wxButton(transPanel, ID_ADD_TRANSITION, "Add Transition");
        addTransGroup->Add(btnAddTransition, 0, wxALIGN_RIGHT | wxALL, 5);

        transSizer->Add(addTransGroup, 0, wxEXPAND | wxALL, 5);

        wxStaticBoxSizer* tableGroup = new wxStaticBoxSizer(wxVERTICAL, transPanel, "Stored Transitions");

        transitionsGrid = new wxGrid(transPanel, ID_TRANSITIONS_GRID);
        int colCount = 2 + 3 * static_cast<int>(MultiTapeTuringMachine::MAX_TAPES);
        transitionsGrid->CreateGrid(0, colCount);

        transitionsGrid->SetColLabelValue(0, "From");
        transitionsGrid->SetColLabelValue(1, "To");
        for (size_t i = 0; i < MultiTapeTuringMachine::MAX_TAPES; ++i) {
            transitionsGrid->SetColLabelValue(2 + 3 * i, wxString::Format("T%zu Read", i + 1));
            transitionsGrid->SetColLabelValue(3 + 3 * i, wxString::Format("T%zu Write", i + 1));
            transitionsGrid->SetColLabelValue(4 + 3 * i, wxString::Format("T%zu Move", i + 1));
        }

        transitionsGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
        transitionsGrid->AutoSizeColumns();

        tableGroup->Add(transitionsGrid, 1, wxEXPAND | wxALL, 5);

        wxButton* btnRemove = new wxButton(transPanel, ID_REMOVE_TRANSITION, "Remove Selected");
        tableGroup->Add(btnRemove, 0, wxALIGN_RIGHT | wxALL, 5);

        transSizer->Add(tableGroup, 1, wxEXPAND | wxALL, 5);

        transPanel->SetSizer(transSizer);
        notebook->AddPage(transPanel, "Transitions");

        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(notebook, 1, wxEXPAND);
        SetSizer(mainSizer);

        Bind(wxEVT_BUTTON, &MainFrame::OnStep, this, ID_STEP);
        Bind(wxEVT_BUTTON, &MainFrame::OnRun, this, ID_RUN);
        Bind(wxEVT_BUTTON, &MainFrame::OnStop, this, ID_STOP);
        Bind(wxEVT_BUTTON, &MainFrame::OnReset, this, ID_RESET);
        Bind(wxEVT_SPINCTRL, &MainFrame::OnTapeCountChanged, this, ID_TAPE_COUNT);
        Bind(wxEVT_SLIDER, &MainFrame::OnSpeedChange, this, ID_SPEED_SLIDER);
        Bind(wxEVT_TIMER, &MainFrame::OnTimer, this, ID_TIMER);
        Bind(wxEVT_BUTTON, &MainFrame::OnAddTransition, this, ID_ADD_TRANSITION);
        Bind(wxEVT_BUTTON, &MainFrame::OnRemoveTransition, this, ID_REMOVE_TRANSITION);
        Bind(wxEVT_BUTTON, &MainFrame::OnCompile, this, ID_COMPILE);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadFile, this, ID_LOAD_FILE);
        Bind(wxEVT_BUTTON, &MainFrame::OnSaveFile, this, ID_SAVE_FILE);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadTemplate, this, ID_TEMPLATE_ALPHABET);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadTemplate, this, ID_TEMPLATE_ALPHANUMERIC);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadTemplate, this, ID_TEMPLATE_BINARY_INVERTER);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadTemplate, this, ID_TEMPLATE_BINARY_INVERTER_2TAPES);
        Bind(wxEVT_BUTTON, &MainFrame::OnLoadTemplate, this, ID_TEMPLATE_UNARY_ADDITION);

        RebuildTapes(1);
    }

    void ClearAllTransitions() {
        storedTransitions.Clear();
        int rows = transitionsGrid->GetNumberRows();
        if (rows > 0) {
            transitionsGrid->DeleteRows(0, rows);
        }
        transitionsGrid->ForceRefresh();
    }

    void UpdateAcceptRejectStatus() {
        if (!machine) {
            lblStatus->SetLabel("No machine");
            lblStatus->SetForegroundColour(*wxBLACK);
            return;
        }

        try {
            std::string currentState = machine->GetCurrentState();

            if (currentState == "q_reject" || currentState == "reject") {
                lblStatus->SetLabel("REJECTED");
                lblStatus->SetForegroundColour(wxColour(200, 0, 0));
                return;
            }
            else if (currentState == "q_accept" || currentState == "accept") {
                lblStatus->SetLabel("ACCEPTED");
                lblStatus->SetForegroundColour(wxColour(0, 180, 0));
                return;
            }

            if (machine->IsAcceptState()) {
                lblStatus->SetLabel("ACCEPTED");
                lblStatus->SetForegroundColour(wxColour(0, 180, 0));
                return;
            }

            // Проверяем, может ли машина выполнить шаг
            bool canStep = false;
            try {
                for (size_t i = 0; i < machine->GetActiveTapeCount(); ++i) {
                    machine->GetSymbolAtHead(i);
                }
                canStep = true;
            }
            catch (...) {
                canStep = false;
            }

            if (!canStep) {
                // Если машина остановилась без явного accept/reject
                lblStatus->SetLabel("HALTED - No transition");
                lblStatus->SetForegroundColour(wxColour(255, 140, 0));
            }
            else {
                lblStatus->SetLabel("Running...");
                lblStatus->SetForegroundColour(*wxBLACK);
            }

            // Останавливаем таймер если он работает и машина остановлена
            if (!canStep && timer && timer->IsRunning()) {
                timer->Stop();
                btnRun->Enable();
            }
        }
        catch (const std::exception& e) {
            wxLogDebug("Status update error: %s", e.what());
            lblStatus->SetLabel("Error");
            lblStatus->SetForegroundColour(*wxRED);
        }
    }

    void RebuildTapes(int count) {
        // Останавливаем таймер перед перестройкой
        if (timer && timer->IsRunning()) {
            timer->Stop();
        }

        // Удаляем старые объекты
        for (size_t i = 0; i < tapeCanvases.GetSize(); ++i) {
            TapeCanvas* canvas = tapeCanvases[i];
            if (canvas) {
                canvas->Destroy();
            }
        }

        for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
            wxTextCtrl* edit = inputEdits[i];
            if (edit) {
                edit->Destroy();
            }
        }

        tapeCanvases.Clear();
        inputEdits.Clear();

        // Пересоздаем макет
        if (tapesSizer) {
            tapesSizer->Clear(true);
        }

        // Создаем новые элементы
        for (int i = 0; i < count; ++i) {
            wxPanel* rowPanel = new wxPanel(tapesScroll);
            rowPanel->SetBackgroundColour(wxColour(50, 50, 50));
            wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

            wxTextCtrl* input = new wxTextCtrl(rowPanel, wxID_ANY, "", wxDefaultPosition, wxSize(100, -1));
            input->SetHint(wxString::Format("Tape %d Input", i + 1));
            inputEdits.Append(input);
            rowSizer->Add(input, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

            TapeCanvas* canvas = new TapeCanvas(rowPanel, i);
            canvas->SetMinSize(wxSize(-1, 80));
            tapeCanvases.Append(canvas);
            rowSizer->Add(canvas, 1, wxEXPAND | wxALL, 2);

            rowPanel->SetSizer(rowSizer);
            tapesSizer->Add(rowPanel, 0, wxEXPAND | wxALL, 2);
        }

        tapesScroll->FitInside();
        tapesScroll->Layout();

        // Принудительное обновление
        Layout();
        Refresh();

        ResetMachine(count);
    }

    void RecreateMachine() {
        int count = spinTapeCount->GetValue();

        try {
            // уничтожаем старую машину
            machine.reset();

            // Создаем новую машину
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            // Восстанавливаем переходы
            for (size_t i = 0; i < storedTransitions.GetSize(); ++i) {
                const auto& trans = storedTransitions[i];
                machine->AddTransition(
                    trans.fromState,
                    trans.readSymbols,
                    trans.toState,
                    trans.writeSymbols,
                    trans.moves
                );
            }

            // Восстанавливаем входные данные
            Sequence<std::string> inputs;
            for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
                wxTextCtrl* edit = inputEdits[i];
                std::string input = edit->GetValue().ToStdString();
                inputs.Append(input);
            }

            if (!inputs.IsEmpty()) {
                machine->InitializeTapes(inputs); 
            }

            UpdateUI();

        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Error Recreating Machine", wxICON_ERROR);

            // Создаем простую машину при ошибке
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);
            UpdateUI();
        }
    }

    void ResetMachine(int count) {
        try {
            // Пересоздаем машину
            RecreateMachine();

            lblStatus->SetLabel("Machine Reset");

        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Reset Error", wxICON_ERROR);
            lblStatus->SetLabel("Reset Failed");
        }
    }

    void UpdateUI() {
        if (!machine) {
            lblState->SetLabel("State: (no machine)");
            lblStep->SetLabel("Step: 0");
            lblStatus->SetLabel("Ready");
            lblStatus->SetForegroundColour(*wxBLACK);

            for (size_t i = 0; i < tapeCanvases.GetSize(); ++i) {
                if (tapeCanvases[i]) {
                    tapeCanvases[i]->UpdateData(nullptr, 0);
                }
            }
            return;
        }

        try {
            std::string currentState = machine->GetCurrentState();
            size_t stepCount = machine->GetStepCount();

            lblState->SetLabel("State: " + currentState);
            lblStep->SetLabel("Step: " + std::to_string(stepCount));

            UpdateAcceptRejectStatus();

            // Обновляем ленты
            auto positions = machine->GetHeadPositions();
            for (size_t i = 0; i < tapeCanvases.GetSize(); ++i) {
                if (i < positions.size() && i < tapeCanvases.GetSize()) {
                    const BidirectionalLazyTape<char>* tape = machine->GetTape(i);
                    if (tape && tapeCanvases[i]) {
                        tapeCanvases[i]->UpdateData(tape, positions[i]);
                    }
                }
            }
        }
        catch (const std::exception& e) {
            wxLogDebug("UI update error: %s", e.what());
            lblState->SetLabel("State: ERROR");
            lblStep->SetLabel("Step: ERROR");
            lblStatus->SetLabel("Error");
            lblStatus->SetForegroundColour(*wxRED);
        }
    }

    void OnAddTransition(wxCommandEvent& evt) {
        wxString fromState = txtFromState->GetValue().Trim();
        wxString toState = txtToState->GetValue().Trim();

        if (fromState.IsEmpty() || toState.IsEmpty()) {
            wxMessageBox("States cannot be empty!", "Error", wxICON_ERROR);
            return;
        }

        std::array<char, MultiTapeTuringMachine::MAX_TAPES> readSyms;
        std::array<char, MultiTapeTuringMachine::MAX_TAPES> writeSyms;
        std::array<int, MultiTapeTuringMachine::MAX_TAPES> moves;

        readSyms.fill(' ');
        writeSyms.fill(' ');
        moves.fill(0);

        for (size_t i = 0; i < MultiTapeTuringMachine::MAX_TAPES; ++i) {
            wxString readStr = readEdits[i]->GetValue();
            wxString writeStr = writeEdits[i]->GetValue();
            int moveIdx = moveChoices[i]->GetSelection();

            if (!readStr.IsEmpty()) {
                readSyms[i] = readStr[0]; 
            }

            if (!writeStr.IsEmpty()) {
                writeSyms[i] = writeStr[0]; 
            }

            if (moveIdx == 0) moves[i] = 0;
            else if (moveIdx == 1) moves[i] = -1;
            else if (moveIdx == 2) moves[i] = 1;
        }

        StoredTransition trans;
        trans.fromState = fromState.ToStdString();
        trans.toState = toState.ToStdString();
        trans.readSymbols = readSyms;
        trans.writeSymbols = writeSyms;
        trans.moves = moves;

        // 1. Добавляем в storedTransitions
        storedTransitions.Append(trans);

        // 2. Сохраняем текущие входные данные
        Sequence<std::string> currentInputs;
        for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
            wxTextCtrl* edit = inputEdits[i];
            std::string input = edit->GetValue().ToStdString(); 
            currentInputs.Append(input);
        }

        // 3. Пересоздаем машину
        int count = spinTapeCount->GetValue();

        try {
            // Уничтожаем старую машину
            machine.reset();

            // Создаем новую машину
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            // 4. Восстанавливаем все переходы из storedTransitions
            for (size_t i = 0; i < storedTransitions.GetSize(); ++i) {
                const auto& t = storedTransitions[i];
                machine->AddTransition(
                    t.fromState,
                    t.readSymbols,
                    t.toState,
                    t.writeSymbols,
                    t.moves
                );
            }

            // 5. Восстанавливаем входные данные
            if (!currentInputs.IsEmpty() && machine) {
                machine->InitializeTapes(currentInputs);
            }

            // 6. Добавляем строку в таблицу
            int row = transitionsGrid->GetNumberRows();
            transitionsGrid->AppendRows(1);

            transitionsGrid->SetCellValue(row, 0, fromState);
            transitionsGrid->SetCellValue(row, 1, toState);

            int col = 2;
            for (size_t i = 0; i < MultiTapeTuringMachine::MAX_TAPES; ++i) {
                char readChar = readSyms[i];
                char writeChar = writeSyms[i];
                int moveVal = moves[i];

                transitionsGrid->SetCellValue(row, col++, wxString(readChar));
                transitionsGrid->SetCellValue(row, col++, wxString(writeChar));
                transitionsGrid->SetCellValue(row, col++, wxString::Format("%d", moveVal));
            }

            // 7. Очищаем форму
            txtFromState->SetValue("q0");
            txtToState->SetValue("q1");
            for (size_t i = 0; i < readEdits.GetSize(); ++i) readEdits[i]->SetValue("");
            for (size_t i = 0; i < writeEdits.GetSize(); ++i) writeEdits[i]->SetValue("");
            for (size_t i = 0; i < moveChoices.GetSize(); ++i) moveChoices[i]->SetSelection(0);

            // 8. Обновляем UI
            UpdateUI();
            lblStatus->SetLabel("Transition Added");

        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Error", wxICON_ERROR);

            // Создаем простую машину при ошибке
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);
            UpdateUI();
            lblStatus->SetLabel("Error - Transition Not Added");
        }
        if (machine) {
            machine->SetAcceptState("q_accept");
        }
    }

    void OnRemoveTransition(wxCommandEvent& evt) {
        // Проверяем, существует ли машина
        if (!machine) {
            wxMessageBox("No machine to modify!", "Error", wxICON_ERROR);
            return;
        }

        int currentRows = transitionsGrid->GetNumberRows();
        if (currentRows <= 0) {
            wxMessageBox("No transitions to remove!", "Error", wxICON_WARNING);
            return;
        }

        int row = transitionsGrid->GetGridCursorRow();
        if (row < 0 || row >= currentRows) {
            wxMessageBox("Select a transition to remove!", "Error", wxICON_WARNING);
            return;
        }

        // Сохраняем текущие данные машины
        Sequence<std::string> currentInputs;
        for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
            wxTextCtrl* edit = inputEdits[i];
            std::string input = edit->GetValue().ToStdString();
            currentInputs.Append(input);
        }

        // 1. Удаляем переход из storedTransitions
        if (row < (int)storedTransitions.GetSize()) {
            storedTransitions.RemoveAt(row);
        }

        // 2. Удаляем строку из таблицы
        transitionsGrid->DeleteRows(row, 1);

        // 3. Пересоздаем машину
        int count = spinTapeCount->GetValue();

        try {
            // Уничтожаем старую машину
            machine.reset();

            // Создаем новую машину
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            // 4. Восстанавливаем все переходы из storedTransitions
            for (size_t i = 0; i < storedTransitions.GetSize(); ++i) {
                const auto& trans = storedTransitions[i];
                machine->AddTransition(
                    trans.fromState,
                    trans.readSymbols,
                    trans.toState,
                    trans.writeSymbols,
                    trans.moves
                );
            }

            // 5. Восстанавливаем входные данные
            if (!currentInputs.IsEmpty() && machine) {
                machine->InitializeTapes(currentInputs);
            }

            // 6. Обновляем UI
            UpdateUI();
            lblStatus->SetLabel("Transition Removed");

        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Error", wxICON_ERROR);

            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);
            UpdateUI();
            lblStatus->SetLabel("Error - Machine Reset");
        }
    }

    void OnStep(wxCommandEvent& evt) {
        try {
            bool moved = machine->ExecuteStep();
            UpdateUI();

            std::string currentState = machine->GetCurrentState();

            if (!moved) {
                UpdateAcceptRejectStatus();
            }
        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Error", wxICON_ERROR);
            if (timer && timer->IsRunning()) {
                timer->Stop();
            }
            btnRun->Enable();
            UpdateAcceptRejectStatus();
        }
    }

    void OnRun(wxCommandEvent& evt) {
        int delay = 1100 - sliderSpeed->GetValue();
        timer->Start(delay);
        btnRun->Disable();
        lblStatus->SetLabel("Running...");
    }

    void OnStop(wxCommandEvent& evt) {
        timer->Stop();
        btnRun->Enable();
        lblStatus->SetLabel("Stopped");
    }

    void OnReset(wxCommandEvent& evt) {
        OnStop(evt);

        Sequence<std::string> inputs;
        for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
            wxTextCtrl* edit = inputEdits[i];
            inputs.Append(edit->GetValue().ToStdString());
        }

        int count = spinTapeCount->GetValue();

        try {
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            machine->SetAcceptState("q_accept");
            machine->SetAcceptState("accept");

            // Восстанавливаем переходы
            for (size_t i = 0; i < storedTransitions.GetSize(); ++i) {
                const auto& trans = storedTransitions[i];
                machine->AddTransition(trans.fromState, trans.readSymbols, trans.toState, trans.writeSymbols, trans.moves);
            }

            // Инициализируем ленты
            if (!inputs.IsEmpty()) {
                machine->InitializeTapes(inputs);
            }

            UpdateUI();
            lblStatus->SetLabel("Reset Complete");

        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Reset Error", wxICON_ERROR);
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            machine->SetAcceptState("q_accept");
            machine->SetAcceptState("accept");

            UpdateUI();
        }
    }

    void OnTapeCountChanged(wxSpinEvent& evt) {
        RebuildTapes(evt.GetValue());
    }

    void OnSpeedChange(wxCommandEvent& evt) {
        if (timer->IsRunning()) {
            OnRun(evt);
        }
    }

    void OnTimer(wxTimerEvent& evt) {
        try {
            std::string currentState = machine->GetCurrentState();


            bool moved = machine->ExecuteStep();
            UpdateUI();

            currentState = machine->GetCurrentState();

            if (!moved || currentState == "q_accept" || currentState == "q_reject") {
                timer->Stop();
                btnRun->Enable();
                UpdateAcceptRejectStatus();
            }
        }
        catch (const std::exception& e) {
            timer->Stop();
            btnRun->Enable();
            wxMessageBox(e.what(), "Runtime Error");
            UpdateAcceptRejectStatus();
        }
    }

    void OnCompile(wxCommandEvent& evt) {
        try {
            std::string code = programText->GetValue().ToStdString();
            int tapeCount = spinTapeCount->GetValue();

            std::string error;
            Sequence<TuringMachineCompiler::ParsedTransition> compiledTransitions =
                TuringMachineCompiler::Compile(code, tapeCount, error);

            if (!error.empty()) {
                wxMessageBox(error, "Compilation Errors", wxICON_WARNING);
            }

            // Сохраняем текущие входные данные
            Sequence<std::string> currentInputs;
            for (size_t i = 0; i < inputEdits.GetSize(); ++i) {
                currentInputs.Append(inputEdits[i]->GetValue().ToStdString());
            }

            // 1. Очищаем storedTransitions
            storedTransitions.Clear();

            // 2. Очищаем таблицу
            int rows = transitionsGrid->GetNumberRows();
            if (rows > 0) {
                transitionsGrid->DeleteRows(0, rows);
            }

            // 3. Создаем новую машину
            machine = std::make_unique<MultiTapeTuringMachine>("q0", tapeCount);

            // 4.Добавляем accept states
            machine->SetAcceptState("q_accept");
            machine->SetAcceptState("accept");
            machine->SetAcceptState("q_reject"); 
            machine->SetAcceptState("reject");
            machine->SetAcceptState("q1");
            machine->SetAcceptState("q2");
            machine->SetAcceptState("halt");
            machine->SetAcceptState("end");
            machine->SetAcceptState("stop");
            machine->SetAcceptState("final");

            // 5. Добавляем переходы
            for (size_t i = 0; i < compiledTransitions.GetSize(); ++i) {
                const auto& trans = compiledTransitions[i];
                StoredTransition stored;
                stored.fromState = trans.fromState;
                stored.toState = trans.toState;
                stored.readSymbols = trans.readSymbols;
                stored.writeSymbols = trans.writeSymbols;
                stored.moves = trans.moves;

                storedTransitions.Append(stored);

                // Добавляем переход в машину
                machine->AddTransition(
                    trans.fromState,
                    trans.readSymbols,
                    trans.toState,
                    trans.writeSymbols,
                    trans.moves
                );

                // Добавляем строку в таблицу
                int row = transitionsGrid->GetNumberRows();
                transitionsGrid->AppendRows(1);

                transitionsGrid->SetCellValue(row, 0, wxString(trans.fromState));
                transitionsGrid->SetCellValue(row, 1, wxString(trans.toState));

                int col = 2;
                for (size_t j = 0; j < MultiTapeTuringMachine::MAX_TAPES; ++j) {
                    transitionsGrid->SetCellValue(row, col++, wxString(1, trans.readSymbols[j]));
                    transitionsGrid->SetCellValue(row, col++, wxString(1, trans.writeSymbols[j]));
                    transitionsGrid->SetCellValue(row, col++, wxString::Format("%d", trans.moves[j]));
                }
            }

            transitionsGrid->AutoSizeColumns();

            // 6. Восстанавливаем входные данные
            if (!currentInputs.IsEmpty() && machine) {
                machine->InitializeTapes(currentInputs);
            }

            // 7. Проверяем, какие состояния являются accept states
            wxLogDebug("Machine created with accept states: q_accept, q_reject, accept, reject, q1, q2");

            // 8. Обновляем UI
            UpdateUI();
            Layout();
            Refresh();

            compileStatus->SetLabel(wxString::Format("Compiled: %zu transitions", compiledTransitions.GetSize()));
            lblStatus->SetLabel("Compiled successfully");

        }
        catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Compilation error: %s", e.what()), "Error", wxICON_ERROR);

            // Восстанавливаем машину при ошибке
            int count = spinTapeCount->GetValue();
            machine = std::make_unique<MultiTapeTuringMachine>("q0", count);

            // Устанавливаем accept states
            machine->SetAcceptState("q_accept");
            machine->SetAcceptState("accept");
            machine->SetAcceptState("q_reject"); 

            UpdateUI();
        }
    }

    void OnLoadFile(wxCommandEvent& evt) {
        wxFileDialog openDialog(this, "Open Turing Machine Program", "", "",
            "TM files (*.tm)|*.tm|Text files (*.txt)|*.txt|All files (*.*)|*.*",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (openDialog.ShowModal() == wxID_CANCEL) return;

        std::ifstream file(openDialog.GetPath().ToStdString());
        if (!file.is_open()) {
            wxMessageBox("Cannot open file!", "Error", wxICON_ERROR);
            return;
        }

        std::string line, content;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        file.close();

        programText->SetValue(content);
        compileStatus->SetLabel("File loaded successfully");
    }

    void OnSaveFile(wxCommandEvent& evt) {
        wxFileDialog saveDialog(this, "Save Turing Machine Program", "", "",
            "TM files (*.tm)|*.tm|Text files (*.txt)|*.txt|All files (*.*)|*.*",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (saveDialog.ShowModal() == wxID_CANCEL) return;

        std::ofstream file(saveDialog.GetPath().ToStdString());
        if (!file.is_open()) {
            wxMessageBox("Cannot save file!", "Error", wxICON_ERROR);
            return;
        }

        file << programText->GetValue().ToStdString();
        file.close();

        compileStatus->SetLabel("File saved successfully");
    }

    void OnLoadTemplate(wxCommandEvent& evt) {
        std::string templateCode;

        switch (evt.GetId()) {
        case ID_TEMPLATE_ALPHABET:
            templateCode = TuringTemplates::CopyEnglishAlphabet();
            break;
        case ID_TEMPLATE_ALPHANUMERIC:
            templateCode = TuringTemplates::CopyAlphanumeric();
            break;
        case ID_TEMPLATE_BINARY_INVERTER:
            templateCode = TuringTemplates::BinaryInverter();
            break;
        case ID_TEMPLATE_BINARY_INVERTER_2TAPES:
            templateCode = TuringTemplates::BinaryInverter2Tapes();
            break;
        case ID_TEMPLATE_UNARY_ADDITION:
            templateCode = TuringTemplates::UnaryAddition();
            break;
        default:
            return;
        }

        programText->SetValue(templateCode);
        compileStatus->SetLabel("Template loaded - Ready to compile");
    }

    ~MainFrame() {
        // Останавливаем таймер
        if (timer && timer->IsRunning()) {
            timer->Stop();
            btnRun->Enable();  // Включаем кнопку если она была отключена
        }

        if (timer) {
            Unbind(wxEVT_TIMER, &MainFrame::OnTimer, this, ID_TIMER);
        }

        machine.reset();

        storedTransitions.Clear();

        tapeCanvases.Clear();
        inputEdits.Clear();
        readEdits.Clear();
        writeEdits.Clear();
        moveChoices.Clear();

        if (timer) {
            delete timer;
            timer = nullptr;
        }
    }
};
