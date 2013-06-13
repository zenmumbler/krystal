//  krystal_test.cpp - part of krystal
//  (c) 2013 by Arthur Langereis (@zenmumbler)

#include <fstream>
#include <iostream>

#include "reader.h"
#include "util.h"

int main() {
	auto file = std::ifstream("mydata.json");
	auto doc = krystal::parse(file);
	
	for (int i=0; i<doc.size(); ++i)
		std::cout << doc[i]["title"] << '\n';

//	auto doc2 = krystal::value(krystal::value_type::Array);
//	doc2.push_back("This is a string");
//	doc2.push_back(krystal::value(0));
//	doc2.push_back(krystal::value(true));
//	
//	auto doc3 = krystal::value(krystal::value_type::Object);
//	doc3.insert("some_key", std::move(doc2));
//	
//	std::cout << "doc2: " << doc2 << '\n'; // null
//	std::cout << "doc3[some_key][0] " << doc3["some_key"][0] << '\n';
}
