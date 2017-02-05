#include "parser.hpp"
#include "io.hpp"

#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

#include <codecvt>
#include <locale>
#include <clocale>

int main(int argc, char *argv[])
{
	dvl::init_glob_locale();

	std::wfstream wf("test.txt", std::ios::out | std::ios::binary);
	wf << L"ðĸſððłŋłð@ĸðſŋð€þ→ŋł²" << std::endl;
	wf.flush();
	wf.close();

	return 0;
}
