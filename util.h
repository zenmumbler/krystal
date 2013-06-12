// krystal.h
// part of Krystal JSON Reader
// (c) 2013 by Arthur Langereis (@zenmumbler)

#ifndef __krystal_util__
#define __krystal_util__

#include "reader.h"
#include "value.h"

namespace krystal {
	
	class document_builder : public reader_delegate {
		std::shared_ptr<value> root_;
		
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
		
	public:
		std::shared_ptr<value> rootValue();
	};

	
	std::shared_ptr<value> parse(std::istream& is);
}

#endif
