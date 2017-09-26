#include "id.hpp"

#ifndef PID_HPP_
#define PID_HPP_


namespace dvl
{
////////////////////////////////////////////////////////////////////////////
	// routine identifier
	//

	/**
	 * Provides uniue identifiers for routines of the parser. Routines are build by the following
	 * pattern: 27 bit for a group-identifier, 27 bit for the element-identifier and 8 bits to
	 * identify the type of the routine with the specified id. These ids are used to identify both
	 * routines and exceptions associated with these routines. In addition specific ids for
	 * classes like the parser are provided to represent internal routines.
	 *
	 * A single id may identify multiple routines, though these should be used for the same
	 * purpose, as specified by the name associated with the routine.
	 *
	 * This call basically provides means of bit-mask-manipulation of the internally
	 * stored value (type uint64_t). Order of storage from LSB to MSG is group, element,
	 * type.
	 *
	 * A pid identifies an entity by the means of ordering them into groups according to the
	 * group-attribute and specifying a certain element wihtin that group by the
	 * element-atttribute. The type-attribute solely specifies the type of the entity, but isn't used
	 * for identification and has a fixed name and fixed values.
	 *
	 * @link dvl::pid_table
	 */
	class pid : public id<uint64_t, 27, 27, 8>
	{
	public:
		/**
		 * Creates a new id with group-, element- and type-specifiers set to zero.
		 * This is the default-constructor. Usually other means would be used for intialization.
		 *
		 * @see id::id()
		 */
		pid(): id(){}

		/**
		 * Creates a new id with the specified constant as the value of the id. Since the id
		 * basically only provides means of simplified bit-masking, this would be equivalent to
		 * <code>group | (element << 27) | type << (54)</code>
		 *
		 * @param t the value of the id
		 */
		pid(uint64_t t): id(t){}

		/**
		 * Creates a new pid with the specified values for group-, element- and type-attribute
		 * of the id.
		 *
		 * @param group the group of the id
		 * @param element the element-id within the group
		 * @param type of the entity associated with this id
		 */
		pid(uint32_t group, uint32_t element, uint8_t type): id({(uint64_t) group, (uint64_t) element, (uint64_t) type}){}

		template<int part>
		id& set(uint64_t v) = delete;

		template<int part>
		uint64_t get() = delete;

		/**
		 * Sets the type-attribute of the id to the specified value.
		 *
		 * @param type the new type of the pid
		 *
		 * @see get_type
		 */
		pid& set_type(uint8_t type)
		{
			id::set<2>(type);
			return *this;
		}

		/**
		 * Sets the group-attribute of the id to the specified value
		 *
		 * @oaram group the new group of the pid
		 *
		 * @see get_group
		 */
		pid& set_group(uint32_t group)
		{
			id::set<0>(group);
			return *this;
		}

		/**
		 * Sets the element-attrbute of the id to the specified value
		 *
		 * @param element the new element-attribute of the pid
		 *
		 * @see set_element
		 */
		pid& set_element(uint32_t element)
		{
			id::set<1>(element);
			return *this;
		}

		/**
		 * Gets the type of the entity related with this id
		 *
		 * @return type-attribute of the id
		 */
		uint8_t get_type() const
		{
			return id::get<2>();
		}

		/**
		 * Gets the group of the entity related with this id
		 *
		 * @return the group-attribute of this id
		 */
		uint32_t get_group() const
		{
			return id::get<0>();
		}

		/**
		 * Gets the element-attribute of the entity related with this id
		 *
		 * @return the element-attribute of this id
		 */
		uint32_t get_element() const
		{
			return id::get<1>();
		}
	};

	/**
	 * Provides means of adding names to the subdivision of sets of pids
	 * into groups and further into elements. Note that type-attributes are associated with
	 * fixed names (@link types).
	 *
	 * This class provides a specialization of id_table for pid as instance of id.
	 * The internal structure used is a trie for group- and element-attributes. Type-attributes
	 * are not affected by this structure as they are fixed.
	 *
	 * @see pid
	 * @see id_table
	 */
	class pid_table : public id_table<uint64_t, 27, 27, 8>
	{
	private:
		/**
		 * Used as default-value if there exists no name for the group/element/type
		 * that is being resolved.
		 *
		 * @see get_type
		 * @see get_element
		 * @see get_group
		 */
		static const std::wstring UNKNOWN;

		/**
		 * Internal identifier used to access the group-attribute in the name table
		 */
		static const int GROUP = 0;

		/**
		 * Internal identifier used to access the element-attribute in the name table
		 */
		static const int ELEMENT = 1;

		/**
		 * Internal identifier used to access the type-attribute in the name table
		 */
		static const int TYPE = 2;

		/**
		 * typedef for super-class
		 */
		typedef id_table<uint64_t, 27, 27, 8> super;

