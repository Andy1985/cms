// SafeIPTest.cpp

#include "../Supports/MiniLib/UnitTest.hpp"
#include "../Source/CmsMessenger/SafeIPList.h"

///////////////////////////////////////////////////////////////////////////

class SafeIPTest
{
public:
	UNIT_TEST_SUITE(SafeIPTest);

	UNIT_TEST(TestSet);
	UNIT_TEST(TestCheck);

	UNIT_TEST_SUITE_END();

private:
	void TestSet();
	void TestCheck();
};

UNIT_TEST_REGISTER(SafeIPTest);

///////////////////////////////////////////////////////////////////////////

void SafeIPTest::TestSet()
{
	SafeIP ip;
	UNIT_TEST_ASSERT_EQUAL( std::string("127.0.0.1"), ip.ToString() );

	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("1.2.3.4") );
	UNIT_TEST_ASSERT_EQUAL( std::string("1.2.3.4"), ip.ToString() );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("4.3.2.1") );
	UNIT_TEST_ASSERT_EQUAL( std::string("4.3.2.1"), ip.ToString() );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("12.34.56.78") );
	UNIT_TEST_ASSERT_EQUAL( std::string("12.34.56.78"), ip.ToString() );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("202.212.222.232") );
	UNIT_TEST_ASSERT_EQUAL( std::string("202.212.222.232"), ip.ToString() );

	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("192.168.0.*") );
	UNIT_TEST_ASSERT_EQUAL( std::string("192.168.0.*"), ip.ToString() );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("10.0.1.128-255") );
	UNIT_TEST_ASSERT_EQUAL( std::string("10.0.1.128-255"), ip.ToString() );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("202.10.2-5.128-192") );
	UNIT_TEST_ASSERT_EQUAL( std::string("202.10.2-5.128-192"), ip.ToString() );
}

void SafeIPTest::TestCheck()
{
	SafeIP ip;
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(127, 0, 0, 1)) );

	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("202.212.222.232") );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(202, 212, 222, 232)) );
	
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("192.168.0.*") );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(192, 168, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(192, 168, 0, 127)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(192, 168, 0, 255)) );
	
	UNIT_TEST_ASSERT_EQUAL( true, ip.Set("10.0.1.128-255") );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(10, 0, 1, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, ip.Check(IPAddress(10, 0, 1, 127)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(10, 0, 1, 128)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(10, 0, 1, 202)) );
	UNIT_TEST_ASSERT_EQUAL( true, ip.Check(IPAddress(10, 0, 1, 255)) );
}

///////////////////////////////////////////////////////////////////////////

