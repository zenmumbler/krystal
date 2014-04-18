// document.hpp - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __KRYSTAL_DOCUMENT__
#define __KRYSTAL_DOCUMENT__

#include <iosfwd>
#include <memory>
#include <iterator>
#include "reader.hpp"
#include "alloc.hpp"

namespace krystal {

	template <typename ValueClass>
	class document {
		std::unique_ptr<krystal::lake> mem_pool_;
		ValueClass root_;

	public:
		using value_type = ValueClass;

		document(std::unique_ptr<krystal::lake> mem_pool, ValueClass&& root)
		: mem_pool_ { std::move(mem_pool) }, root_ { std::move(root) }
		{}

		// forward const value APIs (container ones only, as a doc can only be array, object or null)
		inline value_kind type() const { return root_.type(); }
		inline bool is_a(const value_kind vtype) const { return type() == vtype; }
		bool is_null() const { return is_a(value_kind::Null); }
		bool is_false() const { return is_a(value_kind::False); }
		bool is_true() const { return is_a(value_kind::True); }
		bool is_bool() const { return is_false() || is_true(); }
		bool is_number() const { return is_a(value_kind::Number); }
		bool is_string() const { return is_a(value_kind::String); }
		bool is_array() const { return is_a(value_kind::Array); }
		bool is_object() const { return is_a(value_kind::Object); }
		bool is_container() const { return is_object() || is_array(); }

		size_t size() const { return root_.size(); }
		
		bool contains(const std::string& key) const { return root_.contains(key); }
		
		const ValueClass& operator[](const std::string& key) const { return root_[key]; }
		const ValueClass& operator[](const size_t index) const { return root_[index]; }
		
		decltype(root_.begin()) begin() const { return root_.begin(); }
		decltype(root_.end()) end() const { return root_.end(); }
		
		void debugPrint(std::ostream& os) const { return root_.debugPrint(os); }
	};

	// document non-members
	template <typename ValueClass>
	auto begin(const document<ValueClass>& d) -> decltype(d.begin()) { return d.begin(); }
	template <typename ValueClass>
	auto end(const document<ValueClass>& d) -> decltype(d.end()) { return d.end(); }

	template <typename ValueClass>
	std::ostream& operator<<(std::ostream& os, const document<ValueClass>& t) { t.debugPrint(); return os; }



	namespace { const std::string DOC_ROOT_KEY {"___DOCUMENT___"}; }


	template <typename CharT>
	class document_builder : public reader_delegate {
		template <typename U>
		using Allocator = lake_alloc<U>;

		std::unique_ptr<krystal::lake> mem_pool_;
		basic_value<CharT, Allocator> root_, *cur_node_ = nullptr;
		std::vector<basic_value<CharT, Allocator>*> context_stack_;
		std::string next_key_;
		bool had_error = false;
		
		friend class reader;
		
		template <typename ...Args>
		void append(Args&&... args) {
			basic_value<CharT, Allocator>* mv;
			
			if (cur_node_->is_object()) {
				mv = &cur_node_->emplace(next_key_, std::forward<Args>(args)...);
				next_key_.clear();
			}
			else // array
				mv = &cur_node_->emplace_back(std::forward<Args>(args)...);
			
			if (mv->is_container()) {
				context_stack_.push_back(mv);
				cur_node_ = mv;
			}
		}


		void null_value() override {
			append(value_kind::Null, mem_pool_.get());
		}
		
		void false_value() override {
			append(value_kind::False, mem_pool_.get());
		}
		
		void true_value() override {
			append(value_kind::True, mem_pool_.get());
		}
		
		void number_value(double num) override {
			append(num);
		}
		
		void string_value(const std::string& str) override {
			if (cur_node_->is_array() || next_key_.size())
				append(str, mem_pool_.get());
			else
				next_key_ = str;
		}
		
		void array_begin() override {
			append(value_kind::Array, mem_pool_.get());
		}
		
		void array_end() override {
			context_stack_.pop_back();
			cur_node_ = context_stack_.back();
		}
		
		void object_begin() override {
			append(value_kind::Object, mem_pool_.get());
		}
		
		void object_end() override {
			context_stack_.pop_back();
			cur_node_ = context_stack_.back();
		}
		
		void error(const std::string& msg, ptrdiff_t offset) override {
			had_error = true;
		}

	public:
		document_builder()
		: mem_pool_ { new krystal::lake() }
		, root_{ value_kind::Object, mem_pool_.get() }
		, next_key_{ DOC_ROOT_KEY }
		{
			context_stack_.reserve(32);
			context_stack_.push_back(&root_);
			cur_node_ = &root_;
		}
		
		krystal::document<basic_value<CharT, Allocator>> document() {
			// the document_builder instance is useless after the call to document()
			if (had_error) {
				return { std::move(mem_pool_), { value_kind::Null, mem_pool_.get() } };
			}
			return { std::move(mem_pool_), std::move(root_[DOC_ROOT_KEY]) };
		}
	};





	template <typename ForwardIterator>
	auto parse(ForwardIterator first, ForwardIterator last)
		-> decltype(document_builder<typename std::iterator_traits<ForwardIterator>::value_type>().document())
	{
		using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

		auto delegate = std::make_shared<document_builder<CharT>>();
		reader r { delegate };
		reader_stream<ForwardIterator> ris { std::move(first), std::move(last) };
		
		r.parse_document(ris);
		
		return delegate->document();
	}
	
	template <typename IStream>
	auto parse_stream(IStream &is)
		-> decltype(document_builder<typename IStream::char_type>().document())
	{
		is >> std::noskipws;
		std::istream_iterator<typename IStream::char_type> first{is};
		return parse(first, {});
	}

	template <typename CharT>
	auto parse_string(std::basic_string<CharT> json_string)
		-> decltype(document_builder<CharT>().document())
	{
		return parse(begin(json_string), end(json_string));
	}

}

#endif
