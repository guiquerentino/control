#include "blocker.h"
#include <TlHelp32.h>
#include <fstream>
#include <ctime>
static std::wstring HostsPath(){wchar_t sys[MAX_PATH];GetSystemDirectoryW(sys,MAX_PATH);return std::wstring(sys)+L"\\drivers\\etc\\hosts";}
static bool RuleActive(const Rule& r){time_t t=time(nullptr);tm lt;localtime_s(&lt,&t);int wd=lt.tm_wday;wd=wd?wd:7;int min=lt.tm_hour*60+lt.tm_min;if(std::find(r.days.begin(),r.days.end(),wd)==r.days.end())return false;return min>=r.start&&min<r.end;}
void ApplyRules(const Config& cfg,bool active){std::wofstream f(HostsPath(),std::ios::app);if(!f.is_open())return;for(auto& r:cfg.rules)if(r.site)if(active)f<<L"127.0.0.1 "<<std::wstring(r.target.begin(),r.target.end())<<L"\n";}
void RemoveSites(const Config& cfg){std::wstring path=HostsPath();std::wifstream in(path);if(!in.is_open())return;std::wstring data((std::istreambuf_iterator<wchar_t>(in)),{});in.close();for(auto&r:cfg.rules)if(r.site){std::wstring s=L"127.0.0.1 "+std::wstring(r.target.begin(),r.target.end())+L"\n";size_t pos;while((pos=data.find(s))!=std::wstring::npos)data.erase(pos,s.size());}
std::wofstream out(path);out<<data;}
void KillProcess(const std::string& name){HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snap==INVALID_HANDLE_VALUE)return;PROCESSENTRY32 pe{};pe.dwSize=sizeof(pe);if(Process32First(snap,&pe))do{std::wstring exe=pe.szExeFile;std::string e(exe.begin(),exe.end());if(_stricmp(e.c_str(),name.c_str())==0){HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,pe.th32ProcessID);if(h){TerminateProcess(h,1);CloseHandle(h);}}}while(Process32Next(snap,&pe));CloseHandle(snap);} 
void CheckRules(const Config& cfg){for(auto&r:cfg.rules)if(!r.site&&RuleActive(r))KillProcess(r.target);}
