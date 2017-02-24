#include "parser.hpp"

#include <queue>

////////////////////////////////////////////////////////////////////////////////
// id
//

dvl::pid_table dvl::pt;

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
	id<int, 8, 8> i({4, 4});

	pt.set_name(pid().set_group(GROUP_INTERNAL), 0, "TYPE_INTERNAL");
}

////////////////////////////////////////////////////////////////////////////////
// lnstruct
//

dvl::lnstruct dvl::lnstruct::EMPTY = dvl::lnstruct(0l, 0l);

//no-recursively deletes deeply nested structure of lnstructs
dvl::lnstruct::~lnstruct()
{
	std::queue<lnstruct*> q;
	q.push(this);

	while(!q.empty())
	{
		lnstruct *ln = q.front();
		q.pop();

		if(ln == nullptr || ln == &EMPTY)
			continue;

		q.push(ln->next);
		q.push(ln->child);

		//avoid recursive deletion
		ln->next = ln->child = nullptr;

		delete ln;
	}
}

////////////////////////////////////////////////////////////////////////////////
// paser_impl
//

//TODO simple parser-tree traversal (no exception-handling, just printing ids, etc)

void
dvl::parser_impl::parse(std::wistream& is)
	throw(parser_exception)
{
	//check input stream
	if(is.eof() || is.bad())
		throw parser_exception(PARSER, "Input-stream erronous");

	//check base
	if(base == nullptr)
		throw parser_exception(PARSER, "No base defined");

	//push base as starting-value onto stack
	routines.push(op_enc(base, false));
	lnstructs.push(&lnstruct::EMPTY);

	//initialize parser
	this->str = &is;

	while(!routines.empty())
	{
		//retrieve next routine
		routine *r = routines.top().r;

		//unhandled exceptions will be thrown to caller of parse
		routine::next n = r->parse(*this);

		if(e != nullptr)
		{
			//handled exception thrown by routine => handled by parent
			//wind back by stepping up to parent and deleting all produced children
			delete r;
			routines.pop();

			delete lnstructs.top();
			lnstructs.pop();
		}

		if(n.next == nullptr)
		{
			//no next operation on this level => step down until an operation that
			//is repeatable is reached or the stack is empty
			while(!routines.empty() && !routines.top().repeat)
			{
				routines.pop();
				lnstructs.pop();
			}
		}else if(n.as == routine::next::AS_CHILD)
			//next routine should run as child => just push routine onto stack
			routines.push(op_enc(n.next, false));
		else
		{
			//next routine should run as peer => make room for routine
			//TODO place lnstruct as next vs child
			routines.pop();
			lnstructs.pop();

			routines.push(op_enc(n.next, false));
		}
	}

	//if exception wasn't cleared, throw to caller
	check_exception_status();
}

void
dvl::parser_impl::set_repeat_flag(bool repeat)
	throw(dvl::parser_exception)
{
	if(routines.empty())
		throw parser_exception(PARSER, "No routine for repetition available");

	routines.top().repeat = repeat;
}

void
dvl::parser_impl::store(lnstruct *l)
	throw(parser_exception)
{
	//TODO means to set insertion-mode for lnstruct

}

void
dvl::parser_impl::throw_to_prnt(parser_exception ex)
{
	//TODO exceptions throw by

	//exception-status is already set
	if(e == &ex)
		return;

	if(e != nullptr)
		delete e;

	//TODO allocate copy of exception
}

void
dvl::parser_impl::check_exception_status()
	throw(parser_exception)
{

}

std::wistream&
dvl::parser_impl::get_stream()
{
	return *str;
}
