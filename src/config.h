#ifndef CONFIG_H
#define CONFIG_H

#include "rule.h"
#include <string>
#include <vector>
#include <map>

struct Config {
    std::string password;
    std::vector<Rule> rules;
    std::map<int,int> dailyLimits;
};

bool LoadConfig(const std::wstring& path, Config& cfg);
bool SaveConfig(const std::wstring& path, const Config& cfg);

#endif // CONFIG_H
