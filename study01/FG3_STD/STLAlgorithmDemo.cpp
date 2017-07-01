#include "stdafx.h"

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
void Demo07()
{
	std::vector<int> vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "初始元素顺序为：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//swap
	std::swap(vec[0], vec[1]);
	std::cout << "\n调用swap之后元素顺序为：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//iter_swap
	std::iter_swap(vec.begin(), vec.begin() + 1);
	std::cout << "\n调用iter_swap之后元素顺序为：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//swap_rangs
	std::swap_ranges(vec.begin(), vec.begin() + 5, vec.begin() + 5);
	std::cout << "\n调用swap_rangs之后元素顺序为：\n";
	std::copy(vec.begin(), vec.end(), output);
	std::cout << std::endl;
}
void Demo08()
{
	std::vector<int> vec1{ 1,3,5,7,9 };
	std::vector<int> vec2{ 2,4,5,7,9 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "vec1元素为:";
	std::copy(vec1.cbegin(), vec1.cend(), output);
	std::cout << "\nvec2元素为:";
	std::copy(vec2.cbegin(), vec2.cend(), output);

	//copy_backward
	//std::copy_backward(vec1.cbegin(), vec1.cend(), std::inserter(result, result.end()));
	std::vector<int> result1(10);
	std::copy_backward(vec1.cbegin(), vec1.cend(), result1.end());
	std::cout << "\ncopy_backward操作之后result1元素为:";
	std::copy(result1.cbegin(), result1.cend(), output);

	//merge
	std::vector<int> result2;
	merge(vec1.cbegin(), vec1.cend(),vec2.cbegin(),
		vec2.cend(), std::back_inserter(result2));//可以
	
	//merge(vec1.cbegin(), vec1.cend(), vec2.cbegin(),
		//vec2.cend(), std::inserter(result2, result2.end())); //可以

	//merge(vec1.cbegin(), vec1.cend(), vec2.cbegin(),
		//vec2.cend(), std::front_inserter(result2));//不行
	std::cout << "\nmerge操作之后result2元素为:";
	std::copy(result2.cbegin(), result2.cend(), output);

	//Unique  去重
	auto endLocation = std::unique(result2.begin(), result2.end());
	std::cout << "\n对result2进行去重之后元素为:";
	std::copy(result2.begin(), endLocation, output);

	//reverse
	std::reverse(vec1.begin(), vec1.end());
	std::cout << "\n对vec1进行逆置：";
	std::copy(vec1.begin(), vec1.end(), output);
}
void Demo09()
{
	std::vector<int> vec{ 1,3,5,7,9,1,3,5,7,9 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "vec元素为：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//inpalce_merge
	std::inplace_merge(vec.begin(), vec.begin() + 5, vec.end());
	std::cout << "\n调用inplace_merge之后vec元素为：\n";
	std::copy(vec.cbegin(), vec.cend(), output);

	//unique_copy
	std::vector<int> result1;
	unique_copy(vec.cbegin(), vec.cend(), std::back_inserter(result1));
	std::cout << "\n调用unique_copy之后result1元素为：\n";
	std::copy(result1.cbegin(), result1.cend(), output);

	//reverse_copy
	std::vector<int> result2;
	std::reverse_copy(vec.cbegin(), vec.cend(), back_inserter(result2));
	std::cout << "\n调用reverse_copy之后result2元素为：\n";
	std::copy(result2.cbegin(), result2.cend(), output);
	std::cout << std::endl;
}
void Demo10()
{
	std::vector<int> v1{ 1,2,3,4,5,6,7,8,9,10 };
	std::vector<int> v2{ 4,5,6,7,8 };
	std::vector<int> v3{ 4,5,6,11,15 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "v1: ";
	std::copy(v1.cbegin(), v1.cend(), output);
	std::cout << "\nv2: ";
	std::copy(v2.cbegin(), v2.cend(), output);
	std::cout << "\nv3: ";
	std::copy(v3.cbegin(), v3.cend(), output);

	//includes
	if (std::includes(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend()))
		std::cout << "\nv1 includes v2";
	else
		std::cout << "\na1 does not include v2";

	if (std::includes(v1.cbegin(), v1.cend(), v3.cbegin(), v3.cend()))
		std::cout << "\nv1 includes v3";
	else
		std::cout << "\nv1 does not include v3";

	//set_different  在集合1，但不在集合2
	std::vector<int> diff;
	auto ret1 = std::set_difference(v1.cbegin(), v1.cend(), 
		v2.cbegin(), v2.cend(), std::back_inserter(diff));
	std::cout << "\nset_difference of v1 and v2:\n";
	std::copy(diff.begin(), diff.end(), output);

	//set_intersection 集合1和集合2共有的
	std::vector<int> intersection;
	auto ret2 = std::set_intersection(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend(), std::back_inserter(intersection));
	std::cout << "\nset_intersection of v1, and v2:\n";
	std::copy(intersection.cbegin(), intersection.cend(), output);

	//set_symmtetric_difference  找集合1有，集合2没有；集合1没有，集合2有的元素。
	std::vector<int> symmtric_diff;
	std::set_symmetric_difference(v1.cbegin(), v1.cend(), v3.cbegin(), v3.cend(), std::back_inserter(symmtric_diff));
	std::cout << "\nset_symmetric_difference of v1 and v3 is:\n";
	std::copy(symmtric_diff.cbegin(), symmtric_diff.cend(), output);

	//set_union 合并两个集合
	std::vector<int> unionVec;
	std::set_union(v1.cbegin(), v1.cend(), v3.cbegin(), v3.cend(), std::back_inserter(unionVec));
	std::cout << "\nset_union of v1 and v3:\n";
	std::copy(unionVec.cbegin(), unionVec.cend(), output);
	std::cout << std::endl;
}
void Demo11()
{
	std::vector<int> vec{ 2, 2, 4, 4, 4, 6, 6, 6, 6, 8 };
	std::ostream_iterator<int> output(std::cout, " ");
	std::cout << "vec:";
	std::copy(vec.cbegin(), vec.cend(), output);

	//lower_bound
	auto low = std::lower_bound(vec.cbegin(), vec.cend(), 6);
	std::cout << "\nLower bound of 6 is element " << (low - vec.begin()) << " of vec";

	//upper_bound
	auto upp = std::upper_bound(vec.cbegin(), vec.cend(), 6);
	std::cout << "\nUpper bound of 6 is element " << (upp - vec.cbegin()) << " of vec";

	//equal_range
	auto ran = std::equal_range(vec.cbegin(), vec.cend(), 6);
	std::cout << "\n一步到位" << (ran.first - vec.cbegin()) << "--" << (ran.second - vec.cbegin());
}
void Demo12()
{
	std::cout << "The minimum of 12 and 7 is: " << std::min(12, 7);
	std::cout << "\nthe maximum of 12 and 7 is " << std::max(12, 7);

	auto ret = std::minmax(12, 7);
	std::cout << "\nmin :" << ret.first << " max:" << ret.second;

}

void Demo13()
{
	std::vector<int> origin;
	//int count = 1000000;
	int count = 1000;
	int i = 3;

	//先用10套, 同理100万
	//先求长度，排除1和2，vector大小是3-10一共8个数字count - 2, i 初始值是3
	generate_n(std::back_inserter(origin), count - 2, [&]() {return i++;});

	//
	auto ret = std::accumulate(origin.cbegin(), origin.cend(), std::vector<int>{2}, [](std::vector<int> &vec, int val) {
		
		if (std::none_of(vec.cbegin(), 
			std::find_if(vec.cbegin(), vec.cend(), [=](auto x) { return x > sqrt(val); }),
			[=](int prime) {return val % prime == 0; }))
		{
			vec.push_back(val);
		}

		return vec;
	});
	
	std::copy(ret.cbegin(), ret.cend(), std::ostream_iterator<int>(std::cout, " "));
	getchar();
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
	//Demo07();

	//copy_backward, merge, unique, reverse
	//Demo08();

	//inplace_merge, unique_copy, reverse_copy
	//Demo09();

	//includes, set_difference, set_intersection,
	//set_symmet, ric_difference, set_union
	//Demo10();

	//lower_bound, upper_bound, equal_range
	//Demo11();

	//min, max, minmax
	//Demo12();

	//求1-100万所有质数
	Demo13();

}