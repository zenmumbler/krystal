// value.cpp - part of krystal
// (c) 2013 by Arthur Langereie

#include <iostream>
#include <stdexcept>
#include <utility>
#include <iterator>
#include "value.hpp"

namespace krystal {
	
	std::string to_string(const value_type type) {
		static std::unordered_map<int, std::string> vt2n {
			{ (int)value_type::Null,   "null" },
			{ (int)value_type::False,  "false" },
			{ (int)value_type::True,   "true" },
			{ (int)value_type::Number, "number" },
			{ (int)value_type::String, "string" },
			{ (int)value_type::Array,  "array" },
			{ (int)value_type::Object, "object" }
		};
		
		return vt2n[static_cast<int>(type)];
	}

}
