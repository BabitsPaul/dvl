#ifndef _PARSER_H_
#define _PARSER_H_

#include "id.hpp"

#include <cstdint>
#include <exception>
#include <string>
#include <istream>
#include <deque>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <stack>
#include <queue>

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// routine indentifier
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
	 * @see pid__table.types
	 */
					TYPE_EMPTY = 5,
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

	////////////////////////////////////////////////////////////////////////////
	// parser exception
	//

	/**
	 * Represents exceptions thrown by the parser or any of its submodules (routines).
	 * The pid held by the exception is the pid of the routine by which the exception was
	 * thrown or the pid identifying the source of the error, like \link PARSER \endlink, if the
	 * exception is parser-internal.
	 *
	 * @see pid
	 * @see parser_impl
	 * @see routine
	 * @see PARSER
	 */
	class parser_exception : public std::exception
	{
	private:
		/**
		 * The id of the @link parser_routine if the exception was caused by a routine,
		 * else a pid representing the specific entity by which the exception was thrown.
		 */
		pid id;

		/**
		 * The message associated with this exception
		 */
		std::string msg;
	public:
		/**
		 * Constructs a new parser-exception of the given parameters
		 */
		parser_exception(const pid id, std::string msg): id(id), msg(msg){}

		/**
		 * Returns the error-message of this exception
		 *
		 * @return the error message of this exception
		 * @see std::exception::what
		 */
		const char* what() const throw() {return msg.c_str();}

		/**
		 * Getter for the pid associated with this exception. The result will usually
		 * be either an internal pid or the pid of the routine producing the exception.
		 *
		 * @return the pid of the routine/entity causing this exception
		 * @see pid
		 * @see routine
		 */
		const pid& get_id() const {return id;}

		/**
		 * Generates an exact clone of this parser_exception.
		 *
		 * @return a clone of this instance of parser_exception
		 */
		virtual parser_exception* clone() const
		{
			return new parser_exception(id, msg);
		}

		//////////////////////////////////
		//standard exception messages

		/**
		 * Constructs the message for the parser_exception that will be thrown
		 * if an routine gets assigned an invalid pid
		 *
		 * @param cn the name of the routine
		 * @param id the id associated with the routine
		 * @return an exception message of the form "Invalid pid - Type mismatch for <cn>"
		 *
		 * @see routine
		 */
		static std::string invalid_pid(std::string cn){
			return std::string("Invalid pid - Type mismatch for ")  + cn;
		}

		/**
		 * Constructs the message for the parser exception that will be thrown if
		 * the parser_tree_builder attempts to insert a new routine into the tree
		 * in a mode that mismatches the type of the current routine.
		 *
		 * @return the message "Can't insert - type of current routine is invalid"
		 * @see parser_tree_builder
		 */
		static std::string ptree_builder_invalid_routine(){
			return "Can't insert - type of current routine is invalid";
		}

		/**
		 * Constructs the message for the parser that will be thrown if a
		 * routine will be run more often than it should run.
		 *
		 * @return "Invalid call - routine may not represent more than one entity"
		 * @see routine
		 * @see parser_routine_factory
		 */
		static std::string routine_invalid_repeat(){
			return "Invalid call - routine may not represent more than one entity";
		}

		/**
		 * Constructs the message for the parser_exception that will be thrown if
		 * an attempt is made to insert a lnstruct in an entity that may not have children.
		 *
		 * @param cn the name of the routine on which the entity was inserted
		 * @return "Invalid operation - <cn> doesn't allow child entities"
		 * @see parser_routine_factory
 		 */
		static std::string lnstruct_invalid_insertion(std::string cn){
			return "Invalid operation - " + cn + " doesn't allow child entities";
		}

		/**
		 * Constructs the message for the parser_exception that will be thrown if
		 * an attempt is made to insert a lnstruct into an entity that wasn't initialized
		 * yet.
		 *
		 * @return "Output not initialized yet - may not insert entity"
		 * @see parser_routine_factory
		 */
		static std::string lnstruct_premature_insertion(){
			return "Output not initialized yet - may not insert entity";
		}

		/**
		 * Constructs the message for an exception that will be thrown
		 * upon receiving an unexpected nullpointer.
		 *
		 * @param msg additional info for the exception, "" by default
		 * @return "Nullpointer exception - <msg>"
		 */
		static std::string nullptr_error(std::string msg = ""){
			return "Nullpointer exception - " + msg;
		}
	};

	////////////////////////////////////////////////////////////////////////////
	// lnstruct
	//

	/**
	 * Represents a structure that was found while parsing an input-stream. The structure
	 * is represented by offset in the stream and offset of the end of the structure in
	 * the stream. It also keeps pointers to the next and child-struct of this lnstruct, thus
	 * building a nested tree-like structure made of linked lists.
	 *
	 * Every lnstruct is associated with a type of routine by the corresponding @link pid.
	 *
	 * @see routine
	 * @see pid
	 */
	class lnstruct
	{
	private:
		/**
		 * Starting-offset of the structure in the input-stream
		 */
		long start;

		/**
		 * End-offset of the structure in the input-stream (inclusive)
		 */
		long end;

		/**
		 * PID of the routine that parsed this structure
		 */
		pid id;

		/**
		 * Next and child-lnstructs of this structure.
		 */
		lnstruct *next, *child;
	public:
		/**
		 * Constructs a new lnstruct with the specified pid and start-offset.
		 * End and associated lnstructs will be set to default-values (0/nullptr)
		 */
		lnstruct(pid id, long start):start(start), end(0l), id(id),
										next(nullptr), child(nullptr){}
		virtual ~lnstruct();

		/**
		 * Updates the end of the lnstruct to the specified value. Note that this value must
		 * be greater/equal than the start, as otherwise the lnstruct will be invalid and a
		 * parser_exception will be thrown.
		 *
		 * @see parser_exception
		 */
		void set_end(long end)
			throw(parser_exception)
		{
			if(end < start)
				throw parser_exception(id, "Invalid end specification - must be >= start");

			this->end = end;
		}

		/**
		 * Accessor for the starting-index of this lnstruct.
		 *
		 * @see start
		 */
		long get_start() const {return start;}

		/**
		 * Accessor for the end-index of this lnstruct
		 *
		 * @see end
		 */
		long get_end() const {return end;}


		/**
		 * Accessor for the lnstruct following this structure
		 *
		 * @see next
		 */
		lnstruct*& get_next(){ return next; }

		/**
		 * Accessor for the child-lnstruct of this structure
		 *
		 * @see child
		 */
		lnstruct*& get_child(){ return child; }

		/**
		 * Generates a wstring representing the structure associated with this
		 * string in an indentation-based tree-view.
		 *
		 * @param indent the indent used for this structure (child-structure will receive indent + "\t")
		 */
		std::wstring structure(pid_table &pt, std::wstring indent=L"") const
		{
			std::wstring result = L"";
			result.append(indent);
			result.append(pt.to_string(id, true));
			result.append(L" start=").append(std::to_wstring(start));
			result.append(L" end=").append(std::to_wstring(end));
			result.append(L"\n");

			if(child != nullptr)
				result.append(child->structure(pt, indent + L"\t"));

			if(next != nullptr)
				result.append(next->structure(pt, indent));

			return result;
		}

		/**
		 * Counts the total number of lnstructs that are on the same level with
		 * this lnstruct and follow it. Counts the number of next elements that are
		 * not null.
		 *
		 * @return the number of elements on the same level as this node
		 */
		int level_count() const;

		/**
		 * Calculates the total number of lnstructs within the tree of lnstructs
		 * that has this node as root.
		 *
		 * @return number of nodes in the subtree that has this routine as root
		 */
		int total_count() const;

		/**
		 * Calculates the height of this tree as the number of nodes "vertically above"
		 * this node.
		 *
		 * @return the height of the subtree
		 */
		int height() const;

		bool is_tree() const;
	};

	////////////////////////////////////////////////////////////////////////////
	// routine interface
	//

	/**
	 * Baseclass of all routines. Any type of routine has a pid associated with it, identifying
	 * the specific type of structure that was parsed by this structure. Note that direct subclasses
	 * of this class do only define structures, but don't provide any specific functional interface
	 * for parsing input.
	 *
	 * Routines are owned by the @link routine_tree_builder that constructed them.
	 * Destructors for routines may delete any other resource allocated by the routine
	 * but may not delete any routines to which pointers are held.
	 *
	 * @see pid
	 */
	class routine
	{
	private:
		/**
		 * The pid of the routine
		 */
		pid id;
	public:
		routine(pid id): id(id){}
		virtual ~routine(){};

		const pid& get_pid(){ return id; }
	};

	////////////////////////////////////////////////////////////////////////////
	// empty_routine
	//

	/**
	 * Implements a noop in a routine. This routine should have no side-effects and simply return
	 * without any error or reading any value.
	 *
	 * @see routine
	 */
	class empty_routine : public routine
	{
	public:
		empty_routine(): routine(EMPTY){}
	};

	////////////////////////////////////////////////////////////////////////////
	// fork_routine
	//

	/**
	 * Defines an OR-structure, thus allowing to search for a valid structure-definition
	 * amongst a set of given structures.
	 */
	class fork_routine : public routine
	{
	private:
		/**
		 * A vector of all subroutines that may be forked off this routine
		 */
		std::vector<routine*> fork;
	public:
		fork_routine(pid id, std::vector<routine*> fork):
			routine(id),
			fork(fork){
			if(id.get_type() != TYPE_FORK)
				throw parser_exception(PARSER, parser_exception::invalid_pid("fork_routine"));
		}

		/**
		 * Adds a new fork to this routine
		 *
		 * @see routine_tree_builder
		 */
		void add_fork(routine* r){ fork.emplace_back(r); }
	//TODO results in syntax-error: protected:
		std::vector<routine*>& forks(){ return fork; }
	};

	////////////////////////////////////////////////////////////////////////////
	// loop_routine
	//

	/**
	 * Allows a sub-routine to be repeated multiple times.
	 */
	class loop_routine : public routine
	{
	public:
		/**
		 * Constant representing INFINITY, thus allowing a loop to run arbitrarily long.
		 */
		static const unsigned int _INFINITY = ~0;
	private:
		/**
		 * Number of minimum and maximum iterations that need to be performed by
		 * this routine in order to be valid (both inclusive)
		 */
		unsigned int min_iterations, max_iterations;

		/**
		 * The routine that will be looped.
		 */
		routine *loop;
	public:
		loop_routine(pid id, routine *loop, unsigned int min_iterations = 0, unsigned int max_iterations = _INFINITY):
		 	routine(id),
			min_iterations(min_iterations), max_iterations(max_iterations), loop(loop)
		{
			if(id.get_type() != TYPE_LOOP)
				throw parser_exception(PARSER, parser_exception::invalid_pid("loop_routine"));
		}

		/**
		 * Sets the routine to loop over.
		 *
		 * @see routine_tree_builder
		 */
		void set_loop(routine* loop) { this->loop = loop; }

		/**
		 * Gets the routine to loop over
		 */
		routine *get_loop(){ return loop; }

		/**
		 * Sets the minimum number of iterations to complete for a valid input.
		 * Note that setting the minimum-number of iterations to INFINITY
		 * will result in the loop running indefinitely and terminating in error-state
		 * as soon as invalid input is provided.
		 *
		 * @see INFINITY
		 */
		void set_min_iterations(unsigned int min_iterations){ this->min_iterations = min_iterations; }

		/**
		 * Sets the maximum number of iterations to complete for a valid input
		 *
		 * @see INFINITY
		 * @see routine_tree_builder
		 */
		void set_max_iterations(unsigned int max_iterations){ this->max_iterations = max_iterations; }

		/**
		 * Gets the minimum number of iterations required to successfully complete
		 * the entity defined by this routine
		 *
		 * @return minimum number of iterations
		 * @see INFINITY
		 */
		unsigned int get_min_iterations(){ return min_iterations; }

		/**
		 * Gets the maximum number of iterations to represent the
		 * entity defined by this routine
		 *
		 * @return maximum number of iterations
		 * @see INFINITY
		 */
		unsigned int get_max_iterations(){ return max_iterations; }
	};

	////////////////////////////////////////////////////////////////////////////
	// logic_routine
	//

	/**
	 * Represents a simple structural-routine used to build a syntax-tree of single
	 * routines.
	 */
	class struct_routine : public routine
	{
	private:
		/**
		 * Child-/Sub- and next routine of this routine.
		 * The next routine specifies the structure following the
		 * subroutine of this routine.
		 */
		routine *c, *n;
	public:
		/**
		 * Creates a new logic-routine of the given parameters
		 *
		 * @throws parser_exception if an invalid pid was passed
		 */
		struct_routine(pid id, routine *child, routine *next):
			routine(id), c(child), n(next){
			if(id.get_type() != TYPE_STRUCT)
				throw parser_exception(PARSER, parser_exception::invalid_pid("logic_routine"));
		}

		/**
		 * Sets the subroutine of this routine to the specified value
		 *
		 * @param child the new subroutine
		 * @see c
		 * @see get_child
		 */
		void set_child(routine *child){ this->c = child; }

		/**
		 * Sets the next routine of this routine to the specified value
		 *
		 * @param next the new next routine
		 * @see n
		 * @see get_next
		 */
		void set_next(routine *next){ this->n = next; }

		/**
		 * Gets the subroutine of this routine
		 *
		 * @return the childroutine of this routine
		 * @see c
		 * @see set_child
		 */
		routine* get_child(){ return c; }

		/**
		 * Gets the routine following this routine
		 *
		 * @return the next routine of this routine
		 * @see n
		 * @see set_next
		 */
		routine* get_next(){ return n; }
	};

	////////////////////////////////////////////////////////////////////////////
	// echo routine
	//

	/**
	 * A routine that echos a text and doesn't cause any side-effects apart from
	 * outputting a specified text to the console and inserting a lnstruct into
	 * the output tree.
	 *
	 * This entity defines a leaf in both the routine-graph (@link routine_tree_builder)
	 * and the output-tree (@link lnstruct).
	 *
	 * Instances of this routine are solely meant for debugging-purposes and should not be used
	 * apart from the specific intention of testing the implementation of certain routines.
	 */
	class echo_routine : public routine
	{
	private:
		/**
		 * The message this routine displays when run.
		 */
		std::wstring msg;

		/**
		 * A reference to the stream to which the routine will output its
		 * message
		 */
		std::wostream &str;
	public:
		/**
		 * Constructs a new echo_routine with the specified message that will be
		 * printed if the routine is run. The pid of the routine will be initialized
		 * as ECHO (not overrideable).
		 *
		 * @param s the routine to run
		 * @param str the stream to print to
		 * @see ECHO
		 */
		echo_routine(std::wstring s, std::wostream &str = std::wcout):
			routine(ECHO), msg(s), str(str){};

		/**
		 * Getter for the message associated with this routine
		 *
		 * @return the message this routine should display when running
		 * @see msg
		 */
		const std::wstring& get_msg(){ return msg; }

		/**
		 * Getter for the stream to which this routine will output
		 * its message.
		 *
		 * @return the stream to which this routine should output
		 * @see str
		 */
		std::wostream &get_stream(){ return str; }
	};

	/////////////////////////////////////////////////////////////////////////////////
	// string matcher routine
	//

	//FEATURE: expand functionality by char-sets, regex, etc.

	/**
	 * Defines a string-matcher-routine that will terminate successfully if the
	 * sequence of bytes in the input-stream matches the string specified within this
	 * routine. If the input-string doesn't match a parser_exception will be thrown.
	 */
	class string_matcher_routine : public routine
	{
	private:
		/**
		 * String to match against input
		 */
		std::wstring str;
	public:
		/**
		 * Constructs a new routine with the specified pid and
		 * string to match
		 */
		string_matcher_routine(pid id, std::wstring str)
			throw(parser_exception):
			routine(id),
			str(str)
		{
			if(id.get_type() != TYPE_STRING_MATCHER)
				throw parser_exception(id, parser_exception::invalid_pid("string_matcher_routine"));
		}

		/**
		 * Getter for the string to match against input
		 *
		 * @return string to match
		 * @see str
		 */
		const std::wstring& get_str(){ return str; }
	};

	////////////////////////////////////////////////////////////////////////////////////
	// stack trace routine
	//

	/**
	 * Provides a method to display the stack-trace of a parser during runtime.
	 * If run, this routine will display the stack-trace.
	 *
	 * @see parser_routine_interface::visit
	 */
	class stack_trace_routine : public routine
	{
	public:
		stack_trace_routine():
			routine(STACK_TRACE){}
	};

	/////////////////////////////////////////////////////////////////////////////////
	// tree builder
	//

	/**
	 * Builds a tree of routines to parse a specific input. This class represents
	 * a state-machine that operates on a current routine, which can be altered by
	 * building up edges to further routines. To step back to a specific routine, the
	 * builder provides two options: check-points, which stack up, via push_checkpoint() and
	 * pop_checkpoint() and naming/access by names via name(std::wstring) and operator[](std::wstring)
	 *
	 * (Technically incorrect, as the generated structure is a rooted directed graph)
	 *
	 * A routine allocated by this routine_tree_builder will be owned by this routine.
	 * This means that the routine-tree generated by a builder can only be used as long
	 * as the builder exists.
	 *
	 * @see routine
	 */
	class routine_tree_builder
	{
	public:
		/**
		 * specifies the insertion-mode of new routines based on the type of the routine.
		 * If possible the insertion-mode will be set automatically to the appropriate value
		 * (e.g.: loop_routines, which allow for a single way of insertion). Otherwise the
		 * insertion-mode will be required to be setup via set_insertion_mode(insertion_mode& m)
		 *
		 * @see insertion_mode
		 * @see insert_node(routine*)
		 */
		enum insertion_mode
				{
					NONE,
					AS_CHILD,
					AS_NEXT,
					AS_LOOP,
					AS_FORK
				};
	private:
		/**
		 * root of the parser-tree.
		 * This routine will be used as entry-point for parsing. This mmeans any
		 * parsing-operation will start evaluating at this node
		 *
		 * @see mark_root()
		 * @see get()
		 */
		routine *root;

		/**
		 * the current node.
		 *
		 * And operations on the builder will involve this node, Denotes
		 * the current state of the builder for insertion- and marker-operations
		 */
		routine *r;

		/**
		 * table of all named routines.
		 * Any named routine is mapped to its name in this table.
		 *
		 * @see name(std::wstring)
		 * @see operator[](std::wstring)
		 */
		std::map<std::wstring, routine*> name_table;

		/**
		 * List of all routines produced by this routine_tree_builder.
		 * Any instance stored here must be deallocated once the builder
		 * gets destroyed. (See the ownership policy for routines).
		 *
		 * @see routine
		 */
		std::vector<routine*> routines;

		/**
		 * Any checkpoints will be stored here to be restored later.
		 *
		 * @see push_checkpoint()
		 * @see pop_checkpoint()
		 */
		std::stack<routine*> checkpoints;

		/**
		 * Stores the insertion-mode that will be used for the next
		 * insertion into the tree.
		 *
		 * @see insertion_mode
		 * @see insert_node(routine*)
		 * @see set_insertion_mode(insertion_mode)
		 */
		insertion_mode ins_mode = insertion_mode::NONE;

		/**
		 * Inserts the specified routine into the tree
		 * in relation to the last routine r, as specified by
		 * ins_mode. Used by loop(pid, unsigned int, unsigned int), fork(pid),
		 * logic(pid) and by_ptr(routine*)
		 *
		 * @param nr new routine to insert
		 *
		 * @see r
		 * @see ins_mode
		 */
		void insert_node(routine* nr) throw(parser_exception);
	public:
		/**
		 * creates a new routine-tree-builder in its initial configuration, which
		 * is no root and current routine specified and insertion-mode set to
		 * unspecified
		 *
		 * @see r
		 * @see root
		 * @see ins_mode
		 */
		routine_tree_builder(): root(nullptr), r(nullptr){}

		/**
		 * Destroys the routine_tree_builder. In this process any routine
		 * allocated by this routine_tree_builder will be destroyed
		 *
		 * @see routine
		 */
		~routine_tree_builder(){
			std::for_each(routines.begin(), routines.end(), [](routine* r){delete r;});
		}

		/**
		 * markes the currently active routine as root. Any parsing
		 * will start from the node marked as root. There can only be
		 * one node marked as root at a time, but it is possible to switch
		 * root-status to another routine
		 *
		 * @see get()
		 * @see r
		 * @see root
		 */
		routine_tree_builder& mark_root() throw(parser_exception);

		/**
		 * returns the routine marked as root.
		 *
		 * @see mark_root()
		 * @see root
		 */
		routine* get() throw(parser_exception);

		/**
		 * Assigns the specified name to the currently active routine. Note that
		 * routines are unique identifiers within the context of a single
		 * routine_tree_builder. This means that assigning the same name to two
		 * routines will result in the second routine owning the name and the first
		 * routine remaining unnamed. A routine may have multiple names.
		 *
		 * @param str the new name of the current routine
		 *
		 * @see operator[](std::wstring)
		 * @see name_table
		 */
		routine_tree_builder& name(std::wstring str) throw(parser_exception);

		/**
		 * Sets the active routine to the routine with the specified name.
		 *
		 * @param str the name of the searched routine
		 *
		 * @see r
		 * @see name_table
		 * @see name(std::wstring)
		 */
		routine_tree_builder& operator[](std::wstring str) throw(parser_exception);

		/**
		 * Stores the current state of the routine_tree_builder as checkpoint.
		 * Checkpoints may be restored later on. Note that the current state of the
		 * builder only consists of the current routine! Checkpoints can be restored
		 * in LIFO-order.
		 *
		 * @see checkpoints
		 * @see pop_checkpoints()
		 */
		routine_tree_builder& push_checkpoint() throw(parser_exception);

		/**
		 * Restores the state of the most recent checkpoint to the builder.
		 * This state only consists of the current routine!
		 *
		 * @see checkpoints
		 * @see push_checkpoint()
		 */
		routine_tree_builder& pop_checkpoint() throw(parser_exception);

		/**
		 * Generates and inserts a new loop-routine as with the specified parameters and
		 * stores it as the current routine.
		 *
		 * @param id the id of the inserted loop-routine
		 * @param min_iterations minimum-iterations performed by the routine
		 * @param max_iterations maximum-iterations performed by the routine
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see loop_routine
		 */
		routine_tree_builder& loop(pid id, unsigned int min_iterations, unsigned int max_iterations);

		/**
		 * Generates and inserts a new fork-routine in the routine-tree and
		 * stores it as the current routine
		 *
		 * @param id the pid of the fork-routine
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see fork_routine
		 */
		routine_tree_builder& fork(pid id);

		/**
		 * Generates and inserts a new logic-routine in the routine-tree and
		 * stores it as the current routine
		 *
		 * @param id the pid of the logic-routine
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see logic_routine
		 */
		routine_tree_builder& logic(pid id);

		/**
		 * Inserts the specified routine by pointer and sets it as the current
		 * routine. This allows inserting arbitrary (e.g. by the builder not supported)
		 * types of routines into the tree. Inserting a routine via this function is
		 * considered an ownership-transfer and implies that the builder will delete
		 * the routine (either on insertion if an error is thrown or at destruction of
		 * the builder).
		 *
		 * @param r the routine to insert into the tree
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see routine
		 */
		routine_tree_builder& by_ptr(routine* r);

		/**
		 * Sets the insertion-mode of the routine-tree builder. The parameter will be used
		 * to insert the next routine based on the current routine and the next routine
		 * that will be inserted. Can be overridden by repeated calls.
		 *
		 * @param insertion_mode the insertion-mode used for the next routine
		 *
		 * @see insert_node(routine*)
		 */
		routine_tree_builder& set_insertion_mode(insertion_mode m);

		/**
		 * Returns the currently active routine. This access-method can be used to access
		 * the active routines and alter parameters that either weren't exposed by the
		 * insertion-methods or need to be changed after insertion.
		 *
		 * @see r
		 */
		routine* get_current();
	};

	////////////////////////////////////////////////////////////////////////////////////
	// routine interface
	//

	/**
	 * Interface of the parser that gets provided to the routines.
	 * This class provides features required to mark a routine for
	 * repetition, handle exceptions thrown by children,
	 * thus providing means to  emulate exception-handling in a recursion-like maner
	 * and register child- and next routines for execution.
	 *
	 * If a routine terminates with an error-state, the input-stream will be reset to the point
	 * before the routine was run.
	 */
	class routine_interface
	{
	public:
		virtual ~routine_interface(){}

		/**
		 * Sets the repeat-flag for a given parser-routine
		 */
		virtual void repeat() = 0;

		/**
		 * Runs the specified routine as following up on the current routine. Note that
		 * child-routines will always be run first. Multiple calls to this method in the same
		 * run will have the effect of overriding each other. Only the last call during a run
		 * will have effect on the execution.
		 *
		 * Note that this method only accepts "standard"-routines, NOT parser_routines.
		 *
		 * @param r the routine that should run after this routine completed
		 */
		virtual void run_as_next(routine* r) = 0;

		/**
		 * Runs the specified routine as child of this routine. Multiple calls to this
		 * method within the same run will of a routine will have the effect of overriding each
		 * other. Only the last call will have any effect.
		 *
		 * Note that this method only accepts "standard"-routines, NOT parser_routines.
		 *
		 * @param r the routine to run
		 */
		virtual void run_as_child(routine* r) = 0;

		/**
		 * Exits without any side-effects, if the child-routine didn't throw any exception,
		 * otherwise the exception will be rethrown by the method. Needs to be called by any
		 * routine using this interface and owning child-routines!
		 *
		 * If no child-routine was run, this will be considered as faultfree status and
		 * the function will terminate without error
		 *
		 * @throws parser_exception if any child-routine threw an exception
		 * @see parser_exception
		 */
		virtual void check_child_exception() throw(parser_exception) = 0;

		/**
		 * returns the input-stream associated with this parser
		 */
		virtual std::wistream& get_istream() = 0;

		/**
		 * Used to display a stacktrace for the specified stack_trace_routine
		 *
		 * @param r the routine for which the stacktrace will be displayed.
		 */
		virtual void visit(stack_trace_routine& r) = 0;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// parser_routine_factory
	//

	/**
	 * builds a wrapper for a specific routine that adds functionality required
	 * by the parser.
	 *
	 * @see pid
	 * @see routine
	 * @see routine_interface
	 */
	class parser_routine_factory
	{
	public:
		/**
		 * Implementation of routines that interface with parsers. These routines
		 * are single-use and will be deleted once the entity represented by such a
		 * routine is complete, or the routine terminated with an error. This means that
		 * this any routine inheriting from this class must not be run more often
		 * than once and should throw an exception if run is called multiple times without
		 * the repeat-flag set. This is ensured by the implementation of parser_routine
		 * itself. Instances of this class will be produced by the parser_routine_factory.
		 *
		 * @see routine_interface
		 * @see routine
		 * @see parser_routine::legal_run
		 */
		class parser_routine : public routine
		{
		private:
			/**
			 * Flag to determine whether a repeated run of this routine is legal.
			 * Set to true on initialization
			 *
			 * @see repeat
			 * @see ri_run
			 */
			bool legal_run;

			/**
			 * Flag to determine whether an insertion is legal. There may be exactly one child
			 * placed per run. This flag is used to prevent further placements.
			 *
			 * @see place_child
			 * @see ri_run
			 */
			bool legal_insert;
		public:
			parser_routine(pid id): routine(id), legal_run(true), legal_insert(false){}

			virtual ~parser_routine(){}

			/**
			 * returns the lnstruct generated by this routine. By contract
			 * multiple calls on the same instance must always return the same pointer.
			 *
			 * This function must return a valid value after the first call to run. In
			 * contrast before the first call to run a nullptr may be returned as well.
			 * This method may not throw an exception. Since the lnstruct produced by this
			 * class is owned by the parser processing the routine, this method must return
			 * the lnstruct generated by the routine once it is allocated.
			 *
			 * @return the lnstruct produced by this routine
			 * @see lnstruct
			 */
			virtual lnstruct* get_result() = 0;

		protected:
			/**
			 * Callback to place the lnstruct produced by the child in the
			 * parents lnstruct. Note that this function will only be called
			 * if the previous child-routine didn't fail.
			 *
			 * This method may throw an exception in case of an invalid insertion.
			 * E.g. if the routine itself wasn't run yet, or the input is invalid.
			 * By contract this method must throw an exception if the routine wasn't run
			 * so far.
			 *
			 * @param l the lnstruct to insert
			 * @throws parser_exception if the lnstruct cant be placed
			 * @see parser_exception::lnstruct_premature_insertion
			 */
			virtual void place_child(lnstruct* l) throw(parser_exception) = 0;

			/**
			 * Runs the current routine on the given parser_interface
			 *
			 * @throws parser_exception if the rule defined by the routine is violated
			 * @see ri_run
			 */
			virtual void run(routine_interface &ri) throw(parser_exception) = 0;

		public:
			/**
			 * Called by the routine_interface to run this routine. This method
			 * will check the legal_run method to assert the routine won't be used
			 * repeadately and throws a parser_exception if this constraint is violated.
			 *
			 * @param ri the routine_interface on which this routine runs
			 * @see repeat
			 * @see legal_run
			 * @see run
			 */
			void ri_run(routine_interface &ri)
				throw(parser_exception)
			{
				if(!legal_run)
					throw parser_exception(get_pid(), parser_exception::routine_invalid_repeat());

				legal_run = false;
				legal_insert = true;

				run(ri);
			}

			/**
			 * Called by the routine-interface to place a child-element. Prevents
			 * repetitive/invalid calls of place_child. I.e. at most one child-element
			 * may be placed in this routine per call to run. It is not allowed to insert
			 * a nullptr as child, as this would possibly break the output-tree apart.
			 *
			 * @see place_child
			 * @see legal_insert
			 */
			void ri_place_child(lnstruct *l)
				throw(parser_exception)
			{
				if(!legal_insert)
					throw parser_exception(get_pid(), parser_exception::lnstruct_invalid_insertion("routine"));

				if(l == nullptr)
					throw dvl::parser_exception(get_pid(), dvl::parser_exception::nullptr_error("Child may not be null"));

				legal_insert = false;

				place_child(l);
			}

		protected:
			/**
			 * Called by inheriting routines to set the repeat-flag for this routine.
			 * A call to this method will mark the routine for repetition and mark
			 * the routine as legal for repetition.
			 *
			 * @param ri the routine_interface on which this routine will be marked for repetition
			 * @see ri_run
			 * @see legal_run
			 */
			void repeat(routine_interface &ri)
			{
				ri.repeat();

				legal_run = true;
			}

			/**
			 * Returns true if the current run is valid. I.e. if it is either the first run, or
			 * the last run called repeat.
			 *
			 * @return true if the run is valid
			 * @see legal_run
			 * @see repeat
			 */
			bool valid_run()
			{
				return legal_run;
			}
		};

		/**
		 * Typedef for a function to transform a standard-routine into a
		 * parser_routine
		 *
		 * @see routine
		 * @see parser_routine
		 */
		typedef std::function<parser_routine*(routine*)> transform;

		parser_routine_factory();
		virtual ~parser_routine_factory(){}

		/**
		 * Translates a routine into a parser_routine
		 * based on the type provided with the pid of the
		 * routine given as parameter
		 *
		 * @see pid
		 * @see routine
		 * @see parser_routine
		 */
		parser_routine* build_routine(routine* r) throw(parser_exception);

		/**
		 * Registers a transformation-routine to generate
		 * a parser_routine from a given routine
		 */
		void register_transformation(uint8_t type, transform t);
	private:
		/**
		 * Maps types onto the respective transformation-function
		 *
		 * @see pid
		 * @see transform
		 */
		std::map<uint8_t, transform> transformations;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// proutine typedef
	//

	typedef parser_routine_factory::parser_routine proutine;

	//////////////////////////////////////////////////////////////////////////////////
	// parser context
	//

	/**
	 * A context-structure holding all relevant objects required to run
	 * the parser.
	 *
	 * @see parser
	 */
	struct parser_context
	{
	public:
		parser_context(std::wistream &str, routine_tree_builder &builder, pid_table &pt):
			str(str), builder(builder), pt(pt)
		{}

		/**
		 * The input-stream associated with this context
		 */
		std::wistream &str;

		/**
		 * The routine_tree_builder that was used to generate the syntax-tree
		 *
		 * @see routine_tree_builder
		 */
		routine_tree_builder &builder;

		/**
		 * Lookup table for names of pids that will be used within this context
		 *
		 * @see pid_table
		 */
		pid_table &pt;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// stack_callback_interface
	//

	/**
	 * Interface that is used to keep the stack of a module of the parser in sync with
	 * all other modules. There is no need to separate state and future state-transitions
	 * caused by this interface, as callbacks will be managed by the stack_callback and
	 * handled in apropriate order. This interface is used by @link callback_action in
	 * order to perform operations related to a specific action.
	 *
	 * The basic order of action should be to work off a run of a single routine and
	 * work off all required actions batchwise as specified for @link stack_callback.
	 *
	 * @see stack_callback
	 * @see callback_actions
	 */
	class stack_callback_interface
	{
	public:
		virtual ~stack_callback_interface(){}

		/**
		 * Pop one frame of the stack of this interface. Calling this
		 * method multiple times within a single batch should pop
		 * an according number of frames.
		 */
		virtual void pop() = 0;

		/**
		 * Pop a stack from this frame exception-driven. Calling this
		 * method multiple times within a single batch should pop
		 * an according number of frames.
		 *
		 * @param e the exception causing the stack-unwinding
		 */
		virtual void pop(const parser_exception &e) = 0;

		/**
		 * Push a new frame onto the stack that will run the specified routine.
		 * Calling the method multiple times within a single batch should push
		 * all frames onto the stack.
		 *
		 * @param p the routine to run
		 */
		virtual void push(proutine *p) = 0;

		/**
		 * Emplace a new routine to run next after the one in the current frame.
		 * Note that if this function is called multiple times while handling a single batch,
		 * all calls affect the same frame and the corresponding state-changes will be
		 * implementation-specific. It is recommended to only let the last call take effect.
		 *
		 * @param p the routine to run next
		 */
		virtual void next_routine(proutine *p) = 0;

		/**
		 * Marks the routine in the current frame for repetition.
		 */
		virtual void repeat() = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// callback_actions
	//

	/**
	 * Provides a collection of actions that perform operations on
	 * stack_callback_interfaces based on their type.
	 *
	 * @see stack_callback_interface
	 */
	struct callback_actions
	{
	public:
		/**
		 * Provides the basic interface for any action
		 */
		class callback_action
		{
		public:
			virtual ~callback_action(){}

			/**
			 * Will be called in order to perform a specific action on a callback_interface
			 *
			 * @param callback the stack_callback_interface on which the action will be performed
			 */
			virtual void run(stack_callback_interface *callback) = 0;
		};

		/**
		 * Action related to popping a frame off the stack
		 *
		 * @see stack_routine_interface::pop()
		 */
		class pop_action : public callback_action
		{
		public:
			/**
			 * Pops a single frame of the stack of the routine specified by the callback.
			 *
			 * @param callback the callback_interface from which the frame will be popped
			 */
			void run(stack_callback_interface *callback){ callback->pop(); }
		};

		/**
		 * Action related to popping a frame off the stack exception-driven
		 *
		 * @see stack_routine_interface::pop(const parser_exception&)
		 */
		class pop_ex_action : public callback_action
		{
		private:
			/**
			 * Reference to the exception that was caught.
			 */
			const parser_exception &e;
		public:
			/**
			 * Builds a new action based on the exception that was caught
			 *
			 * @param e the exception that needs to be handled
			 * @see e
			 */
			pop_ex_action(const parser_exception &e): e(e){}

			/**
			 * Pops a single frame off the stack of the routine specifiied by callback
			 *
			 * @param callback the interface to pop a frame off
			 */
			void run(stack_callback_interface *callback){ callback->pop(e); }
		};

		/**
		 * Action associated with pushing a single frame onto the stack
		 *
		 * @see stack_routine_interface::push(proutine*)
		 */
		class push_action : public callback_action
		{
		private:
			/**
			 * The routine that will run on the new frame
			 */
			proutine *const p;
		public:
			/**
			 * Builds a new push-action that will push the specified routine onto the stack
			 *
			 * @param p the routine to push
			 */
			push_action(proutine *const p): p(p){}

			/**
			 * Pushes a new frame on the stack of the specified interface
			 *
			 * @param callback the stack_callback_interface to push the frame onto
			 */
			void run(stack_callback_interface *callback){ callback->push(p); }
		};

		class next_action : public callback_action
		{
		private:
			proutine *const p;
		public:
			next_action(proutine *const p): p(p){}

			void run(stack_callback_interface *callback){ callback->next_routine(p); }
		};

		class repeat_action : public callback_action
		{
		public:
			void run(stack_callback_interface *callback){ callback->repeat(); }
		};
	};

	//////////////////////////////////////////////////////////////////////////////////
	// stack callback
	//

	/**
	 * Mediator that is used to keep stacks of single modules of the parser in sync.
	 * In order to keep the stack-frames in the parser in sync it is necessary to
	 * make sure any modifications to the stack like pushing a new frame,popping one
	 * moving on to the next routine in the same frame, or even unwinding the frame
	 * will be done via this interface.
	 *
	 * This call is solely used as a synchronous messaging-pipe. Note that by contract
	 * none of the actions issued via this callback will affect the externally visible
	 * state of the interfaces associated with it.
	 *
	 * Note that the callback is non-reflexive. This means the calling object (specified as
	 * parameter in the methods) will not receive a callback, but is responsible on it's own
	 * for handling further actions.
	 *
	 * Note that action wont take place in the order in which they were specified, but by
	 * modifying the current frame before pushing a new one, or dropping actions if conflicting
	 * events with higher priority get scheduled. E.g. popping a frame exception-driven will always
	 * override repetitions or pushing new frames.
	 *
	 * TODO ordering of events according to represented action
	 *
	 * @see routine_manager
	 * @see output_manager
	 * @see parser
	 * @see stack_callback_interface
	 */
	class stack_callback
	{
	private:
		std::vector<stack_callback_interface*> interfaces;
	public:
		void register_callback_interface(stack_callback_interface *callback);

		bool is_initialized(){ return false; }

		void pop(const stack_callback_interface *callback);
		void pop(const stack_callback_interface *callback, const parser_exception &e);
		void push(const stack_callback_interface *callback, proutine *p);
		void next(const stack_callback_interface *callback, proutine *p);
		void repeat(const stack_callback_interface *callback);
		void step(const stack_callback_interface *callback);
	};
}

#endif
