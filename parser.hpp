#ifndef _PARSER_H_
#define _PARSER_H_

#include "id.hpp"

#include <cstdint>
#include <exception>
#include <string>
#include <istream>
#include <stack>

//TODO naming for IDs

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// routine indentifier
	//

	class pid : id<uint64_t, 27, 27, 8>
	{
	public:
		pid(): id(0){}
		pid(uint64_t t): id(t){}

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
			id::set<1>(group);
			return *this;
		}

		pid& set_element(uint32_t element)
		{
			id::set<0>(element);
			return *this;
		}

		uint8_t get_type() const
		{
			return id::get<2>();
		}

		uint32_t get_group() const
		{
			return id::get<1>();
		}

		uint32_t get_element() const
		{
			return id::get<0>();
		}
	};

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
	private:
		static lnstruct EMPTY;

		long start;

		//end is inclusive
		long end;

		pid id;

		lnstruct *next, *child;
	public:
		lnstruct(pid id, long start):start(start), end(0l), id(id),
										next(&EMPTY), child(&EMPTY){}

		void set_end(long end)
			throw(parser_exception)
		{
			if(end < start)
				throw parser_exception(id, "Invalid end specification - must be >= start");

			this->end = end;
		}

		long get_start() const {return start;}
		long get_end() const {return end;}

		void set_next(lnstruct *next){ this->next = next; }
		void set_child(lnstruct *child){ this->child = child; }
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
		virtual void run_as_child(routine *r) throw(parser_exception) = 0;
		virtual void run_as_follower(routine *r) throw(parser_exception) = 0;

		virtual void set_repeat_flag(bool repeat) throw(parser_exception) = 0;

		virtual void store(lnstruct *l) throw(parser_exception) = 0;

		virtual void throw_to_prnt(parser_exception p) = 0;
		virtual void check_exception_status() throw(parser_exception) = 0;

		virtual std::istream& get_stream() = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// parser general interface
	//

	class parser_interface
	{
	public:
		virtual void parse(std::istream& is) throw(parser_exception) = 0;

		virtual void reset() = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// routine
	//

	class routine
	{
	private:
		pid id;
	public:
		routine(pid id): id(id){}

		virtual void parse(routine_interface& rif) throw(parser_exception) = 0;
	};

	////////////////////////////////////////////////////////////////////////////
	// parser impl
	//

	struct op_enc
	{
		static const int AS_CHILD = 0;
		static const int AS_NEXT = 1;

		routine *r;

		bool repeat;

		int as;
	};

	class parser_impl : public parser_interface, public routine_interface
	{
	private:
		routine *base;

		std::istream* str;

		std::stack<op_enc> routines;
		std::stack<lnstruct*> lnstructs;
	public:
		parser_impl(routine *base): base(base), str(nullptr){};

		void parse(std::istream& is) throw(parser_exception);
		void reset();

		void run_as_child(routine *r) throw(parser_exception) = 0;
		void run_as_follower(routine *r) throw(parser_exception) = 0;

		void set_repeat_flag(bool repeat) throw(parser_exception) = 0;

		void store(lnstruct *l) throw(parser_exception) = 0;

		void throw_to_prnt(parser_exception p) = 0;
		void check_exception_status() throw(parser_exception) = 0;

		std::istream& get_stream() = 0;
	};
}

#endif