		/**
		 * lookup-table for type-names.
		 *
		 * Provided names are:
		 * <table>
		 * 		<caption>Type-names</caption>
		 * 		<tr><th>Type-ID</th>			<th>Name</th></tr>
		 * 		<tr><td>@link TYPE_INTERNAL</td><td>"TYPE_INTERNAL"</td></tr>
		 * 		<tr><td>@link TYPE_FORK</td>	<td>"TYPE_FORK"</td></tr>
		 * 		<tr><td>@link TYPE_LOOP</td>	<td>"TYPE_LOOP"</td></tr>
		 * 		<tr><td>@link TYPE_STRUCT</td>	<td>"TYPE_STRUCT"</td></tr>
		 * 		<tr><td>@link TYPE_FIXED</td>	<td>"TYPE_FIXTED"</td></tr>
		 * </table>
		 *
		 * @see get_type
		 */
		static const std::map<uint8_t, std::wstring> types;

		/**
		 * getter for an element of the name of a id. Returns @link UNKNOWN, if there
		 * is no name present in the table that is associated with the table.
		 *
		 * @see get_type
		 * @see get_element
		 * @see get_group
		 * @see TYPE
		 * @see ELEMENT
		 * @see GROUP
		 */
		const std::wstring& get(pid& id, int e){
			const std::wstring *tmp = super::get_name(id, e);

			if(tmp == nullptr)
				return UNKNOWN;
			else
				return *tmp;
		}
	public:
		/**
		 * Constructs a new pid_table and initializes it with entities that were predefined
		 * in this header.
		 */
		pid_table();

		void set_name(id<uint64_t, 27, 27, 8>, int, std::wstring) = delete;
		const std::wstring& get_name(id<uint64_t, 27, 27, 8>, int) = delete;

		/**
		 * Associates the specified name with the group-attribute of the given id
		 *
		 * @param id of which the group will be named
		 * @param name the name associated with the group
		 *
		 * @see get_group
		 */
		void set_group(pid id, std::wstring name){ super::set_name(id, GROUP, name); }

		/**
		 * Gets the name associated with the given ids group-attribute or @link UNKNOWN
		 * if no name is associated with the id.
		 *
		 * @param id the id of whichs group the associated name is searched
		 *
		 * @see set_group
		 */
		const std::wstring& get_group(pid id){ return get(id, GROUP);}

		/**
		 * Associates the specified name with the element-attribute of the given id
		 * within the group of the id
		 *
		 * @param id of which the element will be named
		 * @param name the name associated with the element
		 *
		 * @see get_element
		 */
		void set_element(pid id, std::wstring name){ super::set_name(id, ELEMENT, name); }

		/**
		 * Gets the name associated with the given ids element-attribute within
		 * the group of the given id or @link UNKNOWN if now name is associated.
		 *
		 * @param id the id of whichs element-attribute the associated name is searched
		 *
		 * @see set_element
		 */
		const std::wstring& get_element(pid id){ return get(id, ELEMENT); }

		/**
		 * Gets the name associated with the type-attribute of the given id or @link UNKNOWN
		 * if no name is associated with the name of the given id. Note that
		 * this will not be affected by the tree-structure of group- and element-attributes.
		 *
		 * @param id the id whichs type-name is being queried
		 * @return the name associated with the type of the id
		 *
		 * @see UNKNOWN
		 */
		const std::wstring& get_type(pid id){
			if(types.count(id.get_type()))
				return types.at(id.get_type());
			else
				return UNKNOWN;
		}

		/**
		 * Sets the group- and element-name of the given name to the values given by
		 * arr, where arr[0] is the group- and arr[1] the element-name of the id.
		 *
		 * @param _id the id for which the names will be set
		 * @param arr the array of names
		 *
		 * @see set_group
		 * @see set_element
		 */
		void set(pid _id, std::array<std::wstring, 2>& arr)
		{
			set_group(_id, arr[0]);
			set_element(_id, arr[1]);
		}

		/**
		 * Returns the string generated by resolving the id-attributes into the
		 * name specified for the given id, separated by "/". The overall structure of the
		 * string would be "<group>/<element>/<type>" in unextended format or
		 * "Group=<group>/Element=<element>/Type=<type>" in extended format.
		 *
		 * @param id the id which should be resolved
		 * @param extended specified whether the extended version should be produced
		 * @return a string-representation of the id
		 *
		 * @see get_group
		 * @see get_element
		 * @see get_type
		 */
		std::wstring to_string(pid id, bool extended=false)
		{
			std::wstring res;

			if(extended) res.append(L"Group=");
			res.append(get_group(id)).append(L"/");

			if(extended) res.append(L"Element=");
			res.append(get_element(id)).append(L"/");

			if(extended) res.append(L"Type=");
			res.append(get_type(id));

			return res;
		}
	};

