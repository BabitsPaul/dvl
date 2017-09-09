#include "util.hpp"

#include <boost/core/demangle.hpp>


std::string
util::demangle(std::string mangled_name)
	throw(std::exception, std::bad_alloc, std::bad_typeid)
{
	return boost::core::demangle(mangled_name.c_str());
}

void
util::print_stacktrace(std::ostream &str, unsigned int max_frames, unsigned int ignore_top)
{
	void **buffer = new void*[max_frames];
	int ct = backtrace(buffer, max_frames);
	char **frames = backtrace_symbols(buffer, max_frames);

	// failed to read frames
	if(frames == nullptr)
	{
		str << "Failed to read stacktrace" << std::endl;
		return;
	}

	// write stack-trace
	for(char **frame = frames + ignore_top; frame < frames + ct; frame++)
		str << *frame << "\n";

	if(ct == max_frames)
		str << "Stack is too large - couldn't load all frames\n";

	str << std::endl;

	delete frames;
	delete buffer;
}
