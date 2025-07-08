#ifndef CONFIG_H
#define CONFIG_H
#include "rule.h"
#include <string>
#include <vector>
struct Config {
    std::string password;
    std::vector<Rule> rules;
};
bool LoadConfig(const std::wstring& path, Config& cfg);
bool SaveConfig(const std::wstring& path, const Config& cfg);
#endif
