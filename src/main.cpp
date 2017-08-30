#include "parser.hpp"
#include "io.hpp"
#include "test.hpp"

#include <iostream>
#include <vector>
#include <sstream>

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

	util::print_stacktrace(std::cout);	// TODO no stacktrace printed

	std::wstringstream wss;
	wss << L"Hello";

	routine *h = new string_matcher_routine({0, 0, dvl::TYPE_STRING_MATCHER}, L"Hello"),
			*b = new string_matcher_routine({0, 0, dvl::TYPE_STRING_MATCHER}, L"Bye"),
			*e = new echo_routine(L"Success");

	routine_tree_builder rb;

	pid_table table;
	parser_routine_factory factory;
	parser_context pc(wss, rb, table, factory);

	try{
		rb.fork(fork).mark_root().push_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_FORK)
				.by_ptr(b).pop_checkpoint().push_checkpoint()
				.set_insertion_mode(routine_tree_builder::insertion_mode::AS_FORK)
				.logic(root).set_insertion_mode(routine_tree_builder::insertion_mode::AS_CHILD).push_checkpoint()
				.by_ptr(h).pop_checkpoint().set_insertion_mode(routine_tree_builder::insertion_mode::AS_NEXT)
				.logic(root).set_insertion_mode(routine_tree_builder::AS_CHILD)
				.by_ptr(e);

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
