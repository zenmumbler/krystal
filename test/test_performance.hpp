// test_performance.hpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_performance() {
	group("performance tests", []{
		using namespace std::chrono;
		
		test("medium sized compact file (200KB)", []{
			std::ifstream perf_file{ "perftests/medium-large.json" };
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parse_stream(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: medium file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

		test("rapidjson's insane test file (670KB)", []{
			std::ifstream perf_file{ "perftests/rapidjson-insane.json" };
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parse_stream(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: rapidjson's file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

		test("very large but very simple file (4.1MB)", []{
			std::ifstream perf_file{ "perftests/large-but-boring.json" };
			auto t0 = high_resolution_clock::now();
			auto doc = krystal::parse_stream(perf_file);
			auto t1 = high_resolution_clock::now();
			
			std::cout << "Perf: large file took " << duration_cast<milliseconds>(t1 - t0).count() << "ms.\n";
		});

	});
}
