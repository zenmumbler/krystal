// test_value.hpp - part of krystal_test
// (c) 2013 by Arthur Langereis (@zenmumbler)

void test_value() {
	group("value class", []{
		group("type constructors", []{
			test("default constructor creates null value", []{
				Value v;
				checkEqual(v.type(), ValueKind::Null);
			});
			
			test("ValueKind constructor creates value of that kind", []{
				for (auto type : all_types())
					checkEqual((Value{ type }).type(), type);
			});
		});
		
		group("value constructors", []{
			test("string data creates string values", []{
				auto cc_str = "const char* string";
				auto std_str = std::string{"std::string"};
				
				auto cc_val = Value{ cc_str };
				auto std_val = Value{ std_str };
				
				checkEqual(cc_val.string(), cc_str);
				checkEqual(std_val.string(), std_str);
			});
			
			test("numbers create number values", []{
				auto int_num = 500000000;
				auto dbl_num = 3.1415926535;
				
				auto int_val = Value{int_num};
				auto dbl_val = Value{dbl_num};
				
				checkEqual(int_val.numberAs<decltype(int_num)>(), int_num);
				checkEqual(dbl_val.number(), dbl_num);
			});
			
			test("bools create bool values", []{
				auto b1 = true;
				auto b2 = false;
				
				auto b1_val = Value{b1};
				auto b2_val = Value{b2};
				
				checkEqual(b1_val.boolean(), b1);
				checkEqual(b2_val.boolean(), b2);
			});
		});
		
		group("moves", []{
			for (auto type : all_types()) {
				test("move-constructing a " + toString(type) + " moves type", [type]{
					auto source = Value{type};
					auto dest = std::move(source);
					
					checkEqual(source.type(), ValueKind::Null);
					checkEqual(dest.type(), type);
				});
				
				test("move-assigning a " + toString(type) + " moves type", [type]{
					auto source = Value{type};
					auto dest = Value{};
					dest = std::move(source);
					
					checkEqual(source.type(), ValueKind::Null);
					checkEqual(dest.type(), type);
				});
			}
			
			test("moving moves value data", []{
				auto dest = Value{},
				iv = Value{ 100 },
				dv = Value{ 48390.32789 },
				sv = Value{ "some string" },
				av = Value{ ValueKind::Array },
				ov = Value{ ValueKind::Object };
				
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
				checkEqual(dest["monkeys"].numberAs<int>(), 12);
			});
		});
		
		group("type tests", []{
			for (auto type : all_types()) {
				test("param and direct tests should be equal for " + toString(type), [type]{
					auto val = Value{type};
					checkEqual(val.isNull(),   val.isA(ValueKind::Null));
					checkEqual(val.isFalse(),  val.isA(ValueKind::False));
					checkEqual(val.isTrue(),   val.isA(ValueKind::True));
					checkEqual(val.isNumber(), val.isA(ValueKind::Number));
					checkEqual(val.isString(), val.isA(ValueKind::String));
					checkEqual(val.isArray(),  val.isA(ValueKind::Array));
					checkEqual(val.isObject(), val.isA(ValueKind::Object));
				});
			}
			
			test("only container types should identify as such", []{
				checkFalse((Value{ ValueKind::Null }).isContainer());
				checkFalse((Value{ ValueKind::False }).isContainer());
				checkFalse((Value{ ValueKind::True }).isContainer());
				checkFalse((Value{ ValueKind::Number }).isContainer());
				checkFalse((Value{ ValueKind::String }).isContainer());
				checkTrue ((Value{ ValueKind::Array }).isContainer());
				checkTrue ((Value{ ValueKind::Object }).isContainer());
			});
			
			test("only bool types should identify as such", []{
				checkFalse((Value{ ValueKind::Null }).isBool());
				checkTrue ((Value{ ValueKind::False }).isBool());
				checkTrue ((Value{ ValueKind::True }).isBool());
				checkFalse((Value{ ValueKind::Number }).isBool());
				checkFalse((Value{ ValueKind::String }).isBool());
				checkFalse((Value{ ValueKind::Array }).isBool());
				checkFalse((Value{ ValueKind::Object }).isBool());
			});
		});
		
		group("arrays", []{
			test("normal indexed for loop should iterate over all values linearly", []{
				auto arr = Value{ ValueKind::Array };
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
				auto arr = Value{ ValueKind::Array };
				arr.emplace_back(0);
				arr.emplace_back(100);
				arr.emplace_back(200);
				arr.emplace_back(300);
				
				int count = 0;
				for (auto kv : arr) {
					checkEqual(kv.first.numberAs<int>(), count);
					checkEqual(kv.second.numberAs<int>(), 100 * count);
					++count;
				}
				checkEqual(count, arr.size());
			});
		});

		group("objects", []{
			test("range-based for should iterate over all key-value pairs in undefined order", []{
				auto obj = Value{ ValueKind::Object };
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
