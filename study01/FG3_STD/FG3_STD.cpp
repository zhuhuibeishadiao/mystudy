// FG3_STD.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>


void TestGenerate_n()
{
	
	auto vec = std::vector<int>{};
	std::generate_n(std::back_inserter(vec), 10000, [=]() {
		UINT num = 0;
		_rdrand32_step(&num);
		return num;
	});
	std::cout << "nihao";
}

int main()
{
	TestGenerate_n();
	return 0;
}

