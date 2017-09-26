#ifndef _IO_H_
#define _IO_H_

namespace dvl
{
	/**
	 * Initializes the global locale for this application. This
	 * enforces UTF8 for std::wcout/std::wcin and any wide file-streams
	 */
	void init_glob_locale();
}

#endif
