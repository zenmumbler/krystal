krystal
=======

A C++11 JSON data reader with a simple but powerful API.<br>
By Arthur Langereis [(@zenmumbler)](http://twitter.com/zenmumbler/)

Design
------

- light-weight, clean codebase
	- 7 classes and 1 ABC (interface) in a single namespace
	- no macros or other global namespace pollution
	- depends on, and _only_ on the C++ standard library
- C++11 only
	- uses new language and library features to keep design simple
	- move-only semantics for `value` instances to avoid costly copies
- focus on ease of use
	- simple API with clear rules
	- integrated with language constructs (e.g. range for loop)
	- clear error messages in parser for syntax errors and in exceptions when using values
	- speed and memory usage limited by your std lib, but second only to rapidjson right now using clang & libc++
- fully conformant to JSON spec
	- correctly parses entire jsonchecker test suite
- UTF-8 only files and strings for now
- number values are doubles, limiting exact int values to +/- 2^53
- SAX and DOM style access

Examples
--------

Default parse call generates document in a single call.

	auto file = std::ifstream{"game_data.json"};
	auto doc = krystal::parse_stream(file);

	if (doc.is_container())
		...; // succesfully parsed

Use subscripting for array and object values.

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

Add the krystal directory to your compiler's include path, add `krystal.cpp` to the list of files to
compile and `#include "krystal.hpp"` in files where you want to use krystal.


Status
------

The parser passes all of JSON.org's jsonchecker tests and is fully functional for UTF-8 files and data.
Value objects are production ready but I have some ideas to expand on their functionality, usage in 
real-world apps is now needed to progress the library.

Tested on Clang 3.2 and 3.3 compilers with the libc++ standard library.
Currently fails to compile on GCC 4.8.1. GCC wants all allocator typedefs present and then there are
some errors that are related to GCC doing weird things with the decltyped return types for the
parse_x functions.
