#include "stdafx.h"
#include <windows.h>
//各种随机数Demo

//产生N个随机数，真随机
void Demo001()
{
	//创建真随机
	auto vec = std::vector<UINT>{};
	std::generate_n(std::back_inserter(vec), 10000, [=]() {
		UINT num = 0;
		_rdrand32_step(&num);
		return num;
	});

}

void TestRandomNumberDemo()
{
	Demo001();
}