#ifndef PIRE_STUB_CPPUNIT_H_INCLUDED
#define PIRE_STUB_CPPUNIT_H_INCLUDED

#include <stub/singleton.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <string>

namespace Impl {
	inline CppUnit::TestSuite* globalSuite() { return Pire::Singleton<CppUnit::TestSuite>(); }
}

#define SIMPLE_UNIT_TEST_SUITE(N) \
	namespace TestSuite_ ## N { \
		CppUnit::TestSuite* s_suite; \
		struct Registration { \
			Registration() \
			{ \
				s_suite = new CppUnit::TestSuite(std::string(#N)); \
				::Impl::globalSuite()->addTest(s_suite); \
			} \
		} s_registry; \
	} \
	namespace TestSuite_ ## N
	
#define SIMPLE_UNIT_TEST(N) \
	class TestCase_ ## N: public CppUnit::TestCase { \
	public: \
		TestCase_ ## N(): CppUnit::TestCase(std::string(#N)) {} \
		void runTest(); \
	}; \
	struct TestRegistration_ ## N { \
		TestRegistration_ ## N() { s_suite->addTest(new TestCase_ ## N); } \
	} s_registry_ ## N; \
	void TestCase_ ## N::runTest()

#define UNIT_ASSERT CPPUNIT_ASSERT
#define UNIT_ASSERT_EQUAL CPPUNIT_ASSERT_EQUAL

#endif



