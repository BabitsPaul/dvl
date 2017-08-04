#include "test.hpp"

#include <string>

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
protected:
	/**
	 * Runs the test. Note that this method should be repeatable.
	 * I.e. the second run of a test should produce the same output as the
	 * first one.
	 */
	virtual void run_test() = 0;

	template<typename T>
	void assert_equal(T a, T b, std::string msg){
		if(a != b)
		{
			failure = true;
			failure_message = msg;
		}
	}

	template<typename T>
	bool assert_not_equal(T a, T b, std::string msg){
		if(a == b)
		{
			failure = true;
			failure_message = msg;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////
// parser_routine_factory
//

void allocate_resources()
{

}

void deallocate_resources()
{

}

void test_all()
{

}
