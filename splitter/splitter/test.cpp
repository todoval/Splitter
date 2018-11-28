#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


template <typename T>
bool its_lvalue(T&&) //return true if T is Lvalue
{
	return std::is_lvalue_reference<T>{};
}

template<typename T>
void cArg(T value)
{
	if (its_lvalue(&value))
		std::cout << true;
	else std::cout << false;
}

int msain()
{
	char x = ':';
	cArg(':');
	std::cout << its_lvalue(':');
	return 0;
}

/*
template <typename T>
constexpr bool is_lvalue(T&&) //return true if T is Lvalue
{
return std::is_lvalue_reference<T>{};
}

template<typename T>
constexpr void checkArg(T && value)
{
static bool s = true;
if (!is_lvalue(std::forward<T>(value))) //if is Rvalue
{
//check wether the rvalue value is a char or not for compile error
if (typeid(value).name() != typeid(char).name()) //if type is not char
{
s = false;
static_assert(s, "Rvalue is not of a character type"); //throw error
}
}
std::cout << value << "is = " << is_lvalue(std::forward<T>(value)) << std::endl;
}

template <typename First, typename... Rest>
constexpr void checkArg(First && firstValue, Rest &&... rest)
{
checkArg(std::forward<First> (firstValue));
checkArg(std::forward<Rest> (rest) ...);
}

template<typename ... TList>
constexpr bool checkCompileErrors(TList && ... args) // check for compile errors - Rvalue is different than char error, multiple Lvalues in a row error
{
checkArg(std::forward<TList>(args) ...);
return true;
//return sizeof...(args) < 8;
}

}*/

