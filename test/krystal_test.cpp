// krystal_test.cpp - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_map>

#include "krystal.hpp"
#include "Inquisition/Inquisition.h"

using namespace Inquisition;
using namespace krystal;

static std::vector<ValueKind> all_types() {
	return {
		ValueKind::Null, ValueKind::False, ValueKind::True,
		ValueKind::Number, ValueKind::String,
		ValueKind::Array, ValueKind::Object
	};
}


static std::string toString(const ValueKind kind) {
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


namespace krystal {
	// Inquisition compat shim until I convert it over to camelCase.
	std::string to_string(const ValueKind kind) {
		return toString(kind);
	}
}


static std::string readTextFile(const std::string & filePath) {
	std::ifstream file { filePath };
	if (! file.is_open()) {
		return "CANNOT FIND FILE " + filePath;
	}
	
	file.unsetf(std::ios::skipws);
	file.seekg(0, std::ios::end);
	auto fileSize = static_cast<size_t>(file.tellg());
	
	std::string contents;
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	
	contents.assign(buffer.begin(), buffer.end());
	return contents;
}




#include "test_value.hpp"
#include "test_reader.hpp"
#include "test_jsonchecker.hpp"
#include "test_performance.hpp"

int main() {
	test_value();
	test_reader();
	test_jsonchecker();
	test_performance();
	
	auto r = makeReport<SimpleTestReport>(std::ref(std::cout));
	runAll(r);
	
	return r->failures() + r->errors();
}