	/**
	 * the type-identifier of forks.
	 *
	 * @see pid
	 * @see pid_table
	 * @see fork_routine
	 * @see pid_table.types
	 */
	constexpr uint8_t TYPE_FORK = 1,
	/**
	 * the type-identifier of loops.
	 *
	 * @see pid
	 * @see pid_table
	 * @see fork_routine
	 * @see pid_table.types
	 */
					TYPE_LOOP = 2,
	/**
	 * the type-identifier of structural routines
	 *
	 * @see pid
	 * @see pid_table
	 * @see logic_routine
	 * @see pid_table.types
	 */
					TYPE_STRUCT = 3,
	/**
	 * the type identifier associated with string-matchers
	 *
	 * @see pid
	 * @see pid_table
	 * @see logic_routine
	 * @see pid_table.types
	 */
					TYPE_STRING_MATCHER = 4,
	/**
	 * type-identifier associated with empty routines
	 *
	 * @see pid
	 * @see pid_table
	 * @see empty_routine
	 * @see pid_table.types
	 */
					TYPE_EMPTY = 5,

	/**
	 * type-identifier associated with charset-routines
	 *
	 * @see pid
	 * @see pid_table
	 * @see charset_routine
	 * @see pid_table.types
	 */
					TYPE_CHARSET = 6,

	/**
	 * <em> Reserved for future use. At the moment this type remains unused, but reserved </em>
	 *
	 * type-identifier associated with regex-routines
	 *
	 * @see pid
	 * @see pid_table
	 * @see regex_routine
	 * @see pid_table.types
	 *
	 * TODO mark for future use (doxygen tag???)
	 */
					TYPE_REGEX = 7,

	/**
	 * type-identifier associated with lambda_routines
	 *
	 * @see pid
	 * @see pid_table
	 * @see pid_table.types
	 * @see lambda_routine
	 */
					TYPE_LAMBDA = 8,
	/**
	 * the type-identifier associated with internal ids used for the
	 * parser itself and diagnostic routines.
	 *
	 * @see pid
	 * @see pid_table
	 * @see parser_impl
	 * @see pid_table.types
	 */
					TYPE_INTERNAL = 0;

	/**
	 * the group-identifier associated with internal ids used for the
	 * parser itself. The name associated with this group-id is "INTERNAL"
	 *
	 * @see init_parser_module
	 * @see pid
	 * @see pid_table
	 * @see parser_impl
	 */
	constexpr uint32_t GROUP_INTERNAL = 0,	//reserved for the parser
	/**
	 * the group-identifier associated with helper-routines like stacktrace-
	 * and echo-routines. The name associated with this group-id is "DIAGNOSTIC"
	 *
	 * @see init_parser_module
	 * @see pid
	 * @see pid_table
	 * @see parser_impl
	 */
					GROUP_DIAGNOSTIC = 1;	//diagnostic routines

	/**
	 * Represents an empty entity. This pid is used to represent routines
	 * and lnstructs that represent nothing. Group = GROUP_INTERNAL, Element = 0.
	 * Elementname = "EMPTY"
	 *
	 * @see GROUP_INTERNAL
	 * @see TYPE_EMPTY
	 * @see empty_routine
	 */
	const pid EMPTY = pid({GROUP_INTERNAL, 0l, TYPE_EMPTY}),
	/**
	 * This pid is associated with the parser. Any exception thrown directly by the parser
	 * will hold this pid.
	 * Group = GROUP_INTERNAL, Element = 1, Type = TYPE_INTERNAL. Elementname = "PARSER"
	 *
	 * @see GROUP_INTERNAL
	 * @see TYPE_INTERNAL
	 */
				PARSER = pid({GROUP_INTERNAL, 1l, TYPE_INTERNAL}),
	/**
	 * Identifies the root of the output-tree of the parser. The lnstruct returned
	 * by the parser will always have this pid.
	 * Group = GROUP_INTERNAL, Element = 2, Type = TYPE_INTERNAL
	 *
	 * @see GROUP_INTERNAL
	 * @see TYPE_INTERNAL
	 */
				ROOT = pid({GROUP_INTERNAL, 2l, TYPE_INTERNAL}),

	/**
	 * Identifies the helper-identites inserted by loop-routines into the lnstruct-tree
	 * to structurize output.
	 *
	 * GROUP = GROUP_INTERNAL, Element = 3, Type = TYPE_INTERNAL
	 *
	 * @see GROUP_INTERNAL
	 * @see TYPE_INTERNAL
	 * @see loop_routine
	 * @see parser_tree_builder
	 */
				LOOP_HELPER = pid({GROUP_INTERNAL, 3l, TYPE_INTERNAL}),
	/**
	 * Identifies echo-routines.
	 * Group = GROUP_DIAGNOSTIC, Element = 0, Type = TYPE_INTERNAL
	 * Elementname = "ECHO"
	 *
	 * @see GROUP_DIAGNOSTIC
	 * @see TYPE_INTERNAL
	 * @see echo_routine
	 */
				ECHO = pid({GROUP_DIAGNOSTIC, 0l, TYPE_INTERNAL}),
	/**
	 * Identifies routines used to display the current stacktrace.
	 * Group = GROUP_DIAGNOSTIC, Element == 0, Type = TYPE_INTERNAL.
	 *
	 * Possible feature
	 *
	 * @see GROUP_DIAGNOSTIC
	 * @see TYPE_INTERNAL
	 */
				STACK_TRACE = pid({GROUP_DIAGNOSTIC, 1l, TYPE_INTERNAL});
}


#endif /* PID_HPP_ */
