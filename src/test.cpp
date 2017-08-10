#include "test.hpp"

#include <memory>
#include <string>
#include <iomanip>

#include "parser.hpp"


// compile with -D UNIT_TEST in order to enable unit-testing.
#ifdef UNIT_TEST

///////////////////////////////////////////////////////////////////////////////
// test base-class
//

/**
 * Defines a set of basic unit-tests used to ensure that classes
 * run accordingly with their contract.
 */

class test
{
private:
	/**
	 * Name of the unit-test
	 */
	std::string name;

	/**
	 * Description of the unit-test
	 */
	std::string description;

	/**
	 * Failure-message. Will be set if any failure occurs and
	 * should be usable to reproduce the failure that occured.
	 */
	std::string failure_message;

	/**
	 * Failure flag
	 */
	bool failure = false;
public:
	/**
	 * Initializes a test to with its name and description. The test
	 * will be marked as not failed initially
	 *
	 * @see name
	 * @see description
	 */
	test(std::string name, std::string description):
		name(name), description(description)
	{}

	virtual ~test(){}

	/**
	 * Runs the test. Before running the test the error-flags will be reset
	 */
	void run()
	{
		failure = false;

		run_test();
	}

	const std::string &get_name(){ return name; }
	const std::string &get_description(){ return description; }
	const std::string &get_error_message(){ return failure_message; }
	bool failed(){ return failure; }

	void failed_with_error(std::string msg)
	{
		failure = true;
		failure_message = msg;
	}
protected:
	/**
	 * Runs the test. Note that this method should be repeatable.
	 * I.e. the second run of a test should produce the same output as the
	 * first one.
	 */
	virtual void run_test() = 0;

	template<typename A, typename B>
	void assert_equal(A a, B b, std::string msg){
		if(a != b)
		{
			failure = true;
			failure_message = msg;
		}
	}

	template<typename A, typename B>
	void assert_not_equal(A a, B b, std::string msg){
		if(a == b)
		{
			failure = true;
			failure_message = msg;
		}
	}

	void assert_true(bool b, std::string msg){
		if(!b)
		{
			failure = true;
			failure_message = msg;
		}
	}

	void assert_throws(std::function<void(void)throw(std::exception)> f, std::string msg)
	{
		try{
			f();

			failure = true;
			failure_message = msg;
		}catch(...)
		{}
	}

