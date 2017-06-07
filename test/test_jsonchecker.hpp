// test_jsonchecker.cpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_jsonchecker() {
	group("jsonchecker tests", []{
	
		test("faulty test data should not parse", []{
			const int faulty_tests = 33;
			
			for (int tix=1; tix <= faulty_tests; ++tix) {
				if (tix == 18) // test 18 tests for limited nesting, which does not apply to krystal
					continue;
				
				std::ifstream fail_file{ "jsonchecker/fail" + toString(tix) + ".json" };
				auto val = krystal::parseStream(fail_file);
				checkEqual(val.type(), ValueKind::Null);
			}
		});

		test("valid test data should yield no errors", []{
			const int valid_tests = 3;
			
			for (int tix=1; tix <= valid_tests; ++tix) {
				std::ifstream pass_file{ "jsonchecker/pass" + toString(tix) + ".json" };
				auto val = krystal::parseStream(pass_file);
				checkTrue(val.isContainer());
			}
		});
		
		test("pass1.json document parse should yield exact document equivalent", []{
			std::ifstream pass_file{ "jsonchecker/pass1.json" };
			auto doc = krystal::parseStream(pass_file);
			using BasicValue = decltype(doc)::ValueType;

			auto check_type_and_str = [](const BasicValue& val, const std::string& sval) {
				return checkEqual(val.type(), ValueKind::String) && checkEqual(val.string(), sval);
			};
			auto check_type_and_num = [](const BasicValue& val, double nval) {
				// FIXME: move this to Inquisition itself
				auto equal_double = [](double a, double b) {
					return (std::abs(a-b) < 2.0 * std::numeric_limits<double>::epsilon() || // For small a, b
					        std::abs(a-b) < std::min(a, b) / 1.0e15); // for large a, b
				};
				
				return checkEqual(val.type(), ValueKind::Number) && checkTrue(equal_double(val.number(), nval));
			};
			auto check_type_and_size = [](const BasicValue& val, ValueKind type, size_t size) {
				return checkEqual(val.type(), type) && checkEqual(val.size(), size);
			};
			auto check_has_key = [](const BasicValue&val, const std::string& key) {
				return checkTrue(val.contains(key));
			};
			
			if (checkTrue(doc.isArray()) && checkEqual(doc.size(), 20)) {
				check_type_and_str(doc[0], "JSON Test Pattern pass1");
				check_type_and_size(doc[1], ValueKind::Object, 1);
				check_type_and_size(doc[2], ValueKind::Object, 0);
				check_type_and_size(doc[3], ValueKind::Array, 0);
				check_type_and_num(doc[4], -42);
				checkEqual(doc[5].type(), ValueKind::True);
				checkEqual(doc[6].type(), ValueKind::False);
				checkEqual(doc[7].type(), ValueKind::Null);
				check_type_and_size(doc[8], ValueKind::Object, 32);
				check_type_and_num(doc[9], 0.5);
				check_type_and_num(doc[10], 98.6);
				check_type_and_num(doc[11], 99.44);
				check_type_and_num(doc[12], 1066);
				check_type_and_num(doc[13], 1e1);
				check_type_and_num(doc[14], 0.1e1);
				check_type_and_num(doc[15], 1e-1);
				check_type_and_num(doc[16], 1e00);
				check_type_and_num(doc[17], 2e+00);
				check_type_and_num(doc[18], 2e-00);
				check_type_and_str(doc[19], "rosebud");
				
				// embedded object doc[1]
				if (check_has_key(doc[1], "object with 1 member")) {
					if (check_type_and_size(doc[1]["object with 1 member"], ValueKind::Array, 1))
						check_type_and_str(doc[1]["object with 1 member"][0], "array with 1 element");
				}

				// embedded object doc[8]
				if (check_has_key(doc[8], "integer"))
					check_type_and_num(doc[8]["integer"], 1234567890);
				if (check_has_key(doc[8], "real"))
					check_type_and_num(doc[8]["real"], -9876.543210);
				if (check_has_key(doc[8], "e"))
					check_type_and_num(doc[8]["e"], 0.123456789e-12);
				if (check_has_key(doc[8], "E"))
					check_type_and_num(doc[8]["E"], 1.234567890E+34);
				if (check_has_key(doc[8], ""))
					check_type_and_num(doc[8][""], 23456789012E66);
				if (check_has_key(doc[8], "zero"))
					check_type_and_num(doc[8]["zero"], 0);
				if (check_has_key(doc[8], "one"))
					check_type_and_num(doc[8]["one"], 1);

				if (check_has_key(doc[8], "space"))
					check_type_and_str(doc[8]["space"], " ");
				if (check_has_key(doc[8], "backslash"))
					check_type_and_str(doc[8]["backslash"], "\\");
				if (check_has_key(doc[8], "controls"))
					check_type_and_str(doc[8]["controls"], "\b\f\n\r\t");
				if (check_has_key(doc[8], "slash"))
					check_type_and_str(doc[8]["slash"], "/ & /");
				if (check_has_key(doc[8], "alpha"))
					check_type_and_str(doc[8]["alpha"], "abcdefghijklmnopqrstuvwyz");
				if (check_has_key(doc[8], "ALPHA"))
					check_type_and_str(doc[8]["ALPHA"], "ABCDEFGHIJKLMNOPQRSTUVWYZ");
				if (check_has_key(doc[8], "digit"))
					check_type_and_str(doc[8]["digit"], "0123456789");
				if (check_has_key(doc[8], "0123456789"))
					check_type_and_str(doc[8]["0123456789"], "digit");
				if (check_has_key(doc[8], "special"))
					check_type_and_str(doc[8]["special"], "`1~!@#$%^&*()_+-={':[,]}|;.</>?");
				if (check_has_key(doc[8], "hex"))
					check_type_and_str(doc[8]["hex"], "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A");

				if (check_has_key(doc[8], "true"))
					checkEqual(doc[8]["true"].type(), ValueKind::True);
				if (check_has_key(doc[8], "false"))
					checkEqual(doc[8]["false"].type(), ValueKind::False);
				if (check_has_key(doc[8], "null"))
					checkEqual(doc[8]["null"].type(), ValueKind::Null);
				if (check_has_key(doc[8], "array"))
					check_type_and_size(doc[8]["array"], ValueKind::Array, 0);
				if (check_has_key(doc[8], "object"))
					check_type_and_size(doc[8]["object"], ValueKind::Object, 0);

				if (check_has_key(doc[8], "address"))
					check_type_and_str(doc[8]["address"], "50 St. James Street");
				if (check_has_key(doc[8], "url"))
					check_type_and_str(doc[8]["url"], "http://www.JSON.org/");
				if (check_has_key(doc[8], "comment"))
					check_type_and_str(doc[8]["comment"], "// /* <!-- --");
				if (check_has_key(doc[8], "# -- --> */"))
					check_type_and_str(doc[8]["# -- --> */"], " ");
				
				auto check_1to7_array = [&](const BasicValue& val) {
					for (int ix=0; ix<7; ++ix)
						check_type_and_num(val[ix], ix+1);
				};

				if (check_has_key(doc[8], " s p a c e d ")) {
					if (check_type_and_size(doc[8][" s p a c e d "], ValueKind::Array, 7))
						check_1to7_array(doc[8][" s p a c e d "]);
				}
				if (check_has_key(doc[8], "compact")) {
					if (check_type_and_size(doc[8]["compact"], ValueKind::Array, 7))
						check_1to7_array(doc[8]["compact"]);
				}

				if (check_has_key(doc[8], "jsontext"))
					check_type_and_str(doc[8]["jsontext"], "{\"object with 1 member\":[\"array with 1 element\"]}");
				if (check_has_key(doc[8], "quotes"))
					check_type_and_str(doc[8]["quotes"], "&#34; \u0022 %22 0x22 034 &#x22;");
				if (check_has_key(doc[8], "/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"))
					check_type_and_str(doc[8]["/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"], "A key can be any string");
			}
		});

		test("pass2.json enormous subscript access should work", []{
			std::ifstream pass_file{ "jsonchecker/pass2.json" };
			auto doc = krystal::parseStream(pass_file);

			checkEqual(doc[0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0].string(), "Not too deep");
		});
	});
}
