#ifndef _PARSER_H_
#define _PARSER_H_

#include "id.hpp"
#include <cstdint>

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

		uint8_t get_type()
		{
			return id::get<2>();
		}

		uint32_t get_group()
		{
			return id::get<1>();
		}

		uint32_t get_element()
		{
			return id::get<0>();
		}
	};

	////////////////////////////////////////////////////////////////////////////
	// lnstruct
	//

	class lnstruct
	{
	private:
		long start;

		long end;

		pid id;
	public:
		lnstruct(pid id, long start):start(start), end(0l), id(id){}

		void set_end(long end)
		{
			this->end = end;
		}

		long get_start() const {return start;}
		long get_end() const {return end;}
	};

	////////////////////////////////////////////////////////////////////////////
	// routine
	//

	class routine
	{

	};
}

#endif
