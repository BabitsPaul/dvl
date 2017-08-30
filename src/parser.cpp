#include "parser.hpp"

#include <queue>
#include <cstdlib>
#include <set>

////////////////////////////////////////////////////////////////////////////////
// id
//

const std::wstring
dvl::pid_table::UNKNOWN = L"Unknown";

const std::map<uint8_t, std::wstring>
dvl::pid_table::types = {{TYPE_INTERNAL,			L"TYPE_INTERNAL"},
						 {TYPE_FORK, 				L"TYPE_FORK"},
					 	 {TYPE_LOOP,				L"TYPE_LOOP"},
					 	 {TYPE_STRUCT,				L"TYPE_STRUCT"},
					 	 {TYPE_STRING_MATCHER,		L"TYPE_STRING_MATCHER"},
						 {TYPE_EMPTY,				L"TYPE_EMPTY"}};

dvl::pid_table::pid_table()
{
	//register internal ids
	set_group(pid().set_group(GROUP_INTERNAL), L"INTERNAL");

	set_element(EMPTY, L"EMPTY");
	set_element(PARSER, L"PARSER");

	//register diagnostic routines
	set_group(pid().set_group(GROUP_DIAGNOSTIC), L"DIAGNOSTIC");

	set_element(ECHO, L"ECHO");
}

////////////////////////////////////////////////////////////////////////////////
// lnstruct
//

//non-recursively deletes deeply nested structure of lnstructs
dvl::lnstruct::~lnstruct()
{
	//check if free is valid for deallocation of objects
	std::queue<lnstruct*> q;
	q.push(next);
	q.push(child);

	while(!q.empty())
	{
		lnstruct *ln = q.front();
		q.pop();

		if(ln == nullptr)
			continue;

		q.push(ln->next);
		q.push(ln->child);

		ln->next = nullptr;
		ln->child = nullptr;

		delete ln;
	}
}

int
dvl::lnstruct::level_count()
	const
{
	lnstruct *ln = const_cast<lnstruct*>(this);
	int ct = 0;

	while(ln != nullptr)
	{
		ct++;
		ln = ln->get_next();
	}

	return ct;
}

int
dvl::lnstruct::height()
	const
{
	lnstruct *ln = const_cast<lnstruct*>(this);
	int ct = 0;

	while(ln != nullptr)
	{
		ct++;
		ln = ln->get_child();
	}

	return ct;
}

int
dvl::lnstruct::total_count()
	const
{
	std::stack<lnstruct*> st;
	st.push(const_cast<lnstruct*>(this));

	int ct = 0;

	while(!st.empty())
	{
		if(st.top()->get_child() != nullptr)
			st.push(st.top()->get_child());
		else if(st.top()->get_next() != nullptr)
		{
			lnstruct *n = st.top()->get_next();
			st.pop();
			st.push(n);

			ct++;
		}
		else
		{
			while(!st.empty() && st.top()->get_next() == nullptr)
			{
				st.pop();
				ct++;
			}

			if(!st.empty())
			{
				lnstruct *tmp = st.top();
				st.pop();
				st.push(tmp->get_next());

				ct++;
			}
		}
	}

	return ct;
}

