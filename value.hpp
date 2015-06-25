// value.hpp - part of krystal
// (c) 2013-4 by Arthur Langereis (@zenmumbler)

#ifndef KRYSTAL_VALUE_H
#define KRYSTAL_VALUE_H

#include "alloc.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <stdexcept>

namespace krystal {


enum class ValueKind {
	Null,
	False,
	True,
	Number,
	String,
	Array,
	Object
};


template <template<typename T> class Allocator>
class Iterator;


template <template<typename T> class Allocator = std::allocator>
class BasicValue {
	template <typename K>
	using AllocType = Allocator<K>;
	using ValueType = BasicValue<Allocator>;

	using StringAlloc = AllocType<char>;
	using StringData = std::basic_string<char, std::char_traits<char>, StringAlloc>;

	using ArrayAlloc = AllocType<ValueType>;
	using ArrayData = std::vector<ValueType, ArrayAlloc>;
	
	using ObjectAlloc = AllocType<std::pair<const std::string, ValueType>>;
	using ObjectData = std::unordered_map<std::string, ValueType, std::hash<std::string>, std::equal_to<std::string>, ObjectAlloc>;

	using ArrayIterator = typename ArrayData::const_iterator;
	using ObjectIterator = typename ObjectData::const_iterator;
	

	friend class Iterator<Allocator>;

	ValueKind kind_;
	union {
		StringData str_;
		ArrayData arr_;
		ObjectData obj_;
		double num_;
	};

public:
	BasicValue() : BasicValue(ValueKind::Null) {}
	BasicValue(const BasicValue& rhs) = delete;
	BasicValue<Allocator>& operator=(const BasicValue<Allocator>& rhs) = delete;

	BasicValue(BasicValue<Allocator>&& rhs) noexcept
	: kind_{rhs.kind_}
	{
		switch(kind_) {
			case ValueKind::String:
				new (&str_) decltype(str_){std::move(rhs.str_)};
				break;
			case ValueKind::Array:
				new (&arr_) decltype(arr_){std::move(rhs.arr_)};
				break;
			case ValueKind::Object:
				new (&obj_) decltype(obj_){std::move(rhs.obj_)};
				break;
			case ValueKind::Number:
				num_ = rhs.num_;
				break;
			default:
				break;
		}
		
		// -- destruct and reset rhs's data
		rhs.~BasicValue();
		rhs.kind_ = ValueKind::Null;
	}

	BasicValue<Allocator>& operator=(BasicValue<Allocator>&& rhs) noexcept {
		if (kind_ == rhs.kind_) {
			// -- no need for con/destructors, straight up move assignment
			switch(kind_) {
				case ValueKind::String:
					str_ = std::move(rhs.str_);
					break;
				case ValueKind::Array:
					arr_ = std::move(rhs.arr_);
					break;
				case ValueKind::Object:
					obj_ = std::move(rhs.obj_);
					break;
				case ValueKind::Number:
					num_ = rhs.num_;
					break;
				default:
					break;
			}
		}
		else {
			this->~BasicValue();
			kind_ = rhs.kind_;
			switch(kind_) {
				case ValueKind::String:
					new (&str_) decltype(str_){std::move(rhs.str_)};
					break;
				case ValueKind::Array:
					new (&arr_) decltype(arr_){std::move(rhs.arr_)};
					break;
				case ValueKind::Object:
					new (&obj_) decltype(obj_){std::move(rhs.obj_)};
					break;
				case ValueKind::Number:
					num_ = rhs.num_;
					break;
				default:
					break;
			}
		}
		
		// -- destruct and reset rhs's data
		rhs.~BasicValue();
		rhs.kind_ = ValueKind::Null;
		
		return *this;
	}

	~BasicValue() {
		switch(kind_) {
			case ValueKind::String:
				str_.~basic_string();
				break;
			case ValueKind::Array:
				arr_.~vector();
				break;
			case ValueKind::Object:
				obj_.~unordered_map();
				break;
			default:
				break;
		}
	}


