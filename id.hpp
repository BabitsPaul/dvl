#ifndef _ID_H_
#define _ID_H_

#include <map>
#include <string>

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// variadic parameters
	//

	template<typename T>
	inline
	constexpr T
	mgetparam_h(unsigned int)
	{
		return 0 ? -1 : throw "Invalid index";
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
	inline
	constexpr T
	msum_h(unsigned int count)
	{
		return count == 0 ? 0 : throw "Invalid parameter number";
	}

	template<typename T, T t, T... v>
	inline
	constexpr T
	msum_h(unsigned int count)
	{
		return (count == 0 ? 0 : t + msum_h<T, v...>(count - 1));
	}

	// sums up count values from t
	template<typename T, T... t>
	constexpr T
	msum(unsigned int count)
	{
		return msum_h<T, t...>(count);
	}

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
			static_assert(part > -1, "Invalid part-id");

			t &= ~mask<part>;
			t |= (v << offset<part>) & mask<part>;

			return *this;
		}

		template<int part>
		T get() const
		{
			static_assert(part < sizeof...(split), "Invalid part-id");
			static_assert(part > -1, "Invalid part-id");

			return (t & mask<part>) >> offset<part>;
		}

		bool operator==(const id o) const{return o.t == t;}
	};

	////////////////////////////////////////////////////////////////////////////
	// id table
	//

	template<typename T, int level, int... split>
	struct trie_node;

	template<typename T, int... split>
	class id_table
	{
	private:
		typedef id<T, split...> int_id;

		trie_node<T, sizeof...(split), split...> root = {"", {}};
	public:
		void set_name(int_id id, int lvl, std::string name)
		{
			root._get_name(id, lvl + 1) = name;
		}

		const std::string& get_name(int_id id, int lvl)
		{
			return root._get_name(id, lvl + 1);
		}
	};

	//TODO order: access is reverse (leaves => root)
	template<typename T, int level, int... split>
	struct trie_node
	{
		std::string name;

		std::map<T, trie_node<T, level - 1, split...>> c;

	 	std::string& _get_name(id<T, split...> _id, int depth)
		{
			return depth == 0 ? name : c[_id.template get<sizeof...(split) - level>()]._get_name(_id, depth - 1);
		}
	};

	template<typename T, int... split>
	struct trie_node<T, 0, split...>
	{
		std::string name;

	 	std::string& _get_name(id<T, split...>, int depth)
		{
			if(depth == 0)
				return name;

			throw std::string("Too large depth ").append(std::to_string(depth));
		}
	};
}

#endif