bool
dvl::lnstruct::is_tree()
	const
{
	std::set<lnstruct*> s;
	std::stack<lnstruct*> st;
	st.push(const_cast<lnstruct*>(this));

	while(!st.empty())
	{
		lnstruct *ln = st.top();

		if(s.find(ln) != s.end())
			return false;

		s.emplace(ln);

		if(ln->get_child() != nullptr)
			st.push(ln->get_child());

		if(ln->get_next() != nullptr)
			st.push(ln->get_next());
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
// tree builder
//

void
dvl::routine_tree_builder::insert_node(routine *rn)
	throw(parser_exception)
{
	routines.emplace_back(rn);

	try{
		if(r == nullptr)
		{
			//first node inserted => can't set into relation with
			//any other node
			r = rn;
			return;
		}

		if(rn == nullptr)
		{
			delete rn;
			throw parser_exception(PARSER, "No routine specified");
		}

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
	friend class dvl::parser_routine_factory;

	typedef dvl::parser_routine_factory::parser_routine base_routine;

	class parser_fork_routine : public base_routine
	{
	private:
		dvl::fork_routine* fr;

		std::vector<dvl::routine*>::iterator f_iter;

		dvl::lnstruct *base;
		dvl::lnstruct *last_success;
	public:
		parser_fork_routine(dvl::fork_routine* fr)
			: base_routine(fr->get_pid())

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
			if(base == nullptr)
				throw dvl::parser_exception(fr->get_pid(), dvl::parser_exception::lnstruct_premature_insertion());

			if(last_success != nullptr)
				throw dvl::parser_exception(fr->get_pid(), "Found multiple matching definitions");

			last_success = l;
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
			// routine runs for the first time
			if(base == nullptr)
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

				base_routine::repeat(ri);
			}
		}
	};

	class parser_empty_routine : public base_routine
	{
	private:
		dvl::lnstruct *ln = nullptr;
	public:
		parser_empty_routine() : base_routine(dvl::EMPTY){}

		dvl::lnstruct* get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct*)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(dvl::EMPTY, dvl::parser_exception::lnstruct_invalid_insertion("emtpy_routine"));
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(dvl::EMPTY, ri.get_istream().tellg());
		}
	};

	class parser_loop_routine : public base_routine
	{
	private:
		unsigned int run_ct = 0;

		dvl::loop_routine *r;

		dvl::lnstruct *ln;
		dvl::lnstruct **insert_pos;
	public:
		parser_loop_routine(dvl::loop_routine *r) : base_routine(r->get_pid()),
			r(r), ln(nullptr), insert_pos(nullptr)
		{}

		dvl::lnstruct* get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct* c)
			throw(dvl::parser_exception)
		{
			if(insert_pos == nullptr)
				throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_premature_insertion());

			//wrap ln into helper to keep next-slot free
			dvl::lnstruct *helper = new dvl::lnstruct(dvl::LOOP_HELPER, c->get_start());
			helper->set_end(c->get_end());
			helper->get_child() = c;

			//insert helper at next insertion-position
			*insert_pos = helper;
			insert_pos = &helper->get_next();
		}

		void run(dvl::routine_interface& ri)
			throw(dvl::parser_exception)
		{
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
					throw;
				else
					return;
			}

			// maximum iterations count reached -> run child once, then terminate
			if(run_ct + 1 == r->get_max_iterations() &&
				r->get_max_iterations() != dvl::loop_routine::_INFINITY)
			{
				ri.run_as_child(r->get_loop());
				return;
			}

			base_routine::repeat(ri);		//mark routine for repetition
			ri.run_as_child(r->get_loop());	//run routine to loop over as next
			run_ct++;						//increment loop count
		}
	};

	class parser_struct_routine : public base_routine
	{
	private:
		dvl::struct_routine *r;

		dvl::lnstruct *ln;
	public:
		parser_struct_routine(dvl::struct_routine *r): base_routine(r->get_pid()),
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
				throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_premature_insertion());

			ln->get_child() = l;
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			// place routines to run
			if(r->get_child() != nullptr)
				ri.run_as_child(r->get_child());

			if(r->get_next() != nullptr)
				ri.run_as_next(r->get_next());
		}
	};

	class parser_matcher_routine : public base_routine
	{
	private:
		const std::wstring &s;

		dvl::lnstruct *ln;
	public:
		parser_matcher_routine(dvl::string_matcher_routine *r): base_routine(r->get_pid()),
			s(r->get_str()), ln(nullptr)
		{}

		dvl::lnstruct *get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct *)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_invalid_insertion("string_matcher_routine"));
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			// compare input to predefined string
			const wchar_t *str = s.c_str();
			for(const wchar_t *c = str; c < str + s.length(); c++)
			{
				wint_t sc = ri.get_istream().get();

				if(sc == WEOF)
					throw dvl::parser_exception(get_pid(), "Reached EOF");

				if((wint_t) *c != sc)
					throw dvl::parser_exception(get_pid(), "Mismatch in string");
			}
		}
	};

	class parser_echo_routine : public base_routine
	{
	private:
		dvl::lnstruct *ln;

		dvl::echo_routine *er;
	public:
		parser_echo_routine(dvl::echo_routine *r) : base_routine(r->get_pid()),
			ln(nullptr), er(r)
		{}

		dvl::lnstruct *get_result(){ return ln; }

		void place_child(dvl::lnstruct *)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_invalid_insertion("parser_echo_routine"));
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			er->get_stream() << er->get_msg() << std::endl;
		}
	};

	class parser_stack_routine : public base_routine
	{
	private:
		dvl::lnstruct *ln;

		dvl::stack_trace_routine *r;
	public:
		parser_stack_routine(dvl::stack_trace_routine *r) : base_routine(r->get_pid()),
			ln(nullptr), r(r){}

		dvl::lnstruct *get_result()
		{
			return ln;
		}

		void place_child(dvl::lnstruct *)
			throw(dvl::parser_exception)
		{
			throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_invalid_insertion("parser_stack_routine"));
		}

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			ri.visit(*r);
		}
	};
};

