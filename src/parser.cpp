#include "parser.hpp"

#include <queue>

////////////////////////////////////////////////////////////////////////////////
// id
//

const std::wstring
dvl::pid_table::UNKNOWN = L"Unknown";

dvl::pid_table dvl::pt;

const std::map<uint8_t, std::wstring>
dvl::pid_table::types = {{TYPE_INTERNAL,			L"TYPE_INTERNAL"},
						 {TYPE_FORK, 				L"TYPE_FORK"},
					 	 {TYPE_LOOP,				L"TYPE_LOOP"},
					 	 {TYPE_STRUCT,				L"TYPE_STRUCT"},
					 	 {TYPE_STRING_MATCHER,		L"TYPE_STRING_MATCHER"},
						 {TYPE_EMPTY,				L"TYPE_EMPTY"}};

void
dvl::init_parser_module()
{
	//register internal ids
	pt.set_group(pid().set_group(GROUP_INTERNAL), L"INTERNAL");

	pt.set_element(EMPTY, L"EMPTY");
	pt.set_element(PARSER, L"PARSER");

	//register diagnostic routines
	pt.set_group(pid().set_group(GROUP_DIAGNOSTIC), L"DIAGNOSTIC");

	pt.set_element(ECHO, L"ECHO");
}

////////////////////////////////////////////////////////////////////////////////
// lnstruct
//

//no-recursively deletes deeply nested structure of lnstructs
dvl::lnstruct::~lnstruct()
{
	std::queue<lnstruct*> q;
	q.push(this);

	while(!q.empty())
	{
		lnstruct *ln = q.front();
		q.pop();

		if(ln == nullptr)
			continue;

		q.push(ln->next);
		q.push(ln->child);

		//avoid recursive deletion
		ln->next = ln->child = nullptr;

		delete ln;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// tree builder
//

void
dvl::routine_tree_builder::insert_node(routine *rn)
	throw(parser_exception)
{
	if(r == nullptr)
	{
		//first node inserted => can't set into relation with
		//any other node
		r = rn;
		return;
	}

	if(rn == nullptr)
		throw parser_exception(PARSER, "No routine specified");

	switch(ins_mode)
	{
	case insertion_mode::NONE:
		throw parser_exception(PARSER, "No insertion-mode specified");
	case AS_CHILD:
		if(r->get_pid().get_type() != TYPE_STRUCT)
			throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

		((logic_routine*) r)->set_child(rn);
		break;
	case insertion_mode::AS_NEXT:
		if(r->get_pid().get_type() != TYPE_STRUCT)
			throw parser_exception(PARSER, parser_exception::ptree_builder_invalid_routine());

		((logic_routine*) r)->set_next(rn);
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
	routine *rn = new logic_routine(id, nullptr, nullptr);
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

///////////////////////////////////////////////////////////////////////////////////////////
// parser_routine_factory
//

/**
 * just an implementation of the standard-types implemented as default. This structure
 * solely serves to keep implementations of parser_routine from floating freely in the code.
 * It should be considered as a collection of utilies or a separate namespace
 *
 * @see dvl::parser_routine_factory
 */
struct routine_factory_util
{
	//no need to check pids, as these will already be checked by models used to
	//create implementations

	//TODO unify error-messages (in progress)
	//TODO assert routines will run precisely once (possibly via embedding calls to
	// routine_interface into parser_routine
	friend class dvl::parser_routine_factory;

	class parser_fork_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		dvl::fork_routine* fr;

		std::vector<dvl::routine*>::iterator f_iter;

		dvl::lnstruct *base;
		dvl::lnstruct *last_success;
	public:
		parser_fork_routine(dvl::fork_routine* fr)
			: dvl::parser_routine_factory::parser_routine(fr->get_pid())

		{
			this->fr = fr;
			base = nullptr;
			last_success = nullptr;
			f_iter = fr->forks().begin();
		}

		~parser_fork_routine(){}	// no need for cleanup, only inherits attributes by this routine

		dvl::lnstruct* get_result()
		{
			return base;
		}

		void place_child(dvl::lnstruct* l)
			throw(dvl::parser_exception)
		{
			if(last_success != nullptr)
				throw dvl::parser_exception(fr->get_pid(), "Found multiple matching definitions");

			last_success = l;
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
			// routine runs for the first time
			if(f_iter == fr->forks().begin())
				base = new dvl::lnstruct(dvl::routine::get_pid(), ri.get_istream().tellg());
			else
			{
				// catch exception-status of last routine run (no need to handle)
				try{
					ri.check_child_exception();
				}catch(const dvl::parser_exception& e)
				{
					//not much to do here
				}
			}

			if(f_iter == fr->forks().end())
			{
				if(last_success == nullptr)
					throw dvl::parser_exception(fr->get_pid(), "No matching definition found");

				base->get_child() = last_success;

				return;
			}
			else
			{
				ri.run_as_child(*f_iter);
				f_iter++;
			}
		}
	};

	class parser_empty_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		dvl::lnstruct *ln = nullptr;
	public:
		parser_empty_routine() : dvl::parser_routine_factory::parser_routine(dvl::EMPTY){}

		dvl::lnstruct* get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct* l)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(dvl::EMPTY, "Invalid operation - Empty routines may not have children");
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
			if(ln != nullptr)
				throw dvl::parser_exception(get_pid(), dvl::parser_exception::routine_invalid_repeat());

			ln = new dvl::lnstruct(dvl::EMPTY, ri.get_istream().tellg());
		}
	};

	class parser_loop_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		unsigned int run_ct = 0;

		dvl::loop_routine *r;

		dvl::lnstruct *ln;
		dvl::lnstruct **insert_pos;
	public:
		parser_loop_routine(dvl::loop_routine *r) : dvl::parser_routine_factory::parser_routine(r->get_pid()),
			r(r), ln(nullptr), insert_pos(nullptr)
		{}

		dvl::lnstruct* get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct* ln)
			throw(dvl::parser_exception)
		{
			if(insert_pos == nullptr)
				throw dvl::parser_exception(get_pid(), "Can't build output yet");

			//place new lnstruct at predefined position and update
			*insert_pos = ln;
			insert_pos = &ln->get_next();
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
			// TODO wrap in simple logic_routine to assert next-slot is free in lnstruct

			//initialize ln and insert_pos
			if(ln == nullptr)
			{
				ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());
				insert_pos = &ln->get_child();
			}

			// check status of last child-run
			try{
				ri.check_child_exception();
			}catch(dvl::parser_exception& e)
			{
				// rethrow error, if not sufficient iterations were completed, else
				// terminate the loop.
				if(run_ct < r->get_min_iterations() ||
						r->get_min_iterations() == dvl::loop_routine::_INFINITY)
					throw e;
				else
					return;
			}

			// maximum iterations count reached -> terminate
			if(run_ct == r->get_max_iterations() &&
				r->get_max_iterations() != dvl::loop_routine::_INFINITY)
			{
				return;
			}

			ri.repeat();					//mark routine for repetition
			ri.run_as_child(r->get_loop());	//run routine to loop over as next
			run_ct++;						//increment loop count
		}
	};

	class parser_logic_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		dvl::logic_routine *r;

		dvl::lnstruct *ln;
	public:
		parser_logic_routine(dvl::logic_routine *r): dvl::parser_routine_factory::parser_routine(r->get_pid()),
			r(r), ln(nullptr)
		{}

		dvl::lnstruct *get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct* l)
			throw(dvl::parser_exception)
		{
			if(ln == nullptr)
				throw dvl::parser_exception(get_pid(), "Can't place lnstruct");

			ln->get_child() = l;
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			// ensure lnstruct runs precisely once
			// and initialize ln
			if(ln == nullptr)
				ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());
			else
				throw dvl::parser_exception(get_pid(), "Invalid state, cant run logic-routine twice");

			// place routines to run
			if(r->get_child() != nullptr)
				ri.run_as_child(r->get_child());

			if(r->get_next() != nullptr)
				ri.run_as_next(r->get_next());
		}
	};

	class parser_matcher_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		const std::wstring &s;

		dvl::lnstruct *ln;
	public:
		parser_matcher_routine(dvl::string_matcher_routine *r): dvl::parser_routine_factory::parser_routine(r->get_pid()),
			s(r->get_str()), ln(nullptr)
		{}

		dvl::lnstruct *get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct *l)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), "Matcher routine accepts no child-elements");
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			if(ln != nullptr)
				throw dvl::parser_exception(get_pid(), "Matcher routine is not repeatable");

			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			// compare input to predefined string
			const wchar_t *str = s.c_str();
			for(const wchar_t *c = str; c < str + s.length(); c++)
				if((wint_t) *c != ri.get_istream().get())	//TODO handle WEOF
					throw dvl::parser_exception(get_pid(), "Mismatch in string");
		}
	};

	class parser_echo_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		dvl::lnstruct *ln;

		const std::wstring& str;
	public:
		parser_echo_routine(dvl::echo_routine *r) : dvl::parser_routine_factory::parser_routine(r->get_pid()),
			ln(nullptr), str(r->get_msg())
		{

		}

		dvl::lnstruct *get_result(){ return ln; }

		void place_child(dvl::lnstruct *l)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), "Can't place child in echo-routine");
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			if(ln != nullptr)
				throw dvl::parser_exception(get_pid(), "Can't run routine twice");

			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			std::wcout << str << std::endl;
		}
	};

	class parser_stack_routine : public dvl::parser_routine_factory::parser_routine
	{
	private:
		dvl::lnstruct *ln;
	public:
		parser_stack_routine(dvl::stack_trace_routine *r) : dvl::parser_routine_factory::parser_routine(r->get_pid()),
			ln(nullptr){}

		dvl::lnstruct *get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct *l)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), "Can't place child-element in stack-trace-routine");
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			if(ln != nullptr)
				throw dvl::parser_exception(get_pid(), "Cant run routine twice");

			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			//TODO print status via visitor-pattern
		}
	};
};

