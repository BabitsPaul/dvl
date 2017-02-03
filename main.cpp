#include "parser.hpp"

#include <iostream>
#include <cstdio>

int main(int argc, char *argv[])
{
	dvl::pid id(0);

	id.set_type(7);

	//std::cout << id.get_type() << std::endl;
	printf("%x\n", id.get_type());

	return 0;
}
