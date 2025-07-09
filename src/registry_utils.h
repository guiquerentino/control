#ifndef REGISTRY_UTILS_H
#define REGISTRY_UTILS_H

#include <string>

bool ReadPasswordHash(std::string& hash);
bool WritePasswordHash(const std::string& hash);

#endif // REGISTRY_UTILS_H
