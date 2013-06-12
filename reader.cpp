//  reader.cpp - part of Krystal
//  (c) 2013 by Arthur Langereis (@zenmumbler)

#include <iostream>
#include <sstream>
#include "reader.h"

namespace krystal {
	void reader::skip_white(std::istream& is) {
		while (is && std::isspace(is.peek()))
			is.get();
	}

	
	void reader::parse_literal(std::istream& is) {
		static std::string null_tok {"null"}, true_tok{"true"}, false_tok{"false"};

		std::string token;
		is >> token;
		if (token == true_tok)
			delegate_->true_value();
		else if (token == false_tok)
			delegate_->false_value();
		else if (token == null_tok)
			delegate_->null_value();
		else
			error("Expected value but found `" + token + "`.", is);
	}

	
	void reader::parse_number(std::istream& is) {
		double val;
		is >> val;
		delegate_->number_value(val);
	}

	
	void reader::parse_string(std::istream& is) {
		is.get(); // opening "
		
		std::ostringstream ss;
		while (is) {
			auto ch = is.get();
			if (ch == '"')
				break;
			else
				ss.put(ch);
		}
		
		if (! is) {
			error("Unexpected EOF while parsing string.", is);
			return;
		}
		
		delegate_->string_value(ss.str());
	}

	
	void reader::parse_array(std::istream& is) {
		is.get(); // opening [
		skip_white(is);
		delegate_->array_begin();
		
		auto ch = is.peek();
		while (ch != ']') {
			parse_value(is);
			skip_white(is);
			if (error_occurred)
				return;

			ch = is.peek();
			if (ch == ',') {
				is.get();
				skip_white(is);
			}
			else if (ch != ']') {
				error("Expected `,` or `]` but found `" + std::string{static_cast<char>(ch)} + "`.", is);
				return;
			}
		}

		if (! is) {
			error("Unexpected EOF while parsing array.", is);
			return;
		}

		is.get(); // closing ]
		skip_white(is);
		delegate_->array_end();
	}

	
	void reader::parse_object(std::istream& is) {
		is.get(); // opening {
		skip_white(is);
		delegate_->object_begin();

		auto ch = is.peek();
		while (!error_occurred && is && ch != '}') {
			parse_string(is); // key
			skip_white(is);
			if (is.get() != ':') {
				error("Expected `:` but found `" + std::string{static_cast<char>(ch)} + "`.", is);
				return;
			}
			skip_white(is);
			parse_value(is);  // value
			skip_white(is);
			if (error_occurred)
				return;

			ch = is.peek();
			if (ch == ',') {
				is.get();
				skip_white(is);
			}
			else if (ch != '}') {
				error("Expected `,` or `}` but found `" + std::string{static_cast<char>(ch)} + "`.", is);
				return;
			}
		}
		
		if (! is) {
			error("Unexpected EOF while parsing object.", is);
			return;
		}
		
		is.get(); // closing }
		skip_white(is);
		delegate_->object_end();
	}
	
	
	void reader::parse_value(std::istream& is) {
		if (error_occurred) return;

		// The order of tests in this function is based on a quick
		// check of frequency of types of values in typical JSON documents.
		// String and numbers are by far the most prevalent, followed
		// closely by objects, then arrays. Bools and Nulls are relatively uncommon.
		
		auto ch = is.peek();
		if ((ch >= '0' && ch <= '9') || ch == '-')
			parse_number(is);
		else {
			switch (ch) {
				case '"':
					parse_string(is); break;
				case '{':
					parse_object(is); break;
				case '[':
					parse_array(is); break;
				case 'n': case 't': case 'f':
					parse_literal(is); break;
				default:
					error("Expected a value but found `" + std::string{static_cast<char>(ch)} + "`.", is);
					break;
			}
		}
	}
	
	
	void reader::error(const std::string& msg, std::istream& is) {
		error_occurred = true;
		delegate_->error(msg, is);
	}

	
	bool reader::parseDocument(std::istream& is) {
		skip_white(is);
		
		switch (is.peek()) {
			case '{': parse_object(is); break;
			case '[': parse_array(is); break;
				
			default:
				error("Document must be an array or object.", is);
				break;
		}
		
		skip_white(is);
		auto ch = is.get();
		if (is)
			error("Unexpected data found after end of document: `" + std::string{static_cast<char>(ch)} + "`.", is);
		
		return !error_occurred;
	}
}
