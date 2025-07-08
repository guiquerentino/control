#ifndef RULE_H
#define RULE_H
#include <string>
#include <vector>
struct Rule {
    std::string target;
    bool site;
    std::vector<int> days;
    int start;
    int end;
};
#endif
