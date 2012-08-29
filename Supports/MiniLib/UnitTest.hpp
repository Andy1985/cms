// UnitTest.hpp
#ifndef __UNIT_TEST_HPP__
#define __UNIT_TEST_HPP__
///////////////////////////////////////////////////////////////////////////
// Usage example:
/*

int Foo()
{
	throw 1;
}

class UnitTestSomething
{
public:
	UNIT_TEST_SUITE(UnitTestSomething);
	UNIT_TEST(Test1);
	UNIT_TEST(Test2);
	UNIT_TEST(Test3);
	UNIT_TEST_SUITE_END();
    
private:
	void Test1()
	{
		UNIT_TEST_ASSERT( 1 == 2 );
	}

	void Test2()
	{
		UNIT_TEST_ASSERT( Foo() );
	}

	void Test3()
	{
		UNIT_TEST_ASSERT_EQUAL( 2, 1 + 1 );
	}
};

UNIT_TEST_REGISTER(UnitTestSomething);

int main()
{
	return UnitTest::Run();
}

*/
///////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4702) // unreachable code
#pragma warning(disable: 4514) // unreferenced inline function has been removed
#pragma warning(disable: 4710) // function not inlined

#if (_MSC_VER <= 1200)
#pragma inline_depth(0)
#endif

#endif

#include <iostream>
#include <sstream>
///////////////////////////////////////////////////////////////////////////

template <typename T>
class UnitTestRunnerBase
{
public:
	UnitTestRunnerBase()
		: next_(NULL)
	{
		if (root_ == NULL)
		{
			root_ = this;
		}
		else
		{
			UnitTestRunnerBase* p = root_;
			while (p->next_) p = p->next_;
			p->next_ = this;
		}
	}
	virtual ~UnitTestRunnerBase() { }

protected:
	virtual void Run()
	{
		if (next_) next_->Run();
	}

public:
	static int RunAll()
	{
		if (root_)
		{
			root_->Run();
		}

		if ( ! CheckAllOK())
		{
			std::cout
				<< "Test Result - "
				<< "run: " << successCount_ + failureCount_ + errorCount_ << ", "
				<< "success: " << successCount_ << ", "
				<< "failure: " << failureCount_ << ", "
				<< "error: " << errorCount_ << "." << std::endl;
			return 1;
		}
		else
		{
			std::cout
				<< "Test run OK (" << successCount_ << ")." << std::endl;
			return 0;
		}
	}

private:
	UnitTestRunnerBase* next_;
private:
	static UnitTestRunnerBase* root_;

public:
	static void AddSuccess() { ++successCount_; }
	static void AddFailure() { ++failureCount_; }
	static void AddError()   { ++errorCount_; }

	static bool CheckAllOK() { return (failureCount_ == 0) && (errorCount_ == 0); }

private:
	static int successCount_;
	static int failureCount_;
	static int errorCount_;
};

template <typename T> UnitTestRunnerBase<T>* UnitTestRunnerBase<T>::root_;

template <typename T> int UnitTestRunnerBase<T>::successCount_ = 0;
template <typename T> int UnitTestRunnerBase<T>::failureCount_ = 0;
template <typename T> int UnitTestRunnerBase<T>::errorCount_   = 0;

///////////////////////////////////////////////////////////////////////////

template <class T>
class UnitTestRunner : public UnitTestRunnerBase<void>
{
public:
	virtual void Run()
	{
		if (test.Run())
		{
			UnitTestRunnerBase<void>::Run();
		}
	}

private:
	T test;
};

///////////////////////////////////////////////////////////////////////////

class UnitTest
{
public:
	static int Run()
	{
		return UnitTestRunnerBase<void>::RunAll();
	}
};

///////////////////////////////////////////////////////////////////////////

#define UNIT_TEST_SUITE(suite) \
	private: \
	typedef char __private_type; \
	typedef char __check_name_t[ sizeof(suite::__private_type) ]; \
	public: \
	bool Run() \
	{ \
		const char* suiteName = #suite

#define UNIT_TEST(test) \
		std::cout << suiteName << "::" << #test << " : "; \
		test(); \
		if ( ! UnitTestRunnerBase<void>::CheckAllOK()) return false; \
		UnitTestRunnerBase<void>::AddSuccess(); \
		std::cout << "OK!" << std::endl

#define UNIT_TEST_SUITE_END() \
		return true; \
	} \
	private: \
	typedef __private_type __dummy

#define UNIT_TEST_REGISTER(fixture) \
	static UnitTestRunner<fixture> autoTest ## fixture

///////////////////////////////////////////////////////////////////////////

#define UNIT_TEST_MESSAGE(msg) \
	std::cout << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl;

#define UNIT_TEST_ASSERT(condition) \
	try \
	{ \
		if ( ! (condition)) \
		{ \
			std::cout << "Failed!" << std::endl; \
			UNIT_TEST_MESSAGE("Assert failed! Expression: " # condition); \
			UnitTestRunnerBase<void>::AddFailure(); \
			return; \
		} \
	} \
	catch (...) \
	{ \
		std::cout << "Error!" << std::endl; \
		UNIT_TEST_MESSAGE("Caught exception! Expression: " # condition); \
		UnitTestRunnerBase<void>::AddError(); \
		return; \
	} \
	do { break; } while(true)

template <typename T, typename U>
inline bool ExpectEqual(T expect, U actual, const char* actualExpression, std::string& message)
{
	if (expect != actual)
	{
		std::cout << "Failed!" << std::endl;
		std::stringstream ss;
		ss << "Expect{" << expect << "} != Actual{" << actual << "}, Actual expression:" << actualExpression;
		message = ss.str();
		return false;
	}
	return true;
}

#define UNIT_TEST_ASSERT_EQUAL(expect, actual) \
	try \
	{ \
		std::string message; \
		if ( ! ExpectEqual(expect, actual, #actual, message)) \
		{ \
			UNIT_TEST_MESSAGE( message ) \
			UnitTestRunnerBase<void>::AddFailure(); \
			return; \
		} \
	} \
	catch (...) \
	{ \
		std::cout << "Error!" << std::endl; \
		UNIT_TEST_MESSAGE("Caught exception! Expression: " #expect " == " #actual ); \
		UnitTestRunnerBase<void>::AddError(); \
		return; \
	} \
	do { break; } while(true)

///////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#pragma inline_depth()
#endif

///////////////////////////////////////////////////////////////////////////
#endif//__UNIT_TEST_HPP__

