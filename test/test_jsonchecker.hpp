// test_jsonchecker.cpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_jsonchecker() {
	group("jsonchecker tests", []{

		test("faulty test data", []{
			const int faulty_tests = 33;
			
			for (int tix=1; tix <= faulty_tests; ++tix) {
				std::ifstream fail_file{ "jsonchecker/fail" + to_string(tix) + ".json" };
				auto val = krystal::parse(fail_file);
				check_equal(val.type(), value_type::Null);
			}
		});

		test("valid test data", []{
			const int valid_tests = 3;
			
			for (int tix=1; tix <= valid_tests; ++tix) {
				std::ifstream pass_file{ "jsonchecker/pass" + to_string(tix) + ".json" };
				auto val = krystal::parse(pass_file);
				check_true(val.is_container());
			}
		});

	});
}
