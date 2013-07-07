// document.hpp - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __KRYSTAL_DOCUMENT__
#define __KRYSTAL_DOCUMENT__

#include <iosfwd>
#include <memory>
#include "reader2.hpp"
#include "alloc.hpp"

namespace krystal {

	namespace { const std::string DOC_ROOT_KEY {"___DOCUMENT___"}; }
	
	template <typename CharT>
	class document_builder : public reader_delegate {
		template <typename U>
		using Allocator = std::allocator<U>;

		basic_value<CharT, Allocator> root_, *cur_node_ = nullptr;
		std::vector<basic_value<CharT, Allocator>*> context_stack_;
		std::string next_key_;
		
		friend class reader;
		
		void append(basic_value<CharT, Allocator> val) {
			bool val_is_container = val.is_container();
			basic_value<CharT, Allocator>* mv;
			
			if (cur_node_->is_object()) {
				mv = &cur_node_->insert(next_key_, std::move(val));
				next_key_.clear();
			}
			else // array
				mv = &cur_node_->push_back(std::move(val));

			if (val_is_container) {
				context_stack_.push_back(mv);
				cur_node_ = mv;
			}
		}
		
		void null_value() {
			append({ value_type::Null });
		}
		
		void false_value() {
			append({ value_type::False });
		}
		
		void true_value() {
			append({ value_type::True });
		}
		
		void number_value(double num) {
			append(basic_value<CharT, Allocator>{ num });
		}
		
		void string_value(const std::string& str) {
			if (cur_node_->is_array() || next_key_.size())
				append({ str.data() });
			else
				next_key_ = str;
		}
		
		void array_begin() {
			append({ value_type::Array });
		}
		
		void array_end() {
			context_stack_.pop_back();
			cur_node_ = context_stack_.back();
		}
		
		void object_begin() {
			append({ value_type::Object });
		}
		
		void object_end() {
			context_stack_.pop_back();
			cur_node_ = context_stack_.back();
		}
		
		void error(const std::string& msg, ptrdiff_t offset) {
			std::cout << "ERROR at position " << offset << ": " << msg << '\n';
		}

	public:
		document_builder()
		: root_{ value_type::Object }
		, next_key_{ DOC_ROOT_KEY }
		{
			context_stack_.reserve(32);
			context_stack_.push_back(&root_);
			cur_node_ = &root_;
		}
		
		basic_value<CharT, Allocator> document() {
			// the document_builder instance is useless after the call to document()
			return std::move(root_[DOC_ROOT_KEY]);
		}
	};



	// -- standard value types
	using value = decltype(document_builder<char>().document());




	template <typename ForwardIterator>
	auto parse(ForwardIterator first, ForwardIterator last)
		-> decltype(document_builder<typename std::iterator_traits<ForwardIterator>::value_type>().document())
	{
		using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

		auto delegate = std::make_shared<document_builder<CharT>>();
		reader r { delegate };
		reader_stream<ForwardIterator> ris { std::move(first), std::move(last) };
		
		if (! r.parse_document(ris))
			return { value_type::Null };
		
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