	// conversion constructors
	BasicValue(ValueKind kind, const Lake* args)
	: kind_{kind}
	{
		switch(kind_) {
			case ValueKind::String:
				new (&str_) decltype(str_){ StringAlloc(args) };
				break;
			case ValueKind::Array:
				new (&arr_) decltype(arr_){ ArrayAlloc(args) };
				break;
			case ValueKind::Object:
				new (&obj_) decltype(obj_){ ObjectAlloc{args} };
				break;
			default:
				num_ = 0.0;
				break;
		}
	}

	BasicValue(ValueKind kind)
	: kind_{kind}
	{
		switch(kind_) {
			case ValueKind::String:
				new (&str_) decltype(str_){ StringAlloc{} };
				break;
			case ValueKind::Array:
				new (&arr_) decltype(arr_){ ArrayAlloc{} };
				break;
			case ValueKind::Object:
				new (&obj_) decltype(obj_){ ObjectAlloc{} };
				break;
			default:
				num_ = 0.0;
				break;
		}
	}

	BasicValue(const std::string& sval, const Lake* args)
	: kind_{ValueKind::String}
	{
		new (&str_) decltype(str_){ std::begin(sval), std::end(sval), StringAlloc{ args } };
	}

	BasicValue(const std::string& sval)
	: kind_{ValueKind::String}
	{
		new (&str_) decltype(str_){ std::begin(sval), std::end(sval), StringAlloc{} };
	}

	BasicValue(const char* ccval, const Lake* args) : BasicValue(std::string{ccval}, args) {}

	BasicValue(const char* ccval) : BasicValue(std::string{ccval}) {}

	constexpr explicit BasicValue(int ival) : kind_{ValueKind::Number}, num_ { (double)ival } {}
	constexpr explicit BasicValue(double dval) : kind_{ValueKind::Number}, num_ { dval } {}
	constexpr explicit BasicValue(bool bval) : kind_{bval ? ValueKind::True : ValueKind::False}, num_ { 0.0 } {}

	// type tests
	ValueKind type() const { return kind_; }
	bool isA(const ValueKind type) const { return kind_ == type; }
	bool isNull() const { return isA(ValueKind::Null); }
	bool isFalse() const { return isA(ValueKind::False); }
	bool isTrue() const { return isA(ValueKind::True); }
	bool isBool() const { return isFalse() || isTrue(); }
	bool isNumber() const { return isA(ValueKind::Number); }
	bool isString() const { return isA(ValueKind::String); }
	bool isArray() const { return isA(ValueKind::Array); }
	bool isObject() const { return isA(ValueKind::Object); }
	bool isContainer() const { return isObject() || isArray(); }
	
	bool boolean() const {
		if (! isBool())
			throw std::runtime_error("Trying to call boolean() on a non-bool value.");
		
		return isTrue();
	}
	
	double number() const {
		if (! isNumber())
			throw std::runtime_error("Trying to call number() on a non-number value.");
		
		return num_;
	}
	
	template <typename Arith>
	Arith numberAs() const {
		auto num = number();
		return static_cast<Arith>(num);
	}

	std::string string() const {
		if (! isString())
			throw std::runtime_error("Trying to call string() on a non-string value.");
		
		return { std::begin(str_), std::end(str_) };
	}
	
	
	size_t size() const {
		if (isObject())
			return obj_.size();
		if (isArray())
			return arr_.size();
		return 1;
	}
	
	bool contains(const std::string& key) const {
		if (! isObject())
			throw std::runtime_error("Trying to check for a key in a non-object value.");

		return obj_.find(key) != obj_.cend();
	}
	
