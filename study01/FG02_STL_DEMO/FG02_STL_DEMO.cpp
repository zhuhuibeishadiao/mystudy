/**************************************************************************
Author: FrogGod
Date:2017-05-26
Description:STL的函数的操作
**************************************************************************/

#include "stdafx.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
using namespace std;

class Person
{
public:
	Person(){}
	Person(int num) :m_nNum(num){}
	int m_nNum;
public:
	//操作符重载
	bool operator ==(int num);
private:
	
};

void ForEachDemo();
void ForEachFun1(Person &p);
void FindDemo();
void FindIfDemo();

vector<Person> g_vecPer;
void initData()
{
	//往全局变量vector放元素，其实是复制一个过去
	Person p1(123);
	Person p2(10);
	Person p21(10);
	Person p22(10);
	Person p3(200);
	Person p4(1987);
	Person p5(999);
	g_vecPer.push_back(p1);
	g_vecPer.push_back(p2);
	g_vecPer.push_back(p21);
	g_vecPer.push_back(p22);
	g_vecPer.push_back(p3);
	g_vecPer.push_back(p4);
	g_vecPer.push_back(p5);
	
}

int main()
{
	initData();
	ForEachDemo();
	FindDemo();
	//FindIfDemo();
	
    return 0;
}

/************************************************************************/
/*            容器for_eachDemo                                         */
void ForEachDemo()
{
	for_each(g_vecPer.begin(), g_vecPer.end(), ForEachFun1);
}
void ForEachFun1(Person &p)
{
	//可以做一些改变数据的操作
	//p.m_nNum += 10000;
	cout << p.m_nNum << endl;
}
/************************************************************************/


/************************************************************************/
/*                    容器findDemo                                   */
void FindDemo()
{
	vector<Person>::iterator it = find(g_vecPer.begin(), g_vecPer.end(), 10);
	if (it == g_vecPer.end())
	{
		cout << "没找到" << endl;
		return;
	}
	g_vecPer.erase(it);
}
bool Person::operator==(int num)
{
	if (m_nNum != num)
		return false;
	return true;
}
/************************************************************************/


/************************************************************************/
/*                   容器find_ifDemo                                  */
bool FindIfFun(Person& p)
{
	return p.m_nNum == 10 ? true : false;
}
void FindIfDemo()
{
	for (auto it = g_vecPer.begin(); it != g_vecPer.end(); it++)
	{
		it = find_if(it, g_vecPer.end(), FindIfFun);
		if (it == g_vecPer.end())break;
		cout << it->m_nNum << endl;
	}
}
/************************************************************************/

/************************************************************************/
/*                                                                      */




/************************************************************************/










