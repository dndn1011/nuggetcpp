#pragma once

namespace nugget::gl {
	int OpenWindow();
	void Update();
	void MainLoop(const std::function<void()>& updateCallback);
}
