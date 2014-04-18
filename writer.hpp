// writer - part of krystal
// (c) 2014 by Arthur Langereis (@zenmumbler)

#ifndef __KRYSTAL_WRITER__
#define __KRYSTAL_WRITER__

#include <vector>
#include <memory>
#include <iterator>
#include "reader.hpp"


namespace krystal {



template <typename CharT, template<typename T> class Allocator>
void traverseDepthFirst(const basic_value<CharT, Allocator>& val, ) {
	
}



template <typename ValueClass, typename OutputIterator>
void write(const ValueClass& val, OutputIterator& stream) {
	
}


template <typename ValueClass>
std::basic_string<typename ValueClass::char_type>
stringify(const ValueClass& val) {
	std::vector<typename ValueClass::char_type> json;
	// TODO: reserve guesstimate min size of bytes for vector

	write(val, std::back_inserter(json));

	return { json.begin(), json.end() };
}


} // namespace krystal


#endif
