// util.h - part of krystal
// (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __krystal_document__
#define __krystal_document__

#include "value.hpp"
#include "reader.hpp"

namespace krystal {
	
	class document_builder : public reader_delegate {
		value root_;
		std::vector<value*> context_stack_;
		std::string next_key_;
		
		friend class reader;
		
		virtual void null_value() override;
		virtual void false_value() override;
		virtual void true_value() override;
		virtual void number_value(double) override;
		virtual void string_value(const std::string&) override;
		
		virtual void array_begin() override;
		virtual void array_end() override;
		
		virtual void object_begin() override;
		virtual void object_end() override;
		
		virtual void error(const std::string&, std::istream& is) override;
		
		void append(value val);
		
	public:
		document_builder();
		value document();
	};

	
	value parse(std::istream& json_stream);
	value parse(std::string json_string);
}

#endif
