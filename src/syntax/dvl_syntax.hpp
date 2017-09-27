#ifndef SYNTAX_DVL_SYNTAX_HPP_
#define SYNTAX_DVL_SYNTAX_HPP_

#include "../parser.hpp"

namespace syntax
{
	/**
	 * Constant names for special routines
	 */
	extern const std::wstring ROOT_NAME,
								COMMENT_NAME,
								RULE_NAME,
								STRING_NAME,
								SPACES_NAME,
								NEWLINE_NAME,
								CHARSET_NAME,
								NAME_NAME,
								PNUM_NAME,
								STRUCT_RULE_NAME,
								CONCAT_RULE_NAME,
								OPTION_RULE_NAME,
								BRACKET_RULE_NAME,
								ATOM_NAME,
								REPETITION_RULE_NAME,
								REPETITION_RANGE_NAME,
								DEFINITION_NAME;

	extern const uint32_t GROUP_SYNTAX_TREE;
	extern const dvl::pid SYNTAX_ROOT,
							// defines the id for routines that don't require a unique-identifier
							ANONYMOUS_STRUCT,
							ANONYMOUS_STRING,
							ANONYMOUS_FORK,

							// utility-routines
							SPACES,
							NEWLINE,

							// comment
							COMMENT,
							COMMENT_INDICATOR,
							COMMENT_CONTENT,

							// string
							STRING,
							STRING_START,
							STRING_CONTENT,
							STRING_TERMINATOR,

							// charset
							CHARSET,
							CHARSET_INDICATOR,
							CHARSET_CONTENT,
							CHARSET_TERMINATOR,

							// name
							NAME,

							// atom
							ATOM,

							// rule
							RULE,

							// concat rule
							CONCAT_RULE,
							CONCAT_A,
							CONCAT_B,

							// option_rule
							OPTION_RULE,
							OPTION_A,
							OPTION_B,

							// bracket_rulee
							BRACKET_RULE,
							BRACKET_CONTENT,

							// struct_rule
							STRUCT_RULE,

							// pnum
							PNUM,

							// repetition_rule
							REPETITION_RULE,
							REPETITION_RANGE,
							REPETITION_TYPE_SINGLE,
							REPETITION_TYPE_RANGE,
							REPETITION_RANGE_LB,
							REPETITION_RANGE_UB,
							REPETITION_OPERATOR,

							// definition
							DEFINITION;

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
	 * operations will be interpreted from left to right. Between operators and definitions spaces may
	 * be chosen arbitrarily.
	 *
	 * @param context the context on which the process will reflect
	 */
	void
	build_syntax_file_definition(dvl::parser_context &context);
}

#endif /* SYNTAX_DVL_SYNTAX_HPP_ */
