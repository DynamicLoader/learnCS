#ifndef __UI_ADV_HPP__
#define __UI_ADV_HPP__

// STL headers
#include <iostream>
#include <string>
#include <future>
#include <chrono>
#include <thread> // std::async

// custom headers
#include "./gui.h"
#include "./calculator.h"

class UI_ADV {
private:
    UIMain* ui = nullptr;

public:
    UI_ADV(UIMain* ui)
    {
        this->ui = ui;
        ui->btnAdvOpen->callback(advOpenCB, this);
        ui->btnAdvSave->callback(advSaveCB, this);
        ui->btnAdvAC->callback(advACCB,this);
        ui->btnAdvCalc->callback(advCalcCB,this);
    }

    void advSwitched(){
        this->ui->txtR->value("");
    }

    static void advOpenCB(Fl_Widget* w, void* ptr)
    {
        auto ui = ((UI_ADV*)ptr)->ui;
        char* filename = fl_file_chooser("Load file", "*", "");
        if (!filename) // Cancelled
            return;
        int ret = ui->tbfAdv.loadfile(filename);
        if (ret) {
            fl_message_title("HRCore Calculator - Error");
            fl_beep(FL_BEEP_ERROR);
            fl_alert("Error reading file: %s \n"
                     "May be the file is unreadable or its size > 128 KB?",
                filename);
            return;
        }
        return;
    }

    static void advSaveCB(Fl_Widget* w, void* ptr)
    {
        UIMain* ui = ((UI_ADV*)ptr)->ui;
        Fl_Text_Buffer& buf = ui->tbfAdvOut;
        Fl_File_Chooser cho(".", "*", Fl_File_Chooser::CREATE, "Save To...");
        cho.show();
        while (cho.shown()) {
            Fl::wait();
        }
        if (!cho.count())
            return;
        ui->txtStatus->value("Saving Files...");

        int ret = buf.savefile(cho.value());
        if (ret) {
            fl_message_title("HRCore Calculator - Error");
            fl_beep(FL_BEEP_ERROR);
            fl_alert("Error while saving to %s \n"
                     "Maybe the file is not writable or data size > 128 KB?",
                cho.value());
        }
        ui->txtStatus->value("Ready.");
        return;
    }

    static void advACCB(Fl_Widget*, void* ptr)
    {
        UIMain* ui = ((UI_ADV*)ptr)->ui;
        ui->tbfAdvOut.text("");
        ui->tbfAdv.text("");
    }

    static void advCalcCB(Fl_Widget*, void* ptr)
    {
        UIMain* ui = ((UI_ADV*)ptr)->ui;
        // Lock the elements
        ui->tab0->deactivate();
        ui->txtStatus->value("Preparing operand...");
        ui->busy = true;
        Fl::wait();
        char* ta = ui->tbfAdv.text();
        ui->btnStop->activate();
        ui->txtStatus->value("Calculating...");
        stopCalc = false;
        uint8_t mode = (ui->optAB->value() << 4) + ui->optBC->value();
        auto x = std::async(std::launch::async, advCalculate, ta);
        while (x.wait_for(std::chrono::milliseconds(20)) != std::future_status::ready) {
            Fl::wait();
        }
        delete ta; // Release memory allocated in tbf::text()

        ui->tbfAdvOut.text(x.get().c_str());
        ui->txtStatus->value("Ready.");
        ui->busy = false;
        stopCalc = false;
        ui->btnStop->deactivate();
        ui->tab0->activate();
    }
};

#endif