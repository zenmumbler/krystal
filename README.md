krystal
=======

A C++11 JSON data reader with a simple but powerful API.<br>
By Arthur Langereis [(@zenmumbler)](http://twitter.com/zenmumbler/)

Design
------

- light-weight, clean codebase
	- a few classes and functions contained in a single `krystal` namespace
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
- UTF-8 only files and strings, http://utf8everywhere.org/
- number values are doubles, limiting exact int values to +/- 2^53
- SAX and DOM style access

Examples
--------

Convenient parse calls generate a document in a single call.

	// from a stream
	auto file = std::ifstream{"game_data.json"};
	auto doc = krystal::parseStream(file);
	
	// or from a string
	std::string json { "...json data..." };
	auto doc = krystal::parseString(json);

	// you will always get a valid Value object
	// failed parse will yield a Null value, otherwise an Array or Object
	if (doc.isContainer())
		...; // succesfully parsed

Use subscripting for array and object values.

	auto delay = doc["levels"][0]["zombie spawn delay"].number();
	auto name = doc["levels"][0]["level name"].string();

Iterate over arrays or objects with normal range for syntax. Each loop yields a pair of `krystal::Value`s.
For arrays, `first` is a number value with the index, for objects, `first` is the key string value.

	for (auto kv : doc["levels"]) {
		auto levelIndex = kv.first.numberAs<int>();
		auto& levelData = kv.second;

		...
	}

Usage
-----

krystal is a header-only library. Add the krystal directory to your include path and `#include`
the `krystal.hpp` umbrella header in your sources.


Status
------

The parser passes all of JSON.org's jsonchecker tests and is fully functional for UTF-8 files and data.
Currently, only parsing is supported, a JSON writer is in progress.

Tested on Clang 3.2, 3.3 and 3.4 compilers with the libc++ standard library.
Does not compile using GCC 4.8.1, GCC 4.9 should work but I have not verified that yet.
