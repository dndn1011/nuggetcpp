#pragma once
#include <string>
#include "debug.h"

namespace nugget::ui {
	struct Dimension {
		float f;
		enum class Units {
			none,
			pixel,
			percent,
			scale,
		} unit = Units::none;

		std::string ToString() const;

		bool operator==(const Dimension& other) const {
			return unit == other.unit && f == other.f;
		}

		Dimension() : f(0), unit(Units::none), originalValue{} {}
		Dimension(float v, Units u) : f(v), unit(u), originalValue{} {}
		Dimension(float val) : f(val), unit(Units::none), originalValue{} {}

		operator float() const {
			return f;
		}

	private:
		float originalValue;

	};
}