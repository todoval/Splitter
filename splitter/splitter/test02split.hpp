#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <type_traits>

namespace splitter_impl
{
	//compile error checking code

	template<bool has_lvalue, typename... TL1> struct trait;
	template<bool has_lvalue> //empty, end of recursion
	struct trait<has_lvalue>
	{
		template<typename... T >
		static void split(T &&... args)
		{
		}
	};
	template<bool has_lvalue, typename Head, typename... TL1>
	struct trait<has_lvalue, Head, TL1...>
	{
		template<typename... T>
		static void split(T &&... args)
		{
			static_assert(!(std::is_lvalue_reference_v<Head> && has_lvalue), "Compile error - 2 LValues in a row"); //two lvalues in a row
			static_assert(std::is_lvalue_reference_v<Head> || std::is_same_v<Head, char>, "Compile error - RValue not a char"); //rvalue not a char
			trait<std::is_lvalue_reference_v<Head>, TL1...>::split(std::forward<T>(args)...);
		}
	};

	template <typename ... TList>
	class splitClass
	{
	private:
	public:
		std::tuple<TList ...> arguments; //arguments stored in a tuple
		std::string data; //stream of data saved in a string that needs to be parsed

		template<typename ... TList>
		splitClass(TList &&... args) : arguments(std::forward<TList>(args)...) //constructor for arguments
		{
		}

		void set_data(std::string _data)
		{
			data = _data;
		}

		void parse() //actual parsing code
		{
			std::istringstream inputStream(data); //converting the string back into stream
			print(std::forward<std::tuple<TList ...>>(arguments), inputStream);
		}


		template<int index, int size, typename... Ts>
		struct print_tuple
		{
			void operator() (std::tuple<Ts ...> & t, std::istringstream & inputStream, bool wasRval)
			{
				char c;
				if (std::is_same<typename std::tuple_element<size - index - 1, std::tuple<Ts...>>::type, char>::value) //no need to check for arguments in the right order - they already have to be
				{
					//the type of the argument is rvalue
					if (!wasRval)
					{
						//therefore the argument before this rvalue has to be an lvalue
						print_tuple<index - 1, size, Ts...>{}(t, inputStream, true);
					}
					else
					{
						//therefore the argument before this rvalue was an rvalue too
						//just check if this rvalue is at the beginning of the stream
						inputStream.get(c);
						auto toCheck = std::get<size - index - 1>(t);
						//if (c != toCheck) //check wether the rvalue is the same as the argument
						{
							//throw std::logic_error("Delimiter not found"); 
						}
						print_tuple<index - 1, size, Ts...>{}(t, inputStream, true);
					}
				}
				else
				{
					//the type of the argument is lvalue
					auto && lval = std::get<size - index - 1>(t); //get lvalue
					auto rval = std::get<size - index>(t); //get next delimiter
					inputStream.get(c);
					std::string temp = "";
					while (c != rval && !inputStream.eof()) // loop getting single characters
					{
						temp = temp + c;
						inputStream.get(c);
					}
					if (inputStream.eof()) //logic error when there is an rvalue that is not found 
					{
						throw std::logic_error("Delimiter not found");
					}
					std::istringstream iss(temp);
					iss >> lval;
					// is lvalue, so go deeper in recursion to find an rvalue
					print_tuple<index - 1, size, Ts...>{}(t, inputStream, false);
				}
			}
		};

		template<int size, typename... Ts>
		struct print_tuple<0, size, Ts...>
		{
			void operator() (std::tuple<Ts ...> & t, std::istringstream & inputStream, bool wasRval)
			{
				if (std::is_same<typename std::tuple_element<0, std::tuple<Ts...>>::type, char>::value) //check the type of last argument
				{
					//the type is rvalue
				}
				{
					//the type is lvalue
					auto && lval = std::get<size - 1>(t);
					inputStream >> lval;
				}
			}
		};

		template<typename... Ts>
		void print(std::tuple<Ts ...> & t, std::istringstream & inputStream)
		{
			const auto size = std::tuple_size<std::tuple<Ts...>>::value;
			print_tuple<size - 1, size, Ts...>{}(t, inputStream, false);
		}

	};
}


namespace splitter
{
	template <typename ... TList>
	std::istringstream& operator >> (std::istringstream& is, splitter_impl::splitClass<TList ...>& sl)
	{
		std::string s(std::istreambuf_iterator<char>(is), {}); //convert stream into a string
		sl.set_data(s);
		sl.parse();
		return is;
	}

	
	template< typename ... TList>
	splitter_impl::splitClass<TList ...> split(TList && ... args)
	{
		splitter_impl::trait<false, TList...>::split(std::forward<TList>(args)...);
		splitter_impl::splitClass<TList ...> tempSpl(std::forward_as_tuple((args)...));
		return tempSpl;
	}

}

