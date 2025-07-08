#ifndef GUI_H
#define GUI_H
#include "config.h"
#include <Windows.h>
class App{
public:
    App();
    int Run(HINSTANCE h);
private:
    Config cfg;
    bool active;
    HWND hwnd;
    HFONT font;
    NOTIFYICONDATA nid{};
    void Load();
    void Save();
    void CreateUI();
    void UpdateButtons();
    void UpdateList();
    void TimerCheck();
    static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
};
#endif
