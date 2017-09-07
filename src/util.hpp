#ifndef UTIL_HPP_
#define UTIL_HPP_

#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

#if defined __GNUG__ || defined __clang__
	#define __ABI_AVAILABLE
	#include <cxxabi.h>
#endif

#include <string>
#include <exception>
#include <iostream>

namespace util
{
	/**
	 * Generates a demangled string of the provided mangled name.
	 * This function uses cxxabi, which is only available for clang and
	 * the GNU compiler-chain. If another compiler with different library is used
	 * this method will return it's input unaltered.
	 *
	 * @param mangled_name the name to demangle
	 * @return the demangled version of the provided name
	 * @throw exception if an unknown error occured
	 * @throw bad_alloc if the memory allocation failed
	 * @throw bad_typeid if the passed name is invalid
	 */
	std::string	demangle(std::string mangled_name)
		throw(std::exception, std::bad_alloc, std::bad_typeid);

	void print_stacktrace(std::ostream &str, unsigned int max_frames = 63);

	// TODO specify stream
	/**
	 * Life-time logging.
	 *
	 * Usage: class name : private life_time_log<name> ...
	 *
	 * Logging can be enabled by defining LIFE_TIME_TRACE
	 */
	template<bool log, typename T>
	class life_time_log
	{
	protected:
		life_time_log(){}
		life_time_log(const life_time_log&){}
		life_time_log(life_time_log&&){}

		virtual ~life_time_log(){}
	};

	template<typename T>
	class life_time_log<true, T>
	{
	protected:
		life_time_log()
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name()) << std::endl;
		}

		life_time_log(const life_time_log&)
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name()) << " copy-ctor" << std::endl;
		}

		life_time_log(life_time_log&&)
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name()) << " move-ctor" << std::endl;
		}

		virtual ~life_time_log()
		{
			std::cout << "Deallocating instance of " << demangle(typeid(T).name()) << std::endl;
		}
	};


#ifdef LIFE_TIME_TRACE
	#define _LIFE_TIME_TRACE_FLAG true
#else
	#define _LIFE_TIME_TRACE_FLAG false
#endif //LIFE_TIME_TRACE

	template<typename T>
	class life_time_log_global : private life_time_log<_LIFE_TIME_TRACE_FLAG, T>{};
}
#endif // _STACKTRACE_H_


#endif /* UTIL_HPP_ */