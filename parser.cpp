#include "parser.hpp"

////////////////////////////////////////////////////////////////////////////////
// id
//

uint8_t dvl::TYPE_INTERNAL = 0;
uint8_t dvl::TYPE_FORK = 1;
uint8_t dvl::TYPE_LOOP = 2;
uint8_t dvl::TYPE_STRUCT = 3;
uint8_t dvl::TYPE_FIXED = 4;

uint32_t dvl::GROUP_INTERNAL = 0;

dvl::pid dvl::EMPTY = dvl::pid(0l);
dvl::pid dvl::PARSER = dvl::pid(1l);

void
dvl::init_parser_module()
{
	//TODO register ids with name-register
}

////////////////////////////////////////////////////////////////////////////////
// lnstruct
//

dvl::lnstruct dvl::lnstruct::EMPTY = dvl::lnstruct(0l, 0l);

////////////////////////////////////////////////////////////////////////////////
//
//

void
dvl::parser_impl::parse(std::istream& is)
	throw(parser_exception)
{
	//check input stream
	if(is.eof() || is.bad())
		throw parser_exception(PARSER, "Input-stream erronous");

	//check base
	if(base == nullptr)
		throw parser_exception(PARSER, "No base defined");

	//initialize parser
	this->str = &is;

	
	while(!routines.empty())
	{

	}
}

void
dvl::parser_impl::reset()
{

}