dvl::parser_routine_factory::parser_routine_factory()
{
	register_transformation(TYPE_FORK, [](routine* r)->routine_factory_util::parser_fork_routine*{
		return new routine_factory_util::parser_fork_routine((fork_routine*) r);
	});

	register_transformation(TYPE_EMPTY, [](routine*)->routine_factory_util::parser_empty_routine*{
		return new routine_factory_util::parser_empty_routine();
	});

	register_transformation(TYPE_LOOP, [](routine* r)->routine_factory_util::parser_loop_routine*{
		return new routine_factory_util::parser_loop_routine((loop_routine*) r);
	});

	register_transformation(TYPE_STRUCT, [](routine *r)->routine_factory_util::parser_struct_routine*{
		return new routine_factory_util::parser_struct_routine((struct_routine*) r);
	});

	register_transformation(TYPE_STRING_MATCHER, [](routine *r)->routine_factory_util::parser_matcher_routine*{
		return new routine_factory_util::parser_matcher_routine((string_matcher_routine*) r);
	});

	register_transformation(TYPE_INTERNAL, [](routine *r)->parser_routine_factory::parser_routine*{
		switch(r->get_pid().get_group())
		{
		case GROUP_INTERNAL:
			switch(r->get_pid().get_element())
			{
			case 0:	//empty routine
				return new routine_factory_util::parser_empty_routine();
			}
			break;
		case GROUP_DIAGNOSTIC:
			switch(r->get_pid().get_element())
			{
			case 0:	// echo routine
				return new routine_factory_util::parser_echo_routine((echo_routine*) r);
			case 1:
				return new routine_factory_util::parser_stack_routine((stack_trace_routine*) r);
			}
			break;
		}

		throw dvl::parser_exception(PARSER, "No routines with the specified group available");
	});
}

dvl::parser_routine_factory::parser_routine*
dvl::parser_routine_factory::build_routine(routine* r)
	throw(parser_exception)
{
	if(transformations.find(r->get_pid().get_type()) != transformations.end())
		return transformations[r->get_pid().get_type()](r);
	else
		throw parser_exception(PARSER, "No generator for routine of specified type found");
}

void
dvl::parser_routine_factory::register_transformation(uint8_t type, transform t)
{
	transformations[type] = t;
}

//////////////////////////////////////////////////////////////////////////////////////
// parser
//

dvl::parser::stack_frame::stack_frame(const stack_frame &f):
		stream_marker(f.stream_marker)
{
	cur = f.cur;
	next = f.next;
	repeat = f.repeat;
	repeated = f.repeated;
	result = f.result;

	if(f.next_insert == &f.result)
		next_insert = &result;
	else
		next_insert = f.next_insert;
}

