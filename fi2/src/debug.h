#pragma once
#include <string>

namespace nugget::debug {
    void Break();
}

namespace nugget::debug {
    void OutputToDebuggerAndConsole(const char* cstr);
    void OutputToDebuggerAndConsole(const std::string& str);
}

#define outputAlways(...)     (nugget::debug::OutputToDebuggerAndConsole((std::format("{}({}): {}: ",__FILE__, __LINE__, __FUNCTION__) + std::format(__VA_ARGS__))))
#define testAlways(a,...)  ((a)?(void)0:(output(__VA_ARGS__)))

#if defined(NDEBUG)
#define output(...) /**/
#define check(a,...)  /**/
#define IDC(a) ("<unavailable>")
#else
#include <format>
#define IDC(a) (IDToString(a))
#define output(...)     (nugget::debug::OutputToDebuggerAndConsole((std::format("{}({}): {}: ",__FILE__, __LINE__, __FUNCTION__) + std::format(__VA_ARGS__))))
#define printid(a) output("@ID@ {}\n",IDC(a));

#define check(a,...)  ((a)?(void)0:(output(__VA_ARGS__),nugget::debug::Break()))

#endif

