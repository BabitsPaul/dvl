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

	template<typename T>
	inline
	constexpr T
	mgetparam_h(unsigned int)
	{
		throw "Invalid index";
	}

	template<typename T, T t, T... v>
	inline
	constexpr T
	mgetparam_h(unsigned int index)
	{
		return (index == 0 ? t : mgetparam_h<T, v...>(index - 1));
	}

	template<typename T, T... t>
	constexpr T
	mgetparam(unsigned int index)
	{
		return mgetparam_h<T, t...>(index);
	}

	////////////////////////////////////////////////////////////////////////////
	// sum
	//

	template<typename T>
	constexpr T
	msum_h_err()
	{
		throw "Invalid parameter number";
	}

	template<typename T>
	inline
	constexpr T msum_h(unsigned int count)
	{
		//static_assert(count >= 0, "Invalid count");

		return (count == 0 ? 0 : msum_h_err<T>());
	}

	template<typename T, T t, T... v>
	inline
	constexpr T msum_h(unsigned int count)
	{
		return (count == 0 ? 0 : t + msum_h<T, v...>(count - 1));
	}

	// sums up count values from t
	template<typename T, T... t>
	constexpr T msum(unsigned int count)
	{
		return msum_h<T, t...>(count);
	}
}

#endif //_META_H_
