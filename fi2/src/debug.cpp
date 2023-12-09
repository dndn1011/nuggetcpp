#include <windows.h>
#include <iostream>
#include "debug.h"

namespace nugget::debug {
	void OutputToDebuggerAndConsole(const char* str) {
		OutputDebugStringA(str);
		std::cout << str;
	}
	void OutputToDebuggerAndConsole(const std::string &str) {
		OutputDebugStringA(str.c_str());
		std::cout << str;
	}
	void Break() {
		output("-----> Program broken\n");
		if (::IsDebuggerPresent()) {
			::__debugbreak();
		} else {
			exit(-1);
		}
	}
}