void
dvl::parser::unwind()
	throw(parser_exception)
{
	try{
		{	// pop first frame irrespective of it's state
			lnstruct *ln = s.top().result;
			if(s.top().next != nullptr && s.top().next != s.top().cur)
				delete s.top().next;
			delete s.top().cur;

			s.pop();

			s.top().cur->ri_place_child(ln);	// chain output
		}

		while(!s.empty() && !s.top().repeat && s.top().next == nullptr)
		{
			stack_frame &f = s.top();
			lnstruct *ln = f.result;
			delete f.cur;

			s.pop();

			if(!s.empty())	// TODO keep helper?
				s.top().cur->ri_place_child(ln);	// chain output
		}

		if(!s.empty())
		{
			// step
			if(s.top().repeat)
				s.top().repeat = false;
			else
				s.top().switch_to_next_routine();
		}
	}catch(const parser_exception &ex)
	{
		std::cout << "exception in unwind: " << ex.what() << std::endl;

		if(e != nullptr)
			delete e;

		e = ex.clone();

		// ex
		unwind_ex();
	}
}

void
dvl::parser::unwind_ex()
	throw(parser_exception)
{
	if(s.size() == 0)
		throw parser_exception(PARSER, "empty stack");

	// pop
	{
		delete s.top().result;
		if(s.top().next != nullptr && s.top().next != s.top().cur)
			delete s.top().next;
		delete s.top().cur;

		context.str.seekg(s.top().stream_marker, std::ios::beg);

		s.pop();
	}

	// pop_ex
	while(!s.empty() && !s.top().repeat)
	{
		// reset stream position
		context.str.seekg(s.top().stream_marker, std::ios::beg);

		delete s.top().result;
		if(s.top().next != nullptr && s.top().next != s.top().cur)
			delete s.top().next;
		delete s.top().cur;

		s.pop();
	}

	// step
	if(!s.empty())
		s.top().repeat = false;
}

dvl::parser::parser(parser_context &context)
	throw(parser_exception)
	:context(context)
{
	if(context.builder.get() == nullptr)
		throw parser_exception(PARSER, "No definition available");

	if(!context.str)
		throw parser_exception(PARSER, "Can't read input");

	stack_frame f(context.str.tellg());
	f.cur = new output_helper(result, context.builder.get());
	s.push(f);
}

dvl::parser::~parser()
{
	while(!s.empty())
	{
		stack_frame &f = s.top();

		delete f.cur;

		if(f.next != nullptr)
			delete f.next;

		// TODO delete output???

		s.pop();
	}
}

void
dvl::parser::run()
	throw(parser_exception)
{
	while(!s.empty())
	{
		update.reset();

		std::wcout << L"Running routine :" << context.pt.to_string(s.top().cur->get_pid()) << std::endl;

		try{
			// run
			s.top().cur->ri_run(*this);

			// done
			e = nullptr;	// reset exception-flag
		}catch(parser_exception &ex)
		{
			std::cout << "exception in run: " << ex.what() << std::endl;

			e = ex.clone();

			unwind_ex();

			continue;
		}

		// proc
		stack_frame &f = s.top();

		if(!f.repeated)
		{
			// place output
			*f.next_insert = f.cur->get_result();
			f.next_insert = &(*f.next_insert)->get_next();

			f.repeated = true;
		}

		f.repeat = update.repeat;

		if(update.next != nullptr)
		{
			if(f.next != nullptr)
				delete f.next;

			f.next = context.factory.build_routine(update.next);
		}

		if(update.child != nullptr)
		{
			stack_frame nf(context.str.tellg());
			nf.cur = context.factory.build_routine(update.child);

			s.push(nf);

			// step
			continue;
		}

		// step
		if(update.repeat)
			s.top().repeat = false;
		else if(update.next != nullptr)
			s.top().switch_to_next_routine();
		else
			unwind();
	}
}

void
dvl::parser::visit(stack_trace_routine&)
{
	// TODO
	std::cout << "Here should go a stacktrace" << std::endl;
}
