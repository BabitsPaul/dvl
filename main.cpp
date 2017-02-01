#include "meta.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	int tmp1 = dvl::msum<int, 0, 3, 2>();

	std::cout << tmp1 << std::endl;

	return 0;
}
