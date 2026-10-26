#ifdef _WIN32
#include <windows.h>
#endif
#include <algorithm>
#include <codecvt>
#include <locale>
#include "tie_cxx_foundation.h"

namespace tie {

  T_STRING CTieObject::ToString() {
    return GetTieClassName() + _TSTR("@") + PRIMITIVE_TO_STR(reinterpret_cast<int64_t>(this));
  }

  T_STRING CTieObject::GetTieClassName() {
    return _TSTR("CTieObject");
  }

  std::string WStringToString(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(ws);
  }

  std::wstring StringToWString(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
  }

  std::unique_ptr<const char[]> ConvertWStringToChars(const std::wstring& ws) {
#ifdef _WIN32
    const int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(),
      static_cast<int>(ws.size()), nullptr, 0, nullptr, nullptr);
    std::unique_ptr<char[]> ptr = std::make_unique<char[]>(size_needed + 1);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), static_cast<int>(ws.size()),
      ptr.get(), size_needed, nullptr,nullptr);
    ptr[size_needed] = '\0';
    return ptr;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8_str = converter.to_bytes(ws.c_str());
    std::unique_ptr<char[]> ptr = std::make_unique<char[]>(utf8_str.size() + 1);
    std::copy(utf8_str.begin(), utf8_str.end(), ptr.get());
    ptr[utf8_str.size()] = '\0';
    return ptr;
#endif
  }
}
