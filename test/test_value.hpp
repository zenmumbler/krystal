// test_value.hpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_value() {
	group("value class", []{
		group("type constructors", []{
			test("default constructor creates null value", []{
				value v;
				check_equal(value_type::Null, v.type());
			});
			
			test("value_type constructor creates value of that type", []{
				for (auto type : all_types())
					check_equal(type, (value{ type }).type());
			});
		});
		
		group("value constructors", []{
			test("string data creates string values", []{
				auto cc_str = "const char* string";
				auto std_str = std::string{"std::string"};
				
				auto cc_val = value{ cc_str };
				auto std_val = value{ std_str };
				
				check_equal(cc_str, cc_val.string());
				check_equal(std_str, std_val.string());
			});
			
			test("numbers create number values", []{
				auto int_num = 500000000;
				auto dbl_num = 3.1415926535;
				
				auto int_val = value{int_num};
				auto dbl_val = value{dbl_num};
				
				check_equal(int_num, int_val.number_as<decltype(int_num)>());
				check_equal(dbl_num, dbl_val.number());
			});
			
			test("bools create bool values", []{
				auto b1 = true;
				auto b2 = false;
				
				auto b1_val = value{b1};
				auto b2_val = value{b2};
				
				check_equal(b1, b1_val.boolean());
				check_equal(b2, b2_val.boolean());
			});
		});
		
		group("moves", []{
			for (auto type : all_types()) {
				test("move-constructing a " + to_string(type) + " moves type", [type]{
					auto source = value{type};
					auto dest = std::move(source);
					
					check_equal(value_type::Null, source.type());
					check_equal(type, dest.type());
				});
				
				test("move-assigning a " + to_string(type) + " moves type", [type]{
					auto source = value{type};
					auto dest = value{};
					dest = std::move(source);
					
					check_equal(value_type::Null, source.type());
					check_equal(type, dest.type());
				});
			}
			
			test("moving moves value data", []{
				auto dest = value{},
				iv = value{ 100 },
				dv = value{ 48390.32789 },
				sv = value{ "some string" },
				av = value{ value_type::Array },
				ov = value{ value_type::Object };
				
				av.push_back(value{ 50 });
				av.push_back(value{ true });
				av.push_back(value{ "charm" });
				
				ov.insert("important", value{ "don't forget" });
				ov.insert("monkeys", value{ 12 });
				
				dest = std::move(iv);
				check_equal(100, dest.number());
				dest = std::move(dv);
				check_equal(48390.32789, dest.number());
				dest = std::move(sv);
				check_equal("some string", dest.string());
				
				dest = std::move(av);
				check_equal(3, dest.size());
				check_equal(50, dest[0].number());
				check_equal(true, dest[1].boolean());
				check_equal("charm", dest[2].string());
				
				dest = std::move(ov);
				check_equal(2, dest.size());
				check_equal("don't forget", dest["important"].string());
				check_equal(12, dest["monkeys"].number_as<int>());
			});
		});
		
		group("type tests", []{
			for (auto type : all_types()) {
				test("param and direct tests should be equal for " + to_string(type), [type]{
					auto val = value{type};
					check_equal(val.is_null(),   val.is_a(value_type::Null));
					check_equal(val.is_false(),  val.is_a(value_type::False));
					check_equal(val.is_true(),   val.is_a(value_type::True));
					check_equal(val.is_number(), val.is_a(value_type::Number));
					check_equal(val.is_string(), val.is_a(value_type::String));
					check_equal(val.is_array(),  val.is_a(value_type::Array));
					check_equal(val.is_object(), val.is_a(value_type::Object));
				});
			}
			
			test("only container types should identify as such", []{
				check_false((value{ value_type::Null }).is_container());
				check_false((value{ value_type::False }).is_container());
				check_false((value{ value_type::True }).is_container());
				check_false((value{ value_type::Number }).is_container());
				check_false((value{ value_type::String }).is_container());
				check_true ((value{ value_type::Array }).is_container());
				check_true ((value{ value_type::Object }).is_container());
			});
			
			test("only bool types should identify as such", []{
				check_false((value{ value_type::Null }).is_bool());
				check_true ((value{ value_type::False }).is_bool());
				check_true ((value{ value_type::True }).is_bool());
				check_false((value{ value_type::Number }).is_bool());
				check_false((value{ value_type::String }).is_bool());
				check_false((value{ value_type::Array }).is_bool());
				check_false((value{ value_type::Object }).is_bool());
			});
		});
		
		group("arrays", []{
			test("normal indexed for loop should iterate over all values linearly", []{
				auto arr = value{ value_type::Array };
				arr.push_back(value{ std::string{"*", 10} });
				arr.push_back(value{ std::string{"*", 20} });
				arr.push_back(value{ std::string{"*", 30} });
				arr.push_back(value{ std::string{"*", 40} });
				
				for (auto ix=0L; ix<arr.size(); ++ix) {
					const auto& val = arr[ix];
					check_equal(std::string{"*", 10 * (ix + 1)}, val.string());
				}
			});
			
			test("range-based for should iterate over all index-value pairs linearly", []{
				auto arr = value{ value_type::Array };
				arr.push_back(value{ 0 });
				arr.push_back(value{ 100 });
				arr.push_back(value{ 200 });
				arr.push_back(value{ 300 });
				
				int count = 0;
				for (auto kv : arr) {
					check_equal(count, kv.first.number_as<int>());
					check_equal(100 * count, kv.second.number_as<int>());
					++count;
				}
				check_equal(arr.size(), count);
			});
		});
		
		group("objects", []{
			test("range-based for should iterate over all key-value pairs in undefined order", []{
				auto obj = value{ value_type::Object };
				obj.insert("key0", value{ false });
				obj.insert("key1", value{ true });
				obj.insert("key2", value{ false });
				obj.insert("key3", value{ true });
				
				int count = 0;
				for (auto kv : obj) {
					check_equal("key", kv.first.string().substr(0, 3));
					auto index = std::stoi(kv.first.string().substr(3, 1));
					check_equal((index & 1) == 1, kv.second.boolean());
					++count;
				}
				check_equal(obj.size(), count);
			});
		});
	});
}
