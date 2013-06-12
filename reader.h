//  reader.h - part of Krystal
//  (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __krystal__reader__
#define __krystal__reader__

#include <memory>

namespace krystal {

	class reader_delegate {
	public:
		virtual ~reader_delegate() {}
		
		virtual void null_value() = 0;
		virtual void false_value() = 0;
		virtual void true_value() = 0;
		virtual void number_value(double) = 0;
		virtual void string_value(const std::string&) = 0;
		
		virtual void array_begin() = 0;
		virtual void array_end() = 0;
		
		virtual void object_begin() = 0;
		virtual void object_end() = 0;
		
		virtual void error(const std::string&, std::istream&) = 0;
	};
	
	
	class reader {
		bool error_occurred = false;
		std::shared_ptr<reader_delegate> delegate_;
		
		void skip_white(std::istream& is);
		void parse_literal(std::istream& is);
		void parse_number(std::istream& is);
		void parse_string(std::istream& is);
		void parse_array(std::istream& is);
		void parse_object(std::istream& is);
		void parse_value(std::istream& is);
		
		void error(const std::string& msg, std::istream& is);
		
	public:
		reader(std::shared_ptr<reader_delegate> delegate) : delegate_{ delegate } {}
		
		bool parseDocument(std::istream& is);
	};
	
}

#endif
