#include "gui.h"
#include "blocker.h"
#include "password.h"
#include "resource.h"
#include <shlobj.h>
#include <commdlg.h>
#include <string>

static std::string Narrow(const wchar_t* w){std::string s;while(*w){s.push_back((char)*w++);}return s;}

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
static INT_PTR CALLBACK ChangePassDlgProc(HWND d,UINT m,WPARAM w,LPARAM l){
    auto* buf=reinterpret_cast<wchar_t*>(GetWindowLongPtrW(d,GWLP_USERDATA));
    switch(m){
        case WM_INITDIALOG:
            SetWindowLongPtrW(d,GWLP_USERDATA,l);
            return TRUE;
        case WM_COMMAND:
            if(LOWORD(w)==IDOK){
                if(buf){
                    GetDlgItemTextW(d,IDC_EDIT_OLD,buf,256);
                    GetDlgItemTextW(d,IDC_EDIT_NEW,buf+256,256);
                    GetDlgItemTextW(d,IDC_EDIT_CONFIRM,buf+512,256);
                }
                EndDialog(d,1);
            }
            if(LOWORD(w)==IDCANCEL) EndDialog(d,0);
            break;
    }
    return FALSE;
}
static INT_PTR CALLBACK SetPassDlgProc(HWND d,UINT m,WPARAM w,LPARAM l){
    auto* buf=reinterpret_cast<wchar_t*>(GetWindowLongPtrW(d,GWLP_USERDATA));
    switch(m){
        case WM_INITDIALOG:
            SetWindowLongPtrW(d,GWLP_USERDATA,l);
            return TRUE;
        case WM_COMMAND:
            if(LOWORD(w)==IDOK){
                if(buf){
                    GetDlgItemTextW(d,IDC_EDIT_SET_NEW,buf,256);
                    GetDlgItemTextW(d,IDC_EDIT_SET_CONFIRM,buf+256,256);
                }
                EndDialog(d,1);
            }
            if(LOWORD(w)==IDCANCEL) EndDialog(d,0);
            break;
    }
    return FALSE;
}

App::App(){active=true;hwnd=nullptr;font=nullptr;}
void App::Load(){wchar_t path[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,0,path);std::wstring p=std::wstring(path)+L"\\control.ini";LoadConfig(p,cfg);if(!cfg.password.empty()&&cfg.password.size()<8){cfg.password=HashPassword(cfg.password);Save();}if(cfg.password.empty()){wchar_t buf[512]{};if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_SET_PASSWORD),nullptr,SetPassDlgProc,(LPARAM)buf)){if(wcscmp(buf,buf+256)==0&&buf[0]){cfg.password=HashPassword(Narrow(buf));Save();}}}}
void App::Save(){wchar_t path[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,0,path);std::wstring p=std::wstring(path)+L"\\control.ini";SaveConfig(p,cfg);}
void App::CreateUI(){
    hwnd=CreateWindowExW(0,L"ControlWnd",L"Control",WS_OVERLAPPEDWINDOW,100,100,460,340,nullptr,nullptr,GetModuleHandleW(nullptr),this);
    font=CreateFontW(18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
    CreateWindowW(L"BUTTON",L"Pause",WS_CHILD|WS_VISIBLE,10,230,100,30,hwnd,(HMENU)1,GetModuleHandleW(nullptr),nullptr);
    CreateWindowW(L"BUTTON",L"Resume",WS_CHILD|WS_VISIBLE,120,230,100,30,hwnd,(HMENU)3,GetModuleHandleW(nullptr),nullptr);
    CreateWindowW(L"BUTTON",L"Change Password",WS_CHILD|WS_VISIBLE,230,230,150,30,hwnd,(HMENU)4,GetModuleHandleW(nullptr),nullptr);
    CreateWindowW(L"BUTTON",L"Add App",WS_CHILD|WS_VISIBLE,10,270,100,30,hwnd,(HMENU)5,GetModuleHandleW(nullptr),nullptr);
    CreateWindowW(L"LISTBOX",nullptr,WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL,10,10,410,200,hwnd,(HMENU)2,GetModuleHandleW(nullptr),nullptr);
    for(int id=1;id<=5;++id)SendMessageW(GetDlgItem(hwnd,id),WM_SETFONT,(WPARAM)font,TRUE);
    ShowWindow(hwnd,SW_SHOW);
    UpdateButtons();
}
void App::UpdateList(){HWND list=GetDlgItem(hwnd,2);SendMessageW(list,LB_RESETCONTENT,0,0);for(auto&r:cfg.rules){std::wstring t(r.target.begin(),r.target.end());SendMessageW(list,LB_ADDSTRING,0,(LPARAM)t.c_str());}}
void App::TimerCheck(){if(active){CheckRules(cfg);}}
void App::UpdateButtons(){EnableWindow(GetDlgItem(hwnd,1),active);EnableWindow(GetDlgItem(hwnd,3),!active);}
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
            if(LOWORD(w)==1||LOWORD(w)==3){
                wchar_t input[256]{};
                if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_PASSWORD_DIALOG),h,PasswordDlgProc,(LPARAM)input)){
                    std::string s=Narrow(input);
                    if(VerifyPassword(a->cfg.password,s)){
                        a->active=LOWORD(w)==3;
                        if(a->active){RemoveSites(a->cfg);}else{ApplyRules(a->cfg,true);} 
                        a->UpdateButtons();
                    }
                }
            }
            if(LOWORD(w)==4){
                wchar_t buf[768]{};
                if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_CHANGE_PASSWORD),h,ChangePassDlgProc,(LPARAM)buf)){
                    std::string oldp=Narrow(buf);
                    std::string newp=Narrow(buf+256);
                    std::string conf=Narrow(buf+512);
                    if(newp==conf && VerifyPassword(a->cfg.password,oldp)){
                        a->cfg.password=HashPassword(newp);
                        a->Save();
                    }
                }
            }
            if(LOWORD(w)==5){
                wchar_t file[MAX_PATH]{};
                OPENFILENAMEW ofn{};
                ofn.lStructSize=sizeof(ofn);
                ofn.hwndOwner=h;
                ofn.lpstrFilter=L"Executables\0*.exe\0All Files\0*.*\0";
                ofn.lpstrFile=file;
                ofn.nMaxFile=MAX_PATH;
                ofn.Flags=OFN_FILEMUSTEXIST;
                ofn.lpstrTitle=L"Select Application";
                if(GetOpenFileNameW(&ofn)){
                    std::wstring name=file;
                    size_t pos=name.find_last_of(L"\\/");
                    if(pos!=std::wstring::npos) name=name.substr(pos+1);
                    Rule r{};
                    r.target=std::string(name.begin(),name.end());
                    r.site=false;
                    for(int d=1;d<=7;++d) r.days.push_back(d);
                    r.start=0;
                    r.end=1440;
                    a->cfg.rules.push_back(r);
                    if(!a->active) ApplyRules(a->cfg,true);
                    a->Save();
                    a->UpdateList();
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
