// reader.hpp - part of krystal
// (c) 2013-4 by Arthur Langereis

#ifndef KRYSTAL_READER_H
#define KRYSTAL_READER_H

#include "value.hpp"

#include <cmath>
#include <iosfwd>
#include <array>
#include <algorithm>

namespace krystal {

	
class ReaderDelegate {
public:
	virtual ~ReaderDelegate() = default;
	
	virtual void nullValue() = 0;
	virtual void falseValue() = 0;
	virtual void trueValue() = 0;
	virtual void numberValue(double) = 0;
	virtual void stringValue(const std::string&) = 0;
	
	virtual void arrayBegin() = 0;
	virtual void arrayEnd() = 0;
	
	virtual void objectBegin() = 0;
	virtual void objectEnd() = 0;
	
	virtual void error(const std::string&, ptrdiff_t offset) = 0;
};



template <typename ForwardIterator>
class ReaderStream {
	ReaderStream(const ReaderStream<ForwardIterator>& rhs) = delete;

public:
	// std stream typedefs
	using char_type = typename std::iterator_traits<ForwardIterator>::value_type;
	using int_type = typename std::char_traits<char_type>::int_type;
	using difference_type = typename std::iterator_traits<ForwardIterator>::difference_type;

	ReaderStream(ForwardIterator first, ForwardIterator last)
	: first_{first}, last_{last}, offset_{0}, nextChar_{-1}, eof_{ first_ == last_ }
	{
		if (first_ != last_)
			nextChar_ = *first_;
	}

	int_type peek() const {
		return nextChar_;
	}
	
