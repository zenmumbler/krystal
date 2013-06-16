//  krystal_test.cpp - part of krystal
//  (c) 2013 by Arthur Langereis (@zenmumbler)

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "krystal.hpp"

#include "Inquisition/Inquisition.h"

using namespace Inquisition;
using namespace krystal;

static std::vector<value_type> all_types() {
	return {
		value_type::Null, value_type::False, value_type::True,
		value_type::Number, value_type::String,
		value_type::Array, value_type::Object
	};
}

#include "test_value.hpp"
#include "test_reader.hpp"

int main() {
	test_value();
	test_reader();
	
	auto r = makeReport<SimpleTestReport>(std::ref(std::cout));
	runAll(r);
	
	return r->failures() + r->errors();
}
