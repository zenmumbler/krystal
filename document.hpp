// document.hpp - part of krystal
// (c) 2013-5 by Arthur Langereis (@zenmumbler)

#ifndef KRYSTAL_DOCUMENT_H
#define KRYSTAL_DOCUMENT_H

#include "reader.hpp"
#include "alloc.hpp"

#include <iosfwd>
#include <memory>
#include <iterator>

namespace krystal {


/*

values is packed value data in a linear buffer
representation of values:
Null: not present
False: not present
True: not present
Number: double
String: char[]
Array: uint32 length
  followed by values of array
Object: uint32 length
  followed by sets of hashed keys (uint32) and values of object

data:
array<ValueKind (uint8)>
array<value> per above

problem: implement:

{
	"aap": [10, 100, 1000, 5000],
	"kaas": "neus",
	"sub": {
		"plop": true,
		"sop": "xx"
	}
}

types:  [ _, O, A, N,  N,   N,    N,    S,        O,  T, S ]
values: [ _, 3, 4, 10, 100, 1000, 5000, "neus\0", 1,     "xx\0" ]
offsets [ _, 0, 4, 8,  16,  24,   32,   36,       44,    45 ]
map: {
	"aap":   0,
	"kaas": 36,
	"sub":  44
}

for (auto& v : doc["sub"]) {
}

ValueProxy
	Index (uint)
	size() { values[offsets[index]] }

*/


template <typename ValueClass>
class Document {
	std::unique_ptr<krystal::Lake> memPool_;
	ValueClass root_;

public:
	using ValueType = ValueClass;

	Document(std::unique_ptr<krystal::Lake> memPool, ValueClass&& root)
	: memPool_ { std::move(memPool) }, root_ { std::move(root) }
	{}

	// forward const value APIs (container ones only, as a doc can only be array, object or null)
	inline ValueKind type() const { return root_.type(); }
	inline bool isA(const ValueKind vtype) const { return type() == vtype; }
	bool isNull() const { return isA(ValueKind::Null); }
	bool isFalse() const { return isA(ValueKind::False); }
	bool isTrue() const { return isA(ValueKind::True); }
	bool isBool() const { return isFalse() || isTrue(); }
	bool isNumber() const { return isA(ValueKind::Number); }
	bool isString() const { return isA(ValueKind::String); }
	bool isArray() const { return isA(ValueKind::Array); }
	bool isObject() const { return isA(ValueKind::Object); }
	bool isContainer() const { return isObject() || isArray(); }

	size_t size() const { return root_.size(); }
	
	bool contains(const std::string& key) const { return root_.contains(key); }
	
	const ValueClass& operator[](const std::string& key) const { return root_[key]; }
	const ValueClass& operator[](const size_t index) const { return root_[index]; }
	
	decltype(root_.begin()) begin() const { return root_.begin(); }
	decltype(root_.end()) end() const { return root_.end(); }
	
	void debugPrint(std::ostream& os) const { return root_.debugPrint(os); }
};

// Document non-members
template <typename ValueClass>
auto begin(const Document<ValueClass>& d) -> decltype(d.begin()) { return d.begin(); }
template <typename ValueClass>
auto end(const Document<ValueClass>& d) -> decltype(d.end()) { return d.end(); }

template <typename ValueClass>
std::ostream& operator<<(std::ostream& os, const Document<ValueClass>& t) { t.debugPrint(); return os; }



namespace { const std::string DOC_ROOT_KEY {"___DOCUMENT___"}; }


class DocumentBuilder : public ReaderDelegate {
	template <typename U>
	using Allocator = LakeAllocator<U>;

	std::unique_ptr<krystal::Lake> memPool_;
	BasicValue<Allocator> root_, *curNode_ = nullptr;
	std::vector<BasicValue<Allocator>*> contextStack_;
	std::string nextKey_;
	bool hadError_ = false;
	
	friend class Reader;
	
	template <typename ...Args>
	void append(Args&&... args) {
		BasicValue<Allocator>* mv;
		
		if (curNode_->isObject()) {
			mv = &curNode_->emplace(nextKey_, std::forward<Args>(args)...);
			nextKey_.clear();
		}
		else // array
			mv = &curNode_->emplace_back(std::forward<Args>(args)...);
		
		if (mv->isContainer()) {
			contextStack_.push_back(mv);
			curNode_ = mv;
		}
	}


	void nullValue() override {
		append(ValueKind::Null, memPool_.get());
	}
	
	void falseValue() override {
		append(ValueKind::False, memPool_.get());
	}
	
	void trueValue() override {
		append(ValueKind::True, memPool_.get());
	}
	
	void numberValue(double num) override {
		append(num);
	}
	
	void stringValue(const std::string& str) override {
		if (curNode_->isArray() || nextKey_.size())
			append(str, memPool_.get());
		else
			nextKey_ = str;
	}
	
	void arrayBegin() override {
		append(ValueKind::Array, memPool_.get());
	}
	
	void arrayEnd() override {
		contextStack_.pop_back();
		curNode_ = contextStack_.back();
	}
	
	void objectBegin() override {
		append(ValueKind::Object, memPool_.get());
	}
	
	void objectEnd() override {
		contextStack_.pop_back();
		curNode_ = contextStack_.back();
	}
	
	void error(const std::string& msg, ptrdiff_t offset) override {
		hadError_ = true;
	}

public:
	DocumentBuilder()
	: memPool_ { new krystal::Lake() }
	, root_{ ValueKind::Object, memPool_.get() }
	, nextKey_{ DOC_ROOT_KEY }
	{
		contextStack_.reserve(32);
		contextStack_.push_back(&root_);
		curNode_ = &root_;
	}

	krystal::Document<BasicValue<Allocator>> document() {
		// the DocumentBuilder instance is useless after the call to document()
		if (hadError_) {
			return { std::move(memPool_), { ValueKind::Null, memPool_.get() } };
		}
		return { std::move(memPool_), std::move(root_[DOC_ROOT_KEY]) };
	}
};



template <typename ForwardIterator>
auto parse(ForwardIterator first, ForwardIterator last)
	-> decltype(DocumentBuilder().document())
{
	auto delegate = std::make_shared<DocumentBuilder>();
	Reader r { delegate };
	ReaderStream<ForwardIterator> ris { std::move(first), std::move(last) };
	
	r.parseDocument(ris);
	
	return delegate->document();
}

template <typename IStream>
auto parseStream(IStream &is)
	-> decltype(DocumentBuilder().document())
{
	is >> std::noskipws;
	std::istream_iterator<typename IStream::char_type> first{is};
	return parse(first, {});
}

auto parseString(std::string json_string)
	-> decltype(DocumentBuilder().document())
{
	return parse(begin(json_string), end(json_string));
}


} // ns krystal

#endif
