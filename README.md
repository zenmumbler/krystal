krystal
=======

A C++11 JSON data reader with a simple but powerful API.<br>
By Arthur Langereis ([@zenmumbler](http://twitter.com/zenmumbler/))

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
	- header-only framework
	- simple API with clear rules
	- integrated with language constructs (e.g. range for loop)
	- clear error messages in parser for syntax errors and in exceptions when using values
	- speed and memory usage limited by your std lib, but second only to rapidjson right now using clang & libc++
- fully conformant to JSON spec
	- correctly parses entire jsonchecker test suite
- UTF-8 only files and strings, [http://utf8everywhere.org/]()
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

/!\ Does not compile against stdlibc++, because krystal exploits UB by using an incomplete type
as the value of `vector` and `unordered_map`, the last of which stdlibc++ does not support
(legitimately) See:
[http://stackoverflow.com/questions/13089388/how-to-have-an-unordered-map-where-the-value-type-is-the-class-its-in]()

I was not aware of this when I started work on krystal and am currently looking into how to remedy this,
likely by wrapping the values in a `unique_ptr` as those explicitly allow incomplete types.

Does work with Clang 3.2, 3.3 and 3.4 compilers with the libc++ standard library.
Will not work on current (May 2014) MSVC compilers, even the CTP versions, because MSVC is not fully C++11 conformant yet.

JSON output is not yet supported.
