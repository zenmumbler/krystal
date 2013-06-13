Krystal
=======

It's C++11. It reads JSON. Why not.
In development.

Example
-------

	auto file = std::ifstream("somefile.json");
	auto doc = krystal::parse(file);

	std::cout << doc["key"][3]["sub_key"] << '\n';


Goals
-----

- primarily for myself to practice with variants, parsers, etc.
- clear and clean design and code base
- use C++11 standard library and language features
	- no reinventing of wheels wrt strings, maps, vectors, etc.
- no other dependencies for normal library use
	- will use my Inquisition library for tests
- initially
	- read-only data
