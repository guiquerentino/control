#include "password.h"

std::string HashPassword(const std::string& p){
    unsigned long h = 2166136261u;
    for (auto c : p) { h ^= (unsigned char)c; h *= 16777619; }
    return std::to_string(h);
}

bool VerifyPassword(const std::string& hash, const std::string& input){
    return hash == HashPassword(input);
}
