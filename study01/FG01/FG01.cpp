// fg0.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <memory>

using namespace std;

void example1();
void example2();

int main()
{
	example1();

	return 0;
}

//类型声明以及转换的一些例子
void example1()
{
	
	//可以用make_shared分配内存
	typedef struct _my_int
	{
		int a[100];
	}my_int;
	auto my = std::make_shared<my_int>();
	for (int i = 0; i < 100; i++)
		my->a[i] = i + 1;
	printf("%d\r\n", my->a[7]);


	//智能指针使用 
	std::shared_ptr<int> my2_p(new int[100]);
	for (int i = 0; i < 100; i++)
	{
		(my2_p.get())[i] = i % 2;
	}
	printf("%d\r\n", my2_p.get()[1]);

	//内存单元应该是变了，但是const变量，push的时候是直接push 2
	//凡事用到常量a的都用2直接代替了
	const int a = 2;
	int *b = const_cast<int *>(&a);
	*b = 100;
	int c = a + 2;
	printf("a=%d,b=%d\r\n", a, *b);

	//基本数据类型转换
	float floatValue = 21.7;
	int intValue = 7;
	cout << static_cast<int>(floatValue) / 7 << endl;
	cout << static_cast<double>(intValue) / 3 << endl;

	//任何指针转换成整型的地址值
	int ii;
	char *p = "this is a example";
	ii = reinterpret_cast<int>(p);//指针地址
	cout << hex << ii << endl;
}

void example2()
{

}