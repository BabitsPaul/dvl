#ifndef _META_H_
#define _META_H_

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// controls
	//

	template<typename T, bool C, T IF, T ELSE>
	struct mcondition
	{
		static constexpr T V = IF;
	};

	template<typename T, T IF, T ELSE>
	struct mcondition<T, false, IF, ELSE>
	{
		static constexpr T V = ELSE;
	};

	////////////////////////////////////////////////////////////////////////////
	// variadic parameters
	//

	template<typename T, unsigned int index, T t, T... v>
	T mgetparam_h()
	{
		return (index == 0 ? t : mgetparam_h<T, index - 1, v...>());
	}

	template<typename T, unsigned int index, T... t>
	T mgetparam()
	{
		static_assert(sizeof...(t) > index, "Index out of range");

		return mgetparam_h<T, index, t...>();
	}

	////////////////////////////////////////////////////////////////////////////
	// sum
	//

	template<typename T, unsigned int count>
	inline
	constexpr T msum_h()
	{
		static_assert(count >= 0, "Invalid count");

		return 0;
	}

	template<typename T, unsigned int count, T t, T... v>
	inline
	constexpr T msum_h()
	{
		return (count == 0 ? 0 : t + msum_h<T, count - 1, v...>());
	}

	// sums up count values from t
	template<typename T, unsigned int count, T... t>
	constexpr T msum()
	{
		return msum_h<T, count, t...>();
	}

	////////////////////////////////////////////////////////////////////////////
	// mask
	//

	
}

#endif //_META_H_
