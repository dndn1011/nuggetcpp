#pragma once
#include <string>
#include <functional>

namespace nugget::system::files {
	void Monitor(const std::string &path, std::function<void(const std::string &path)> func);
}
