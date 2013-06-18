// document.hpp - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __KRYSTAL_DOCUMENT__
#define __KRYSTAL_DOCUMENT__

#include <iosfwd>
#include <memory>
#include "reader2.hpp"

namespace krystal {

	namespace { const std::string DOC_ROOT_KEY {"___DOCUMENT___"}; }

	class document_builder : public reader_delegate {
		value root_;
		std::vector<value*> context_stack_;
		std::string next_key_;
		
		friend class reader;
		
		void append(value val) {
			auto cur_node = context_stack_.back();
			bool val_is_container = val.is_container();
			
			if (cur_node->is_object()) {
				cur_node->insert(next_key_, std::move(val));
				if (val_is_container) {
					auto mv = &(*cur_node)[next_key_];
					context_stack_.push_back(mv);
				}
				next_key_.clear();
			}
			else { // array
				cur_node->push_back(std::move(val));
				if (val_is_container) {
					auto mv = &(*cur_node)[cur_node->size()-1];
					context_stack_.push_back(mv);
				}
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
			append(value{ num });
		}
		
		void string_value(const std::string& str) {
			if (context_stack_.back()->is_array() || next_key_.size())
				append(str);
			else
				next_key_ = str;
		}
		
		void array_begin() {
			append({ value_type::Array });
		}
		
		void array_end() {
			context_stack_.pop_back();
		}
		
		void object_begin() {
			append({ value_type::Object });
		}
		
		void object_end() {
			context_stack_.pop_back();
		}
		
		void error(const std::string& msg, ptrdiff_t offset) {
			std::cout << "ERROR at position " << offset << ": " << msg << '\n';
		}

	public:
		document_builder()
		: root_{ value_type::Object }
		, next_key_{ DOC_ROOT_KEY }
		{
			context_stack_.push_back(&root_);
		}
		
		value document() {
			// the document_builder instance is useless after the call to document()
			return std::move(root_[DOC_ROOT_KEY]);
		}
	};


	template <typename ForwardIterator>
	value parse(ForwardIterator first, ForwardIterator last) {
		auto delegate = std::make_shared<document_builder>();
		reader r { delegate };
		reader_stream<ForwardIterator> ris { std::move(first), std::move(last) };
		
		if (! r.parse_document(ris))
			return { value_type::Null };
		
		return delegate->document();
	}
	
	template <typename IStream>
	value parse_stream(IStream &is) {
		is >> std::noskipws;
		std::istream_iterator<typename IStream::char_type> first{is};
		return parse(first, {});
	}

	template <typename Char>
	value parse_string(std::basic_string<Char> json_string) {
		return parse(begin(json_string), end(json_string));
	}

}

#endif