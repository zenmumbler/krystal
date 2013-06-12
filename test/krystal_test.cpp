//
//  krystal_test.cpp
//  Krystal
//
//  Created by Arthur Langereis on 2013/6/9.
//
//

#include <fstream>
#include <iostream>

#include "reader.h"
#include "util.h"

int main() {
	auto file = std::ifstream("mydata.json");
	auto doc = krystal::parse(file);

//	std::cout << "value: " << *doc;
}
