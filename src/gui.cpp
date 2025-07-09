// gui.cpp
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0400

#include "gui.h"
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <thread>
#include "blocker.h"
#include "password.h"
#include "resource.h"
#include "registry_utils.h"

#pragma comment(lib, "wbemuuid.lib")

static std::string Narrow(const wchar_t* w){
    std::string s; while(*w) s.push_back(static_cast<char>(*w++)); return s;
}

static INT_PTR CALLBACK PasswordDlgProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK ChangePassDlgProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK SetPassDlgProc(HWND, UINT, WPARAM, LPARAM);

static void RealTimeMonitor(Config& cfg){
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IWbemLocator* locator = nullptr;
    CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&locator);
    IWbemServices* svc = nullptr;
    locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &svc);
    svc->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t(
        "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'"
    ), WBEM_FLAG_SEND_STATUS, nullptr, nullptr);
    IUnsecuredApartment* app = nullptr;
    CoCreateInstance(CLSID_UnsecuredApartment, nullptr, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**)&app);
    IWbemObjectSink* sink = nullptr;
    class EventSink : public IWbemObjectSink {
        long m_ref;
        Config& cfg;
    public:
        EventSink(Config& c): m_ref(1), cfg(c){}
        STDMETHODIMP QueryInterface(REFIID riid, void** ppv){
            *ppv = this; AddRef(); return S_OK;
        }
        STDMETHODIMP_(ULONG) AddRef(){ return InterlockedIncrement(&m_ref); }
        STDMETHODIMP_(ULONG) Release(){
            ULONG u = InterlockedDecrement(&m_ref);
            if(!u) delete this;
            return u;
        }
        STDMETHODIMP Indicate(LONG, IWbemClassObject** objs){
            for(int i=0; objs[i]; ++i){
                VARIANT v; objs[i]->Get(L"TargetInstance", 0, &v, nullptr, nullptr);
                IWbemClassObject* inst = (IWbemClassObject*)v.punkVal;
                VARIANT name; inst->Get(L"Name", 0, &name, nullptr, nullptr);
                std::string proc = Narrow(name.bstrVal);
                for(auto& r : cfg.rules){
                    if(!r.site && _stricmp(proc.c_str(), r.target.c_str())==0){
                        SYSTEMTIME st; GetLocalTime(&st);
                        int wday = st.wDayOfWeek ? st.wDayOfWeek : 7;
                        if(std::find(r.days.begin(), r.days.end(), wday) != r.days.end()){
                            KillProcess(proc);
                        }
                    }
                }
                VariantClear(&name);
                VariantClear(&v);
            }
            return WBEM_S_NO_ERROR;
        }
        STDMETHODIMP SetStatus(LONG, HRESULT, BSTR, IWbemClassObject*){ return S_OK; }
    } *es = new EventSink(cfg);
    app->CreateObjectStub(es, &sink);
    svc->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t(
        "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'"
    ), WBEM_FLAG_SEND_STATUS, nullptr, sink);
    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0));
}

App::App(): active(true), hwnd(nullptr), font(nullptr) {}

void App::Load(){
    std::string stored;
    if(!ReadPasswordHash(stored)){
        wchar_t buf[512]{};
        if(DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_SET_PASSWORD), nullptr, SetPassDlgProc, reinterpret_cast<LPARAM>(buf))){
            if(wcscmp(buf, buf+256)==0 && buf[0]){
                cfg.password = HashPassword(Narrow(buf));
                WritePasswordHash(cfg.password);
            }
        }
    } else cfg.password = stored;
    wchar_t p[MAX_PATH]{};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, p);
    LoadConfig(std::wstring(p) + L"\\control.ini", cfg);
}

void App::Save(){
    wchar_t p[MAX_PATH]{};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, p);
    SaveConfig(std::wstring(p) + L"\\control.ini", cfg);
}

