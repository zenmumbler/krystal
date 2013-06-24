//
//  value.h
//  Krystal
//
//  Created by Arthur Langereis on 2013/6/10.
//
//

#ifndef __krystal__value__
#define __krystal__value__

#include <string>
#include <vector>
#include <unordered_map>
#include <iosfwd>

#include "alloc.hpp"

namespace krystal {

	enum class value_type {
		Null,
		False,
		True,
		Number,
		String,
		Array,
		Object
	};
	
	std::string to_string(const value_type type);
	
	class iterator;
	
	
	class value {
		using string_data = std::basic_string<char, std::char_traits<char>>;
		using array_data = std::vector<value>;
		using object_data = std::unordered_map<std::string, value>;

		using array_iterator = typename array_data::const_iterator;
		using object_iterator = typename object_data::const_iterator;

		friend class iterator;

		value_type type_;
		union {
			string_data str_;
			array_data arr_;
			object_data obj_;
			double num_;
		};

	public:
		value() : value(value_type::Null) {}
		value(const value& rhs) = delete;
		value(value&& rhs);
		value& operator=(const value& rhs) = delete;
		value& operator=(value&& rhs);
		~value();
		
		// conversion constructors
		value(value_type type);
		value(std::string sval);
		value(const char* ccval) : value(std::string{ccval}) {}
		constexpr explicit value(int ival) : type_{value_type::Number}, num_ { (double)ival } {}
		constexpr explicit value(double dval) : type_{value_type::Number}, num_ { dval } {}
		constexpr explicit value(bool bval) : type_{bval ? value_type::True : value_type::False}, num_ { 0.0 } {}

		// type tests
		value_type type() const { return type_; }
		bool is_a(const value_type type) const { return type_ == type; }
		bool is_null() const { return is_a(value_type::Null); }
		bool is_false() const { return is_a(value_type::False); }
		bool is_true() const { return is_a(value_type::True); }
		bool is_bool() const { return is_false() || is_true(); }
		bool is_number() const { return is_a(value_type::Number); }
		bool is_string() const { return is_a(value_type::String); }
		bool is_array() const { return is_a(value_type::Array); }
		bool is_object() const { return is_a(value_type::Object); }
		bool is_container() const { return is_object() || is_array(); }
		
		// value data
		bool boolean() const;
		double number() const;
		std::string string() const;

		template <typename Arith>
		Arith number_as() const {
			auto num = number();
			return static_cast<Arith>(num);
		}
		
		// container data
		size_t size() const;
		bool contains(const std::string& key) const;

		value& insert(std::string key, value&& val);
		value& push_back(value&& val);
		
		const value& operator[](const std::string& key) const;
		value& operator[](const std::string& key);
		const value& operator[](const size_t index) const;
		value& operator[](const size_t index);

		iterator begin() const;
		iterator end() const;
		
		void debugPrint(std::ostream& os) const;
	};

	std::ostream& operator<<(std::ostream& os, const value& t);
	
	iterator begin(const value& val);
	iterator end(const value& val);

	
	class iterator {
		using key_type = value;
		using mapped_type = const value&;
		
		bool is_object;
		int arr_index = 0;
		value::array_iterator arr_it;
		value::object_iterator obj_it;
		
		friend class value;

		iterator(value::array_iterator a_it, int index = 0)
		: is_object(false), arr_it(a_it), arr_index(index) {}
		iterator(value::object_iterator o_it)
		: is_object(true), obj_it(o_it) {}

	public:
		using iterator_category = std::forward_iterator_tag;
		using it_value_type = std::pair<key_type, mapped_type>;
		using reference = it_value_type;
		
		it_value_type current() const {
			if (is_object)
				return { value{obj_it->first}, obj_it->second };
			return { value{arr_index}, *arr_it };
		}
		
		reference operator *() const { return current(); }
		reference operator ->() const { return current(); }
		const iterator& operator ++() {
			if (is_object)
				++obj_it;
			else {
				++arr_it;
				++arr_index;
			}
			return *this;
		}
		iterator operator ++(int) {
			iterator ret(*this);
			this->operator++();
			return ret;
		}
		
		bool operator ==(const iterator& rhs) const {
			if (is_object)
				return obj_it == rhs.obj_it;
			return arr_it == rhs.arr_it;
		}
		bool operator !=(const iterator& rhs) const {
			return !this->operator==(rhs);
		}
	};
}

#endif
