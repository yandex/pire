/*
 * cppunit.h --
 *
 * Copyright (c) 2007-2010, Dmitry Prokoptsev <dprokoptsev@gmail.com>,
 *                          Alexander Gololobov <agololobov@gmail.com>
 *
 * This file is part of Pire, the Perl Incompatible
 * Regular Expressions library.
 *
 * Pire is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Pire is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser Public License
 * along with Pire.  If not, see <http://www.gnu.org/licenses>.
 */


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
				s_suite = new CppUnit::TestSuite(ystring(#N)); \
				::Impl::globalSuite()->addTest(s_suite); \
			} \
		} s_registry; \
	} \
	namespace TestSuite_ ## N
	
#define SIMPLE_UNIT_TEST(N) \
	class TestCase_ ## N: public CppUnit::TestCase { \
	public: \
		TestCase_ ## N(): CppUnit::TestCase(ystring(#N)) {} \
		void runTest(); \
	}; \
	struct TestRegistration_ ## N { \
		TestRegistration_ ## N() { s_suite->addTest(new TestCase_ ## N); } \
	} s_registry_ ## N; \
	void TestCase_ ## N::runTest()

#define UNIT_ASSERT(x) CPPUNIT_ASSERT(x)
#define UNIT_ASSERT_EQUAL(real,expected) CPPUNIT_ASSERT_EQUAL(expected,real)

#endif