	int_type get() {
		if (first_ != last_) {
			auto ch = nextChar_;
			++offset_;
			if (++first_ != last_)
				nextChar_ = static_cast<unsigned char>(*first_);
			else
				nextChar_ = std::char_traits<char_type>::eof();
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
	int_type nextChar_;
	bool eof_;
};



class Reader {
	ReaderDelegate& delegate_;
	bool errorOccurred = false;
	std::string nullToken {"null"}, trueToken{"true"}, falseToken{"false"};

public:
	Reader(ReaderDelegate& delegate) : delegate_{ delegate } {}

	template <typename ForwardIterator>
	void skipWhite(ReaderStream<ForwardIterator>& is) {
		while (is.good() && std::isspace(is.peek()))
			is.get();
	}
	
	
	template <typename ForwardIterator>
	void parseLiteral(ReaderStream<ForwardIterator>& is) {
		auto token_data = std::vector<char>(size_t(6), '\0');
		auto token = token_data.begin(), token_end = token_data.end();
		auto ch = is.peek();
		
		while (ch >= 'a' && ch <= 'z' && token != token_end) {
			*token++ = (char)is.get();
			ch = is.peek();
		}
		
		auto token_str = std::string{ token_data.begin(), token };
		
		if (token_str == trueToken)
			delegate_.trueValue();
		else if (token_str == falseToken)
			delegate_.falseValue();
		else if (token_str == nullToken)
			delegate_.nullValue();
		else
			error("Expected value but found `" + token_str + "`.", is);
	}


	double pow10(int n) {
		static const auto p10_s = ([]{
			std::array<double, 617> t;
			int k = -309;
			std::generate_n(begin(t), t.size(), [&k]{ return std::pow(10.0, ++k); });
			return std::move(t);
		}());
		
		return n < -308 ? 0.0 : (n > 308 ? std::pow(10.0, n) : p10_s[n + 308]);
	}

	template<typename ForwardIterator>
	void parseNumber(ReaderStream<ForwardIterator>& is) {
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

			frac_part *= pow10(-frac_digits);
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
		if (exp_part != 0) {
			if (exp_minus) exp_part = -exp_part;
			val *= pow10(exp_part);
		}
		if (minus)
			val = -val;
		
		delegate_.numberValue(val);
	}


	template <typename ForwardIterator>
	void parseString(ReaderStream<ForwardIterator>& is) {
		static std::vector<char> ss;
		ss.clear();
		
		auto parseUTF16CodeUnit = [&]{
			int digits = 4;
			uint32_t codeUnit = 0;
			
			while (digits-- > 0 && is.good()) {
				auto ch = is.get();
				codeUnit <<= 4;
				
				if (ch >= '0' && ch <= '9')
					codeUnit += static_cast<int>(ch - '0');
				else if (ch >= 'a' && ch <= 'f')
					codeUnit += 10 + static_cast<int>(ch - 'a');
				else if (ch >= 'A' && ch <= 'F')
					codeUnit += 10 + static_cast<int>(ch - 'A');
				else {
					error("Invalid hexadecimal character in unicode hex literal: `" + std::string{static_cast<char>(ch)} + '`', is);
					return 0u;
				}
			}
			
			return codeUnit;
		};
		
		auto parseUTF16CodePoint = [&]{
			auto codePoint = parseUTF16CodeUnit();
			if (errorOccurred)
				return 0u;
			
			if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
				if (is.get() != '\\' || is.get() != 'u') {
					error("Expected second half of UTF-16 surrogate pair.", is);
					return 0u;
				}
				
				auto pairUnit = parseUTF16CodeUnit();
				if (errorOccurred)
					return 0u;
				
				if (pairUnit < 0xDC00 || pairUnit > 0xDFFF) {
					error("Second half of UTF-16 surrogate pair is invalid.", is);
					return 0u;
				}

				// convert UTF16 surrogate pair to UTF32 codepoint
				codePoint = (((codePoint - 0xD800) << 10) | (pairUnit - 0xDC00)) + 0x10000;
			}
			
			return codePoint;
		};
		
		auto writeCodePointAsUTF8 = [](std::vector<char>& sb, uint32_t codePoint) {
			if (codePoint <= 0x7F)
				sb.push_back(codePoint & 0x7F);
			else if (codePoint <= 0x7FF) {
				sb.push_back(0xC0 | ((codePoint >> 6) & 0xFF));
				sb.push_back(0x80 | (codePoint & 0x3F));
			}
			else if (codePoint <= 0xFFFF) {
				sb.push_back(0xE0 | ((codePoint >> 12) & 0xFF));
				sb.push_back(0x80 | ((codePoint >> 6) & 0x3F));
				sb.push_back(0x80 | (codePoint & 0x3F));
			}
			else {
				sb.push_back(0xF0 | ((codePoint >> 18) & 0xFF));
				sb.push_back(0x80 | ((codePoint >> 12) & 0x3F));
				sb.push_back(0x80 | ((codePoint >> 6) & 0x3F));
				sb.push_back(0x80 | (codePoint & 0x3F));
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
					case '"': case '\\': case '/': ss.push_back(ch); break;
					case 'n': ss.push_back('\n'); break;
					case 'r': ss.push_back('\r'); break;
					case 't': ss.push_back('\t'); break;
					case 'b': ss.push_back('\b'); break;
					case 'f': ss.push_back('\f'); break;
					case 'u':
						writeCodePointAsUTF8(ss, parseUTF16CodePoint());
						if (errorOccurred) return;
						break;
						
					default:
						error("Invalid escape sequence character: `" + std::string{static_cast<char>(ch)} + '`', is);
						return;
				}
			}
			else {
				if (ch >= 0x20)
					ss.push_back(ch);
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
		
		delegate_.stringValue({ begin(ss), end(ss) });
	}


	template <typename ForwardIterator>
	void parseArray(ReaderStream<ForwardIterator>& is) {
		is.get(); // opening [
		skipWhite(is);
		delegate_.arrayBegin();
		
		auto ch = is.peek();
		while (ch != ']') {
			parseValue(is);
			skipWhite(is);
			if (errorOccurred)
				return;
			
			ch = is.peek();
			if (ch == ',') {
				is.get();
				skipWhite(is);
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
		skipWhite(is);
		delegate_.arrayEnd();
	}


	template <typename ForwardIterator>
	void parseObject(ReaderStream<ForwardIterator>& is) {
		is.get(); // opening {
		skipWhite(is);
		delegate_.objectBegin();
		
		auto ch = is.peek();
		while (!errorOccurred && is.good() && ch != '}') {
			parseString(is); // key
			skipWhite(is);
			if (errorOccurred)
				return;
			
			if (is.get() != ':') {
				error("Expected `:` but found `" + std::string{static_cast<char>(ch)} + "`.", is);
				return;
			}
			skipWhite(is);
			parseValue(is);  // value
			skipWhite(is);
			if (errorOccurred)
				return;
			
			ch = is.peek();
			if (ch == ',') {
				is.get();
				skipWhite(is);
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
		skipWhite(is);
		delegate_.objectEnd();
	}


	template <typename ForwardIterator>
	void parseValue(ReaderStream<ForwardIterator>& is) {
		if (errorOccurred) return;
		
		// The order of tests in this function is based on a quick
		// check of frequency of types of values in typical JSON documents.
		// Strings and numbers are by far the most prevalent, followed
		// closely by objects, then arrays. Bools and Nulls are relatively uncommon.
		
		auto ch = is.peek();
		if ((ch >= '0' && ch <= '9') || ch == '-')
			parseNumber(is);
		else {
			switch (ch) {
				case '"':
					parseString(is); break;
				case '{':
					parseObject(is); break;
				case '[':
					parseArray(is); break;
				case 'n': case 't': case 'f':
					parseLiteral(is); break;
				default:
					error("Expected a value but found `" + std::string{static_cast<char>(ch)} + "`.", is);
					break;
			}
		}
	}


	template <typename ForwardIterator>
	void error(const std::string& msg, ReaderStream<ForwardIterator>& is) {
		errorOccurred = true;
		delegate_.error(msg, is.tellg());
	}


	template <typename ForwardIterator>
	bool parseDocument(ReaderStream<ForwardIterator>& is) {
		skipWhite(is);
		
		switch (is.peek()) {
			case '{': parseObject(is); break;
			case '[': parseArray(is); break;
				
			default:
				error("Document must be an array or object.", is);
				break;
		}
		
		if (! errorOccurred) {
			skipWhite(is);
			auto ch = is.get();
			if (! is.eof())
				error("Unexpected data found after end of document: `" + std::string{static_cast<char>(ch)} + "`.", is);
		}
		
		return ! errorOccurred;
	}
};


} // ns krystal

#endif
