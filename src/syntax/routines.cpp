#include "routines.hpp"

/////////////////////////////////////////////////////////////////////////////////
// charset_routine
//

void
dvl::charset_routine::init_matcher(charset_routine &r)
	throw(parser_exception)
{
	const wchar_t *def_str = r.def.c_str();
	const wchar_t *end = def_str + r.def.length();

	// spaces
	for(; def_str < end; def_str++)
		if(*def_str != L' ' && *def_str != L'\t')
			break;

	// inversion
	bool inverted = (*def_str == L'!');

	if(inverted)
	{
		def_str++;

		for(; def_str < end; def_str++)
			if(*def_str != L' ' && *def_str != L'\t')
				break;
	}

	// start of charset-definition
	if(*def_str != L'[')
		throw parser_exception(r.get_pid(), "Invalid charset-definition: unexpected character");

	def_str++;

	if(def_str == end)
		throw parser_exception(r.get_pid(), "Unexpected end of definition");

	// charset definition extraction
	const wchar_t *start_def = def_str,
					*end_def = nullptr;

	bool escaped = false;
	for(; def_str < end; def_str++)
	{
		if(*def_str == L']' && !escaped)
		{
			// found end of definition
			end_def = def_str;	// NOTE: end_def = def_str - 1
			def_str++;
			break;
		}
		else if(*def_str == L'\\')
			escaped = !escaped;
		else
			escaped = false;
	}

	if(end_def == start_def)
		throw parser_exception(r.get_pid(), "The characterset mustn't be empty");

	if(end_def == nullptr)
		throw parser_exception(r.get_pid(), "Unterminated charset definition");

	// transform escape-sequences
	std::vector<wchar_t*> cr_first;

	wchar_t *rep_str = new wchar_t[end_def - start_def];

	wchar_t *o = rep_str;
	const wchar_t *c = start_def;

	escaped = false;
	while(c < end_def)	// NOTE: while(c <= end_def)
	{
		if(*c == L'\\')
		{
			if(escaped)
				*o++ = *c;

			escaped = !escaped;
		}
		else if(escaped)
		{
			if(*c == L'-' || *c == L']' || *c == L'[')
			{
				*o++ = *c;
			}
			else if(*c == L'U')
			{
				if(c + 8 > end_def)
				{
					delete rep_str;
					throw parser_exception(r.get_pid(), "Incomplete character-name");
				}

				std::wstring def(c, 8);
				unsigned int num = std::stoi(def, nullptr, 16);

				*o++ = (wchar_t) num;

				c += 8;
			}
			else if(*c == L'u')
			{
				if(c + 4 > end_def)
				{
					delete rep_str;	// TODO replace by unique-ptr or vector???
					throw parser_exception(r.get_pid(), "Incomplete character-name");
				}

				std::wstring def(c, 4);
				unsigned int num = std::stoi(def, nullptr, 16);

				*o++ = (wchar_t) num;

				c += 4;
			}
			else
			{
				// TODO support \s???
				switch(*c)
				{
				case L'a':
					*o++ = L'\a';
					break;
				case L'b':
					*o++ = L'\b';
					break;
				case L'f':
					*o++ = L'\f';
					break;
				case L'n':
					*o++ = L'\n';
					break;
				case L'r':
					*o++ = L'\r';
					break;
				case L't':
					*o++ = L'\t';
					break;
				case L'v':
					*o++ = L'\t';
					break;
				default:
					delete rep_str;
					throw parser_exception(r.get_pid(), "Unknown escape-sequence");
				}
			}

			escaped = false;
		}
		else
		{
			if(*c == L'-')
				cr_first.emplace_back(o - 1);
			else if(*c == L'[')
			{
				delete rep_str;
				throw parser_exception(r.get_pid(), "Unescaped [ in charset-definition");
			}
			else
				*o++ = *c;
		}

		c++;
	}

	// check char-ranges for correctness
	if(!cr_first.empty())
	{
		wchar_t *last = cr_first[0];
		for(auto it = cr_first.begin() + 1; it != cr_first.end(); it++)
			if(*it - last < 2)
			{
				delete rep_str;
				throw parser_exception(r.get_pid(), "Invalid char-range definition");
			}
	}


	// process definition sequence
	cr_first.emplace_back(o);	// dummy value

	wchar_t *rs = rep_str;
	auto it = cr_first.begin();

	std::set<wchar_t> single;
	std::vector<std::pair<wchar_t, wchar_t>> cr;

	while(rs < o)
	{
		if(*it < rs)
			it++;

		if(*it == rs)
		{
			if(rs + 1 >= o)
			{
				delete rep_str;
				throw parser_exception(r.get_pid(), "Incomplete char-range");
			}
			else
			{
				cr.emplace_back(std::make_pair(*rs, *(rs + 1)));
				rs += 2;
			}
		}
		else
		{
			single.emplace(*rs);
			rs++;
		}
	}

	delete rep_str;

	// validate char-ranges
	if(std::find_if(cr.begin(), cr.end(), [](auto v){ return v.second < v.first; }) != cr.end())
		throw parser_exception(r.get_pid(), "Range out of order");

	// function
	r.matcher = [&, single, cr](wchar_t c)->bool{
		return (single.find(c) != single.end() ||
				std::find_if(cr.begin(), cr.end(), [c](auto v)->bool{
					return v.first <= c && c <= v.second;
				}) != cr.end()) ^ inverted;
	};

	// spaces
	for(; def_str < end; def_str++)
		if(*def_str != L' ' && *def_str != L'\t')
			break;

	// extract repetition-specification
	c = def_str;
	if(c == end)
		r.min_repetition = r.max_repetition = 1;
	else if(*c == L'*')
	{
		r.min_repetition = 0;
		r.max_repetition = _INFINITY;
	}
	else if(*c == L'+')
	{
		r.min_repetition = 1;
		r.max_repetition = _INFINITY;
	}
	else if(*c == L'?')
	{
		r.min_repetition = 0;
		r.max_repetition = 1;
	}
	else if(*c == L'{')
	{
		const wchar_t *open = c;

		for(; c < end && *c != L','; c++);
		const wchar_t *comma = c;
		if(*comma != L',')
			throw parser_exception(r.get_pid(), "Separator not found in repetiion-specification");

		for(; c < end && *c != L'}'; c++);
		const wchar_t *close = c;
		if(*close != L'}')
			throw parser_exception(r.get_pid(), "Terminator not found in repetition-specification");

		if(open + 1 == comma)
			r.min_repetition = 0;
		else
			try{
				r.min_repetition = std::stoi(std::wstring(open + 1, comma - open - 1));
			}catch(const std::out_of_range &e)
			{
				throw parser_exception(r.get_pid(), "Lower bound is too large");
			}catch(const std::invalid_argument &e)
			{
				throw parser_exception(r.get_pid(), "Failed to convert lower bound to integer");
			}

		if(comma + 1 == close)
			r.max_repetition = _INFINITY;
		else
			try{
				r.max_repetition = std::stoi(std::wstring(comma + 1, close - comma - 1));
			}catch(const std::out_of_range &e)
			{
				throw parser_exception(r.get_pid(), "Upper bound is too large");
			}catch(const std::invalid_argument &e)
			{
				throw parser_exception(r.get_pid(), "Failed to convert upper bound to integer");
			}
	}
	else
		throw parser_exception(r.get_pid(), "Expected repetition-specification");
}

