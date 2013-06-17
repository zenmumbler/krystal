krystal
=======

A C++11 JSON data reader with a simple but powerful API.<br>
By Arthur Langereis [(@zenmumbler)](http://twitter.com/zenmumbler/)

Design
------

- light-weight, clean codebase
	- 3 classes and 1 ABC (interface) in a single namespace
	- no macros or other global namespace pollution
	- depends on, and _only_ on the C++ standard library
- C++11 only
	- uses new language and library features to keep design simple
	- move-only semantics for `value` instances to avoid costly copies
- focus on ease of use
	- simple API with clear rules
	- integrated with language constructs (e.g. range for loop)
	- clear error messages in parser for syntax errors and in exceptions when using values
	- speed and memory usage good, limited by your std lib
- fully conformant to JSON spec
	- correctly parses entire jsonchecker test suite
- UTF-8 only files and strings for now
- number values are doubles, limiting exact int values to +/- 2^53
- SAX and DOM style access

Examples
--------

Default parse call generates document in a single call.

	auto file = std::ifstream{"somefile.json"};
	auto doc = krystal::parse(file);

	if (doc.is_container())
		...; // succesfully parsed

Use subscripting for array and object values.

	auto doc = krystal::parse(...);

	auto delay = doc["levels"][0]["zombie spawn delay"].number();
	auto name = doc["levels"][0]["level name"].string();

Iterate over arrays or objects with normal range for syntax. Each loop yields a pair of `krystal::value`s.
For arrays, `first` is a number value with the index, for objects, `first` is the key string value.

	for (auto kv : doc["levels"]) {
		auto level_index = kv.first.number_as<int>();
		auto& level_data = kv.second;

		...
	}

Usage
-----

Add the 3 .cpp files to your project and `#include "krystal.hpp"` where needed and you're done.


Status
------

The parser passes all of JSON.org's jsonchecker tests and is fully functional for UTF-8 files and data.
Value objects are production ready but I have some ideas to expand on their functionality, usage in 
real-world apps is now needed to progress the library.

Tested on Clang 3.2 and Clang 3.3 compilers with the libc++ standard library.