	void assert_no_throw(std::function<void(void)throw(std::exception)> f, std::string msg)
	{
		try{
			f();
		}catch(...)
		{
			failure = true;
			failure_message = msg;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////
// routine-interface for debugging
//

class helper_routine_interface : public dvl::routine_interface
{
private:
	int repeat_ct = 0;

	bool throw_ex = false;
	dvl::parser_exception *e = nullptr;

	dvl::routine *n = nullptr,
			*c = nullptr;

	std::wistream &str;
public:
	helper_routine_interface(std::wistream &str):
		str(str){}

	~helper_routine_interface()
	{
		if(e != nullptr)
			delete e;
	}

	// routine_interface implementation

	void repeat()
	{
		repeat_ct++;
	}

	void run_as_next(dvl::routine *r)
	{
		n = r;
	}

	void run_as_child(dvl::routine *r)
	{
		c = r;
	}

	void check_child_exception()
		throw(dvl::parser_exception)
	{
		if(e != nullptr && throw_ex)
		{
			throw_ex = false;
			throw *e;
		}
	}

	void visit(dvl::stack_trace_routine &)
	{

	}

	std::wistream&
	get_istream(){
		return str;
	}

	// helper interface

	void throw_on_next_run(dvl::parser_exception e)
	{
		this->e = e.clone();
		throw_ex = true;
	}

	int get_repeat_count(){ return repeat_ct; }
	dvl::routine *get_next(){ return n; }
	dvl::routine *get_child(){ return c; }
};


///////////////////////////////////////////////////////////////////////////////////
// parser_routine_factory
//

uint8_t TYPE_LAMBDA = 6;

dvl::pid LAMBDA = {dvl::GROUP_INTERNAL, 4l, TYPE_LAMBDA};

dvl::parser_routine_factory factory;

typedef std::function<void(dvl::routine_interface &ri) throw(dvl::parser_exception)> routine_lambda;
typedef dvl::parser_routine_factory::parser_routine proutine;

/**
 * A lambda-routine provides means to implement a routine with
 * behavior solely defined by the function-pointer/lambda
 * passed to it. This enables easier testing of routines.
 */
class lambda_routine : public dvl::routine
{
private:
	routine_lambda r;
public:
	lambda_routine(routine_lambda r):
		dvl::routine(LAMBDA),
		r(r)
	{}

	routine_lambda get_r(){ return r; }
};

/**
 * Overload factory to support lambda-routines
 */
void init_factory()
{
	class parser_lambda_routine : public proutine{
	private:
		routine_lambda r;

		dvl::lnstruct *ln;
	public:
		parser_lambda_routine(routine_lambda r):
			proutine(LAMBDA),
			r(r),
			ln(nullptr)
		{}

		void place_child(dvl::lnstruct *) throw(dvl::parser_exception){
			throw dvl::parser_exception(get_pid(), dvl::parser_exception::lnstruct_invalid_insertion("parser_lambda_routine"));
		}

		dvl::lnstruct *get_result(){ return ln; }

		void run(dvl::routine_interface &ri)
			throw(dvl::parser_exception)
		{
			ln = new dvl::lnstruct(get_pid(), ri.get_istream().tellg());

			r(ri);

			ln->set_end(ri.get_istream().tellg());
		}
	};

	factory.register_transformation(TYPE_LAMBDA, [](dvl::routine *r)->dvl::parser_routine_factory::parser_routine*{
		return new parser_lambda_routine(((lambda_routine*) r)->get_r());
	});
}

///////////////////////////////////////////////////////////////////////////////////
// basic routine test-stubs
//

class test_routine : public test
{
protected:
	helper_routine_interface ri;
public:
	test_routine(std::string name, std::string description, std::wistream &str):
		test(name, description),
		ri(str)
	{}

	virtual ~test_routine(){}
};

class test_routine_result_stable : public test_routine
{
protected:
	proutine *r;
public:
	test_routine_result_stable(dvl::routine *r, std::string rname):
		test_routine(rname + " stable result", "Tests if the routine returns the same"
				" result after multiple calls to run", std::wcin),
		r(factory.build_routine(r)){}

	~test_routine_result_stable()
	{
		delete r;
	}

	void run_test()
	{
		r->ri_run(ri);

		dvl::lnstruct *ln = r->get_result();

		r->ri_run(ri);

		assert_equal(ln, r->get_result(), "Output is altered after repeated run");
		assert_equal(r->get_result(), r->get_result(), "get_result() non-repeatable");
	}
};

class test_routine_child_placement_premature : public test_routine
{
protected:
	proutine *r;
public:
	test_routine_child_placement_premature(dvl::routine *r, std::string rname):
		test_routine(rname + " premature routine placement", "Tests if the routine doesn't"
				" allow insertion after failure", std::wcin),
	r(factory.build_routine(r)){}

	~test_routine_child_placement_premature()
	{
		delete r;
	}

	void run_test()
	{
		assert_throws([this]()->void{ r->place_child(nullptr); }, "Routine mustn't accept a child "
				"before it ran");
	}
};

class test_routine_child_placement_intime : public test_routine
{
protected:
	proutine *r;
public:
	test_routine_child_placement_intime(dvl::routine *r, std::string rname):
		test_routine(rname + " child placement in time", "Tests if the routine allows"
				" inserting a child-element after having run", std::wcin),
		r(factory.build_routine(r)){}

	~test_routine_child_placement_intime()
	{
		delete r;
	}

	void run_test()
	{
		r->ri_run(ri);

		assert_no_throw([this]()->void{r->place_child(nullptr);}, "Caught exception on child-insertion");
	}
};

///////////////////////////////////////////////////////////////////////////////////
// fork-routine
//

class test_fork_routine : public test_routine
{
protected:
	proutine *pfr;

	dvl::fork_routine *fr;
	dvl::echo_routine *er;
public:
	test_fork_routine(std::string name, std::string description, int child_count):
		test_routine(name, description, std::wcin)
	{
		er = new dvl::echo_routine(L"Hi");
		std::vector<dvl::routine*> routines;
		for(int i = 0; i < child_count; i++)
			routines.emplace_back(er);
		fr = new dvl::fork_routine({0l, 0l, dvl::TYPE_FORK}, routines);
		pfr = factory.build_routine(fr);
	}

	virtual ~test_fork_routine()
	{
		delete pfr;
		delete er;
		delete fr;
	}
};

class test_fork_routine_multiple_matches : public test_fork_routine
{
public:
	test_fork_routine_multiple_matches():
		test_fork_routine("Fork routine multiple matches", "Tests if a fork-routine doesn't"
				" allow multiple insertions on child-routines", 4)
	{}

	void run_test()
	{
		dvl::lnstruct ln(pfr->get_pid(), 0l);

		helper_routine_interface h(std::wcin);
		pfr->ri_run(h);
		pfr->place_child(&ln);
		assert_throws([this, &ln]()->void{ pfr->place_child(&ln);}, "Fork-routine shouldn't "
				" allow multiple matching definitions");
	}
};

class test_fork_routine_no_match : public test_fork_routine
{
public:
	test_fork_routine_no_match():
		test_fork_routine("Fork routine no match", "Tests if a fork-routine fails if no"
				" matches were found", 4)
	{}

	void run_test()
	{
		assert_throws([this]()->void{
			for(int i = 0; i < 5; i++)
				pfr->ri_run(ri);
		}, "Fork routine terminates successfully, failure expected");
		assert_equal(ri.get_repeat_count(), 4, "Incorrect number of repetitions in fork-routine");
	}
};

class test_fork_routine_normal : public test_fork_routine
{
public:
	test_fork_routine_normal():
		test_fork_routine("Fork routine normal functionality", "Tests if a fork routine on a"
				" successful run", 4)
	{}

	void run_test()
	{
		dvl::lnstruct *ln = new dvl::lnstruct(dvl::PARSER, 0l);

		assert_no_throw([this]()->void{ pfr->ri_run(ri); }, "Unexpected failure in ri_run");		//first run
		assert_not_equal(ri.get_child(), nullptr, "Incorrect child-placement");									//check run_as_child
		assert_equal(ri.get_next(), nullptr, "Incorrect next routine");									//check run_as_next
		ri.throw_on_next_run(dvl::parser_exception(dvl::ECHO, "failure"));								//emulate throwing exception in echo_routine
		assert_no_throw([this]()->void{ pfr->ri_run(ri); }, "Unexpected failure in ri_run");		//second run
		assert_no_throw([this, ln]()->void{ pfr->place_child(ln); }, "Failed to place child");			//emulate successful run (place_child)
		ri.throw_on_next_run(dvl::parser_exception(dvl::ECHO, "failure"));								//emulate errornous run of child-routine
		assert_no_throw([this](){ pfr->ri_run(ri); }, "Unexpected failure in ri_run");				//third run
		ri.throw_on_next_run(dvl::parser_exception(dvl::ECHO, "failure"));								//emulate errornous run of child-routine
		assert_no_throw([this](){ pfr->ri_run(ri); }, "Unexpected failure in ri_run");				//4th run
		assert_no_throw([this](){ pfr->ri_run(ri); }, "Unexpected failure in ri_run");				//5th run
		assert_throws([this](){ pfr->ri_run(ri); }, "Invalid repeat - should fail");				//6th run (no repeat)
		assert_equal((unsigned int) ri.get_repeat_count(), 4, "Incorrect repeatcount - expected 4");	//check repeat count
	}
};

class test_fork_routine_no_forks : public test_fork_routine
{
public:
	test_fork_routine_no_forks():
		test_fork_routine("Fork routine no forks", "Tests if a fork routine on"
				" without forks terminates with an error", 0)
	{}

	void run_test()
	{
		assert_throws([this](){ pfr->ri_run(ri); }, "Expected no-match exception");
	}
};

///////////////////////////////////////////////////////////////////////////////////
// parser_empty_routine
//

class test_empty_routine_single_run : public test
{
private:
	proutine *er = nullptr;
public:
	test_empty_routine_single_run():
		test("test_empty_routine_single_run", "Tests if a empty_routine behaves"
				" correctly if run once")
	{}

	~test_empty_routine_single_run()
	{
		if(er != nullptr)
			delete er;
	}

	void run_test()
	{

	}
};

///////////////////////////////////////////////////////////////////////////////////
// test-driver
//

void test_all()
{
	dvl::routine *echo = new dvl::echo_routine(L"Hi");
	dvl::routine *fork = new dvl::fork_routine({0l, 0l, dvl::TYPE_FORK}, {echo, echo});

	std::vector<test*> tests = {
			// fork_routine
			new test_routine_result_stable(fork, "fork_routine"),
			new test_routine_child_placement_premature(fork, "fork_routine"),
			new test_routine_child_placement_intime(fork, "fork_routine"),
			new test_fork_routine_multiple_matches,
			new test_fork_routine_no_match,
			new test_fork_routine_no_forks,
			new test_fork_routine_normal
	};

	// run tests
	std::for_each(tests.begin(), tests.end(), [](test *t)->void
	{
		std::cout << "Testing " << std::setw(40) << t->get_name();

		try{
			t->run();
		}
		catch(dvl::parser_exception &e){	t->failed_with_error(e.what()); }
		catch(std::exception &e){ t->failed_with_error(e.what()); }
		catch(...){	t->failed_with_error("Unknown error"); }

		if(t->failed())
			std::cout << " Failed\nDescription: " << t->get_description() << "\nCause: "
						<< t->get_error_message() << "\n" << std::endl;
		else
			std::cout << " Success" << std::endl;
	});

	// clean up
	delete echo;
	delete fork;
	std::for_each(tests.begin(), tests.end(), [](test *t)->void{ delete t; });
}

#else //ifdef UNIT_TEST

void test_all()
{
	std::wcout << L"Unit-tests are not implemented in this program" << std::endl
				<< L"Recompile with UNIT_TEST defined in the preprocessor" << std::endl
				<< L"To enable unit-testing" << std::endl;
}

#endif
