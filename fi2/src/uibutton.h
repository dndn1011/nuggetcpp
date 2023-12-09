#pragma once
#include "identifier.h"

namespace nugget {
	using namespace identifier;
	namespace ui::button {
		void Create(IDType id);
		void ManageGeometry();
	}
	namespace ui_imp::button {
		void DrawAll();
	}
}