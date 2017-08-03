#include "parser.hpp"
#include "io.hpp"

#include <iostream>
#include <vector>

void setup_params(int, char**)
{

}

int main(int argc, char *argv[])
{
	setup_params(argc, argv);

	using namespace dvl;

	init_glob_locale();

	pid root(5, 5, TYPE_STRUCT),
			loop(5, 6, TYPE_LOOP);

	try{
		routine *c = new echo_routine(L"Child"),
				*n = new echo_routine(L"Next");

		routine_tree_builder rb;
		rb.set_insertion_mode(routine_tree_builder::insertion_mode::AS_CHILD);
		rb.logic(root).mark_root().push_checkpoint().
				by_ptr(c).pop_checkpoint().
				set_insertion_mode(routine_tree_builder::insertion_mode::AS_NEXT).
				loop(loop, 1, 4).by_ptr(n);

		pid_table table;
		parser_context pc(std::wcin, rb, table);

		parser p(pc);
		p.run();
	}catch(std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
