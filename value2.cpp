// krystal.cpp - part of krystal
// (c) 2013 by Arthur Langereie

#include "value2.hpp"

namespace krystal {

	// It is somehwat irksome that 1 cpp file is needed for only this function
	// but this is actually used in testing etc. so there it is.

	std::string to_string(const value_kind type) {
		static std::unordered_map<int, std::string> vt2n {
			{ (int)value_kind::Null,   "null" },
			{ (int)value_kind::False,  "false" },
			{ (int)value_kind::True,   "true" },
			{ (int)value_kind::Number, "number" },
			{ (int)value_kind::String, "string" },
			{ (int)value_kind::Array,  "array" },
			{ (int)value_kind::Object, "object" }
		};
		
		return vt2n[static_cast<int>(type)];
	}

}
