// gui.h
#ifndef GUI_H
#define GUI_H

#include <Windows.h>
#include "config.h"

class App {
public:
    App();
    int Run(HINSTANCE hInstance);
private:
    Config cfg;
    bool active;
    HWND hwnd;
    HFONT font;
    HWND dayCheckbox[7];
    void Load();
    void Save();
    void CreateUI();
    void UpdateButtons();
    void UpdateList();
    void UpdateDayCheckboxes();
    void TimerCheck();
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

#endif // GUI_H