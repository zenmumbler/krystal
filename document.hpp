// document.hpp - part of krystal
// (c) 2013-4 by Arthur Langereis (@zenmumbler)

#ifndef KRYSTAL_DOCUMENT_H
#define KRYSTAL_DOCUMENT_H

#include "reader.hpp"
#include "alloc.hpp"

#include <iosfwd>
#include <memory>
#include <iterator>

namespace krystal {


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


template <typename CharT>
class DocumentBuilder : public ReaderDelegate {
	template <typename U>
	using Allocator = LakeAllocator<U>;

	std::unique_ptr<krystal::Lake> memPool_;
	BasicValue<CharT, Allocator> root_, *curNode_ = nullptr;
	std::vector<BasicValue<CharT, Allocator>*> contextStack_;
	std::string nextKey_;
	bool hadError_ = false;
	
	friend class Reader;
	
	template <typename ...Args>
	void append(Args&&... args) {
		BasicValue<CharT, Allocator>* mv;
		
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

	krystal::Document<BasicValue<CharT, Allocator>> document() {
		// the DocumentBuilder instance is useless after the call to document()
		if (hadError_) {
			return { std::move(memPool_), { ValueKind::Null, memPool_.get() } };
		}
		return { std::move(memPool_), std::move(root_[DOC_ROOT_KEY]) };
	}
};



template <typename ForwardIterator>
auto parse(ForwardIterator first, ForwardIterator last)
	-> decltype(DocumentBuilder<typename std::iterator_traits<ForwardIterator>::value_type>().document())
{
	using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

	auto delegate = std::make_shared<DocumentBuilder<CharT>>();
	Reader r { delegate };
	ReaderStream<ForwardIterator> ris { std::move(first), std::move(last) };
	
	r.parseDocument(ris);
	
	return delegate->document();
}

template <typename IStream>
auto parseStream(IStream &is)
	-> decltype(DocumentBuilder<typename IStream::char_type>().document())
{
	is >> std::noskipws;
	std::istream_iterator<typename IStream::char_type> first{is};
	return parse(first, {});
}

template <typename CharT>
auto parseString(std::basic_string<CharT> json_string)
	-> decltype(DocumentBuilder<CharT>().document())
{
	return parse(begin(json_string), end(json_string));
}


} // ns krystal

#endif
