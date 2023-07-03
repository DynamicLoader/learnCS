#ifndef __UI_EASY_HPP__
#define __UI_EASY_HPP__

// STL headers
#include <iostream>
#include <string>
#include <future>
#include <chrono>
#include <thread> // std::async

// custom headers
#include "./gui.h"
#include "./calculator.h"

// using namespace std;

class UI_Easy {
private:
    UIMain* ui;

public:
    UI_Easy(UIMain* ptr)
    {
        this->ui = ptr;
        ui->optAB->callback(easyChangedCB, this->ui);
        ui->optBC->callback(easyChangedCB, this->ui);
        ui->btnEasyCalc->callback(easyCalcCB, this->ui);
        ui->btnEasyAC->callback(easyACCB, this->ui);
        ui->txtR->value("Mode: A + B + C ");
        ui->btnLoad->callback(easyLoadCB, this->ui);
        ui->btnSave->callback(easySaveCB, this->ui);
    }

    void easySwitched(){
        easyChangedCB(nullptr,this->ui);
    }

    static void easyLoadCB(Fl_Widget* w, void* ptr)
    {
        UIMain* ui = (UIMain*)ptr;
        Fl_Menu_Button* btn = (Fl_Menu_Button*)w;
        int v = btn->value();
        Fl_Text_Buffer* tbf = nullptr;
        switch (v) {
        case 0:
            tbf = &ui->tbfA;
            break;
        case 1:
            tbf = &ui->tbfB;
            break;
        case 2:
            tbf = &ui->tbfC;
            break;
        default:
            return;
        }
        char* filename = fl_file_chooser("Load file", "*", "");
        if (!filename) // Cancelled
            return;
        int ret = tbf->loadfile(filename);
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

    static void easySaveCB(Fl_Widget* w, void* ptr)
    {
        UIMain* ui = (UIMain*)ptr;
        Fl_Text_Buffer& buf = ui->tbfOut;
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

    static char getOp(int in)
    {
        switch (in) {
        case 0:
            return '+';
        case 1:
            return '-';
        case 2:
            return '*';
        case 3:
            return '/';
        default:
            return 0;
        }
    }

    static void easyChangedCB(Fl_Widget* w, void* ptr)
    {
        UIMain* ui = (UIMain*)ptr;
        char tmp[25] = { 0 };
        char ab = getOp(ui->optAB->value());
        char bc = getOp(ui->optBC->value());
        sprintf(tmp, "Mode: A %c B %c C ", ab, bc);
        ui->txtR->value(tmp);
    }

    static void easyCalcCB(Fl_Widget*, void* ptr)
    {
        UIMain* ui = (UIMain*)ptr;
        // Lock the elements
        ui->tab0->deactivate();
        ui->txtStatus->value("Preparing operand...");
        ui->busy = true;
        Fl::wait();
        char *ta = ui->tbfA.text(), *tb = ui->tbfB.text(), *tc = ui->tbfC.text();
        ui->btnStop->activate();
        ui->txtStatus->value("Calculating...");
        stopCalc = false;
        uint8_t mode = (ui->optAB->value() << 4) + ui->optBC->value();
        auto x = std::async(std::launch::async, easyCalculate, ta, tb, tc, mode);
        while (x.wait_for(std::chrono::milliseconds(20)) != std::future_status::ready) {
            Fl::wait();
        }
        delete ta; // Release memory allocated in tbf::text()
        delete tb;
        delete tc;
        ui->tbfOut.text(x.get().c_str());
        ui->txtStatus->value("Ready.");
        ui->busy = false;
        stopCalc = false;
        ui->btnStop->deactivate();
        ui->tab0->activate();
    }

    static void easyACCB(Fl_Widget*, void* ptr)
    {
        UIMain* ui = (UIMain*)ptr;
        ui->tbfA.text("");
        ui->tbfB.text("");
        ui->tbfC.text("");
        ui->tbfOut.text("");
    }
};

#endif