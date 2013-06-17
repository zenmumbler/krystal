//  reader.cpp - part of Krystal
//  (c) 2013 by Arthur Langereis (@zenmumbler)

#include <iostream>
#include <sstream>
#include "reader.hpp"

namespace krystal {
	void reader::skip_white(std::istream& is) {
		while (is && std::isspace(is.peek()))
			is.get();
	}

	
	void reader::parse_literal(std::istream& is) {
		static std::string null_tok {"null"}, true_tok{"true"}, false_tok{"false"};

		std::ostringstream token;
		auto ch = is.peek();
		
		while (ch >= 'a' && ch <= 'z') {
			token << (char)is.get();
			ch = is.peek();
		}
		
		auto token_str = token.str();
		
		if (token_str == true_tok)
			delegate_->true_value();
		else if (token_str == false_tok)
			delegate_->false_value();
		else if (token_str == null_tok)
			delegate_->null_value();
		else
			error("Expected value but found `" + token_str + "`.", is);
	}

	
	void reader::parse_number(std::istream& is) {
		std::stringstream token;
		decltype(is.peek()) ch;
		
		auto munch = [&]{ token << (char)is.get(); ch = is.peek(); };
		
		ch = is.peek();
		if (ch == '-')
			munch();
		
		ch = is.peek();
		if (ch == '0')
			munch();
		else {
			if (ch < '0' || ch > '9') {
				error("The integer part of a number must have at least 1 digit", is);
				return;
			}
			do {
				munch();
			} while (ch >= '0' && ch <= '9');
		}
		
		if (ch == '.') {
			munch();
			
			if (ch < '0' || ch > '9') {
				error("The fraction part of a number must have at least 1 digit", is);
				return;
			}
			do {
				munch();
			} while (ch >= '0' && ch <= '9');
		}
		
		if (ch == 'e' || ch == 'E') {
			munch();
			if (ch == '+' || ch == '-')
				munch();
			if (ch < '0' || ch > '9') {
				error("The exponent part of a number must have at least 1 digit", is);
				return;
			}
			do {
				munch();
			} while (ch >= '0' && ch <= '9');
		}
		
		double val;
		token >> val;

		delegate_->number_value(val);
	}

	
	void reader::parse_string(std::istream& is) {
		std::ostringstream ss;
		
		auto unicode_hex_number = [&]{
			int digits = 4, number = 0;

			while (digits-- > 0 && is) {
				auto ch = is.get();
				number <<= 4;

				if (ch >= '0' && ch <= '9')
					number += static_cast<int>(ch - '0');
				else if (ch >= 'a' && ch <= 'f')
					number += 10 + static_cast<int>(ch - 'a');
				else if (ch >= 'A' && ch <= 'F')
					number += 10 + static_cast<int>(ch - 'A');
				else {
					error("Invalid hexadecimal character in unicode hex literal: `" + std::string{static_cast<char>(ch)} + '`', is);
					return 0;
				}
			}
			
			return number;
		};
		
		auto unicode_literal = [&]{
			auto codepoint = unicode_hex_number();
			if (error_occurred) return 0;

			if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
				if (is.get() != '\\' || is.get() != 'u') {
					error("Expected second half of UTF-16 surrogate pair.", is);
					return 0;
				}

				auto pairpoint = unicode_hex_number();
				if (error_occurred) return 0;

				if (pairpoint < 0xDC00 || pairpoint > 0xDFFF) {
					error("Second half of UTF-16 surrogate pair is invalid.", is);
					return 0;
				}
				codepoint = (((codepoint - 0xD800) << 10) | (pairpoint - 0xDC00)) + 0x10000;
			}

			return codepoint;
		};

		auto write_codepoint_as_utf8 = [](std::ostream& os, int codepoint) {
			if (codepoint <= 0x7F)
				os.put(codepoint & 0x7F);
			else if (codepoint <= 0x7FF) {
				os.put(0xC0 | ((codepoint >> 6) & 0xFF));
				os.put(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0xFFFF) {
				os.put(0xE0 | ((codepoint >> 12) & 0xFF));
				os.put(0x80 | ((codepoint >> 6) & 0x3F));
				os.put(0x80 | (codepoint & 0x3F));
			}
			else {
				os.put(0xF0 | ((codepoint >> 18) & 0xFF));
				os.put(0x80 | ((codepoint >> 12) & 0x3F));
				os.put(0x80 | ((codepoint >> 6) & 0x3F));
				os.put(0x80 | (codepoint & 0x3F));
			}
		};
		
		// opening "
		if (is.get() != '"') {
			error("Expected opening quote for string.", is);
			return;
		}
			
		while (is) {
			auto ch = is.get();
			if (ch == '"')
				break;
			else if (ch == '\\') {
				ch = is.get();
				switch (ch) {
					case '"': case '\\': case '/': ss.put(ch); break;
					case 'n': ss.put('\n'); break;
					case 'r': ss.put('\r'); break;
					case 't': ss.put('\t'); break;
					case 'b': ss.put('\b'); break;
					case 'f': ss.put('\f'); break;
					case 'u':
						write_codepoint_as_utf8(ss, unicode_literal());
						if (error_occurred) return;
						break;
						
					default:
						error("Invalid escape sequence character: `" + std::string{static_cast<char>(ch)} + '`', is);
						return;
				}
			}
			else {
				if (ch >= 0x20)
					ss.put(ch);
				else {
					error("Encountered an unescaped control character #" + std::to_string(ch), is);
					return;
				}
			}
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
			if (error_occurred)
				return;

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
		
		if (! error_occurred) {
			skip_white(is);
			auto ch = is.get();
			if (is)
				error("Unexpected data found after end of document: `" + std::string{static_cast<char>(ch)} + "`.", is);
		}
		
		return !error_occurred;
	}
}
