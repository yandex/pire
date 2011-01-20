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
#include <stub/stl.h>
#include <string>

namespace PireUnit {

class TestRunner;

class TestCase {
public:
	TestCase(const Pire::ystring& name) : mName(name) {}
	virtual ~TestCase() {}
	virtual void runTest() = 0;
	const Pire::ystring& name() const { return mName; }
private:
	Pire::ystring mName;
};

class TestSuite {
public:
	TestSuite(const Pire::ystring& name = "") : mName(name), mRunner(0) {}
	void addTest(TestSuite* suite)  { mSubSuites.push_back(suite); }
	void addTest(TestCase* testCase) { mTestCases.push_back(testCase); }
	TestRunner* runner() { return mRunner; }

	void doRun(TestRunner* runner, const Pire::ystring& filter);

	const Pire::ystring& name() const { return mName; }
private:
	Pire::ystring mName;
	Pire::yvector<TestSuite*> mSubSuites;
	Pire::yvector<TestCase*> mTestCases;
	TestRunner* mRunner;
};

class TestRunner {
public:
	TestRunner();
	void addTest(TestSuite* suite) { mSuites.push_back(suite); }
	bool run(const Pire::ystring& name, bool, bool, bool);
	void runSuite(TestSuite* suite, const Pire::ystring& filter);
	void runCase(TestCase* testCase, const Pire::ystring& filter);
	void checkAssertion(bool expr, const Pire::ystring& exprStr, const Pire::ystring& file, int line);
	void setCheckpoint(const Pire::ystring& file, int line);
private:
	Pire::ystring testFullName();
	static void onSignal(int signame);
private:
	Pire::yvector<TestSuite*> mSuites;
	Pire::yvector<Pire::ystring> mRunningSuites;
	Pire::ystring mRunningTest;
	Pire::ystring mChkptFile;
	int mChkptLine;
	size_t mSuccessCount;
	size_t mFailCount;
};

namespace Impl {
	inline TestSuite* globalSuite() { return Pire::Singleton<TestSuite>(); }
}
}

#define PIREUNIT_CHECKPOINT(file, line) \
	PireUnit::Impl::globalSuite()->runner()->setCheckpoint(file, line)

#define PIREUNIT_ASSERT(x, file, line) \
	PireUnit::Impl::globalSuite()->runner()->setCheckpoint(file, line), \
	PireUnit::Impl::globalSuite()->runner()->checkAssertion(x, "(" #x ") is false", file, line)

#define PIREUNIT_ASSERT_EQUAL(expected, real, file, line) \
	PireUnit::Impl::globalSuite()->runner()->setCheckpoint(file, line), \
	PireUnit::Impl::globalSuite()->runner()->checkAssertion(expected == real, "(" #expected ") != (" #real ")", file, line)


#define SIMPLE_UNIT_TEST_SUITE(N) \
	namespace TestSuite_ ## N { \
		PireUnit::TestSuite* s_suite; \
		struct Registration { \
			Registration() \
			{ \
				s_suite = new PireUnit::TestSuite(ystring(#N)); \
				PireUnit::Impl::globalSuite()->addTest(s_suite); \
			} \
		} s_registry; \
	} \
	namespace TestSuite_ ## N

#define SIMPLE_UNIT_TEST(N) \
	class TestCase_ ## N: public PireUnit::TestCase { \
	public: \
		TestCase_ ## N(): PireUnit::TestCase(ystring(#N)) {} \
		void runTest(); \
	}; \
	struct TestRegistration_ ## N { \
		TestRegistration_ ## N() { s_suite->addTest(new TestCase_ ## N); } \
	} s_registry_ ## N; \
	void TestCase_ ## N::runTest()

#define UNIT_CHECKPOINT() PIREUNIT_CHECKPOINT(__FILE__, __LINE__)
#define UNIT_ASSERT(x) PIREUNIT_ASSERT(x, __FILE__, __LINE__)
#define UNIT_ASSERT_EQUAL(real,expected) PIREUNIT_ASSERT_EQUAL(expected,real, __FILE__, __LINE__)

#endif



