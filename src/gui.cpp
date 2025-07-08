#include "gui.h"
#include "blocker.h"
#include "password.h"
#include <shlobj.h>
#include <string>

static INT_PTR CALLBACK PasswordDlgProc(HWND d,UINT m,WPARAM w,LPARAM l){
    wchar_t* buf=reinterpret_cast<wchar_t*>(GetWindowLongPtrW(d,GWLP_USERDATA));
    switch(m){
        case WM_INITDIALOG:
            SetWindowLongPtrW(d,GWLP_USERDATA,l);
            return TRUE;
        case WM_COMMAND:
            if(LOWORD(w)==IDOK){
                if(buf) GetDlgItemTextW(d,101,buf,256);
                EndDialog(d,1);
            }
            if(LOWORD(w)==IDCANCEL) EndDialog(d,0);
            break;
    }
    return FALSE;
}
App::App(){active=true;hwnd=nullptr;}
void App::Load(){wchar_t path[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,0,path);std::wstring p=std::wstring(path)+L"\\control.ini";LoadConfig(p,cfg);}
void App::Save(){wchar_t path[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,0,path);std::wstring p=std::wstring(path)+L"\\control.ini";SaveConfig(p,cfg);} 
void App::CreateUI(){hwnd=CreateWindowExW(0,L"ControlWnd",L"Control",WS_OVERLAPPEDWINDOW,100,100,400,300,nullptr,nullptr,GetModuleHandleW(nullptr),this);CreateWindowW(L"BUTTON",L"Pause",WS_CHILD|WS_VISIBLE,10,230,80,25,hwnd,(HMENU)1,GetModuleHandleW(nullptr),nullptr);CreateWindowW(L"LISTBOX",nullptr,WS_CHILD|WS_VISIBLE|LBS_NOTIFY,10,10,360,200,hwnd,(HMENU)2,GetModuleHandleW(nullptr),nullptr);ShowWindow(hwnd,SW_SHOW);} 
void App::UpdateList(){HWND list=GetDlgItem(hwnd,2);SendMessageW(list,LB_RESETCONTENT,0,0);for(auto&r:cfg.rules){std::wstring t(r.target.begin(),r.target.end());SendMessageW(list,LB_ADDSTRING,0,(LPARAM)t.c_str());}} 
void App::TimerCheck(){if(active){CheckRules(cfg);}}
LRESULT CALLBACK App::WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
    App* a;
    if(m==WM_NCCREATE){
        CREATESTRUCTW* cs=(CREATESTRUCTW*)l;
        SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)cs->lpCreateParams);
    }
    a=(App*)GetWindowLongPtrW(h,GWLP_USERDATA);
    if(!a) return DefWindowProcW(h,m,w,l);
    switch(m){
        case WM_COMMAND:
            if(LOWORD(w)==1){
                wchar_t input[256]{};
                if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(100),h,PasswordDlgProc,(LPARAM)input)){
                    std::string s;
                    int len=wcslen(input);
                    for(int i=0;i<len;i++) s.push_back((char)input[i]);
                    if(VerifyPassword(a->cfg.password,s)){
                        a->active=!a->active;
                        if(a->active){RemoveSites(a->cfg);}else{ApplyRules(a->cfg,true);} 
                    }
                }
            }
            break;
        case WM_CLOSE:
            DestroyWindow(h);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_TIMER:
            a->TimerCheck();
            break;
        default:
            return DefWindowProcW(h,m,w,l);
    }
    return 0;
}
int App::Run(HINSTANCE h){Load();WNDCLASSW wc{};wc.lpfnWndProc=WndProc;wc.hInstance=h;wc.lpszClassName=L"ControlWnd";RegisterClassW(&wc);CreateUI();SetTimer(hwnd,1,60000,nullptr);UpdateList();MSG msg;while(GetMessageW(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageW(&msg);}Save();return 0;} 
