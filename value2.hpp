// value.hpp - part of krystal
// (c) 2013 by Arthur Langereie

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

	template <typename CharT, template<typename T> class Allocator>
	class iterator;
	
	template <typename CharT = char, template<typename T> class Allocator = std::allocator>
	class basic_value {
		using string_alloc = Allocator<CharT>;
		using string_data = std::basic_string<CharT, std::char_traits<CharT>, string_alloc>;

		using array_alloc = std::scoped_allocator_adaptor<Allocator<string_data>>;
		using array_data = std::vector<basic_value<CharT, Allocator>>;

		using object_alloc = Allocator<std::pair<const std::string, basic_value<CharT, Allocator>>>;
		using object_data = std::unordered_map<std::string, basic_value<CharT, Allocator>, std::hash<std::string>, std::equal_to<std::string>, object_alloc>;

		using array_iterator = typename array_data::const_iterator;
		using object_iterator = typename object_data::const_iterator;
		

		friend class iterator<CharT, Allocator>;

		value_type type_;
		union {
			string_data str_;
			array_data arr_;
			object_data obj_;
			double num_;
		};

	public:
		basic_value() : basic_value(value_type::Null) {}
		basic_value(const basic_value& rhs) = delete;
		basic_value<CharT, Allocator>& operator=(const basic_value<CharT, Allocator>& rhs) = delete;

		basic_value(basic_value<CharT, Allocator>&& rhs)
		: type_{rhs.type_}
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
			rhs.~basic_value();
			rhs.type_ = value_type::Null;
		}

		basic_value<CharT, Allocator>& operator=(basic_value<CharT, Allocator>&& rhs) {
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
				this->~basic_value();
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
			rhs.~basic_value();
			rhs.type_ = value_type::Null;
			
			return *this;
		}

		~basic_value() {
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


		// conversion constructors

		basic_value(value_type type)
		: type_{type}
		{
			switch(type_) {
				case value_type::String:
					new (&str_) decltype(str_){};
					break;
				case value_type::Array:
					new (&arr_) decltype(arr_){};
					arr_.reserve(2);
					break;
				case value_type::Object:
					new (&obj_) decltype(obj_){};
					break;
				default:
					num_ = 0.0;
					break;
			}
		}

		basic_value(const std::string& sval)
		: type_{value_type::String}
		{
			new (&str_) decltype(str_){ sval.data(), sval.data() + sval.size() };
		}

		basic_value(const char* ccval) : basic_value(std::string{ccval}) {}
		constexpr explicit basic_value(int ival) : type_{value_type::Number}, num_ { (double)ival } {}
		constexpr explicit basic_value(double dval) : type_{value_type::Number}, num_ { dval } {}
		constexpr explicit basic_value(bool bval) : type_{bval ? value_type::True : value_type::False}, num_ { 0.0 } {}

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
		
		bool boolean() const {
			if (! is_bool())
				throw std::runtime_error("Trying to call boolean() on a non-bool value.");
			
			return is_true();
		}
		
		double number() const {
			if (! is_number())
				throw std::runtime_error("Trying to call number() on a non-number value.");
			
			return num_;
		}
		
		template <typename Arith>
		Arith number_as() const {
			auto num = number();
			return static_cast<Arith>(num);
		}

		std::string string() const {
			if (! is_string())
				throw std::runtime_error("Trying to call string() on a non-string value.");
			
			return { str_.c_str() };
		}
		
		
		size_t size() const {
			if (is_object())
				return obj_.size();
			if (is_array())
				return arr_.size();
			return 1;
		}
		
		bool contains(const std::string& key) const {
			if (! is_object())
				throw std::runtime_error("Trying to check for a key in a non-object value.");
			
			return obj_.find(key) != obj_.cend();
		}
		
		basic_value<CharT, Allocator>& insert(std::string key, basic_value<CharT, Allocator>&& val) {
			if (! is_object())
				throw std::runtime_error("Trying to insert a keyval into a non-object value.");
			
			if (contains(key))
				obj_.erase(key); // duplicate key, latest wins as per behaviour in all other JSON parsers
			
			return obj_.emplace(key, std::move(val)).first.operator*().second;
		}
		
		basic_value<CharT, Allocator>& push_back(basic_value<CharT, Allocator>&& val) {
			if (! is_array())
				throw std::runtime_error("Trying to push_back a value into a non-array value.");
			
			arr_.push_back(std::move(val));
			return arr_.back();
		}
		
		const basic_value<CharT, Allocator>& operator[](const std::string& key) const {
			if (! is_object())
				throw std::runtime_error("Trying to retrieve a sub-value by key from a non-object value.");
			
			return obj_.at(key);
		}
		
		basic_value<CharT, Allocator>& operator[](const std::string& key) {
			return const_cast<basic_value<CharT, Allocator>&>(const_cast<const basic_value<CharT, Allocator>*>(this)->operator[](key));
		}
		
		const basic_value<CharT, Allocator>& operator[](const size_t index) const {
			if (! is_array())
				throw std::runtime_error("Trying to retrieve a sub-value by index from a non-array value.");
			
			return arr_.at(index);
		}
		
		basic_value<CharT, Allocator>& operator[](const size_t index) {
			return const_cast<basic_value<CharT, Allocator>&>(const_cast<const basic_value<CharT, Allocator>*>(this)->operator[](index));
		}

		iterator<CharT, Allocator> begin() const;
		iterator<CharT, Allocator> end() const;
				

		void debugPrint(std::ostream& os) const {
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

	};


	template <typename CharT, template<typename T> class Allocator>
	std::ostream& operator<<(std::ostream& os, const basic_value<CharT, Allocator>& t) {
		t.debugPrint(os);
		return os;
	}


	template <typename CharT, template<typename T> class Allocator>
	class iterator {
		using key_type = basic_value<CharT, Allocator>;
		using mapped_type = const basic_value<CharT, Allocator>&;
		
		using array_iterator = typename basic_value<CharT, Allocator>::array_iterator;
		using object_iterator = typename basic_value<CharT, Allocator>::object_iterator;
		
		bool is_object;
		int arr_index = 0;
		array_iterator arr_it;
		object_iterator obj_it;
		
		friend class basic_value<CharT, Allocator>;
		
		iterator(array_iterator a_it, int index = 0)
		: is_object(false), arr_it(a_it), arr_index(index) {}
		iterator(object_iterator o_it)
		: is_object(true), obj_it(o_it) {}
		
	public:
		using iterator_category = std::forward_iterator_tag;
		using it_value_type = std::pair<key_type, mapped_type>;
		using reference = it_value_type;
		
		it_value_type current() const {
			if (is_object)
				return { basic_value<CharT, Allocator>{obj_it->first}, obj_it->second };
			return { basic_value<CharT, Allocator>{arr_index}, *arr_it };
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


	// member begin() and end()
	template <typename CharT, template<typename T> class Allocator>
	iterator<CharT, Allocator> basic_value<CharT, Allocator>::begin() const {
		if (! is_container())
			throw std::runtime_error("Trying to call begin() on a non-container value.");
		
		if (is_object())
			return { obj_.begin() };
		return { arr_.begin() };
	}

	template <typename CharT, template<typename T> class Allocator>
	iterator<CharT, Allocator> basic_value<CharT, Allocator>::end() const {
		if (! is_container())
			throw std::runtime_error("Trying to call end() on a non-container value.");
		
		if (is_object())
			return { obj_.end() };
		return { arr_.end() };
	}


	// -- non-member begin() and end()
	template <typename CharT, template<typename T> class Allocator>
	iterator<CharT, Allocator> begin(const basic_value<CharT, Allocator>& val) { return val.begin(); }

	template <typename CharT, template<typename T> class Allocator>
	iterator<CharT, Allocator> end(const basic_value<CharT, Allocator>& val) { return val.end(); }

}

#endif
