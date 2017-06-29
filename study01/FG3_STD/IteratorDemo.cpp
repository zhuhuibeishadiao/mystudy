#include "stdafx.h"

using namespace std;
void Demo1()
{
	cout << "Enter two integers:";
	
	istream_iterator<int> inputInt(cin);
	int number1 = *inputInt;
	++inputInt;
	int number2 = *inputInt;

	ostream_iterator<int> outputInt(cout);
	cout << "sum is:";
	*outputInt = number1 + number2;
	cout << endl;
}

void Demo2()
{
	std::string abc = "abc";
	std::cout << abc << endl;
	string bcd = std::move(abc);
	std::cout << abc << endl;
	std::cout << bcd << endl;
}

void TestIteratordemo()
{
	//Demo1();
	//Demo2();
}