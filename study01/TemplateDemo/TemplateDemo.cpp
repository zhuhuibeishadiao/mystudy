// TemplateDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include "XOR_STRING.h"


template<int index>
void demo1()
{
	
	std::cout << index << std::endl;
}

//模版特化，就是众多模版中特殊的例子
template<>
void demo1<0>()
{
	std::cout << "我草你妈的" << std::endl;
}

template<typename T>
auto fun(T t1, T t2)
{
	return t1 + t2;
}

//在编译器确定穿进来字符串的长度
template<std::size_t S>
static constexpr auto demo2(const char(&str)[S])
{
	printf("%d\n", S);
}

//字符串引用，前提是必须填对真正的长度
void demo3(char (&str)[20])
{
	printf("%s\n",str);
}

int main()
{
	int i;
	float j;
	char k;

	decltype(i) i1;
	std::cout << typeid(i).name() << std::endl;;
	std::cout << typeid(i1).name() << std::endl;;
	std::cout << typeid(j).name() << std::endl;
	std::cout << typeid(k).name() << std::endl;


	/////////////////
	demo1<1>();
	demo1<10>();
	demo1<100>();
	demo1<0>();
	/////////////////
	printf("%s\n", XOR_STRING_A("caonima"));
	/////////////////
	printf("%d\n", fun<int>(4, 2));
	printf("%f\n", fun<float>(1.31, 2.5));

	demo2("abc");

	char str[20] = "caonimabi";
	demo3(str);

	int a;
	const_cast<int *>(&a);
    return 0;
}

