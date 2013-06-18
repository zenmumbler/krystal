//
//  value.cpp
//  Krystal
//
//  Created by Arthur Langereis on 2013/6/10.
//
//

#include <iostream>
#include <stdexcept>
#include <utility>
#include <iterator>
#include "value.hpp"

namespace krystal {
	
	std::string to_string(const value_type type) {
		static std::unordered_map<int, std::string> vt2n {
			{ (int)value_type::Null,   "null" },
			{ (int)value_type::False,  "false" },
			{ (int)value_type::True,   "true" },
			{ (int)value_type::Number, "number" },
			{ (int)value_type::String, "string" },
			{ (int)value_type::Array,  "array" },
			{ (int)value_type::Object, "object" }
		};
		
		return vt2n[static_cast<int>(type)];
	}

	

	value::value(value_type type)
	: type_{type}
	{
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

	value::value(value&& rhs)
	: type_{rhs.type_ }
	{
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

		// -- destruct and reset rhs's data
		rhs.~value();
		rhs.type_ = value_type::Null;
	}
	
	value& value::operator=(value&& rhs) {
		if (type_ == rhs.type_) {
			// -- no need for con/destructors, straight up move assignment
			switch(type_) {
				case value_type::String:
					str_ = std::move(rhs.str_);
					break;
				case value_type::Array:
					arr_ = std::move(rhs.arr_);
					break;
				case value_type::Object:
					obj_ = std::move(rhs.obj_);
					break;
				case value_type::Number:
					num_ = rhs.num_;
					break;
				default:
					break;
			}
		}
		else {
			this->~value();
			type_ = rhs.type_;
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
		}
		
		// -- destruct and reset rhs's data
		rhs.~value();
		rhs.type_ = value_type::Null;

		return *this;
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
	
	value::value(std::string sval)
	: type_{value_type::String}
	{
		new (&str_) decltype(str_){ std::move(sval) };
	}

	
	bool value::boolean() const {
		if (! is_bool())
			throw std::runtime_error("Trying to call boolean() on a non-bool value.");

		return is_true();
	}

	double value::number() const {
		if (! is_number())
			throw std::runtime_error("Trying to call number() on a non-number value.");

		return num_;
	}

	std::string value::string() const {
		if (! is_string())
			throw std::runtime_error("Trying to call string() on a non-string value.");
		
		return str_;
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
	
	value& value::insert(std::string key, value&& val) {
		if (! is_object())
			throw std::runtime_error("Trying to insert a keyval into a non-object value.");

		if (contains(key))
			obj_.erase(key); // duplicate key, latest wins as per behaviour in all other JSON parsers

		return obj_.emplace(key, std::move(val)).first.operator*().second;
	}

	value& value::push_back(value&& val) {
		if (! is_array())
			throw std::runtime_error("Trying to push_back a value into a non-array value.");
	
		arr_.push_back(std::move(val));
		return arr_.back();
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


	iterator value::begin() const {
		if (! is_container())
			throw std::runtime_error("Trying to call begin() on a non-container value.");
		
		if (is_object())
			return iterator{ obj_.begin() };
		return iterator{ arr_.begin() };
	}

	iterator value::end() const {
		if (! is_container())
			throw std::runtime_error("Trying to call end() on a non-container value.");
		
		if (is_object())
			return iterator{ obj_.end() };
		return iterator{ arr_.end() };
	}
	
	// -- non-member begin() and end()
	iterator begin(const value& val) { return val.begin(); }
	iterator end(const value& val) { return val.end(); }

	
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
