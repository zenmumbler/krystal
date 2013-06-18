// reader.hpp - part of krystal
// (c) 2013 by Arthur Langereis

#ifndef __KRYSTAL_READER__
#define __KRYSTAL_READER__

#include <iosfwd>
#include <sstream>
#include "value.hpp"

namespace krystal {
	
	class reader_delegate {
	public:
		virtual ~reader_delegate() = default;
		
		virtual void null_value() = 0;
		virtual void false_value() = 0;
		virtual void true_value() = 0;
		virtual void number_value(double) = 0;
		virtual void string_value(const std::string&) = 0;
		
		virtual void array_begin() = 0;
		virtual void array_end() = 0;
		
		virtual void object_begin() = 0;
		virtual void object_end() = 0;
		
		virtual void error(const std::string&, ptrdiff_t offset) = 0;
	};



	template <typename ForwardIterator>
	class reader_stream {
		reader_stream(const reader_stream<ForwardIterator>& rhs)=delete;
	public:
		using char_type = typename std::iterator_traits<ForwardIterator>::value_type;
		using int_type = typename std::char_traits<char_type>::int_type;
		using difference_type = typename std::iterator_traits<ForwardIterator>::difference_type;
		
		reader_stream(ForwardIterator first, ForwardIterator last)
		: first_{first}, last_{last}, offset_{0}, next_char{-1}, eof_{ first_ == last_ }
		{
			if (first_ != last_)
				next_char = *first_;
		}
	
		int_type peek() const {
			return next_char;
		}
		
		int_type get() {
			if (first_ != last_) {
				auto ch = next_char;
				++offset_;
				if (++first_ != last_)
					next_char = static_cast<unsigned char>(*first_);
				else
					next_char = std::char_traits<char_type>::eof();
				return ch;
			}

			eof_ = true;
			return std::char_traits<char_type>::eof();
		}
		
		difference_type tellg() const { return offset_; }
		bool good() const { return !eof_; }
		bool eof() const { return eof_; }
	
	private:
		ForwardIterator first_, last_;
		difference_type offset_;
		int_type next_char;
		bool eof_;
	};



	class reader {
		std::shared_ptr<reader_delegate> delegate_;
		bool error_occurred = false;

	public:
		reader(std::shared_ptr<reader_delegate> delegate) : delegate_{ delegate } {}

		template <typename ForwardIterator>
		void skip_white(reader_stream<ForwardIterator>& is) {
			while (is.good() && std::isspace(is.peek()))
				is.get();
		}
		
		
		template <typename ForwardIterator>
		void parse_literal(reader_stream<ForwardIterator>& is) {
			static std::string null_tok {"null"}, true_tok{"true"}, false_tok{"false"};
			
			static std::ostringstream token;
			auto ch = is.peek();
			
			while (ch >= 'a' && ch <= 'z') {
				token << (char)is.get();
				ch = is.peek();
			}
			
			auto token_str = token.str();
			token.str({});
			
			if (token_str == true_tok)
				delegate_->true_value();
			else if (token_str == false_tok)
				delegate_->false_value();
			else if (token_str == null_tok)
				delegate_->null_value();
			else
				error("Expected value but found `" + token_str + "`.", is);
		}


		template<typename ForwardIterator>
		void parse_number(reader_stream<ForwardIterator>& is) {
			decltype(is.peek()) ch;
			
			auto munch = [&]{ is.get(); ch = is.peek(); };
			bool minus = false, exp_minus = false;
			double int_part = 0.0, frac_part = 0.0;
			int exp_part = 0, frac_digits = 0;
			
			ch = is.peek();
			if (ch == '-') {
				minus = true;
				munch();
			}
			
			ch = is.peek();
			if (ch == '0')
				munch();
			else {
				if (ch < '0' || ch > '9') {
					error("The integer part of a number must have at least 1 digit", is);
					return;
				}
				do {
					int_part = (10.0 * int_part) + static_cast<double>(ch - '0');
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
					frac_part = (10.0 * frac_part) + static_cast<double>(ch - '0');
					++frac_digits;
					munch();
				} while (ch >= '0' && ch <= '9');

				frac_part *= std::pow(10.0, -frac_digits);
			}
			
			if (ch == 'e' || ch == 'E') {
				munch();
				if (ch == '-') {
					exp_minus = true;
					munch();
				}
				else if (ch == '+')
					munch();
				if (ch < '0' || ch > '9') {
					error("The exponent part of a number must have at least 1 digit", is);
					return;
				}
				do {
					exp_part = (10 * exp_part) + static_cast<int>(ch - '0');
					munch();
				} while (ch >= '0' && ch <= '9');
			}

			double val = int_part + frac_part;
			if (exp_part != 0.0) {
				if (exp_minus) exp_part = -exp_part;
				val *= std::pow(10.0, exp_part);
			}
			if (minus)
				val = -val;
			
			delegate_->number_value(val);
		}


		template <typename ForwardIterator>
		void parse_string(reader_stream<ForwardIterator>& is) {
			static std::ostringstream ss;
			ss.str({});
			
			auto unicode_hex_number = [&]{
				int digits = 4, number = 0;
				
				while (digits-- > 0 && is.good()) {
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
			
			while (is.good()) {
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
			
			if (is.eof()) {
				error("Unexpected EOF while parsing string.", is);
				return;
			}
			
			delegate_->string_value(ss.str());
		}


		template <typename ForwardIterator>
		void parse_array(reader_stream<ForwardIterator>& is) {
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
			
			if (is.eof()) {
				error("Unexpected EOF while parsing array.", is);
				return;
			}
			
			is.get(); // closing ]
			skip_white(is);
			delegate_->array_end();
		}


		template <typename ForwardIterator>
		void parse_object(reader_stream<ForwardIterator>& is) {
			is.get(); // opening {
			skip_white(is);
			delegate_->object_begin();
			
			auto ch = is.peek();
			while (!error_occurred && is.good() && ch != '}') {
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
				else if (is.eof()) {
					error("Unexpected EOF while parsing object.", is);
					return;
				}
				else if (ch != '}') {
					error("Expected `,` or `}` but found `" + std::string{static_cast<char>(ch)} + "`.", is);
					return;
				}
			}
			
			
			is.get(); // closing }
			skip_white(is);
			delegate_->object_end();
		}


		template <typename ForwardIterator>
		void parse_value(reader_stream<ForwardIterator>& is) {
			if (error_occurred) return;
			
			// The order of tests in this function is based on a quick
			// check of frequency of types of values in typical JSON documents.
			// Strings and numbers are by far the most prevalent, followed
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


		template <typename ForwardIterator>
		void error(const std::string& msg, reader_stream<ForwardIterator>& is) {
			error_occurred = true;
			delegate_->error(msg, is.tellg());
		}


		template <typename ForwardIterator>
		bool parse_document(reader_stream<ForwardIterator>& is) {
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
				if (! is.eof())
					error("Unexpected data found after end of document: `" + std::string{static_cast<char>(ch)} + "`.", is);
			}
			
			return !error_occurred;
		}
	};
}

#endif