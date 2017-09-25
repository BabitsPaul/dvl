#include "parser.hpp"
#include "io.hpp"
#include "test.hpp"
#include "dvl_syntax.hpp"

#include <iostream>
#include <vector>
#include <sstream>
#include <cwchar>

void setup_params(int, char**)
{

}

int main(int argc, char *argv[])
{
	using namespace dvl;
	using namespace syntax;

	setup_params(argc, argv);

	using namespace dvl;

	init_glob_locale();

	std::wistringstream str(L"Hello world");
	routine_tree_builder b;
	pid_table pt;
	parser_routine_factory f;
	parser_context c(str, b, pt, f);

	build_syntax_file_definition(c);

	return 0;
}
