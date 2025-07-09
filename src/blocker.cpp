#include "blocker.h"
#include <windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <string>

static std::string Narrow(const std::wstring& w){
    return std::string(w.begin(), w.end());
}

static std::wstring HostsPath(){
    wchar_t sys[MAX_PATH];
    GetSystemDirectoryW(sys, MAX_PATH);
    return std::wstring(sys) + L"\\drivers\\etc\\hosts";
}

static bool RuleActive(const Rule& r){
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    if(!lt) return false;
    int wd = lt->tm_wday ? lt->tm_wday : 7;
    int min = lt->tm_hour * 60 + lt->tm_min;
    if(std::find(r.days.begin(), r.days.end(), wd) == r.days.end()) return false;
    return min >= r.start && min < r.end;
}

void ApplyRules(const Config& cfg, bool active){
    std::wstring wpath = HostsPath();
    std::string path = Narrow(wpath);
    std::wofstream f(path.c_str(), std::ios::app);
    if(!f.is_open()) return;
    for(auto& r : cfg.rules)
        if(r.site && active)
            f << L"127.0.0.1 " << std::wstring(r.target.begin(), r.target.end()) << L"\n";
}

void RemoveSites(const Config& cfg){
    std::wstring wpath = HostsPath();
    std::string path = Narrow(wpath);
    std::wifstream in(path.c_str());
    if(!in.is_open()) return;
    std::wstring data((std::istreambuf_iterator<wchar_t>(in)), {});
    in.close();
    for(auto& r : cfg.rules)
        if(r.site){
            std::wstring s = L"127.0.0.1 " + std::wstring(r.target.begin(), r.target.end()) + L"\n";
            size_t pos;
            while((pos = data.find(s)) != std::wstring::npos)
                data.erase(pos, s.size());
        }
    std::wofstream out(path.c_str());
    out << data;
}

static bool iequals(const std::string& a, const std::string& b){
    return a.size() == b.size() &&
        std::equal(a.begin(), a.end(), b.begin(), [](char x, char y){
            return std::tolower((unsigned char)x) == std::tolower((unsigned char)y);
        });
}

void KillProcess(const std::string& name){
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(snap == INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if(Process32First(snap, &pe)){
        do{
            std::wstring exe = pe.szExeFile;
            std::string e = Narrow(exe);
            if(iequals(e, name)){
                HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if(h){
                    TerminateProcess(h, 1);
                    CloseHandle(h);
                }
            }
        } while(Process32Next(snap, &pe));
    }
    CloseHandle(snap);
}

void CheckRules(const Config& cfg){
    for(auto& r : cfg.rules)
        if(!r.site && RuleActive(r))
            KillProcess(r.target);
}
