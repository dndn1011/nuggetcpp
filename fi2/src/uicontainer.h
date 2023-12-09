#pragma once
#include "identifier.h"

namespace nugget {
	using namespace identifier;
	namespace ui::container {
		void Create(IDType id);
		void ManageGeometry(IDType id);
	}
	namespace ui_imp::container {
		void DrawAll();
	}
}