void App::CreateUI(){
    hwnd = CreateWindowExW(
        0, L"ControlWnd", L"Controle de Aplicativos",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 460,
        nullptr, nullptr, GetModuleHandleW(nullptr), this
    );
    font = CreateFontW(
        20,0,0,0,FW_MEDIUM,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI"
    );
    const wchar_t* dias[7] = {L"Seg",L"Ter",L"Qua",L"Qui",L"Sex",L"Sáb",L"Dom"};
    for(int i=0;i<7;++i){
        dayCheckbox[i] = CreateWindowW(
            L"BUTTON", dias[i],
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            20+i*70,20,60,20,
            hwnd, reinterpret_cast<HMENU>(IDC_DAY_1 + i),
            GetModuleHandleW(nullptr), nullptr
        );
        SendMessageW(dayCheckbox[i], WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    }
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
                    WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL,
                    20,50,480,230, hwnd,
                    reinterpret_cast<HMENU>(IDC_LIST_RULES),
                    GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"STATIC", L"", WS_CHILD|WS_VISIBLE|SS_LEFT,
                  20,290,480,20, hwnd,
                  reinterpret_cast<HMENU>(IDC_STATIC_STATUS),
                  GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"BUTTON", L"Pausar", WS_CHILD|WS_VISIBLE,
                  20,320,120,35, hwnd,
                  reinterpret_cast<HMENU>(IDC_BTN_PAUSE),
                  GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"BUTTON", L"Retomar", WS_CHILD|WS_VISIBLE,
                  150,320,120,35, hwnd,
                  reinterpret_cast<HMENU>(IDC_BTN_RESUME),
                  GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"BUTTON", L"Alterar Senha", WS_CHILD|WS_VISIBLE,
                  280,320,180,35, hwnd,
                  reinterpret_cast<HMENU>(IDC_BTN_CHANGE_PASS),
                  GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"BUTTON", L"Adicionar Aplicativo", WS_CHILD|WS_VISIBLE,
                  20,365,180,35, hwnd,
                  reinterpret_cast<HMENU>(IDC_BTN_ADD_APP),
                  GetModuleHandleW(nullptr), nullptr);
    CreateWindowW(L"STATIC", L"Desenvolvido por Guilherme Querentino",
                  WS_CHILD|WS_VISIBLE|SS_CENTER,
                  150,410,220,20, hwnd,
                  reinterpret_cast<HMENU>(IDC_STATIC_INFO),
                  GetModuleHandleW(nullptr), nullptr);
    int ids[] = {IDC_LIST_RULES,IDC_STATIC_STATUS,IDC_BTN_PAUSE,IDC_BTN_RESUME,IDC_BTN_CHANGE_PASS,IDC_BTN_ADD_APP,IDC_STATIC_INFO};
    for(int id:ids) SendMessageW(GetDlgItem(hwnd,id),WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateButtons();
    UpdateList();
}

void App::UpdateButtons(){
    EnableWindow(GetDlgItem(hwnd, IDC_BTN_PAUSE), active);
    EnableWindow(GetDlgItem(hwnd, IDC_BTN_RESUME), !active);
}

void App::UpdateList(){
    HWND list = GetDlgItem(hwnd, IDC_LIST_RULES);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for(auto& r: cfg.rules){
        std::wstring t(r.target.begin(), r.target.end());
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
    }
}

void App::TimerCheck(){}

LRESULT CALLBACK App::WndProc(HWND h, UINT msg, WPARAM w, LPARAM l){
    App* app;
    if(msg==WM_NCCREATE){
        app = static_cast<App*>(((CREATESTRUCTW*)l)->lpCreateParams);
        SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else app = reinterpret_cast<App*>(GetWindowLongPtrW(h, GWLP_USERDATA));
    if(!app) return DefWindowProcW(h,msg,w,l);
    switch(msg){
    case WM_COMMAND:{
        int id = LOWORD(w);
        if(id>=IDC_DAY_1 && id<=IDC_DAY_1+6){
            int day = id - IDC_DAY_1 + 1;
            int sel = SendMessageW(GetDlgItem(app->hwnd, IDC_LIST_RULES), LB_GETCURSEL, 0, 0);
            if(sel>=0 && sel < (int)app->cfg.rules.size()){
                auto& days = app->cfg.rules[sel].days;
                if(SendMessageW(app->dayCheckbox[day-1], BM_GETCHECK,0,0)==BST_CHECKED){
                    if(std::find(days.begin(), days.end(), day)==days.end()) days.push_back(day);
                } else days.erase(std::remove(days.begin(), days.end(), day), days.end());
                app->Save();
            }
        } else if(id==IDC_LIST_RULES && HIWORD(w)==LBN_SELCHANGE){
            app->UpdateList();
        } else if(id==IDC_BTN_PAUSE||id==IDC_BTN_RESUME){
            wchar_t buf[256]{};
            if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_PASSWORD_DIALOG),h,PasswordDlgProc,reinterpret_cast<LPARAM>(buf))){
                std::string s=Narrow(buf);
                if(VerifyPassword(app->cfg.password,s)){
                    app->active = (id==IDC_BTN_RESUME);
                    SetWindowTextW(GetDlgItem(app->hwnd,IDC_STATIC_STATUS),L"Senha correta!");
                    app->UpdateButtons();
                } else SetWindowTextW(GetDlgItem(app->hwnd,IDC_STATIC_STATUS),L"Senha incorreta!");
            }
        } else if(id==IDC_BTN_CHANGE_PASS){
            wchar_t buf[768]{};
            if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(IDD_CHANGE_PASSWORD),h,ChangePassDlgProc,reinterpret_cast<LPARAM>(buf))){
                std::string oldp=Narrow(buf), newp=Narrow(buf+256), conf=Narrow(buf+512);
                if(newp==conf && VerifyPassword(app->cfg.password,oldp)){
                    app->cfg.password=HashPassword(newp);
                    WritePasswordHash(app->cfg.password);
                    app->Save();
                    SetWindowTextW(GetDlgItem(app->hwnd,IDC_STATIC_STATUS),L"Senha alterada com sucesso!");
                } else SetWindowTextW(GetDlgItem(app->hwnd,IDC_STATIC_STATUS),L"Erro ao alterar senha!");
            }
        } else if(id==IDC_BTN_ADD_APP){
            wchar_t file[MAX_PATH]{};
            OPENFILENAMEW ofn{};
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=h;
            ofn.lpstrFilter=L"Executáveis\0*.exe\0Todos os arquivos\0*.*\0";
            ofn.lpstrFile=file;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrTitle=L"Selecionar Aplicativo";
            ofn.Flags=OFN_FILEMUSTEXIST;
            if(GetOpenFileNameW(&ofn)){
                std::wstring name(file);
                size_t p=name.find_last_of(L"\\/");
                if(p!=std::wstring::npos) name=name.substr(p+1);
                Rule r;
                r.target=Narrow(name.c_str());
                r.site=false;
                r.days.clear();
                for(int d=1;d<=7;++d) r.days.push_back(d);
                r.start=0; r.end=1440;
                app->cfg.rules.push_back(r);
                app->Save();
                app->UpdateList();
                SetWindowTextW(GetDlgItem(app->hwnd,IDC_STATIC_STATUS),L"Aplicativo adicionado!");
            }
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(h);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(h,msg,w,l);
    }
    return 0;
}

int App::Run(HINSTANCE hInstance){
    INITCOMMONCONTROLSEX icc{sizeof(icc),ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    Load();
    std::thread(RealTimeMonitor, std::ref(cfg)).detach();
    WNDCLASSW wc{};
    wc.lpfnWndProc=WndProc;
    wc.hInstance=hInstance;
    wc.lpszClassName=L"ControlWnd";
    RegisterClassW(&wc);
    CreateUI();
    MSG msg;
    while(GetMessageW(&msg,nullptr,0,0)){
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    Save();
    PostQuitMessage(0);
    return 0;
}