/////////////////////////////////////////////////////////////////////////////////
// tree builder
//

void
dvl::routine_tree_builder::insert_node(routine *rn)
	throw(parser_exception)
{
	routines.emplace(rn);

	try{
		if(r == nullptr)
		{
			//first node inserted => can't set into relation with
			//any other node
			r = rn;
			return;
		}
		else if(finalized.find(r) != finalized.end())
			throw parser_exception(PARSER, "Routine is marked as non-modifiable");

		if(rn == nullptr)
			throw parser_exception(PARSER, "No routine specified");

		switch(ins_mode)
		{
		case insertion_mode::NONE:
			throw parser_exception(PARSER, "No insertion-mode specified");
		case AS_CHILD:
			if(r->get_pid().get_type() != TYPE_STRUCT)
				throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

			((struct_routine*) r)->set_child(rn);
			break;
		case insertion_mode::AS_NEXT:
			if(r->get_pid().get_type() != TYPE_STRUCT)
				throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

			((struct_routine*) r)->set_next(rn);
			break;
		case insertion_mode::AS_FORK:
			if(r->get_pid().get_type() != TYPE_FORK)
				throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

			((fork_routine*) r)->add_fork(rn);
			break;
		case insertion_mode::AS_LOOP:
			if(r->get_pid().get_type() != TYPE_LOOP)
				throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

			((loop_routine*) r)->set_loop(rn);
			break;
		default:
			throw parser_exception(PARSER, "Invalid insertion-mode");
		}
	}catch(const parser_exception &e)
	{
		// if insertion fails delete the routine and throw an error
		delete rn;
		throw;
	}
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::mark_root()
	throw(parser_exception)
{
	if(r == nullptr)
		throw parser_exception(PARSER, "No current root");

	root = r;

	return *this;
}

dvl::routine*
dvl::routine_tree_builder::get()
	throw(parser_exception)
{
	if(root == nullptr)
		throw parser_exception(PARSER, "No root specified");

	return root;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::name(std::wstring str)
	throw(parser_exception)
{
	if(r == nullptr)
		throw parser_exception(PARSER, "No current root");

	name_table[str] = r;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::operator[](std::wstring str)
	throw(parser_exception)
{
	auto iter = name_table.find(str);
	if(iter == name_table.end())
		throw parser_exception(PARSER, "No routine with this name");

	r = iter->second;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::push_checkpoint()
	throw(parser_exception)
{
	if(r == nullptr)
		throw parser_exception(PARSER, "No current routine");

	checkpoints.push(r);

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::pop_checkpoint()
	throw(parser_exception)
{
	if(!checkpoints.size())
		throw parser_exception(PARSER, "No checkpoints available");

	r = checkpoints.top();
	checkpoints.pop();

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::loop(pid id, unsigned int min_iterations, unsigned int max_iterations)
{
	routine *rn = new loop_routine(id, nullptr, min_iterations, max_iterations);
	insert_node(rn);

	ins_mode = insertion_mode::AS_LOOP;
	r = rn;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::fork(pid id)
{
	routine *rn = new fork_routine(id, std::vector<routine*>());
	insert_node(rn);

	ins_mode = insertion_mode::AS_FORK;
	r = rn;
	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::logic(pid id)
{
	routine *rn = new struct_routine(id, nullptr, nullptr);
	insert_node(rn);

	ins_mode = insertion_mode::AS_CHILD;
	r = rn;
	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::by_ptr(routine *r)
{
	insert_node(r);

	ins_mode = insertion_mode::NONE;
	this->r = r;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::set_insertion_mode(insertion_mode m)
{
	ins_mode = m;

	return *this;
}

dvl::routine*
dvl::routine_tree_builder::get_current()
{
	return r;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::detach()
{
	// reset the currently-active-ptr to the nullptr
	r = nullptr;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::by_name(std::wstring name)
	throw(parser_exception)
{
	auto it = name_table.find(name);

	if(it == name_table.end())
		throw parser_exception(PARSER, "No routine with this name is registered");

	insert_node(it->second);
	r = it->second;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::finalize(std::wstring name)
	throw(parser_exception)
{
	auto it = name_table.find(name);

	if(it == name_table.end())
		throw parser_exception(PARSER, "No routine with this name is registered");

	finalized.emplace(it->second);

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::match_string(pid id, std::wstring match)
{
	routine *rn = new string_matcher_routine(id, match);
	insert_node(rn);

	ins_mode = insertion_mode::NONE;
	r = rn;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::match_set(pid id, std::wstring set_def)
{
	routine *rn = new charset_routine(id, set_def);
	insert_node(rn);

	ins_mode = insertion_mode::NONE;
	r = rn;

	return *this;
}

dvl::routine_tree_builder&
dvl::routine_tree_builder::lambda(pid id, lambda_routine::p_func f)
{
	routine *rn = new lambda_routine(id, f);
	insert_node(rn);

	ins_mode = insertion_mode::NONE;
	r = rn;

	return *this;
}
