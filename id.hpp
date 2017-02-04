#ifndef _ID_H_
#define _ID_H_

#include "meta.hpp"

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// mask
	//

	template<typename T>
	inline
	constexpr T
	mmask_h(unsigned int c)
	{
		return (c == 0 ? 0 : 1 << (c - 1) | mmask_h<T>(c - 1));
	}

	template<typename T>
	constexpr T
	mmask(unsigned int c, unsigned int offset)
	{
		//static_assert(c + offset < sizeof(T), "Mask is too large");

		return mmask_h<T>(c) << offset;
	}

	////////////////////////////////////////////////////////////////////////////
	// id
	//

	template<typename T, int... split>
	class id
	{
	private:
		T t;

		template<int index>
		static constexpr T mask = mmask<T>(mgetparam<int, split...>(index),
												msum<int, split...>(index));

		template<int index>
		static constexpr int offset = msum<T, split...>(index);
	public:
		id(): t(0){}
		id(T t): t(t){}

		operator T() const{
			return t;
		}

		template<int part>
		id& set(T v)
		{
			static_assert(part < sizeof...(split), "Invalid part-id");

			t &= ~mask<part>;
			t |= (v << offset<part>) & mask<part>;

			return *this;
		}

		template<int part>
		T get() const
		{
			static_assert(part < sizeof...(split), "Invalid part-id");

			return (t & mask<part>) >> offset<part>;
		}

		bool operator==(const id o) const{return o.t == t;}
	};
}

#endif
