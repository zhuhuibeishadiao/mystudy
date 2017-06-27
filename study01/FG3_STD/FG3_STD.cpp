// FG3_STD.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
extern void TestIteratordemo();
extern void TestAlgorithmDemo();
//
void TestGenerate_n()
{
	
	//真正的随机是这样的
	auto vec = std::vector<UINT>{};
	std::generate_n(std::back_inserter(vec), 10000, [=]() {
		UINT num = 0;
		_rdrand32_step(&num);
		return num;
	});
	
}



int main()
{
	//文件的
	//TestGenerate_n();

	//其他文件的
	TestIteratordemo();
	TestAlgorithmDemo();
	return 0;
}

