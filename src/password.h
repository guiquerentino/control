#ifndef PASSWORD_H
#define PASSWORD_H
#include <string>
std::string HashPassword(const std::string& p);
bool VerifyPassword(const std::string& hash,const std::string& input);
#endif
