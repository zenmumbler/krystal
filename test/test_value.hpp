// test_value.hpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_value() {
	group("value class", []{
		group("type constructors", []{
			test("default constructor creates null value", []{
				value v;
				checkEqual(v.type(), value_kind::Null);
			});
			
			test("value_kind constructor creates value of that kind", []{
				for (auto type : all_types())
					checkEqual((value{ type }).type(), type);
			});
		});
		
		group("value constructors", []{
			test("string data creates string values", []{
				auto cc_str = "const char* string";
				auto std_str = std::string{"std::string"};
				
				auto cc_val = value{ cc_str };
				auto std_val = value{ std_str };
				
				checkEqual(cc_val.string(), cc_str);
				checkEqual(std_val.string(), std_str);
			});
			
			test("numbers create number values", []{
				auto int_num = 500000000;
				auto dbl_num = 3.1415926535;
				
				auto int_val = value{int_num};
				auto dbl_val = value{dbl_num};
				
				checkEqual(int_val.number_as<decltype(int_num)>(), int_num);
				checkEqual(dbl_val.number(), dbl_num);
			});
			
			test("bools create bool values", []{
				auto b1 = true;
				auto b2 = false;
				
				auto b1_val = value{b1};
				auto b2_val = value{b2};
				
				checkEqual(b1_val.boolean(), b1);
				checkEqual(b2_val.boolean(), b2);
			});
		});
		
		group("moves", []{
			for (auto type : all_types()) {
				test("move-constructing a " + to_string(type) + " moves type", [type]{
					auto source = value{type};
					auto dest = std::move(source);
					
					checkEqual(source.type(), value_kind::Null);
					checkEqual(dest.type(), type);
				});
				
				test("move-assigning a " + to_string(type) + " moves type", [type]{
					auto source = value{type};
					auto dest = value{};
					dest = std::move(source);
					
					checkEqual(source.type(), value_kind::Null);
					checkEqual(dest.type(), type);
				});
			}
			
			test("moving moves value data", []{
				auto dest = value{},
				iv = value{ 100 },
				dv = value{ 48390.32789 },
				sv = value{ "some string" },
				av = value{ value_kind::Array },
				ov = value{ value_kind::Object };
				
				av.emplace_back(50);
				av.emplace_back(true);
				av.emplace_back("charm");
				
				ov.emplace("important", "don't forget");
				ov.emplace("monkeys", 12);
				
				dest = std::move(iv);
				checkEqual(dest.number(), 100);
				dest = std::move(dv);
				checkEqual(dest.number(), 48390.32789);
				dest = std::move(sv);
				checkEqual(dest.string(), "some string");
				
				dest = std::move(av);
				checkEqual(dest.size(), 3);
				checkEqual(dest[0].number(), 50);
				checkEqual(dest[1].boolean(), true);
				checkEqual(dest[2].string(), "charm");
				
				dest = std::move(ov);
				checkEqual(dest.size(), 2);
				checkEqual(dest["important"].string(), "don't forget");
				checkEqual(dest["monkeys"].number_as<int>(), 12);
			});
		});
		
		group("type tests", []{
			for (auto type : all_types()) {
				test("param and direct tests should be equal for " + to_string(type), [type]{
					auto val = value{type};
					checkEqual(val.is_null(),   val.is_a(value_kind::Null));
					checkEqual(val.is_false(),  val.is_a(value_kind::False));
					checkEqual(val.is_true(),   val.is_a(value_kind::True));
					checkEqual(val.is_number(), val.is_a(value_kind::Number));
					checkEqual(val.is_string(), val.is_a(value_kind::String));
					checkEqual(val.is_array(),  val.is_a(value_kind::Array));
					checkEqual(val.is_object(), val.is_a(value_kind::Object));
				});
			}
			
			test("only container types should identify as such", []{
				checkFalse((value{ value_kind::Null }).is_container());
				checkFalse((value{ value_kind::False }).is_container());
				checkFalse((value{ value_kind::True }).is_container());
				checkFalse((value{ value_kind::Number }).is_container());
				checkFalse((value{ value_kind::String }).is_container());
				checkTrue ((value{ value_kind::Array }).is_container());
				checkTrue ((value{ value_kind::Object }).is_container());
			});
			
			test("only bool types should identify as such", []{
				checkFalse((value{ value_kind::Null }).is_bool());
				checkTrue ((value{ value_kind::False }).is_bool());
				checkTrue ((value{ value_kind::True }).is_bool());
				checkFalse((value{ value_kind::Number }).is_bool());
				checkFalse((value{ value_kind::String }).is_bool());
				checkFalse((value{ value_kind::Array }).is_bool());
				checkFalse((value{ value_kind::Object }).is_bool());
			});
		});
		
		group("arrays", []{
			test("normal indexed for loop should iterate over all values linearly", []{
				auto arr = value{ value_kind::Array };
				arr.emplace_back(std::string(10, '*'));
				arr.emplace_back(std::string(20, '*'));
				arr.emplace_back(std::string(30, '*'));
				arr.emplace_back(std::string(40, '*'));
				
				for (auto ix = 0L; ix < arr.size(); ++ix) {
					const auto& val = arr[ix];
					checkEqual(val.string(), std::string(10 * (ix + 1), '*'));
				}
			});
			
			test("range-based for should iterate over all index-value pairs linearly", []{
				auto arr = value{ value_kind::Array };
				arr.emplace_back(0);
				arr.emplace_back(100);
				arr.emplace_back(200);
				arr.emplace_back(300);
				
				int count = 0;
				for (auto kv : arr) {
					checkEqual(kv.first.number_as<int>(), count);
					checkEqual(kv.second.number_as<int>(), 100 * count);
					++count;
				}
				checkEqual(count, arr.size());
			});
		});
		
		group("objects", []{
			test("range-based for should iterate over all key-value pairs in undefined order", []{
				auto obj = value{ value_kind::Object };
				obj.emplace("key0", false);
				obj.emplace("key1", true);
				obj.emplace("key2", false);
				obj.emplace("key3", true);
				
				int count = 0;
				for (auto kv : obj) {
					checkEqual(kv.first.string().substr(0, 3), "key");
					auto index = std::stoi(kv.first.string().substr(3, 1));
					checkEqual(kv.second.boolean(), (index & 1) == 1);
					++count;
				}
				checkEqual(obj.size(), count);
			});
		});
	});
}
