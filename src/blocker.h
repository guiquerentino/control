#ifndef BLOCKER_H
#define BLOCKER_H
#include "config.h"
#include <Windows.h>
void ApplyRules(const Config& cfg,bool active);
void RemoveSites(const Config& cfg);
void CheckRules(const Config& cfg);
#endif
