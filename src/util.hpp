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
#include <sstream>

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

	void print_stacktrace(std::ostream &str, unsigned int max_frames = 63, unsigned int ignore_top = 0);

	/////////////////////////////////////////////////////////////////////////////////////
	// object-lifetime-logger
	//

	/**
	 * A stream-provider to specify the output-stream to which the output
	 * of a life-time-logger will be sent.
	 *
	 * The default_stream_provider will send all output to std::cout. This behavior
	 * can be overriden by setting an alternative stream.
	 *
	 * If different streams should be used for different objects, alternative stream_providers
	 * can be defined. These need to provide a @c static variable @c str of type
	 * @c std::ostream
	 */
	struct default_stream_provider
	{
	public:
		constexpr static std::ostream &str = std::cout;
	};

	/**
	 * Life-time logging.
	 *
	 * Usage: class name : private life_time_log<name> ...
	 *
	 * Logging can be enabled by defining LIFE_TIME_TRACE
	 */
	template<typename T, bool log, typename stream_provider = default_stream_provider>
	class life_time_log
	{
	protected:
		life_time_log(){}
		life_time_log(const life_time_log&){}
		life_time_log(life_time_log&&){}

		virtual ~life_time_log(){}
	};

	template<typename T, typename stream_provider>
	class life_time_log<T, true, stream_provider>
	{
	protected:
		life_time_log()
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name())
					<< " @" << this << std::endl;
		}

		life_time_log(const life_time_log&)
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name()) << " copy-ctor"
					<< " @" << this << std::endl;
		}

		life_time_log(life_time_log&&)
		{
			std::cout << "Allocating new instance of " << demangle(typeid(T).name()) << " move-ctor"
					<< " @" << this << std::endl;
		}

		virtual ~life_time_log()
		{
			std::cout << "Deallocating instance of " << demangle(typeid(T).name())
					<< " @" << this << std::endl;
		}
	};


#ifdef LIFE_TIME_TRACE
	#define _LIFE_TIME_TRACE_FLAG true
#else
	#define _LIFE_TIME_TRACE_FLAG false
#endif //LIFE_TIME_TRACE

	template<typename T, typename stream_provider = default_stream_provider>
	class life_time_log_global : private life_time_log<T, _LIFE_TIME_TRACE_FLAG, stream_provider>{};

	////////////////////////////////////////////////////////////////////////////////////////
	// stack_trace_provider
	//

	template<bool active>
	class _stack_trace_provider
	{
	private:
		_stack_trace_provider(){}
	};

	template<>
	class _stack_trace_provider<true>
	{
	private:
		std::string stack;

		_stack_trace_provider(){
			std::stringstream ss;
			print_stacktrace(ss);

			ss >> stack;
		}
	public:
		virtual ~_stack_trace_provider(){}
	protected:
		const std::string get_stack_trace(){ return stack; }
	};

#ifdef STACK_TRACE_PROVIDER
	#define _STACK_TRACE_ENABLED true
#else
	#define _STACK_TRACE_ENABLED false
#endif

	class stack_trace_provider : private _stack_trace_provider<_STACK_TRACE_ENABLED>{};
}
#endif // _STACKTRACE_H_


#endif /* UTIL_HPP_ */
