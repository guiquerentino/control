#include "config.h"
#include <fstream>
#include <sstream>

static std::string Narrow(const std::wstring& w){return std::string(w.begin(),w.end());}
bool LoadConfig(const std::wstring& path, Config& cfg) {
    std::wifstream f(Narrow(path).c_str());
    if (!f.is_open()) return false;
    std::wstring line;
    Rule r;
    bool inRule=false;
    while (std::getline(f,line)) {
        if (line.empty()) continue;
        if (line[0]=='[' && line.back()==']') {
            if (inRule) cfg.rules.push_back(r);
            r=Rule();
            inRule=true;
            continue;
        }
        auto pos=line.find(L'=');
        if (pos==std::wstring::npos) continue;
        std::wstring key=line.substr(0,pos);
        std::wstring val=line.substr(pos+1);
        if (key==L"password") cfg.password=std::string(val.begin(),val.end());
        if (!inRule) continue;
        if (key==L"type") r.site=val==L"site";
        if (key==L"target") r.target=std::string(val.begin(),val.end());
        if (key==L"days") {
            std::wistringstream ss(val);
            std::wstring n;
            while (std::getline(ss,n,L',')) r.days.push_back(std::stoi(n));
        }
        if (key==L"start") r.start=std::stoi(val);
        if (key==L"end") r.end=std::stoi(val);
    }
    if (inRule) cfg.rules.push_back(r);
    return true;
}
bool SaveConfig(const std::wstring& path, const Config& cfg) {
    std::wofstream f(Narrow(path).c_str());
    if (!f.is_open()) return false;
    f<<L"password="<<std::wstring(cfg.password.begin(),cfg.password.end())<<L"\n";
    for (const auto& r:cfg.rules) {
        f<<L"[rule]\n";
        f<<L"type="<<(r.site?L"site":L"app")<<L"\n";
        f<<L"target="<<std::wstring(r.target.begin(),r.target.end())<<L"\n";
        f<<L"days="; for(size_t i=0;i<r.days.size();++i){if(i)f<<L",";f<<r.days[i];} f<<L"\n";
        f<<L"start="<<r.start<<L"\n";
        f<<L"end="<<r.end<<L"\n";
    }
    return true;
}