	template <typename ...Args>
	BasicValue<Allocator>& emplace(std::string key, Args&&... args) {
		if (! isObject())
			throw std::runtime_error("Trying to insert a keyval into a non-object value.");

		if (contains(key))
			obj_.erase(key); // duplicate key, latest wins as per behaviour in all other JSON parsers
		
		return obj_.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...)).first.operator*().second;
	}
	
	template <typename ...Args>
	BasicValue<Allocator>& emplace_back(Args&&... args) {
		if (! isArray())
			throw std::runtime_error("Trying to push_back a value into a non-array value.");
		
		arr_.emplace_back(std::forward<Args>(args)...);
		return arr_.back();
	}
	
	const BasicValue<Allocator>& operator[](const std::string& key) const {
		if (! isObject())
			throw std::runtime_error("Trying to retrieve a sub-value by key from a non-object value.");
		
		return obj_.at(key);
	}
	
	BasicValue<Allocator>& operator[](const std::string& key) {
		return const_cast<BasicValue<Allocator>&>(const_cast<const BasicValue<Allocator>*>(this)->operator[](key));
	}
	
	const BasicValue<Allocator>& operator[](const size_t index) const {
		if (! isArray())
			throw std::runtime_error("Trying to retrieve a sub-value by index from a non-array value.");
		
		return arr_.at(index);
	}
	
	BasicValue<Allocator>& operator[](const size_t index) {
		return const_cast<BasicValue<Allocator>&>(const_cast<const BasicValue<Allocator>*>(this)->operator[](index));
	}

	Iterator<Allocator> begin() const;
	Iterator<Allocator> end() const;
			

	void debugPrint(std::ostream& os) const {
		switch(kind_) {
			case ValueKind::String:
				os << '"' << str_ << '"';
				break;
			case ValueKind::Number:
				os << num_;
				break;
			case ValueKind::Object:
				os << "Object[" << obj_.size() << "]";
				break;
			case ValueKind::Array:
				os << "Array[" << arr_.size() << "]";
				break;
			case ValueKind::True:
				os << "true";
				break;
			case ValueKind::False:
				os << "false";
				break;
			case ValueKind::Null:
				os << "null";
				break;
		}
	}

};


using Value = BasicValue<>;


template <template<typename T> class Allocator>
std::ostream& operator<<(std::ostream& os, const BasicValue<Allocator>& t) {
	t.debugPrint(os);
	return os;
}


template <template<typename T> class Allocator>
class Iterator {
	using KeyType = BasicValue<Allocator>;
	using MappedType = const BasicValue<Allocator>&;
	
	using ArrayIterator = typename BasicValue<Allocator>::ArrayIterator;
	using ObjectIterator = typename BasicValue<Allocator>::ObjectIterator;
	
	bool isObject;
	int arrIndex = 0;
	ArrayIterator arrIt;
	ObjectIterator objIt;
	
	friend class BasicValue<Allocator>;
	
	Iterator(ArrayIterator a_it, int index = 0)
	: isObject(false), arrIt(a_it), arrIndex(index) {}
	Iterator(ObjectIterator o_it)
	: isObject(true), objIt(o_it) {}
	
public:
	// standard iterator interop
	using iterator_category = std::forward_iterator_tag;
	using reference = std::pair<KeyType, MappedType>;

	
	reference current() const {
		if (isObject)
			return { BasicValue<Allocator>{objIt->first}, objIt->second };
		return { BasicValue<Allocator>{arrIndex}, *arrIt };
	}
	
	reference operator *() const { return current(); }
	reference operator ->() const { return current(); }
	const Iterator& operator ++() {
		if (isObject)
			++objIt;
		else {
			++arrIt;
			++arrIndex;
		}
		return *this;
	}
	Iterator operator ++(int) {
		Iterator ret(*this);
		this->operator++();
		return ret;
	}
	
	bool operator ==(const Iterator& rhs) const {
		if (isObject)
			return objIt == rhs.objIt;
		return arrIt == rhs.arrIt;
	}
	bool operator !=(const Iterator& rhs) const {
		return !this->operator==(rhs);
	}
};


// member begin() and end()
template <template<typename T> class Allocator>
Iterator<Allocator> BasicValue<Allocator>::begin() const {
	if (! isContainer())
		throw std::runtime_error("Trying to call begin() on a non-container value.");
	
	if (isObject())
		return { obj_.begin() };
	return { arr_.begin() };
}

template <template<typename T> class Allocator>
Iterator<Allocator> BasicValue<Allocator>::end() const {
	if (! isContainer())
		throw std::runtime_error("Trying to call end() on a non-container value.");
	
	if (isObject())
		return { obj_.end() };
	return { arr_.end() };
}


// -- non-member begin() and end()
template <template<typename T> class Allocator>
Iterator<Allocator> begin(const BasicValue<Allocator>& val) { return val.begin(); }

template <template<typename T> class Allocator>
Iterator<Allocator> end(const BasicValue<Allocator>& val) { return val.end(); }


} // ns krystal

#endif
