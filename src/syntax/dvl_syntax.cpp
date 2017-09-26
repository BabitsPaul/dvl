#include "dvl_syntax.hpp"

#include <boost/regex.hpp>

void
syntax::build_syntax_file_definition(dvl::parser_context &c)
{
	using namespace dvl;

	typedef routine_tree_builder::insertion_mode ins_mod;

	static const std::wstring ROOT_NAME = L"root",
								COMMENT_NAME = L"comment",
								RULE_NAME = L"rule",
								STRING_NAME = L"string",
								SPACES_NAME = L"spaces",
								NEWLINE_NAME = L"newline",
								CHARSET_NAME = L"charset",
								NAME_NAME = L"name",
								PNUM_NAME = L"pnum",
								STRUCT_RULE_NAME = L"struct_rule",
								CONCAT_RULE_NAME = L"concat_rule",
								OPTION_RULE_NAME = L"option_name",
								BRACKET_RULE_NAME = L"bracket_rule",
								ATOM_NAME = L"atom",
								REPETITION_RULE_NAME = L"repetition_rule",
								REPETITION_RANGE_NAME = L"repetition_range",
								DEFINITION_NAME = L"definition";

	// TODO looping routines must allow alternative routines after completing minimum-number of loops!!!
	// TODO prevent copying of the routine_tree_builder
	// TODO allow decoupling of declaration and initialization of routines
	// TODO prevent self-referencing routines from looping arbitrarily
	// TODO tree_builder: implement followed-by relation-ship as single operation
	// TODO inheritance for pids
	// TODO make constants global for access in reader

	static const uint32_t GROUP_SYNTAX_TREE = 2;
	static const pid SYNTAX_ROOT = {GROUP_SYNTAX_TREE, 0l, TYPE_LOOP},
						// defines the id for routines that don't require a unique-identifier
						ANONYMOUS_STRUCT = {GROUP_SYNTAX_TREE, 100l, TYPE_STRUCT},
						ANONYMOUS_STRING = {GROUP_SYNTAX_TREE, 101l, TYPE_STRING_MATCHER},
						ANONYMOUS_FORK = {GROUP_SYNTAX_TREE, 102l, TYPE_FORK},

						// utility-routines
						SPACES = {GROUP_SYNTAX_TREE, 200l, TYPE_CHARSET},
						NEWLINE = {GROUP_SYNTAX_TREE, 201l, TYPE_STRING_MATCHER},

						// comment
						COMMENT = {GROUP_SYNTAX_TREE, 300l, TYPE_STRUCT},
						COMMENT_INDICATOR = {GROUP_SYNTAX_TREE, 301l, TYPE_CHARSET},
						COMMENT_CONTENT = {GROUP_SYNTAX_TREE, 302l, TYPE_CHARSET},

						// string
						STRING = {GROUP_SYNTAX_TREE, 400l, TYPE_STRUCT},
						STRING_START = {GROUP_SYNTAX_TREE, 401l, TYPE_STRING_MATCHER},
						STRING_CONTENT = {GROUP_SYNTAX_TREE, 402l, TYPE_LAMBDA},
						STRING_TERMINATOR = {GROUP_SYNTAX_TREE, 403l, TYPE_STRING_MATCHER},

						// charset
						CHARSET = {GROUP_SYNTAX_TREE, 500l, TYPE_STRUCT},
						CHARSET_INDICATOR = {GROUP_SYNTAX_TREE, 501l, TYPE_STRING_MATCHER},
						CHARSET_CONTENT = {GROUP_SYNTAX_TREE, 502l, TYPE_LAMBDA},
						CHARSET_TERMINATOR = {GROUP_SYNTAX_TREE, 503l, TYPE_STRING_MATCHER},

						// name
						NAME = {GROUP_SYNTAX_TREE, 600l, TYPE_CHARSET},

						// atom
						ATOM = {GROUP_SYNTAX_TREE, 700l, TYPE_FORK},

						// rule
						RULE = {GROUP_SYNTAX_TREE, 800l, TYPE_STRUCT},

						// concat rule
						CONCAT_RULE = {GROUP_SYNTAX_TREE, 900l, TYPE_STRUCT},
						CONCAT_A = {GROUP_SYNTAX_TREE, 901l, TYPE_STRUCT},
						CONCAT_B = {GROUP_SYNTAX_TREE, 902l, TYPE_STRUCT},

						// option_rule
						OPTION_RULE = {GROUP_SYNTAX_TREE, 1000l, TYPE_STRUCT},
						OPTION_A = {GROUP_SYNTAX_TREE, 1001l, TYPE_STRUCT},
						OPTION_B = {GROUP_SYNTAX_TREE, 1002l, TYPE_STRUCT},

						// bracket_rulee
						BRACKET_RULE = {GROUP_SYNTAX_TREE, 1100l, TYPE_STRUCT},
						BRACKET_CONTENT = {GROUP_SYNTAX_TREE, 1101l, TYPE_STRUCT},

						// struct_rule
						STRUCT_RULE = {GROUP_SYNTAX_TREE, 1200l, TYPE_FORK},

						// pnum
						PNUM = {GROUP_SYNTAX_TREE, 1300l, TYPE_CHARSET},

						// repetition_rule
						REPETITION_RULE = {GROUP_SYNTAX_TREE, 1400l, TYPE_STRUCT},
						REPETITION_RANGE = {GROUP_SYNTAX_TREE, 1401l, TYPE_STRUCT},
						REPETITION_TYPE_SINGLE = {GROUP_SYNTAX_TREE, 1402l, TYPE_STRUCT},
						REPETITION_TYPE_RANGE = {GROUP_SYNTAX_TREE, 1403l, TYPE_STRUCT},
						REPETITION_RANGE_LB = {GROUP_SYNTAX_TREE, 1404l, TYPE_LOOP},
						REPETITION_RANGE_UB = {GROUP_SYNTAX_TREE, 1405l, TYPE_LOOP},
						REPETITION_OPERATOR = {GROUP_SYNTAX_TREE, 1406l, TYPE_FORK},

						// definition
						DEFINITION = {GROUP_SYNTAX_TREE, 1500l, TYPE_FORK};

	// builds the syntax-rules of the syntax-files used by this parser
	routine_tree_builder &b = c.builder;

	// spaces
	b.detach().match_set(SPACES, L"[\t ]*").name(SPACES_NAME).finalize(SPACES_NAME);

	// newline
	b.detach().match_string(NEWLINE, L"\n").name(NEWLINE_NAME).finalize(NEWLINE_NAME);

	// comment
	// # followed by arbitrary number of characters until next new-line
	b.detach().logic(COMMENT).name(COMMENT_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_set(COMMENT_INDICATOR, L"[#]").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_set(COMMENT_CONTENT, L"![\n]*").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.finalize(COMMENT_NAME);

	// string
	// #(<str>)
	b.detach().logic(STRING).name(STRING_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(STRING_START, L"#(").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.lambda(STRING_CONTENT, [](routine_interface &ri) throw(parser_exception)->lnstruct*{
					lnstruct *ln = new lnstruct(STRING_CONTENT, ri.get_istream().tellg());

					ri.check_child_exception();	// shouldn't throw, since this routine is child-less

					// read until first unescaped closing-bracket is encountered. Alternatively the
					// loop terminates if either EOF or the end of the line is encountered
					bool escaped = false;
					wint_t c = ri.get_istream().get();

					while(c != WEOF && c != '\n' && !(c == L')' && !escaped))
					{
						if(c == L'\\')
							escaped = !escaped;
						else
							escaped = false;

						c = ri.get_istream().get();
					}

					// check if string-definition is unterminated
					if(c == WEOF || c == '\n')
					{
						delete ln;
						throw parser_exception(STRING_CONTENT, "Reached EOF while processing definition");
					}

					// reset stream back before bracket
					ri.get_istream().seekg(-1l, std::ios::cur);

					return ln;
				}).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(STRING_TERMINATOR, L")").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.by_name(NEWLINE_NAME)
			.finalize(STRING_NAME);

	// charset
	// $(<set>)
	b.detach().logic(CHARSET).name(CHARSET_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
				.match_string(CHARSET_INDICATOR, L"$(").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
				.lambda(CHARSET_CONTENT, [](routine_interface &ri) throw(parser_exception)->lnstruct*{
					lnstruct *ln = new lnstruct(CHARSET_CONTENT, ri.get_istream().tellg());

					ri.check_child_exception();	// shouldn't throw, since this routine is child-less

					// read until first unescaped closing-bracket is encountered. Alternatively the
					// loop terminates if either EOF or the end of the line is encountered
					bool escaped = false;
					wint_t c = ri.get_istream().get();

					while(c != WEOF && c != '\n' && !(c == L')' && !escaped))
					{
						if(c == L'\\')
							escaped = !escaped;
						else
							escaped = false;

						c = ri.get_istream().get();
					}

					// check if string-definition is unterminated
					if(c == WEOF || c == '\n')
					{
						delete ln;
						throw parser_exception(STRING_CONTENT, "Reached EOF while processing definition");
					}

					// reset stream back before bracket
					ri.get_istream().seekg(-1l, std::ios::cur);

					return ln;
				}).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(CHARSET_TERMINATOR, L")").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.by_name(NEWLINE_NAME)
			.finalize(CHARSET_NAME);

	// name
	b.detach().match_set(NAME, L"[A-Za-z0-9]*").name(NAME_NAME).finalize(NAME_NAME);

	// atom
	b.detach().fork(ATOM).name(ATOM_NAME).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(CHARSET_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK).push_checkpoint()
			.by_name(STRING_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(NAME_NAME)
			.finalize(ATOM_NAME);

	// rule predefinition
	b.detach().logic(RULE).name(RULE_NAME);	// keep unfinalized. This is just a declaration to
											// allow composites

	// concat_rule
	b.detach().logic(CONCAT_RULE).name(CONCAT_RULE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(CONCAT_A).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L".").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(CONCAT_B).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME)
			.finalize(CONCAT_RULE_NAME);

	// option_rule
	b.detach().logic(OPTION_RULE).name(OPTION_RULE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(OPTION_A).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L"|").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(OPTION_B).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME)
			.finalize(OPTION_RULE_NAME);

	// bracket_rule
	b.detach().logic(BRACKET_RULE).name(BRACKET_RULE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L"(").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(BRACKET_CONTENT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L")")
			.finalize(BRACKET_RULE_NAME);

	// TODO unpopped checkpoint in stack!!!

	// struct_rule
	b.detach().fork(STRUCT_RULE).name(STRUCT_RULE_NAME).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(CONCAT_RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK).push_checkpoint()
			.by_name(OPTION_RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(BRACKET_RULE_NAME)
			.finalize(STRUCT_RULE_NAME);

	// pnum
	b.detach().match_set(PNUM, L"[0-9]+").name(PNUM_NAME).finalize(PNUM_NAME);

	// repetition_range
	b.detach().logic(REPETITION_RANGE).name(REPETITION_RANGE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L"{").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.fork(ANONYMOUS_FORK).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
					.logic(REPETITION_TYPE_SINGLE).set_insertion_mode(ins_mod::AS_CHILD)
						.by_name(PNUM_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
					.logic(REPETITION_TYPE_RANGE).set_insertion_mode(ins_mod::AS_CHILD)
						.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
							.loop(REPETITION_RANGE_LB, 0, 1).set_insertion_mode(ins_mod::AS_LOOP)
								.by_name(PNUM_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
						.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
							.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
						.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
							.match_string(ANONYMOUS_STRING, L",").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
						.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
							.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
						.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
							.loop(REPETITION_RANGE_UB, 0, 1).set_insertion_mode(ins_mod::AS_LOOP)
								.by_name(PNUM_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L"}")
				.finalize(REPETITION_RANGE_NAME);

	// repetition_rule
	b.detach().logic(REPETITION_RULE).name(REPETITION_RULE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.fork(REPETITION_OPERATOR).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
				.match_string(ANONYMOUS_STRING, L"*").pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK).push_checkpoint()
				.match_string(ANONYMOUS_STRING, L"+").pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK).push_checkpoint()
				.match_string(ANONYMOUS_STRING, L"?").pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
				.by_name(REPETITION_RANGE_NAME)
			.finalize(REPETITION_RULE_NAME);

	// definition
	b.detach().fork(DEFINITION).name(DEFINITION_NAME).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(REPETITION_RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK).push_checkpoint()
			.by_name(STRUCT_RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
			.by_name(ATOM_NAME)
			.finalize(DEFINITION_NAME);

	// rule
	b.detach().by_name(RULE_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(NAME_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.match_string(ANONYMOUS_STRING, L":=").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(DEFINITION_NAME)
			.finalize(RULE_NAME);

	// root
	b.detach().loop(SYNTAX_ROOT, 0, loop_routine::_INFINITY).name(ROOT_NAME).mark_root().set_insertion_mode(ins_mod::AS_LOOP)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.fork(ANONYMOUS_FORK).push_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
					.by_name(RULE_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_FORK)
					.by_name(COMMENT_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS_STRUCT).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
				.by_name(NEWLINE_NAME)
			.finalize(ROOT_NAME);
}
