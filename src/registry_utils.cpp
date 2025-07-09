#include "registry_utils.h"
#include <windows.h>

static constexpr LPCWSTR RegPath  = L"Software\\ControlApp";
static constexpr LPCWSTR RegValue = L"PasswordHash";

bool ReadPasswordHash(std::string& hash) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RegPath, 0, KEY_READ, &hKey)!=ERROR_SUCCESS) return false;
    DWORD type=0, size=0;
    if (RegQueryValueExW(hKey, RegValue, nullptr, &type, nullptr, &size)!=ERROR_SUCCESS || type!=REG_SZ) {
        RegCloseKey(hKey);
        return false;
    }
    wchar_t* buf = new wchar_t[size/sizeof(wchar_t)];
    if (RegQueryValueExW(hKey, RegValue, nullptr, nullptr, reinterpret_cast<BYTE*>(buf), &size)!=ERROR_SUCCESS) {
        delete[] buf;
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    std::wstring w(buf);
    delete[] buf;
    hash.assign(w.begin(), w.end());
    return true;
}

bool WritePasswordHash(const std::string& hash) {
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, RegPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey,nullptr)!=ERROR_SUCCESS) return false;
    std::wstring w(hash.begin(), hash.end());
    if (RegSetValueExW(hKey, RegValue, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(w.c_str()),
        static_cast<DWORD>((w.size()+1)*sizeof(wchar_t)))!=ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    return true;
}
