// krystal.cpp
// part of Krystal JSON Reader
// (c) 2013 by Arthur Langereis (@zenmumbler)

#include <iostream>
#include "reader.h"
#include "util.h"

namespace krystal {
	
	void document_builder::null_value() {
		std::cout << "null "<< '\n';
	}

	void document_builder::false_value() {
		std::cout << "false "<< '\n';
	}

	void document_builder::true_value() {
		std::cout << "true "<< '\n';
	}

	void document_builder::number_value(double num) {
		std::cout << "N: " << num << '\n';
	}

	void document_builder::string_value(const std::string& str) {
		std::cout << "S: " << str << '\n';
	}
	
	void document_builder::array_begin() {
		std::cout << "[\n";
	}

	void document_builder::array_end() {
		std::cout << "]\n";
	}
	
	void document_builder::object_begin() {
		std::cout << "{\n";
	}

	void document_builder::object_end() {
		std::cout << "}\n";
	}
	
	void document_builder::error(const std::string& msg, std::istream& is) {
		std::cout << "ERROR! " << msg << '\n';
	}
	
	std::shared_ptr<value> document_builder::rootValue() {
		return root_;
	}

	
	
	std::shared_ptr<value> parse(std::istream& is) {
		auto delegate = std::make_shared<document_builder>();
		reader r { std::dynamic_pointer_cast<reader_delegate>(delegate) };
		if (! r.parseDocument(is))
			return {};
		
		return delegate->rootValue();
	}
}
