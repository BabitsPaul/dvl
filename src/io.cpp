#include "io.hpp"

#include <locale>
#include <codecvt>
#include <iostream>

void
dvl::init_glob_locale()
{
	//update default locale with utf8 facet
	std::locale::global(std::locale(std::locale(""), new std::codecvt_utf8<wchar_t>));

	//set for wcout
	//alternative: setlocale(LC_ALL, "")  - works only if default locale of OS is UTF8!!
	std::ios_base::sync_with_stdio(false);	//independent operation to c-streams
	std::wcout.imbue(std::locale(""));
}
