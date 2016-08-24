// test_performance.hpp - part of krystal_test
// (c) 2013-6 by Arthur Langereis (@zenmumbler)

void test_performance() {
	group("performance tests", []{
		using namespace std::chrono;
		
		test("100.000 parses of tiny file (255B)", []{
			// this is from the Oj ruby JSON parser test
			auto perf_file = readTextFile("perftests/teensy.json");
			auto t0 = high_resolution_clock::now();
			for (int x = 0; x < 100000; ++x) {
				 auto doc = krystal::parseString(perf_file);
			}
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: 100K times tiny file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});
		
		test("medium sized compact file (200KB)", []{
			auto perf_file = readTextFile("perftests/medium-large.json");
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parseString(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: medium file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

		test("rapidjson's insane test file (670KB)", []{
			auto perf_file = readTextFile("perftests/rapidjson-insane.json");
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parseString(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: rapidjson's file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

		test("very large but very simple file (4.1MB)", []{
			auto perf_file = readTextFile("perftests/large-but-boring.json");
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parseString(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: large file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

	});
}
