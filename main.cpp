#include "parser.hpp"
#include "io.hpp"

#include <iostream>
#include <cstdio>
#include <string>

int main(int argc, char *argv[])
{
	dvl::init_glob_locale();
	dvl::init_parser_module();

	dvl::id<int, 8, 8, 8, 8> _id;

	_id.set<0>(1);
	_id.set<1>(2);
	_id.set<2>(3);
	_id.set<3>(4);

	dvl::id_table<int, 8, 8, 8, 8> table;
	table.set_name(_id, 0, "A");
	table.set_name(_id, 1, "B");
	table.set_name(_id, 2, "C");
	table.set_name(_id, 3, "D");

	std::wcout << table.get_name(_id, 0).c_str() << std::endl;
	std::wcout << table.get_name(_id, 3).c_str() << std::endl;

	_id.set<2>(3);
	_id.set<3>(4);

	std::wcout << table.get_name(_id, 0).c_str() << std::endl;

	return 0;
}
