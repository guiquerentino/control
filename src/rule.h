#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>

struct Rule {
    bool site;
    std::string target;
    std::vector<int> days;
    int start;
    int end;
};

#endif // RULE_H
