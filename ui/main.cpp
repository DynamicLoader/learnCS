
#include "./gui.h"
#include "./ui.easy.hpp"
#include "./ui.adv.hpp"

UIMain ui;
UI_Easy ui_easy(&ui);
UI_ADV ui_adv(&ui);
flag_t stopCalc(false);

void mainExit(Fl_Widget* w, void* ptr)
{
    UIMain* ui = (UIMain*)ptr;
    Fl_Double_Window* wd = (Fl_Double_Window*)w;
    if (ui->busy) {
        switch (fl_choice("Still calculating! Exit anyway?", nullptr, "No", "Yes")) {
        case 2:
            exit(0);
        default:
            return;
        }
    }
    exit(0);
}

void btnStopCB(Fl_Widget*)
{
    stopCalc = true;
}

void switchCB(Fl_Widget*){
    auto x = ui.tab0->value();
    if(x == ui.tab0_easy){
        ui_easy.easySwitched();
    }else if (x == ui.tab0_adv){
        ui_adv.advSwitched();
    }else{
        ui.txtR->value("");
    }
}

int main()
{
    fl_message_title_default("HRCore Calculator");
    Fl::background(245, 245, 245);
    // Fl::scheme("plastic");
    Fl::scheme("GTK+");

    ui.tab0->callback(switchCB);
    ui.tab0->when(FL_WHEN_CHANGED);
    ui.btnStop->callback(btnStopCB);
    ui.window_main->callback(mainExit, &ui);
    ui.window_main->show();

    return Fl::run();
}