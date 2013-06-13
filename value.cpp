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
				new (&str_) decltype(str_){};
				break;
			case value_type::Array:
				new (&arr_) decltype(arr_){};
				break;
			case value_type::Object:
				new (&obj_) decltype(obj_){};
				break;
			default:
				num_ = 0.0;
				break;
		}
	}

	value::value(value&& rhs) : type_{rhs.type_ } {
		switch(type_) {
			case value_type::String:
				new (&str_) decltype(str_){std::move(rhs.str_)};
				break;
			case value_type::Array:
				new (&arr_) decltype(arr_){std::move(rhs.arr_)};
				break;
			case value_type::Object:
				new (&obj_) decltype(obj_){std::move(rhs.obj_)};
				break;
			case value_type::Number:
				num_ = rhs.num_;
				break;
			default:
				break;
		}

		switch(rhs.type_) {
			case value_type::String:
				rhs.str_.~basic_string();
				break;
			case value_type::Array:
				rhs.arr_.~vector();
				break;
			case value_type::Object:
				rhs.obj_.~unordered_map();
				break;
			default:
				break;
		}

		rhs.type_ = value_type::Null;
	}
	
	value::~value() {
		switch(type_) {
			case value_type::String:
				str_.~basic_string();
				break;
			case value_type::Array:
				arr_.~vector();
				break;
			case value_type::Object:
				obj_.~unordered_map();
				break;
			default:
				break;
		}
	}
	
	value::value(const std::string& sval)
	: type_{value_type::String}
	{
		new (&str_) decltype(str_){ sval };
	}

	size_t value::size() const {
		if (is_object())
			return obj_.size();
		if (is_array())
			return arr_.size();
		return 1;
	}

	bool value::contains(const std::string& key) const {
		if (! is_object())
			throw std::runtime_error("Trying to check for a key in a non-object value.");
		
		return obj_.find(key) != obj_.cend();
	}
	
	void value::insert(std::string key, value&& val) {
		if (! is_object())
			throw std::runtime_error("Trying to insert a keyval into a non-object value.");

		obj_.emplace(key, std::move(val));
	}

	void value::push_back(value&& val) {
		if (! is_array())
			throw std::runtime_error("Trying to push_back a value into a non-array value.");
	
		arr_.push_back(std::move(val));
	}
	
	const value& value::operator[](const std::string& key) const {
		if (! is_object())
			throw std::runtime_error("Trying to retrieve a sub-value by key from a non-object value.");
		
		return obj_.at(key);
	}
	
	value& value::operator[](const std::string& key) {
		return const_cast<value&>(const_cast<const value*>(this)->operator[](key));
	}

	const value& value::operator[](const size_t index) const {
		if (! is_array())
			throw std::runtime_error("Trying to retrieve a sub-value by index from a non-array value.");

		return arr_.at(index);
	}
	
	value& value::operator[](const size_t index) {
		return const_cast<value&>(const_cast<const value*>(this)->operator[](index));
	}


	
	void value::debugPrint(std::ostream& os) const {
		switch(type_) {
			case value_type::String:
				os << '"' << str_ << '"';
				break;
			case value_type::Number:
				os << num_;
				break;
			case value_type::Object:
				os << "Object[" << obj_.size() << "]";
				break;
			case value_type::Array:
				os << "Array[" << arr_.size() << "]";
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
