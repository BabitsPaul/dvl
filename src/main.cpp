#include "parser.hpp"
#include "io.hpp"
#include "test.hpp"

#include <iostream>
#include <vector>
#include <sstream>
#include <boost/regex.hpp>

void setup_params(int, char**)
{

}

int main(int argc, char *argv[])
{
	setup_params(argc, argv);

	using namespace dvl;

	init_glob_locale();

	pid root(5, 5, TYPE_STRUCT),
			fork(5, 6, TYPE_FORK);

	std::wstringstream wss;
	wss << L"Hello123";

	routine_tree_builder rb;	// TODO if initialized in try-catch and exception is caught will cause SIGSEGVfire

	pid_table table;
	parser_routine_factory factory;
	parser_context pc(wss, rb, table, factory);

	parser_routine_factory::default_config(factory);

	charset_routine *r1 = new charset_routine({0, 0, TYPE_CHARSET}, L"[a-zA-Z]{3,}"),
					*r2 = new charset_routine({0, 0, TYPE_CHARSET}, L"[0-9]?");
	string_matcher_routine *r3 = new string_matcher_routine({0, 0, TYPE_STRING_MATCHER}, L"23");

	try{
		rb.logic({0, 0, TYPE_STRUCT}).mark_root().push_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_CHILD)
				.by_ptr(r1).pop_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_NEXT)
				.logic({0, 0, TYPE_STRUCT}).push_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_CHILD)
				.by_ptr(r2).pop_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_NEXT)
				.logic({0, 0, TYPE_STRUCT}).push_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_CHILD)
				.by_ptr(r3);

		parser p(pc);
		p.run();

		delete p.get_result();
	}catch(std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	// test_all();

	return 0;
}
