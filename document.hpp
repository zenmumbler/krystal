// document.hpp - part of krystal
// (c) 2013-4 by Arthur Langereis (@zenmumbler)

#ifndef KRYSTAL_DOCUMENT_H
#define KRYSTAL_DOCUMENT_H

#include "reader.hpp"
#include "alloc.hpp"

#include <iosfwd>
#include <memory>
#include <iterator>
#include <string>

namespace krystal {


template <template<typename T> class Allocator>
class Document {
public:
	using ValueType = BasicValue<Allocator>;

	Document(krystal::Lake *memPool, ValueType&& root)
	: memPool_ { memPool }, root_ { std::move(root) }
	{}

	// forward const value APIs (container ones only, as a doc can only be array, object or null)
	constexpr ValueKind type() const { return root_.type(); }
	constexpr bool isA(const ValueKind vtype) const { return type() == vtype; }
	constexpr bool isNull() const { return isA(ValueKind::Null); }
	constexpr bool isFalse() const { return isA(ValueKind::False); }
	constexpr bool isTrue() const { return isA(ValueKind::True); }
	constexpr bool isBool() const { return isFalse() || isTrue(); }
	constexpr bool isNumber() const { return isA(ValueKind::Number); }
	constexpr bool isString() const { return isA(ValueKind::String); }
	constexpr bool isArray() const { return isA(ValueKind::Array); }
	constexpr bool isObject() const { return isA(ValueKind::Object); }
	constexpr bool isContainer() const { return isObject() || isArray(); }

	constexpr size_t size() const { return root_.size(); }
	
	bool contains(const std::string& key) const { return root_.contains(key); }
	
	const ValueType& operator[](const std::string& key) const { return root_[key]; }
	const ValueType& operator[](const size_t index) const { return root_[index]; }
	
	Iterator<Allocator> begin() const { return root_.begin(); }
	Iterator<Allocator> end() const { return root_.end(); }
	
	void debugPrint(std::ostream& os) const { return root_.debugPrint(os); }

private:
	std::unique_ptr<krystal::Lake> memPool_;
	ValueType root_;
};


// Document non-members
template <template<typename T> class Allocator>
auto begin(const Document<Allocator>& d) -> decltype(d.begin()) { return d.begin(); }
template <template<typename T> class Allocator>
auto end(const Document<Allocator>& d) -> decltype(d.end()) { return d.end(); }

template <template<typename T> class Allocator>
std::ostream& operator<<(std::ostream& os, const Document<Allocator>& t) { t.debugPrint(); return os; }



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
	using DocumentType = Document<Allocator>;

	DocumentBuilder()
	: memPool_ { new krystal::Lake() }
	, root_{ ValueKind::Object, memPool_.get() }
	, nextKey_{ DOC_ROOT_KEY }
	{
		contextStack_.reserve(32);
		contextStack_.push_back(&root_);
		curNode_ = &root_;
	}

	DocumentType document() {
		// the DocumentBuilder instance is useless after the call to document()
		auto pool = memPool_.release();

		if (hadError_) {
			return { pool, { ValueKind::Null, pool } };
		}
		return { pool, std::move(root_[DOC_ROOT_KEY]) };
	}
};



template <typename ForwardIterator>
DocumentBuilder::DocumentType parse(ForwardIterator first, ForwardIterator last) {
	DocumentBuilder delegate;
	Reader r { delegate };
	ReaderStream<ForwardIterator> ris { std::move(first), std::move(last) };
	
	r.parseDocument(ris);
	
	return delegate.document();
}


template <typename IStream>
DocumentBuilder::DocumentType parseStream(IStream &is) {
	is >> std::noskipws;
	std::istream_iterator<typename IStream::char_type> first{is};
	return parse(first, {});
}


DocumentBuilder::DocumentType parseString(std::string json_string) {
	return parse(begin(json_string), end(json_string));
}


} // ns krystal

#endif
