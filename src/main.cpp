#include "parser.hpp"
#include "io.hpp"
#include "test.hpp"

#include <iostream>
#include <vector>
#include <sstream>
#include <cwchar>

void setup_params(int, char**)
{

}

int main(int argc, char *argv[])
{
	setup_params(argc, argv);

	using namespace dvl;

	//init_glob_locale();

	//char c[20];
	//scanf("Please enter some text %s", c);
	//printf("You entered %s\n", c);

	setlocale(LC_ALL, "");
	wchar_t wc[100];
	printf("please enter a text: ");
	scanf("%ls", wc);
	printf("you entered: %ls\n", wc);

	return 0;
}
