// krystal.cpp - master implementation file
// (c) 2013-4 by Arthur Langereis (@zenmumbler)

#include "krystal.hpp"

#include <string>
#include <unordered_map>

namespace krystal {


// ValueKind toString method
std::string toString(const ValueKind kind) {
	static std::unordered_map<int, std::string> vt2n {
		{ (int)ValueKind::Null,   "null" },
		{ (int)ValueKind::False,  "false" },
		{ (int)ValueKind::True,   "true" },
		{ (int)ValueKind::Number, "number" },
		{ (int)ValueKind::String, "string" },
		{ (int)ValueKind::Array,  "array" },
		{ (int)ValueKind::Object, "object" }
	};
	
	return vt2n[static_cast<int>(kind)];
}

// static tokens used by Reader class
std::string Reader::nullToken {"null"}, Reader::trueToken{"true"}, Reader::falseToken{"false"};


} // ns krystal
