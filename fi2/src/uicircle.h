#pragma once
#include "identifier.h"
#include "uientities.h"

namespace nugget {
	using namespace identifier;
	namespace ui::circle {
		void Create(IDType id);
		void ManageGeometrySelf(IDType id);
	}
	namespace ui_imp::circle {
		void DrawAll();
	}
}