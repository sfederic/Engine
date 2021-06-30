module;
#include <Windows.h>
import <string>;
export module VString;

export namespace VString
{
    //Stolen from: https://stackoverflow.com/questions/10737644/convert-const-char-to-wstring
    std::wstring stows(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
}