#include "dvl_syntax.hpp"

#include <boost/regex.hpp>

const std::wstring syntax::ROOT_NAME = L"root",
			syntax::COMMENT_NAME = L"comment",
			syntax::RULE_NAME = L"rule",
			syntax::STRING_NAME = L"string",
			syntax::SPACES_NAME = L"spaces",
			syntax::NEWLINE_NAME = L"newline",
			syntax::CHARSET_NAME = L"charset",
			syntax::NAME_NAME = L"name",
			syntax::PNUM_NAME = L"pnum",
			syntax::STRUCT_RULE_NAME = L"struct_rule",
			syntax::CONCAT_RULE_NAME = L"concat_rule",
			syntax::OPTION_RULE_NAME = L"option_name",
			syntax::BRACKET_RULE_NAME = L"bracket_rule",
			syntax::ATOM_NAME = L"atom",
			syntax::REPETITION_RULE_NAME = L"repetition_rule",
			syntax::REPETITION_RANGE_NAME = L"repetition_range",
			syntax::DEFINITION_NAME = L"definition";

const uint32_t syntax::GROUP_SYNTAX_TREE = 2;
const dvl::pid syntax::SYNTAX_ROOT = {syntax::GROUP_SYNTAX_TREE, 0l, dvl::TYPE_LOOP},
						// defines the id for routines that don't require a unique-identifier
						syntax::ANONYMOUS_STRUCT = {syntax::GROUP_SYNTAX_TREE, 100l, dvl::TYPE_STRUCT},
						syntax::ANONYMOUS_STRING = {syntax::GROUP_SYNTAX_TREE, 101l, dvl::TYPE_STRING_MATCHER},
						syntax::ANONYMOUS_FORK = {syntax::GROUP_SYNTAX_TREE, 102l, dvl::TYPE_FORK},

						// utility-routines
						syntax::SPACES = {syntax::GROUP_SYNTAX_TREE, 200l, dvl::TYPE_CHARSET},
						syntax::NEWLINE = {syntax::GROUP_SYNTAX_TREE, 201l, dvl::TYPE_STRING_MATCHER},

						// comment
						syntax::COMMENT = {syntax::GROUP_SYNTAX_TREE, 300l, dvl::TYPE_STRUCT},
						syntax::COMMENT_INDICATOR = {syntax::GROUP_SYNTAX_TREE, 301l, dvl::TYPE_CHARSET},
						syntax::COMMENT_CONTENT = {syntax::GROUP_SYNTAX_TREE, 302l, dvl::TYPE_CHARSET},

						// string
						syntax::STRING = {syntax::GROUP_SYNTAX_TREE, 400l, dvl::TYPE_STRUCT},
						syntax::STRING_START = {syntax::GROUP_SYNTAX_TREE, 401l, dvl::TYPE_STRING_MATCHER},
						syntax::STRING_CONTENT = {syntax::GROUP_SYNTAX_TREE, 402l, dvl::TYPE_LAMBDA},
						syntax::STRING_TERMINATOR = {syntax::GROUP_SYNTAX_TREE, 403l, dvl::TYPE_STRING_MATCHER},

						// charset
						syntax::CHARSET = {syntax::GROUP_SYNTAX_TREE, 500l, dvl::TYPE_STRUCT},
						syntax::CHARSET_INDICATOR = {syntax::GROUP_SYNTAX_TREE, 501l, dvl::TYPE_STRING_MATCHER},
						syntax::CHARSET_CONTENT = {syntax::GROUP_SYNTAX_TREE, 502l, dvl::TYPE_LAMBDA},
						syntax::CHARSET_TERMINATOR = {syntax::GROUP_SYNTAX_TREE, 503l, dvl::TYPE_STRING_MATCHER},

						// name
						syntax::NAME = {syntax::GROUP_SYNTAX_TREE, 600l, dvl::TYPE_CHARSET},

						// atom
						syntax::ATOM = {syntax::GROUP_SYNTAX_TREE, 700l, dvl::TYPE_FORK},

						// rule
						syntax::RULE = {syntax::GROUP_SYNTAX_TREE, 800l, dvl::TYPE_STRUCT},

						// concat rule
						syntax::CONCAT_RULE = {syntax::GROUP_SYNTAX_TREE, 900l, dvl::TYPE_STRUCT},
						syntax::CONCAT_A = {syntax::GROUP_SYNTAX_TREE, 901l, dvl::TYPE_STRUCT},
						syntax::CONCAT_B = {syntax::GROUP_SYNTAX_TREE, 902l, dvl::TYPE_STRUCT},

						// option_rule
						syntax::OPTION_RULE = {syntax::GROUP_SYNTAX_TREE, 1000l, dvl::TYPE_STRUCT},
						syntax::OPTION_A = {syntax::GROUP_SYNTAX_TREE, 1001l, dvl::TYPE_STRUCT},
						syntax::OPTION_B = {syntax::GROUP_SYNTAX_TREE, 1002l, dvl::TYPE_STRUCT},

						// bracket_rulee
						syntax::BRACKET_RULE = {syntax::GROUP_SYNTAX_TREE, 1100l, dvl::TYPE_STRUCT},
						syntax::BRACKET_CONTENT = {syntax::GROUP_SYNTAX_TREE, 1101l, dvl::TYPE_STRUCT},

						// struct_rule
						syntax::STRUCT_RULE = {syntax::GROUP_SYNTAX_TREE, 1200l, dvl::TYPE_FORK},

						// pnum
						syntax::PNUM = {syntax::GROUP_SYNTAX_TREE, 1300l, dvl::TYPE_CHARSET},

						// repetition_rule
						syntax::REPETITION_RULE = {syntax::GROUP_SYNTAX_TREE, 1400l, dvl::TYPE_STRUCT},
						syntax::REPETITION_RANGE = {syntax::GROUP_SYNTAX_TREE, 1401l, dvl::TYPE_STRUCT},
						syntax::REPETITION_TYPE_SINGLE = {syntax::GROUP_SYNTAX_TREE, 1402l, dvl::TYPE_STRUCT},
						syntax::REPETITION_TYPE_RANGE = {syntax::GROUP_SYNTAX_TREE, 1403l, dvl::TYPE_STRUCT},
						syntax::REPETITION_RANGE_LB = {syntax::GROUP_SYNTAX_TREE, 1404l, dvl::TYPE_LOOP},
						syntax::REPETITION_RANGE_UB = {syntax::GROUP_SYNTAX_TREE, 1405l, dvl::TYPE_LOOP},
						syntax::REPETITION_OPERATOR = {syntax::GROUP_SYNTAX_TREE, 1406l, dvl::TYPE_FORK},

						// definition
						syntax::DEFINITION = {syntax::GROUP_SYNTAX_TREE, 1500l, dvl::TYPE_FORK};

void
syntax::build_syntax_file_definition(dvl::parser_context &c)
{
	using namespace dvl;

	typedef routine_tree_builder::insertion_mode ins_mod;

	// TODO looping routines must allow alternative routines after completing minimum-number of loops!!!
	// TODO allow decoupling of declaration and initialization of routines
	// TODO prevent self-referencing routines from looping arbitrarily
	// TODO tree_builder: implement followed-by relation-ship as single operation
	// TODO inheritance for pids

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
