#ifndef SYNTAX_ROUTINES_HPP_
#define SYNTAX_ROUTINES_HPP_

#include "../ex.hpp"
#include "../id/pid.hpp"
#include "../outp/lnstruct.hpp"

#include <stack>
#include <set>
#include <iostream>
#include <boost/regex.hpp>

namespace dvl
{
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

	////////////////////////////////////////////////////////////////////////////////////
	// charset_routine
	//

	/**
	 * Provides a method to match a charset within the input-stream and allows both
	 * fixed and arbitrary repetition of said characters.
	 *
	 * The syntax of the definition is (backspace denotes semantic operators):
	 * 					<negator>?\[<charset>\]<repetition>?
	 * <charset> := 	\(<char-range>\|<char>\)<charset>\?
	 * <char-range> := 	<char>-<char>
	 * <char> :=		any character, where brackets and backslashes need to be escaped
	 * <repetition> :=	{<num-opt>,<num-opt>}|*|?|+
	 * <num-opt> :=		either no input or a positive integer
	 * <negator> :=		!
	 *
	 * If not specified otherwise via repetition, the charset will default to exactly one
	 * match. If specified in the defintion the lower bound must always be smaller than the
	 * upper-bound. It is valid to ommit either of the parameters, which will automatically fall
	 * back to default-behavior. Lower bounds will automatically be replaced by 0, missing upper
	 * bounds will be replaced by infinity. Alternatively one can specify
	 * the number of valid repetitions via * (no bounds), + (at least one), or ? (at most one).
	 * If the {<num-opt>,<num-opt>}-construct is used, the comma must be present irrespective of
	 * whether any numbers are present in the brackets (note that {,} is equivalent to *).
	 * The upper bound of num-opt is @c 2^32 and the value must be specified in decimal.
	 *
	 * For charsets the lower bound must have a strictly lower value in the
	 * used charset than the upper bound. Both are used inclusively. I.e. "A-A" and A are
	 * interpreted as equivalent. Special characters must be escaped by a \. This applies to the
	 * following characters: \, [, ]. Also the standard escape-sequences are valid. The character-set
	 * must not be empty.
	 *
	 * To negate a pattern, it must be preceeded by a !. In this case the routine will match
	 * any character not contained in the charset
	 *
	 * There may be an arbitrarily large number of spaces or tabs before the charset-definition,
	 * between the charset-definition and the repetition-specification and after the
	 * repetition-specification.
	 *
	 * Violating any of the above listed rules will result in a parser_exception being
	 * thrown upon initialization.
	 *
	 * @see TYPE_CHARSET
	 */
	class charset_routine : public routine
	{
	private:
		/**
		 * The definition of this routines matched chars as plain text
		 */
		std::wstring def;

		/**
		 * The function used to match chars against the char-set defined for
		 * this routine
		 *
		 * @see init_matcher
		 * @see def
		 */
		std::function<bool(wchar_t c)> matcher;

		/**
		 * Minimum number of valid repetitions
		 */
		unsigned int min_repetition;

		/**
		 * Maximum number of valid repetitions
		 */
		unsigned int max_repetition;

		/**
		 * Initializes the routine and processes the string-representation of the routine
		 * into a matcher-functor.
		 *
		 * @see matcher
		 * @see def
		 * @see charset_routine(pid, std::wstring)
		 */
		static void init_matcher(charset_routine&) throw(parser_exception);
	public:
		/**
		 * Defines the infinity-value for unsigned int. Note that the maximum-value
		 * for unsigned int will always be interpreted as @c _INFINITY irrespective of
		 * context!
		 *
		 * @see min_repetition
		 * @see max_repetition
		 */
		static const unsigned int _INFINITY = ~0;

		/**
		 * Constructs a new charset_routine with the provided defintion,
		 * which will be subsequently parsed in @link init_matcher(charset_routine&)
		 *
		 * @param id the id of the routine
		 * @param def the definition of the charset (see @link charset_routine)
		 * @throws parser_exception if either the id or the definition are invalid
		 *
		 * @see init_matcher(charset_routine&)
		 * @see get_matcher()
		 */
		charset_routine(pid id, std::wstring def) throw(parser_exception):
			routine(id),
			def(def)
		{
			if(id.get_type() != TYPE_CHARSET)
				throw parser_exception(get_pid(), parser_exception::invalid_pid("charset_routine"));

			init_matcher(*this);
		}

		/**
		 * Getter for @link matcher
		 *
		 * @return a functor to determine whether a char belongs to the charset
		 *
		 * @see matcher
		 */
		std::function<bool(wchar_t)>& get_matcher(){ return matcher; }

