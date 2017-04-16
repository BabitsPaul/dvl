#ifndef _PARSER_H_
#define _PARSER_H_

#include "id.hpp"

#include <cstdint>
#include <exception>
#include <string>
#include <istream>
#include <stack>
#include <vector>

//TODO naming for IDs

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// routine indentifier
	//

	class pid : public id<uint64_t, 27, 27, 8>
	{
	public:
		pid(): id(){}
		pid(uint64_t t): id(t){}
		pid(uint32_t group, uint32_t element, uint8_t type): id({(uint64_t) group, (uint64_t) element, (uint64_t) type}){}

		template<int part>
		id& set(uint64_t v) = delete;

		template<int part>
		uint64_t get() = delete;

		pid& set_type(uint8_t type)
		{
			id::set<2>(type);
			return *this;
		}

		pid& set_group(uint32_t group)
		{
			id::set<0>(group);
			return *this;
		}

		pid& set_element(uint32_t element)
		{
			id::set<1>(element);
			return *this;
		}

		uint8_t get_type() const
		{
			return id::get<2>();
		}

		uint32_t get_group() const
		{
			return id::get<0>();
		}

		uint32_t get_element() const
		{
			return id::get<1>();
		}
	};

	typedef id_table<uint64_t, 27, 27, 8> pid_table;
	extern pid_table pt;

	//constants
	extern uint8_t TYPE_FORK, 		//fork routine
					TYPE_LOOP, 		//loop routine
					TYPE_STRUCT, 	//structural routine
					TYPE_FIXED,		//type constant
					TYPE_INTERNAL;	//defines internal types and associated values

	extern uint32_t GROUP_INTERNAL;	//no other groups are defined. this group is reserved for the parser

	//pid type = internal, group = internal, element = 0
	extern pid EMPTY, PARSER;

	void init_parser_module();

	////////////////////////////////////////////////////////////////////////////
	// parser exception
	//

	class parser_exception : public std::exception
	{
	private:
		pid id;

		std::string msg;
	public:
		parser_exception(const pid id, std::string msg): id(id), msg(msg){}

		const char* what() const throw() {return msg.c_str();}
		const pid& get_id() const {return id;}
	};

	////////////////////////////////////////////////////////////////////////////
	// lnstruct
	//

	class lnstruct
	{
	public:
		static lnstruct EMPTY;
	private:
		long start;

		//end is inclusive
		long end;

		pid id;

		lnstruct *next, *child;
	public:
		lnstruct(pid id, long start):start(start), end(0l), id(id),
										next(&EMPTY), child(&EMPTY){}
		virtual ~lnstruct();

		void set_end(long end)
			throw(parser_exception)
		{
			if(end < start)
				throw parser_exception(id, "Invalid end specification - must be >= start");

			this->end = end;
		}

		long get_start() const {return start;}
		long get_end() const {return end;}

		lnstruct*& get_next(){ return next; }
		lnstruct*& get_child(){ return child; }
	};

	////////////////////////////////////////////////////////////////////////////
	// routine interface
	//

	/*
	utilized by the routines to tell the parser about their specific behavior,
	to read input, store output and specify how following routines are being processed
	*/

	class routine;

	class routine_interface
	{
	public:
		virtual void set_repeat_flag(bool repeat) throw(parser_exception) = 0;

		virtual void store(lnstruct *l) throw(parser_exception) = 0;

		virtual void throw_to_prnt(parser_exception p) = 0;
		virtual void check_exception_status() throw(parser_exception) = 0;

		virtual std::wistream& get_stream() = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// parser general interface
	//

	class parser_interface
	{
	public:
		virtual void parse(std::wistream& is) throw(parser_exception) = 0;

		//TODO getter for lnstruct corresponding to base
	};

	////////////////////////////////////////////////////////////////////////////
	// routine
	//

	class routine
	{
	private:
		pid id;
	public:
		struct next
		{
			const static int AS_CHILD = 0;
			const static int AS_NEXT = 1;

			routine *next;

			int as;
		};

		routine(pid id): id(id){}
		virtual ~routine(){};

		const pid& get_pid(){ return id; }

		virtual next parse(routine_interface& rif) throw(parser_exception) = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// parser impl
	//

	class parser_impl : public parser_interface, public routine_interface
	{
	private:
		struct op_enc
		{
		friend class parser_impl;

		private:
			routine *r;

			bool repeat;
		public:
			op_enc(routine* r, bool repeat): r(r), repeat(repeat){}
		};

		routine *base;

		std::wistream* str;

		std::stack<op_enc> routines;
		std::stack<lnstruct*> lnstructs;

		parser_exception *e;
	public:
		parser_impl(routine *base): base(base), str(nullptr), e(nullptr){};

		void parse(std::wistream& is) throw(parser_exception);

		void set_repeat_flag(bool repeat) throw(parser_exception);

		void store(lnstruct *l) throw(parser_exception);

		void throw_to_prnt(parser_exception p);
		void check_exception_status() throw(parser_exception);

		std::wistream& get_stream() = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// fork_routine
	//

	class fork_routine : public routine
	{
	private:
		std::vector<routine*> fork;

		std::vector<routine*>::iterator iter;
	public:
		fork_routine(pid id, std::vector<routine*> fork):
			routine(id),
			fork(fork), iter(fork.begin()){}
	};
}

#endif
