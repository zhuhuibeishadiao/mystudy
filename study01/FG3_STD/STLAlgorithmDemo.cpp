#include "stdafx.h"
#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <numeric>
#include <ctime>


void Demo01()
{
	char letter = 'A';
	std::vector<char> chars(10);
	std::ostream_iterator<char> output(std::cout, ",");
	
	fill(chars.begin(), chars.end(), '5');
	std::cout << "chars after filling with 5s:\n";
	copy(chars.cbegin(), chars.cend(), output);

	fill_n(chars.begin(), 5, 'A');
	std::cout << "\n\nchars after filling five elements with As:\n";
	copy(chars.begin(), chars.end(), output);

	generate(chars.begin(), chars.end(), [&]() {
		return letter++;
	});
	std::cout << "\n\nchars after generating letters A-J:\n";
	copy(chars.begin(), chars.end(), output);

	generate_n(chars.begin(), 5, [&]() {
		return letter++;
	});
	std::cout << "\n\nchars after generating K-O for the first five elements\n";
	copy(chars.cbegin(), chars.cend(), output);
	std::cout << std::endl;
}
void Demo02()
{
	const size_t SIZE = 10;
	std::vector<int> v1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<int> v2(v1);
	std::vector<int> v3{ 1, 2, 3, 4, 1000, 6, 7, 8, 9, 10 };
	std::ostream_iterator<int> output(std::cout, ", ");

	std::cout << "v1 contains:";
	copy(v1.cbegin(), v1.cend(), output);
	std::cout << "\nv2 contains:";
	copy(v2.cbegin(), v2.cend(), output);
	std::cout << "\nv3 contains:";
	copy(v3.cbegin(), v3.cend(), output);
	std::cout << std::endl;

	auto result = std::equal(v1.cbegin(), v1.cend(), v2.cbegin());
	std::cout << "a1 " << (result ? "is" : "is not") << " equal a2.\n";
	result = std::equal(v1.cbegin(), v1.cend(), v3.cbegin());
	std::cout << "a1 " << (result ? "is" : "is not") << " equal a3.\n";

	auto location = mismatch(v1.cbegin(), v1.cend(), v3.begin());
	std::cout << "a1和a3在第" << (location.first - v1.begin()) << "位不一样\n";
	std::cout << "a1的为" << *location.first << ", a3的为" << *location.second;

	char str1[10] = "HELLO";
	char str2[10] = "BYE BYE";
	result = std::lexicographical_compare(std::begin(str1), std::end(str1), std::begin(str2), std::end(str2));
	//只要st1 < str2则返回true, str1>=str2则返回false

}
void Demo03()
{
	std::vector<int> init{ 10, 2, 10, 4, 16, 6, 14, 8, 12, 10 };
	std::ostream_iterator<int> output(std::cout, ",");

	//remove
	std::vector<int> v1(init);
	std::cout << "初始v1为:\n";
	copy(v1.begin(), v1.end(), output);

	//移除所有的10
	auto newLastElement = remove(v1.begin(), v1.end(), 10);
	std::cout << "\n移除10之后为：\n";
	copy(v1.begin(), newLastElement, output);//不用能cbegin
	//////////////////////////////////////////////////////////////////////////

	//remove_copy
	std::vector<int> v2(init);
	std::vector<int> v3(10);
	std::cout << "\n\n初始v2为:\n";
	copy(v2.begin(), v2.end(), output);

	//复制v2到v3所有不等于10的值
	newLastElement = remove_copy(v2.cbegin(), v2.cend(), v3.begin(), 10);
	std::cout << "\nv3的值为:\n";
	copy(v3.begin(), newLastElement, output);
	//////////////////////////////////////////////////////////////////////////

	//remove_if
	std::vector<int> v4(init);
	std::cout << "\n\n初始v4为:\n";
	copy(v4.begin(), v4.end(), output);

	//移除符合if的
	std::cout << "\n移除v4中大于9的数字：\n";
	newLastElement = std::remove_if(v4.begin(), v4.end(), [](int x) {
		return x > 9;
	});
	copy(v4.begin(), newLastElement, output);//不能用cbegin
	//////////////////////////////////////////////////////////////////////////

	//remove_copy_if
	std::vector<int> v5(init);
	std::vector<int> v6(10);
	std::cout << "\n\n初始v5为:\n";
	copy(v5.begin(), v5.end(), output);
	newLastElement = remove_copy_if(v5.cbegin(), v5.cend(), v6.begin(), [=](int x) {
		return x > 9;
	});
	std::cout << "\n使用remove_copy_if后v6为:\n";
	copy(v6.begin(), newLastElement, output);
	std::cout << std::endl;
}
void Demo04()
{
	std::vector<int> init{ 10, 2, 10, 4, 16, 6, 14, 8, 12, 10 };
	std::ostream_iterator<int> output(std::cout, " ");

	//replace
	std::vector<int> vec1(init);
	std::cout << "vec1初始：\n";
	std::copy(vec1.begin(), vec1.end(), output);

	replace(vec1.begin(), vec1.end(), 10, 100);
	std::cout << "\n做了relplace之后\n";
	std::copy(vec1.begin(), vec1.end(), output);
	std::cout << "\n###################################\n\n";
	//////////////////////////////////////////////////////////////////////////

	//relace_copy
	std::vector<int> vec2(init);
	std::vector<int> cp2(10);
	std::cout << "vec2初始：\n";
	std::copy(vec2.begin(), vec2.end(), output);

	replace_copy(vec2.cbegin(), vec2.cend(), cp2.begin(), 10, 100);
	std::cout << "\n做了relplace_copy之后";
	std::cout << "\nvec2内容为:\n";
	std::copy(vec2.begin(), vec2.end(), output);
	std::cout << "\ncp2内容为:\n";
	std::copy(cp2.begin(), cp2.end(), output);
	std::cout << "\n###################################\n\n";
	//////////////////////////////////////////////////////////////////////////

	//replace_if
	std::vector<int> vec3(init);
	std::cout << "vec3初始：\n";
	std::copy(vec3.begin(), vec3.end(), output);
	
	replace_if(vec3.begin(), vec3.end(), [&](int x) {
		return x > 9;
	}, 100);
	std::cout << "\n做了relplace_if(替换大于9的数字)之后";
	std::cout << "\nvec3内容为：\n";
	std::copy(vec3.begin(), vec3.end(), output);
	std::cout << "\n###################################\n\n";
	//////////////////////////////////////////////////////////////////////////

	//replace_copy_if
	std::vector<int> vec4(init);
	std::vector<int> cp4(10);
	std::cout << "vec4初始：\n";
	std::copy(vec4.begin(), vec4.end(), output);

	replace_copy_if(vec4.cbegin(), vec4.cend(), cp4.begin(), [&](int x) {
		return x > 9;
	}, 100);
	std::cout << "\n做了relplace_copy_if之后";
	std::cout << "\nvec4内容为:\n";
	std::copy(vec4.begin(), vec4.end(), output);
	std::cout << "\ncp4内容为:\n";
	std::copy(cp4.begin(), cp4.end(), output);
	std::cout << std::endl;
}
void Demo05()
{
	std::ostream_iterator<int> output(std::cout, " ");
	std::vector<int> vec1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<int> vec2{ 100, 2, 8, 1, 50, 3, 8, 8, 9, 10 };

	//random_shuffle
	std::cout << "调用random_shuffle之前：\n";
	copy(vec1.cbegin(), vec1.cend(), output);
	random_shuffle(vec1.begin(), vec1.end());
	std::cout << "\n调用random_shuffle之后：\n";
	copy(vec1.cbegin(), vec1.cend(), output);
	std::cout << "\n#############################\n\n";

	//count， count_if
	std::cout << "vec2元素为：：\n";
	std::copy(vec2.begin(), vec2.end(), output);
	
	auto result = std::count(vec2.cbegin(), vec2.cend(), 8);
	std::cout << "\n使用计算vec2中8的个数：" << result;

	result = std::count_if(vec2.cbegin(), vec2.cend(), [=](int x) {
		return x > 9;
	});
	std::cout << "\n使用计算vect中大于9的个数：" << result;
	std::cout << "\n#############################\n\n";

	//min_element
	std::cout << "vec2中最小值为:" << *std::min_element(vec2.cbegin(), vec2.cend());
	//max_element
	std::cout << "\nvec2中最大值为" <<* std::max_element(vec2.cbegin(), vec2.cend());
	//minmax_element
	auto minAndMax = minmax_element(vec2.cbegin(), vec2.cend());
	std::cout << "\n一次性求出最值，其中最小值为:" << *minAndMax.first << "  最大值为:" << *minAndMax.second;
	std::cout << "\n#############################\n\n";

	//accumulate,第三个参数为初始值
	std::cout << "vec1和为：" << std::accumulate(vec1.cbegin(), vec1.cend(), 10);
	std::cout << "\nvec2和为：" << std::accumulate(vec2.cbegin(), vec2.cend(), 0);
	std::cout << "\n#############################\n\n";

	//for_each遍历
	std::cout << "对vec2的元素求平方再输出：\n";
	for_each(vec2.cbegin(), vec2.cend(), [](int x) {
		std::cout << x * x << " ";
	});
	
	//transform
	//第三个参数是一个输出迭代器，可以是自己
	//第四个参数使用当前元素作为实参，并不能改变，返回的是变换后的值
	std::vector<int> transVec;
	std::map<int, int> map1;
	auto index = 0;
	std::transform(vec1.cbegin(), vec1.cend(), std::inserter(transVec, transVec.end()), [=](int x) {
		return x * x * x;
	});
	//vector转map的例子
	std::transform(vec1.cbegin(), vec1.cend(), std::inserter(map1, map1.end()), [&](int x) {
		index += 2;
		return std::make_pair(index, x);
	});

	std::cout << "\n调用transform后，transVec的元素为：\n";
	copy(transVec.cbegin(), transVec.cend(), output);
}
void Demo06()
{
	std::vector<int> vec{ 10, 2, 17, 5, 16, 8, 13, 11, 20, 7 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "vec元素如下：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//find
	auto location = std::find(vec.cbegin(), vec.cend(), 16);
	if (location != vec.end())
		std::cout << "\n第一个16出现在数组下标为：" << location - vec.cbegin() << std::endl;
	else
		std::cout << "没有找到16" << std::endl;

	location = std::find(vec.cbegin(), vec.cend(), 100);
	if (location != vec.end())
		std::cout << "\n第一个100出现在数组下标为：" << location - vec.cbegin() << std::endl;
	else
		std::cout << "没有找到100" << std::endl;

	//find_if 找true
	location = std::find_if(vec.cbegin(), vec.cend(), [=](int x) {
		return x > 10;
	});
	if (location != vec.cend())
		std::cout << "第一个大于10的数字是" << *location << ", 下标为" << location - vec.cbegin() << "的位置\n";
	else
		std::cout << "没有找到大于10的数字";

	//find_if_not  找false
	location = find_if_not(vec.cbegin(), vec.cend(), [](int x) {
		return x > 10;
	});
	if (location != vec.cend())
		std::cout << "第一个不大于10的数是" << *location << ", 位置在" << location - vec.cbegin() << std::endl;
	else
		std::cout << "找不到不大于10数字\n";

	//sort
	//std::sort(vec.begin(), vec.end());
	std::sort(vec.begin(), vec.end(), [](int a, int b) {//a是前面的，b是后面的
		return a < b;	//小于为真是升序，小于为假是降序
	});
	std::cout << "排序后元素如下：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//binary_search  用来找存在不存在速度贼快
	if (std::binary_search(vec.cbegin(), vec.cend(), 13))
		std::cout << "\n有13\n";
	else
		std::cout << "没有13\n";

	if (std::binary_search(vec.cbegin(), vec.cend(), 100))
		std::cout << "有100\n";
	else
		std::cout << "没有100\n";

	//all_of
	if (all_of(vec.cbegin(), vec.cend(), [&](int x) {
		return x > 10;
	}))
		std::cout << "all_of:所有的元素都大于10\n";
	else
		std::cout << "all_of:并非所有元素都大于10\n";

	//any_of
	if (any_of(vec.cbegin(), vec.cend(), [&](int x) {
		return x > 10;
	}))
		std::cout << "any_of:有些元素大于10\n";
	else
		std::cout << "any_of:所有元素都不大于10\n";

	//none_of
	if (none_of(vec.cbegin(), vec.cend(), [&](int x) {
		return x > 10;
	}))
		std::cout << "none_of:没有一个元素大于10\n";
	else
		std::cout << "none_of:有些元素大于10\n";
}
void TestAlgorithmDemo()
{
	//fill,fill_n,generate,generate_n
	//Demo01();

	//equal, mismatch, lexicographical_compare
	//Demo02();

	//remove，remove_if，remove_copy，remove_copy_if
	//Demo03();

	//replace, replace_if, replace_copy, replace_copy_if
	//Demo04();

	//random_shuffle, count, count_if, min_element, max_element, 
	//minmax_element, accumulate, for_each, transform
	//Demo05();

	//find, find_if, sort, binary_search, all_of, any_of, none_of, find_if_not
	//Demo06();

	//swap, iter_swap, swap_rangs

}