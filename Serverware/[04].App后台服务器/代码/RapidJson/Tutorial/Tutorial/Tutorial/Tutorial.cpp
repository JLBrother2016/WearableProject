// Tutorial.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <document.h>
#include <prettywriter.h>

using namespace rapidjson;
using namespace std;


int main(int argc, char* argv[]) {
	const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } "; // 一个JSON字符串
	printf("Original JSON:\n %s\n", json);
	Document document;		// Default template parameter uses UTF8 and MemoryPoolAllocator.
	document.Parse(json);	// 把Json解析到一个Doucment对象

	assert(document.IsObject()); // document根值是一个Object
	assert(document.HasMember("hello"));	// 查询一下根 Object 中有没有 "hello" 成员。
	assert(document["hello"].IsString());	// "hello" 成员关联到一个 JSON String。
	printf("hello = %s\n", document["hello"].GetString());

	// 访问BOOL类型
	assert(document["t"].IsBool());
	printf("t = %s\n", document["t"].GetBool() ? "true" : "false");
	assert(document["f"].IsBool());
	printf("f = %s\n", document["f"].GetBool() ? "ture" : "false");

	// 判断NULL
	printf("n = %s\n", document["n"].IsNull() ? "null" : "?"); //JSON Null 值可用 IsNull() 查询。

	// 访问数值类型
	assert(document["i"].IsNumber());
	assert(document["i"].IsInt()); // 在此情况下，IsUint()/IsInt64()/IsUInt64() 也会返回 true
	printf("i = %d\n", document["i"].GetInt()); // 另一种用法： (int)document["i"]

	assert(document["pi"].IsNumber());
	assert(document["pi"].IsDouble());
	printf("pi = %g\n", document["pi"].GetDouble());

	{
		// 查询Array

		const Value& a = document["a"]; // 使用引用来连续访问，方便之余还更高效。
		assert(a.IsArray());
		for (SizeType i = 0; i < a.Size(); ++i) {
			printf("a[%d] = %d\n", i, a[i].GetInt()); //注意，RapidJSON 并不自动转换各种 JSON 类型。例如，对一个 String 的 Value 调用 GetInt() 是非法的。
		}

		// 除了使用索引，也可使用迭代器来访问所有元素。
		printf("a = ");
		for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr) {
			printf("%d ", itr->GetInt());
		}
		printf("\n");

		// 使用范围for循环去访问Array内的所有元素
		for (auto& v : a.GetArray()) {
			printf("%d ", v.GetInt());
		}
		printf("\n");
	}

	{
		// 查询Object
		static const char* kTypeNames[] = {
			"Null", "False", "True", "Object", "Array", "String", "Number"
		};
		for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
			printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
		}
	}
	

	{
		// 改变document中的值
		uint64_t f20 = 1;
		for (uint64_t j = 1; j <= 20; ++j) {
			f20 *= j;
		}
		document["i"] = f20; // 另一种形式: document["i"].SetUint64(f20)
	}

	{
		// 在数组中添加元素
		Value& a = document["a"];
		Document::AllocatorType& allocator = document.GetAllocator();
		for (int i = 5; i <= 10; ++i) {
			a.PushBack(i, allocator);
		}
		for (auto& v : a.GetArray()) {
			printf("%d ", v.GetInt());
		}
		printf("\n");
	}

	printf("\nModified JSON with reformatting:\n");
	/*
	StringBuffer sb;
	PrettyWriter<StringBuffer> writer(sb);
	document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	puts(sb.GetString());
	*/

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	document.Accept(writer); // 将document转化为JSON字符串
	puts(buffer.GetString());

	system("pause");
	return 0;
}

