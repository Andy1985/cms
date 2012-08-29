// SafeIPListTest.cpp

#include "../Supports/MiniLib/UnitTest.hpp"
#include "../Source/CmsMessenger/SafeIPList.h"

#include <unistd.h>

///////////////////////////////////////////////////////////////////////////

class SafeIPListTest
{
public:
	~SafeIPListTest();

public:
	UNIT_TEST_SUITE(SafeIPListTest);

	UNIT_TEST(TestLoadAndCheck);

	UNIT_TEST_SUITE_END();

private:
	void TestLoadAndCheck();

private:
	static const char* const filename;
};

UNIT_TEST_REGISTER(SafeIPListTest);

///////////////////////////////////////////////////////////////////////////

const char* const SafeIPListTest::filename = "SafeIPList.tmp";

SafeIPListTest::~SafeIPListTest()
{
	unlink(filename);
}

void SafeIPListTest::TestLoadAndCheck()
{
	SafeIPList list;

	unlink(filename);
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );

	FILE* fp;
	fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, "192.168.1.1");
		fclose(fp);
	}
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );
	
	fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, "192.168.1.*");
		fclose(fp);
	}
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 128)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );
	
	fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, " 192 . 168 . 1 . * ");
		fclose(fp);
	}
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 128)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );
	
	fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, "192.168.1.31-50");
		fclose(fp);
	}
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 31)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 40)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(192, 168, 1, 50)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(192, 168, 1, 128)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );

	fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, "1.2.3.4\n");
		fprintf(fp, "4.3.2.1\n");
		fclose(fp);
	}
	UNIT_TEST_ASSERT_EQUAL( true, list.Load(filename) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(127, 0, 0, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(0, 0, 0, 0)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(1, 2, 3, 4)) );
	UNIT_TEST_ASSERT_EQUAL( true, list.Check(IPAddress(4, 3, 2, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(192, 168, 1, 1)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(202, 202, 202, 202)) );
	UNIT_TEST_ASSERT_EQUAL( false, list.Check(IPAddress(255, 255, 255, 255)) );
	
	
	unlink(filename);
}

///////////////////////////////////////////////////////////////////////////

