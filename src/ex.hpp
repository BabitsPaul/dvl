#ifndef EX_HPP_
#define EX_HPP_

#include "id/pid.hpp"

#include <string>

namespace dvl
{
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
}

#endif /* EX_HPP_ */