		/**
		 * Getter for the number of minimum-repetitions.
		 *
		 * @return the value of @c min_repetition
		 * @see min_repetition
		 * @see _INFINITY
		 */
		const unsigned int &get_min_repetitions(){ return min_repetition; }

		/**
		 * Getter for the number of maximum-repetitions.
		 *
		 * @return the value of @c max_repetition
		 * @see max_repetition
		 * @see _INFINITY
		 */
		const unsigned int &get_max_repetitions(){ return max_repetition; }
	};

	/////////////////////////////////////////////////////////////////////////////////
	// regex_routine
	//

	/**
	 * <em>For future use! At the moment this class is not used by the implementation. </em>
	 *
	 * Routine that matches the stream starting from the given position against the
	 * specified regex. The regex must match starting from the current posiion of the
	 * input-stream in order for the routine to succeed.
	 *
	 * @see TYPE_REGEX
	 */
	class regex_routine : public routine
	{
	private:
		/**
		 * The regex associated with this routine
		 *
		 * @see get_reg()
		 */
		boost::wregex reg;
	public:
		/**
		 * Constructs a new regex_routine for the given regex (@p reg) and
		 * with the specified pid.
		 */
		regex_routine(pid id, std::wstring reg):
			routine(id),
			reg(reg)
		{
			if(id.get_type() != TYPE_REGEX)
				throw parser_exception(id, parser_exception::invalid_pid("regex_routine"));
		}

		/**
		 * Getter for the regex of this routine
		 *
		 * @return the regex of this routine
		 * @see reg
		 */
		const boost::wregex& get_reg(){ return reg; }
	};

	/////////////////////////////////////////////////////////////////////////////////
	// lambda-routine
	//

	class routine_interface;

	class lambda_routine : public routine
	{
	public:
		typedef std::function<lnstruct*(routine_interface&) throw(parser_exception)> p_func;
	private:
		p_func f;
	public:
		lambda_routine(pid id, p_func f):
			routine(id),
			f(f)
		{
			if(id.get_type() != TYPE_LAMBDA)
				throw parser_exception(id, parser_exception::invalid_pid("lambda_routine"));
		}

		p_func &get_f(){ return f; }
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
		std::set<routine*> routines;

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
		 * Set of routines that were marked as finalized. Finalized
		 * routines may not be manipulated anymore. This is useful to define sub-modules,
		 * while preventing accidential rewriting of the structure
		 *
		 * @see finalize(std::wstring)
		 */
		std::set<routine*> finalized;

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
			std::for_each(routines.begin(), routines.end(), [](routine* r){ delete r; });
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
		 * names are unique identifiers within the context of a single
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

		/**
		 * Detaches the last worked on subgraph from this builders active state.
		 * After this action a new subgraph that is disconnected from the original graph
		 * can be built up.
		 *
		 * @see r
		 */
		routine_tree_builder &detach();

		/**
		 * Emplaces a routine by it's name. The builder won't step into the sub-routine in order
		 * to avoid violating the state of the routine.
		 *
		 * @see operator[](std::wstring)
		 * @see name()
		 */
		routine_tree_builder &by_name(std::wstring name) throw(parser_exception);

		/**
		 * Prevents a named routine from being modified any further. This includes that it
		 * is disallowed to traverse the graph from the routine by any means.
		 *
		 * @param name the name of the routine to finalize
		 */
		routine_tree_builder &finalize(std::wstring name) throw(parser_exception);

		/**
		 * Generates and inserts a new string_matcher_routine in the routine-graph
		 * that will match the specified string (@p match)
		 *
		 * @param id the pid of the matcher-routine
		 * @param match the string to match
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see string_matcher_routine
		 */
		routine_tree_builder &match_string(pid id, std::wstring match);

		/**
		 * Generates and inserts a new charset_routine in the routine_graph.
		 *
		 * @param id the pid of the charset_routine
		 * @param set_def the definition of the matched character-set
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see charset_routine
		 */
		routine_tree_builder &match_set(pid id, std::wstring set_def);

		/**
		 * Generates and inserts a new lambda-routine in the routine-graph.
		 *
		 * @param id the pid of the lambda-routine
		 * @param f the parsering-function of the routinne
		 * @return @c *this
		 *
		 * @see r
		 * @see insert_node(routine*)
		 * @see lambda_routine
		 */
		routine_tree_builder &lambda(pid id, lambda_routine::p_func f);
	};
}

#endif /* SYNTAX_ROUTINES_HPP_ */
