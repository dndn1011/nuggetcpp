#pragma once
#include "identifier.h"

namespace nugget {
	using namespace identifier;
	namespace ui::container {
		void Create(IDType id);
		void ManageGeometryChildren(IDType id);
		void ManageGeometrySelf();
	}
	namespace ui_imp::container {
		void DrawAll();
	}
}