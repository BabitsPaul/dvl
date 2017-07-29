#ifndef _ID_H_
#define _ID_H_

#include <map>
#include <string>
#include <array>
#include <stdexcept>
#include <algorithm>

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
	class id;

	template<typename T, int  index, int... split>
	class id_arr_util
	{
	public:
		static void build(id<T, split...>& _id, std::array<T, sizeof...(split)>& arr)
		{
			_id.template set<index>(arr[index]);
			id_arr_util<T, index - 1, split...>::build(_id, arr);
		}

		static void retrieve(id<T, split...>& _id, std::array<T, sizeof...(split)>& arr)
		{
			arr[index] = _id.template get<index>();
			id_arr_util<T, index - 1, split...>::retrieve(_id, arr);
		}
	};

	template<typename T, int... split>
	class id_arr_util<T, 0, split...>
	{
	public:
		static void build(id<T, split...>& _id, std::array<T, sizeof...(split)> arr)
		{
			_id.template set<0>(arr[0]);
		}

		static void retrieve(id<T, split...>& _id, std::array<T, sizeof...(split)> arr)
		{
			arr[0] = _id.template get<0>();
		}
	};

	template<typename T, int... split>
	class id
	{
	private:
		typedef id_arr_util<T, sizeof...(split) - 1, split...> builder;

		T t;

		template<int index>
		static constexpr T mask = mmask<T>(mgetparam<int, split...>(index),
												msum<int, split...>(index));

		template<int index>
		static constexpr int offset = msum<T, split...>(index);
	public:
		id(): t(0){}
		id(T t): t(t){}
		id(std::array<T, sizeof...(split)> arr)
		{
			t = (T) 0;
			builder::build(*this, arr);
		}

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

		std::array<T, sizeof...(split)>
		as_arr() const
		{
			std::array<T, sizeof...(split)> arr;

			builder::retrieve(*this, arr);

			return arr;
		}
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

		trie_node<T, sizeof...(split), split...> root = {false, L"", {}};
	public:
		void set_name(int_id id, int lvl, std::wstring name)
			throw(std::out_of_range)
		{
			root._set_name(id, lvl + 1, name);
		}

		const std::wstring* get_name(int_id id, int lvl)
			throw(std::out_of_range)
		{
			return root._get_name(id, lvl + 1);
		}

		const std::wstring table_to_string(std::wstring def=L"???")
		{
			return root.to_string(L"", def);
		}
	};

	template<typename T, int level, int... split>
	struct trie_node
	{
		typedef std::map<T, trie_node<T, level - 1, split...>> child_map;

		bool name_set = false;

		std::wstring name;

		child_map c;

	 	std::wstring* _get_name(id<T, split...> _id, int depth)
			throw(std::out_of_range)
		{
			if(depth == 0)
				return name_set ? &name : nullptr;
			else if(c.count(_id.template get<sizeof...(split) - level>()))
				return c[_id.template get<sizeof...(split) - level>()]._get_name(_id, depth - 1);
			else
				return nullptr;
		}

		void _set_name(id<T, split...> _id, int depth, std::wstring str)
			throw(std::out_of_range)
		{
			if(depth == 0)
			{
				name = str;
				name_set = true;
			}
			else
				c[_id.template get<sizeof...(split) - level>()]._set_name(_id, depth - 1, str);
		}

		std::wstring to_string(std::wstring indent, std::wstring def = L"???")
			const
		{
			std::wstring result;

			result.append(indent).append(name_set ? name : def).append(L"\n");

			std::wstring child_indent;
			child_indent.append(indent).append(L"\t");

			for(auto const &iter : c)
				result.append(iter.second.to_string(child_indent, def));

			return result;
		}
	};

	template<typename T, int... split>
	struct trie_node<T, 0, split...>
	{
		bool name_set = false;

		std::wstring name;

	 	std::wstring* _get_name(id<T, split...>, int depth)
			throw(std::out_of_range)
		{
			if(depth == 0)
				return name_set ? &name : nullptr;

			throw std::out_of_range("Invalid depth");
		}

		void _set_name(id<T, split...>, int depth, std::wstring str)
			throw(std::out_of_range)
		{
			if(depth == 0)
			{
				name = str;
				name_set = true;
			}
			else
				throw std::out_of_range("Invalid depth");
		}

		std::wstring to_string(std::wstring indent, std::wstring def = L"???")
			const
		{
			return std::wstring(L"").append(indent).append(name_set ? name : def).append(L"\n");
		}
	};
}

#endif
