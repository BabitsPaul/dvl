#ifndef DVL_SYNTAX_HPP_
#define DVL_SYNTAX_HPP_

#include "parser.hpp"

namespace dvl
{
	/**
	 * Sets up the specified parser_context for processing syntax-definition-files.
	 * Note that this method will NOT check whether the context is already initialized
	 * for another parsing-process. This means that in case of an already initialized
	 * context the program may produce unexpected behavior or crash!
	 *
	 * The syntax of such a definition-file looks as follows:
	 * ((<rule> | <comment>) \n)*
	 * <comment> 			:= 	a '#' preceeded by an arbitrary number of spaces and followed by
	 * 							arbitrary text (single-line comment)
	 * <rule>				:= 	<name> ':=' <definition>
	 * <name>				:= 	an arbitrary alpha-numeric string
	 * <definition>			:= 	<struct_rule> | <repetition_rule> | <atoms>
	 * <struct_rule>		:=	'(' <definition> ')' | <definition>'.'<definition> | <definition>'|'<definiton>
	 * <repetition_rule>	:=	<definition>'*' | <definition>'?' | <definition>'+' |
	 * 							<definition> '{' <pnum>? ',' <pnum>? '}' | <definition> '{' <pnum> '}'
	 * <atoms>				:=	<name> | <charset> | <string>
	 * <charset>			:=	'$(' charset-def ')' where charset-def uses the same syntax
	 * 							 as @link charset_routine
	 * <string>				:= 	'#(' str ')' is a string of arbitrary content. To use '(' or ')' these
	 * 							characters must be escaped by \. In addition all other
	 * 							standard-escape-sequences are supported. The string starts directly
	 * 							after the first bracket and ends with the last character before the
	 * 							closing bracket
	 * <pnum>				:=	arbitrary non-negative integer in decimal base
	 *
	 * Provided operators are '( ... )' to specify precedence, '.' to concat definitions,
	 * '|' as "or", ?, +, * and {<pnum>, <pnum>} to specify how often the routine may be repeated
	 * (these operators are equivalent to those used for regex).
	 *
	 * Precedence from higher to lower-order: ( ... ) -> repetition -> '.' -> '|'
	 * operations will be interpreted from left to right.
	 *
	 * @param context the context on which the process will reflect
	 */
	void
	build_syntax_file_defintion(parser_context &context);
}

#endif /* DVL_SYNTAX_HPP_ */
