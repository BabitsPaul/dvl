#include "dvl_syntax.hpp"

#include <boost/regex.hpp>

void
dvl::build_syntax_file_defintion(parser_context &c)
{
	typedef routine_tree_builder::insertion_mode ins_mod;

	static const std::wstring ROOT_NAME = L"root",
								COMMENT_NAME = L"comment",
								RULE_NAME = L"rule",
								STRING_NAME = L"string",
								SPACES_NAME = L"spaces",
								NEWLINE_NAME = L"newline",
								CHARSET_NAME = L"charset";

	// TODO looping routines must allow alternative routines after completing minimum-number of loops!!!
	// TODO prevent copying of the routine_tree_builder

	static const uint32_t GROUP_SYNTAX_TREE = 2;
	static const pid SYNTAX_ROOT = {GROUP_SYNTAX_TREE, 0l, TYPE_LOOP},
						// defines the id for routines that don't require a unique-identifier
						ANONYMOUS = {GROUP_SYNTAX_TREE, 1l, TYPE_STRUCT},

						// utility-routines
						SPACES = {GROUP_SYNTAX_TREE, 2l, TYPE_CHARSET},
						NEWLINE = {GROUP_SYNTAX_TREE, 3l, TYPE_CHARSET},

						// comment
						COMMENT = {GROUP_SYNTAX_TREE, 4l, TYPE_STRUCT},
						COMMENT_INDICATOR = {GROUP_SYNTAX_TREE, 5l, TYPE_CHARSET},
						COMMENT_CONTENT = {GROUP_SYNTAX_TREE, 6l, TYPE_CHARSET},

						// string
						STRING = {GROUP_SYNTAX_TREE, 7l, TYPE_STRUCT},
						STRING_START = {GROUP_SYNTAX_TREE, 8l, TYPE_STRING_MATCHER},
						STRING_CONTENT = {GROUP_SYNTAX_TREE, 9l, TYPE_LAMBDA},
						STRING_TERMINATOR = {GROUP_SYNTAX_TREE, 10l, TYPE_STRING_MATCHER},

						// charset
						CHARSET = {GROUP_SYNTAX_TREE, 11l, TYPE_STRUCT},
						CHARSET_INDICATOR = {GROUP_SYNTAX_TREE, 12l, TYPE_STRING_MATCHER},
						CHARSET_CONTENT = {GROUP_SYNTAX_TREE, 13l, TYPE_LAMBDA},
						CHARSET_TERMINATOR = {GROUP_SYNTAX_TREE, 14l, TYPE_LAMBDA};

	// builds the syntax-rules of the syntax-files used by this parser
	routine_tree_builder &b = c.builder;

	// spaces
	b.detach().match_set(SPACES, L"[\t ]*").name(SPACES_NAME).finalize(SPACES_NAME);

	// newline
	b.detach().match_string(NEWLINE, L"\n").name(NEWLINE_NAME).finalize(NEWLINE_NAME);

	// comment
	// # followed by arbitrary number of characters until next new-line
	b.detach().logic(COMMENT).name(COMMENT_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.by_name(SPACES_NAME).pop_checkpoint()
			.set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.match_set(COMMENT_INDICATOR, L"[#]").pop_checkpoint()
			.set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.match_set(COMMENT_CONTENT, L"![\n]*").pop_checkpoint()
			.set_insertion_mode(ins_mod::AS_NEXT)
			.match_set(NEWLINE, L"[\n]")	// TODO
			.finalize(COMMENT_NAME);

	// string
	// #(<str>)
	b.detach().logic(STRING).name(STRING_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.match_string(STRING_START, L"#(").pop_checkpoint()
			.set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
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
			}).pop_checkpoint()
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
			.match_string(STRING_TERMINATOR, L")")
			.pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
			.by_name(SPACES_NAME)
			.pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.by_name(NEWLINE_NAME)
			.finalize(STRING_NAME);

	// charset
	// $(<set>)
	b.detach().logic(CHARSET).name(CHARSET_NAME).set_insertion_mode(ins_mod::AS_CHILD)
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
			.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.match_string(CHARSET_INDICATOR, L"$(").pop_checkpoint()
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
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
			}).pop_checkpoint()
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
			.match_string(CHARSET_TERMINATOR, L")").pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.logic(ANONYMOUS).push_checkpoint().set_insertion_mode(ins_mod::AS_CHILD)
			.by_name(SPACES_NAME).pop_checkpoint().set_insertion_mode(ins_mod::AS_NEXT)
			.by_name(NEWLINE_NAME)
			.finalize(CHARSET_NAME);

	// rule
	/*

						RULE = {GROUP_SYNTAX_TREE, 10l, TYPE_STRUCT},
						NAME = {GROUP_SYNTAX_TREE, 11l, TYPE_CHARSET};

	 b.detach().logic(RULE).name(RULE_NAME).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.match_set(SPACES, L"[ \t]*").pop_checkpoint()
			.set_insertion_mode(ins_mod::AS_NEXT)
			.logic(RULE).push_checkpoint()
			.set_insertion_mode(ins_mod::AS_CHILD)
			.match_set(NAME, L"[A-Za-z0-9]*").pop_checkpoint();*/

	// root
	b.loop(SYNTAX_ROOT, 0, loop_routine::_INFINITY).name(ROOT_NAME).mark_root();
}
