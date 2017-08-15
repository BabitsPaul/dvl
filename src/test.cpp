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

class assertion_failure : public std::exception
{
private:
	std::string msg;
public:
	assertion_failure():
		msg("assertion failure")
	{}

	const char *what(){ return msg.c_str(); }
};

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
		failure_message = std::string("Exception caught: ") + msg;
	}
protected:
	/**
	 * Runs the test. Note that this method should be repeatable.
	 * I.e. the second run of a test should produce the same output as the
	 * first one.
	 */
	virtual void run_test() = 0;

	template<typename A, typename B>
	void assert_equal(A a, B b, std::string msg)
		throw(assertion_failure)
	{
		if(a != b)
		{
			failure = true;
			failure_message = msg;

			throw assertion_failure();
		}
	}

	template<typename A, typename B>
	void assert_not_equal(A a, B b, std::string msg)
		throw(assertion_failure)
	{
		if(a == b)
		{
			failure = true;
			failure_message = msg;

			throw assertion_failure();
		}
	}

	void assert_true(bool b, std::string msg)
		throw(assertion_failure)
	{
		if(!b)
		{
			failure = true;
			failure_message = msg;

			throw assertion_failure();
		}
	}

	void assert_throws(std::function<void(void)throw(std::exception)> f, std::string msg)
		throw(assertion_failure)
	{
		try{
			f();

			failure = true;
			failure_message = msg;
		}catch(...)
		{}

		if(failure)
			throw assertion_failure();
	}

	void assert_no_throw(std::function<void(void)throw(std::exception)> f, std::string msg)
	{
		try{
			f();
		}catch(...)
		{
			failure = true;
			failure_message = msg;

			throw assertion_failure();
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

	void reset_routines()
	{
		n = c = nullptr;
	}

	void reset_repeat_ct()
	{
		repeat_ct = 0;
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

	dvl::lnstruct *ln;
public:
	test_routine_child_placement_premature(dvl::routine *r, std::string rname):
		test_routine(rname + " premature routine placement", "Tests if the routine doesn't"
				" allow insertion after failure", std::wcin),
		r(factory.build_routine(r))
	{
		ln = new dvl::lnstruct(dvl::EMPTY, 0l);
	}

	~test_routine_child_placement_premature()
	{
		delete r;
		delete ln;
	}

	void run_test()
	{
		assert_throws([this]()->void{ r->ri_place_child(ln); }, "Routine mustn't accept a child "
				"before it ran");
	}
};

class test_routine_child_placement_intime : public test_routine
{
protected:
	proutine *r;

	dvl::lnstruct *ln;
public:
	test_routine_child_placement_intime(dvl::routine *r, std::string rname):
		test_routine(rname + " child placement in time", "Tests if the routine allows"
				" inserting a child-element after having run", std::wcin),
		r(factory.build_routine(r))
	{
		ln = new dvl::lnstruct(dvl::EMPTY, 0l);
	}

	~test_routine_child_placement_intime()
	{
		delete r;
		delete ln;
	}

	void run_test()
	{
		r->ri_run(ri);

		assert_no_throw([this]()->void{r->ri_place_child(ln);}, "Caught exception on child-insertion");
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
		pfr->ri_place_child(&ln);
		assert_throws([this, &ln]()->void{ pfr->ri_place_child(&ln);}, "Fork-routine shouldn't "
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
		assert_no_throw([this, ln]()->void{ pfr->ri_place_child(ln); }, "Failed to place child");			//emulate successful run (place_child)
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

class test_empty_routine : public test_routine
{
protected:
	proutine *per = nullptr;

	dvl::empty_routine *er;
public:
	test_empty_routine(std::string name, std::string description):
		test_routine(name, description, std::wcin)
	{
		er = new dvl::empty_routine;
		per = factory.build_routine(er);
	}

	virtual ~test_empty_routine()
	{
		delete er;
		delete per;
	}
};

class test_empty_routine_single_run : public test_empty_routine
{
public:
	test_empty_routine_single_run():
		test_empty_routine("test_empty_routine_single_run", "Tests if a empty_routine behaves"
				" correctly if run once")
	{}

	void run_test()
	{
		assert_no_throw([this]()->void{ per->ri_run(ri); }, "Unexpected failure on first run");
		assert_equal(nullptr, ri.get_child(), "Echo routine mustnt place child-routine");
		assert_equal(nullptr, ri.get_next(), "Echo routine mustnt place next-routine");
		assert_equal(0, ri.get_repeat_count(), "Echo-routines mustnt be repeatable");
	}
};

class test_empty_routine_multiple_run : public test_empty_routine
{
public:
	test_empty_routine_multiple_run():
		test_empty_routine("Test empty_routine multiple run", "Tests if an empty-routine terminates "
				"with an exception upon running a second time")
	{}

	void run_test()
	{
		assert_no_throw([this]()->void{ per->ri_run(ri); }, "Unexpected failure on first run");
		assert_throws([this]()->void{ per->ri_run(ri); }, "Echo routine shouldn't allow second run");
	}
};

class test_empty_routine_child_placement : public test_empty_routine
{
public:
	test_empty_routine_child_placement():
		test_empty_routine("Test empty routine child placement", "Test if an empty-routine forbids "
				"child-placement")
	{}

	void run_test()
	{
		assert_throws([this]()->void{ per->ri_place_child(nullptr); }, "Empty-routine mustnt allow child-placement");
	}
};

///////////////////////////////////////////////////////////////////////////////////
// parser loop routine
//

class test_loop_routine : public test_routine
{
protected:
	proutine *plr;

	dvl::loop_routine *lr;
	dvl::empty_routine *er;

	void rebuild_plr()
	{
		if(plr != nullptr)
			delete plr;

		plr = factory.build_routine(lr);
	}
public:
	test_loop_routine(std::string name, std::string description):
		test_routine(name, description, std::wcin),
		plr(nullptr)
	{
		er = new dvl::empty_routine;
		lr = new dvl::loop_routine({0l, 0l, dvl::TYPE_LOOP}, er);
		rebuild_plr();
	}

	virtual ~test_loop_routine()
	{
		delete er;
		delete lr;
		delete plr;
	}
};

class test_loop_routine_failure_min_iter : public test_loop_routine
{
private:
	dvl::lnstruct *ln;
public:
	test_loop_routine_failure_min_iter():
		test_loop_routine("Test loop routine success", "Tests if a loop-routine behaves correctly "
				"if invoked correctly")
	{
		lr->set_min_iterations(3);
		rebuild_plr();

		ln = new dvl::lnstruct(dvl::EMPTY, 0l);
	}

	~test_loop_routine_failure_min_iter()
	{
		delete ln;
	}

	void run_test()
	{
		assert_no_throw([this](){
			plr->ri_run(ri);
			plr->ri_place_child(ln);
			plr->ri_run(ri);
		}, "Unexpected failure on valid run");

		ri.throw_on_next_run(dvl::parser_exception(dvl::EMPTY, "whatever"));

		assert_throws([this](){ plr->ri_run(ri); }, "Loop routine should terminate premature");
	}
};

class test_loop_routine_success_min_iter : public test_loop_routine
{
private:
	dvl::lnstruct *ln;
public:
	test_loop_routine_success_min_iter():
		test_loop_routine("test loop routine success min iter", "Tests if a loop-routine behaves "
				"properly if terminated after minimum no of runs")
	{
		ln = new dvl::lnstruct(dvl::EMPTY, 0l);

		lr->set_min_iterations(2);
		rebuild_plr();
	}

	~test_loop_routine_success_min_iter()
	{
		delete ln;
	}

	void run_test()
	{
		assert_no_throw([this](){ plr->ri_run(ri); }, "Unexpected failure on first run");
		assert_equal(nullptr, ri.get_next(), "Shouldn't place next routine");
		assert_not_equal(nullptr, ri.get_child(), "Should place child-routine");
		assert_no_throw([this](){ plr->ri_place_child(ln); }, "Unexpected failure on placement of first element");
		assert_no_throw([this](){ plr->ri_run(ri); }, "Unexpected failure on second run");
		assert_no_throw([this](){ plr->ri_place_child(ln); }, "Unexpected failure on placement of second element");

		ri.throw_on_next_run(dvl::parser_exception(dvl::EMPTY, "whatever"));

		assert_no_throw([this](){ plr->ri_run(ri); }, "Routine should terminate successful despite failure of child-routine");
		assert_throws([this](){ plr->ri_run(ri); }, "Should fail after terminating");

		// check output
		dvl::lnstruct *ln = plr->get_result();
		assert_not_equal(ln, nullptr, "routine should produce output");
		assert_not_equal(ln->get_child(), nullptr, "Missing child-elements of routine");
		assert_equal(ln->get_next(), nullptr, "Routine shouldn't output following element");
		assert_equal(ln->get_child()->level_count(), 2, "Invalid repetition of loop-elements");
		assert_not_equal(ln->get_child()->get_child(), nullptr, "Loop-helper should hold one child-element");
	}
};

// TODO test-granularity: test state-details in each test

class test_loop_routine_success_intermediate_iter : public test_loop_routine
{
public:
	test_loop_routine_success_intermediate_iter():
		test_loop_routine("test loop routine success intermediate iter", "Tests if the routine behaves "
				"correctly if the element terminates in the middle of the entity")
	{
		lr->set_min_iterations(3);
		lr->set_max_iterations(6);
		rebuild_plr();
	}

	void run_test()
	{
		assert_no_throw([this](){
			for(int i = 0; i < 4; i++)
				plr->ri_run(ri);
		}, "Unexpected failure in loop-routine");

		ri.throw_on_next_run(dvl::parser_exception(dvl::EMPTY, "whatever"));

		assert_no_throw([this](){ plr->ri_run(ri); }, "Should terminate entity successful");
		assert_not_equal(plr->get_result(), nullptr, "Routine should produce valid output");
	}
};

class test_loop_routine_success_last_iter : public test_loop_routine
{
public:
	test_loop_routine_success_last_iter():
		test_loop_routine("test loop routine success last iter", "Tests if the loop-routine behaves "
				"correctly, if the entity terminates on the last run (max elements)")
	{
		lr->set_max_iterations(3);
		lr->set_min_iterations(0);
		rebuild_plr();
	}

	void run_test()
	{
		assert_no_throw([this](){
			for(int i = 0; i < 3; i++)
				plr->ri_run(ri);
		}, "Unexpected failure, 4 repetitions allowed on 3-entity routine");

		assert_throws([this](){ plr->ri_run(ri); }, "Shouldn't allow third repetition");
	}
};

///////////////////////////////////////////////////////////////////////////////////
// logic-routine
//

class test_struct_routine : public test_routine
{
protected:
	proutine *plr;

	dvl::routine *er;
	dvl::struct_routine *lr;
public:
	test_struct_routine(std::string name, std::string description):
		test_routine(name, description, std::wcin)
	{
		er = new dvl::empty_routine;
		lr = new dvl::struct_routine({0l, 0l, dvl::TYPE_STRUCT}, er, er);
		plr = factory.build_routine(lr);
	}

	virtual ~test_struct_routine()
	{
		delete er;
		delete lr;
		delete plr;
	}
};

class test_struct_routine_normal_run : public test_struct_routine
{
public:
	test_struct_routine_normal_run():
		test_struct_routine("test struct routine normal run", "Tests if the struct-routine"
				" behaves correctly on a successful run")
	{}

	void run_test()
	{
		assert_no_throw([this](){ plr->ri_run(ri); }, "struct routine should allow single run");
		assert_equal(ri.get_next(), er, "Invalid next routine");
		assert_equal(ri.get_child(), er, "Invalid child routine");
		assert_equal(ri.get_repeat_count(), 0, "Invalid repeat count");
	}
};

///////////////////////////////////////////////////////////////////////////////////
// parser matcher routine
//

///////////////////////////////////////////////////////////////////////////////////
// test-driver
//

void test_all()
{
	dvl::routine *echo = new dvl::echo_routine(L"Hi");
	dvl::routine *fork = new dvl::fork_routine({0l, 0l, dvl::TYPE_FORK}, {echo, echo});
	dvl::routine *loop = new dvl::loop_routine({0l, 0l, dvl::TYPE_LOOP}, echo);
	dvl::routine *structr = new dvl::struct_routine({0l, 0l, dvl::TYPE_STRUCT}, echo, echo);

	std::vector<test*> tests = {
			// fork_routine
			new test_routine_result_stable(fork, "fork_routine"),
			new test_routine_child_placement_premature(fork, "fork_routine"),
			new test_routine_child_placement_intime(fork, "fork_routine"),
			new test_fork_routine_multiple_matches,
			new test_fork_routine_no_match,
			new test_fork_routine_no_forks,
			new test_fork_routine_normal,

			// empty routine
			new test_empty_routine_single_run,
			new test_empty_routine_multiple_run,
			new test_empty_routine_child_placement,

			// loop routine
			new test_routine_result_stable(loop, "loop_routine"),
			new test_routine_child_placement_premature(loop, "loop_routine"),
			new test_routine_child_placement_intime(loop, "loop_routine"),
			new test_loop_routine_failure_min_iter,
			new test_loop_routine_success_min_iter,
			new test_loop_routine_success_intermediate_iter,
			new test_loop_routine_success_last_iter,

			// logic routine
			new test_routine_child_placement_premature(structr, "struct_routine"),
			new test_routine_child_placement_intime(structr, "struct_routine"),
			new test_struct_routine_normal_run
	};

	// run tests
	int failure_ct = 0;

	std::for_each(tests.begin(), tests.end(), [&failure_ct](test *t)->void
	{
		std::cout << "Testing " << std::setw(50) << t->get_name();

		try{
			t->run();
		}
		catch(assertion_failure &af){ std::cout << "\nassertion failure" << std::endl; }
		catch(dvl::parser_exception &e){	t->failed_with_error(e.what()); }
		catch(std::exception &e){ t->failed_with_error(e.what()); }
		catch(...){	t->failed_with_error("Unknown error"); }

		if(t->failed())
		{
			std::cout << " Failed\nDescription: " << t->get_description() << "\nCause: "
						<< t->get_error_message() << "\n" << std::endl;

			failure_ct++;
		}
		else
			std::cout << " Success" << std::endl;
	});

	std::cout << "Total failures: " << failure_ct << std::endl;

	// clean up
	delete echo;
	delete fork;
	delete loop;
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
