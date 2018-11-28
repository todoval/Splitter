#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <type_traits>
#include <sstream>

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

	inline bool compareChar(void * a, char b)
	{
		return *(char *)a == b;
	}

	inline char getChar(void * a)
	{
		return *(char *)a;
	}

	template<int index, int size, typename... TList>
	struct splitter;
	inline std::stringstream newGetLine(std::istream &inputStream, char delim)
	{
		std::stringstream result;
		if (inputStream.eof())
			throw std::logic_error("Delimiter not found");
		while (inputStream.peek() != delim && !inputStream.eof())
		{
			if (inputStream.eof() && (!compareChar(&delim, '\n'))) //logic error when there is an rvalue argument that is not found 
			{
				throw std::logic_error("Delimiter not found");
			}
			char c;
			inputStream.get(c);
			result << c;
		}
		char c;
		inputStream.get(c);
		return result;
	}

	template <typename T>
	inline void readToLval(std::istream &inputStream, T &lval, char delim)
	{
		std::stringstream iss = newGetLine(inputStream, delim);
		iss >> lval; //input in lval
		if (!iss.eof() && (typeid(lval).name() != typeid(char).name())) //therefore the stream didn't end because lval couldn't eat up all chars
		{
			throw std::logic_error("Conversion by >> character failed");
		}
	}

	template<int index, int size, typename... TList>
	struct splitter
	{
		void operator() (std::tuple<TList ...> t, std::istream & inputStream, bool wasRval)
		{
			if (std::is_same<typename std::tuple_element<size - index - 1, std::tuple<TList...>>::type, char>::value)
			{
				//the type of the argument is rvalue
				if (!wasRval)
				{
					//therefore the argument before this rvalue has to be an lvalue
					splitter<index - 1, size, TList...>{}(t, inputStream, true);
				}
				else
				{
					//therefore the argument before this rvalue was an rvalue too
					//just check if this rvalue is at the beginning of the stream
					char c;
					inputStream.get(c);
					if (!compareChar(&std::get<size - index - 1>(t), c))
					{
						throw std::logic_error("Delimiter not found");
					}
					splitter<index - 1, size, TList...>{}(t, inputStream, true);
				}
			}
			else
			{
				//the value on the size - index - 1 index is lvalue
				//therefore the next value must be a rvalue -> we can access it because index is at least 1
				auto && lval = std::get<size - index - 1>(t); //get lvalue
				auto rval = getChar(&std::get<size - index>(t)); //get next delimiter
				readToLval(inputStream, lval, rval);
				splitter<index - 1, size, TList...>{}(t, inputStream, false);
			}
		}
	};

	template<int size, typename... TList>
	struct splitter <0, size, TList...> //end of recursion
	{
		void operator() (std::tuple<TList ...> t, std::istream & inputStream, bool wasRval)
		{
			if (std::is_same<typename std::tuple_element<size - 1, std::tuple<TList...>>::type, char>::value) //check the type of last argument
			{
				//the type of the argument is rvalue
				if (wasRval)
				{
					//therefore the argument before this rvalue was also a rvalue
					//just check if this rvalue is at the beginning of the stream
					char c;
					inputStream.get(c);
					if (!compareChar(&std::get<size - 1>(t), c))
					{
						throw std::logic_error("Delimiter not found");
					}
				}

			}
			else
			{
				//the type of last arg is lvalue
				//therefore everything that is in the inputStream will be in lval
				auto && lval = std::get<size - 1>(t);
				readToLval(inputStream, lval, '\n'); //no next argument, use new line as delim
			}
		}
	};

}

namespace splitter
{
	//splitClass 
	template <typename ... TList>
	class splitClass
	{
	private:
	public:
		std::tuple<TList ...> arguments; //arguments stored in a tuple
		template<typename ... Ts>
		splitClass(Ts &&... args) : arguments(std::forward<Ts>(args)...) //constructor for arguments
		{
		}
		void parse(std::istream & inputStream)
		{
			const auto size = std::tuple_size<std::tuple<TList...>>::value; //get size of the tuple
			splitter_impl::splitter<size - 1, size, TList...>{}(std::forward<std::tuple<TList ...>>(arguments), inputStream, false); //pass all arguments including inputStream to the splitter struct
		}
	};

	template<typename ... TList>
	inline std::istream &operator >> (std::istream &inputStream, splitClass<TList...> && _spl)
	{
		_spl.parse(inputStream);
		return inputStream;
	}

	template< typename ... TList>
	inline splitClass<TList ...> split(TList && ... args)
	{
		splitter_impl::trait<false, TList...>::split(std::forward<TList>(args)...);
		splitClass<TList ...> tempSpl(std::forward_as_tuple((args)...)); 
		return tempSpl;
	}

}