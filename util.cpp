// util.cpp - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#include <iostream>
#include "util.h"

namespace krystal {
	
	const static std::string DOC_ROOT_KEY {"___DOCUMENT___"};

	
	document_builder::document_builder()
	: root_{ value_type::Object }
	, next_key_{ DOC_ROOT_KEY }
	{
		context_stack_.push_back(&root_);
	}
	
	void document_builder::append(value val) {
		auto cur_node = context_stack_.back();
		bool val_is_container = val.is_container();

		if (cur_node->is_object()) {
			cur_node->insert(next_key_, std::move(val));
			if (val_is_container) {
				auto mv = const_cast<value*>(&(*cur_node)[next_key_]);
				context_stack_.push_back(mv);
			}
			next_key_.clear();
		}
		else { // array
			cur_node->push_back(std::move(val));
			if (val_is_container) {
				auto mv = const_cast<value*>(&(*cur_node)[cur_node->size()-1]);
				context_stack_.push_back(mv);
			}
		}
	}
	
	void document_builder::null_value() {
		std::cout << "null "<< '\n';
		append({ value_type::Null });
	}

	void document_builder::false_value() {
		std::cout << "false "<< '\n';
		append({ value_type::False });
	}

	void document_builder::true_value() {
		std::cout << "true "<< '\n';
		append({ value_type::True });
	}

	void document_builder::number_value(double num) {
		std::cout << "N: " << num << '\n';
		append(value{ num });
	}

	void document_builder::string_value(const std::string& str) {
		std::cout << "S: " << str << '\n';
		if (next_key_.size())
			append(str);
		else
			next_key_ = str;
	}
	
	void document_builder::array_begin() {
		std::cout << "[\n";
		append({ value_type::Array });
	}

	void document_builder::array_end() {
		std::cout << "]\n";
		context_stack_.pop_back();
	}
	
	void document_builder::object_begin() {
		std::cout << "{\n";
		append({ value_type::Object });
	}

	void document_builder::object_end() {
		std::cout << "}\n";
		context_stack_.pop_back();
	}
	
	void document_builder::error(const std::string& msg, std::istream& is) {
		std::cout << "ERROR! " << msg << '\n';
	}
	
	value document_builder::document() {
		// the document_builder instance is useless after the call to document()
		return std::move(root_[DOC_ROOT_KEY]);
	}
	
	
	value parse(std::istream& is) {
		auto delegate = std::make_shared<document_builder>();
		reader r { delegate };
		if (! r.parseDocument(is))
			return { value_type::Null };
		
		return delegate->document();
	}
}
