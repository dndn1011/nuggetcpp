#pragma once

namespace nugget::debug {
    void Break();
}

#if defined(NDEBUG)
#define output(...) /**/
#define check(a,...)  /**/
#define IDC(a) ("<unavailable>")
#else
#include <format>
#define IDC(a) (IDToString(a))
namespace nugget::debug {
    void OutputToDebuggerAndConsole(const char* cstr);
    void OutputToDebuggerAndConsole(const std::string &str);
}
#define output(...)     (nugget::debug::OutputToDebuggerAndConsole((std::format("{}({}): {}: ",__FILE__, __LINE__, __FUNCTION__) + std::format(__VA_ARGS__))))
#define printid(a) output("@ID@ {}\n",IDC(a));

#define check(a,...)  ((a)?(void)0:(output(__VA_ARGS__),nugget::debug::Break()))

#endif