dvl::parser_routine_factory::parser_routine_factory()
{
	register_transformation(TYPE_FORK, [](routine* r)->routine_factory_util::parser_fork_routine*{
		return new routine_factory_util::parser_fork_routine((fork_routine*) r);
	});

	register_transformation(TYPE_EMPTY, [](routine* r)->routine_factory_util::parser_empty_routine*{
		return new routine_factory_util::parser_empty_routine();
	});

	register_transformation(TYPE_LOOP, [](routine* r)->routine_factory_util::parser_loop_routine*{
		return new routine_factory_util::parser_loop_routine((loop_routine*) r);
	});

	register_transformation(TYPE_STRUCT, [](routine *r)->routine_factory_util::parser_logic_routine*{
		return new routine_factory_util::parser_logic_routine((logic_routine*) r);
	});

	register_transformation(TYPE_INTERNAL, [](routine *r)->parser_routine_factory::parser_routine*{
		return nullptr;	//TODO split TYPE_INTERNAL into relevant types
	});
}

dvl::parser_routine_factory::parser_routine*
dvl::parser_routine_factory::build_routine(routine* r)
	throw(parser_exception)
{
	if(transformations.count(r->get_pid().get_type()))
		return transformations[r->get_pid().get_type()](r);
	else
		throw parser_exception(PARSER, "No generator for routine of specified type found");
}

void
dvl::parser_routine_factory::register_transformation(uint8_t type, transform t)
{
	transformations[type] = t;
}
