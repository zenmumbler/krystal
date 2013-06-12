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

	
	class value {
		value_type type_;
		union {
			std::string str;
			std::vector<value> arr;
			std::unordered_map<std::string, value> obj;
			double num;
		};
		
	public:
		value(value_type type);
		value(const value& rhs) = delete;
		value(value&& rhs);
		value& operator=(const value& rhs) = delete;
		value& operator=(const value&& rhs) = delete;
		~value();
		
		explicit value(const std::string& sval);
		constexpr explicit value(double dval) : type_{value_type::Number}, num { dval } {}
		constexpr explicit value(bool bval) : type_{bval ? value_type::True : value_type::False}, num { 0.0 } {}
		
		bool is_a(const value_type type) const { return type_ == type; }
		bool is_null() const { return is_a(value_type::Null); }
		bool is_false() const { return is_a(value_type::False); }
		bool is_true() const { return is_a(value_type::True); }
		bool is_bool() const { return is_false() || is_true(); }
		bool is_number() const { return is_a(value_type::Number); }
		bool is_string() const { return is_a(value_type::String); }
		bool is_array() const { return is_a(value_type::Array); }
		bool is_object() const { return is_a(value_type::Object); }

		size_t size() const;
		bool contains(const std::string& key) const;

		void insert(std::string key, value val);
		void push_back(value val);
		
		const value& operator[](const std::string& key) const;
		const value& operator[](const size_t index) const;
		
		void debugPrint(std::ostream& os) const;
	};

	std::ostream& operator<<(std::ostream& os, const value& t);

}

#endif
