//
//  value.cpp
//  Krystal
//
//  Created by Arthur Langereis on 2013/6/10.
//
//

#include <iostream>
#include <stdexcept>
#include "value.h"

namespace krystal {

	value::value(value_type type) : type_{type} {
		switch(type_) {
			case value_type::String:
				new (&str) decltype(str){};
				break;
			case value_type::Array:
				new (&arr) decltype(arr){};
				break;
			case value_type::Object:
				new (&obj) decltype(obj){};
				break;
			default:
				num = 0.0;
				break;
		}
	}

	value::value(value&& rhs) : type_{rhs.type_ } {
		switch(type_) {
			case value_type::String:
				new (&str) decltype(str){std::move(rhs.str)};
				break;
			case value_type::Array:
				new (&arr) decltype(arr){std::move(rhs.arr)};
				break;
			case value_type::Object:
				new (&obj) decltype(obj){std::move(rhs.obj)};
				break;
			case value_type::Number:
				num = rhs.num;
				break;
			default:
				break;
		}
		
		rhs.type_ = value_type::Null;
	}
	
	value::~value() {
		switch(type_) {
			case value_type::String:
				str.~basic_string();
				break;
			case value_type::Array:
				arr.~vector();
				break;
			case value_type::Object:
				obj.~unordered_map();
				break;
			default:
				break;
		}
	}
	
	value::value(const std::string& sval)
	: type_{value_type::String}
	{
		new (&str) decltype(str){ sval };
	}

	size_t value::size() const {
		if (is_object())
			return obj.size();
		if (is_array())
			return arr.size();
		return 1;
	}

	bool value::contains(const std::string& key) const {
		if (! is_object())
			throw std::runtime_error("Trying to check for a key in a non-object value.");
		
		return obj.find(key) != obj.cend();
	}
	
	void value::insert(std::string key, value val) {
		if (! is_object())
			throw std::runtime_error("Trying to insert a keyval into a non-object value.");

		obj.emplace(key, std::move(val));
	}

	void value::push_back(value val) {
		if (! is_array())
			throw std::runtime_error("Trying to push_back a value into a non-array value.");
	
		arr.push_back(std::move(val));
	}
	
	const value& value::operator[](const std::string& key) const {
		if (! is_object())
			throw std::runtime_error("Trying to retrieve a sub-value by key from a non-object value.");
		
		return obj.at(key);
	}

	const value& value::operator[](const size_t index) const {
		if (! is_array())
			throw std::runtime_error("Trying to retrieve a sub-value by index from a non-array value.");

		return arr.at(index);
	}

	
	void value::debugPrint(std::ostream& os) const {
		switch(type_) {
			case value_type::String:
				os << '"' << str << '"';
				break;
			case value_type::Number:
				os << num;
				break;
			case value_type::Object:
				os << "Object[" << obj.size() << "]";
				break;
			case value_type::Array:
				os << "Array[" << arr.size() << "]";
				break;
			case value_type::True:
				os << "true";
				break;
			case value_type::False:
				os << "false";
				break;
			case value_type::Null:
				os << "null";
				break;
		}
	}

	std::ostream& operator<<(std::ostream& os, const value& t) {
		t.debugPrint(os);
		return os;
	}

}
