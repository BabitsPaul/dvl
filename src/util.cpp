#include "util.hpp"

#include <boost/core/demangle.hpp>


std::string
util::demangle(std::string mangled_name)
	throw(std::exception, std::bad_alloc, std::bad_typeid)
{
	return boost::core::demangle(mangled_name.c_str());
}

void
util::print_stacktrace(std::ostream &str, unsigned int max_frames)
{
	// storage array for stack trace address data
	void** addrlist = (void**) malloc(sizeof(void*) * (max_frames + 1));

	// retrieve current stack addresses
	int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

	if (addrlen == 0)
		throw std::exception();

	// resolve addresses into strings containing "filename(function+address)",
	// this array must be free()-ed
	char** symbollist = backtrace_symbols(addrlist, addrlen);

	// allocate string which will be filled with the demangled function name
	size_t funcnamesize = 256;
	char* funcname = (char*)malloc(funcnamesize);

	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (int i = 1; i < addrlen; i++)
	{
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p)
		{
			if (*p == '(')
				begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && begin_offset) {
				end_offset = p;
			break;
			}
		}

		if (begin_name && begin_offset && end_offset
			&& begin_name < begin_offset)
		{
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* ret = abi::__cxa_demangle(begin_name,
							funcname, &funcnamesize, &status);

			if (status == 0) {
				funcname = ret; // use possibly realloc()-ed string
				std::cout << " " << symbollist[i] << " : " << funcname << "+" << begin_offset << std::endl;
			}
			else {
			// demangling failed. Output function name as a C function with
			// no arguments.
			std::cout << " " << symbollist[i] << " : " << funcname << "()+" << begin_offset << std::endl;
			}
		}
		else
		{
			// couldn't parse the line? print the whole line.
			str << symbollist[i] << std::endl;
		}
	}

	free(funcname);
	free(symbollist);
	free(addrlist);
}
