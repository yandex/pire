/*
 * cppunit.cpp --
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


#include "cppunit.h"
#include <stub/stl.h>

namespace PireUnit {

void TestSuite::doRun(TestRunner* runner, const Pire::ystring& filter)
{
	mRunner = runner;

	for (Pire::yvector<TestSuite*>::iterator sit = mSubSuites.begin(); sit != mSubSuites.end(); ++sit)
		mRunner->runSuite(*sit, filter);
	
	for (Pire::yvector<TestCase*>::iterator sit = mTestCases.begin(); sit != mTestCases.end(); ++sit)
		mRunner->runCase(*sit, filter);

	mRunner = 0;
}

bool TestRunner::run(const Pire::ystring& filter, bool, bool, bool)
{
	for (Pire::yvector<TestSuite*>::iterator sit = mSuites.begin(); sit != mSuites.end(); ++sit)
		runSuite(*sit, filter);

	if (!filter.empty() && mSuccessCount + mFailCount == 0) {
		std::cout << "No test named <" << filter << "> found" << std::endl;
		return false;
	}

	std::cout << std::endl;
	if (mFailCount == 0)
		std::cout << "OK(" << mSuccessCount << " tests)" << std::endl;
	else
		std::cout << "FAILED " << mFailCount << " tests, PASSED " << mSuccessCount << std::endl;

	return mFailCount == 0;
}

void TestRunner::runSuite(TestSuite* suite, const Pire::ystring& filter)
{	
	// if the name doesn't match the filter than pass the filter to subtests
	// otherwise run them all (pass empty filter)
	Pire::ystring subTestsFilter;
	if (!filter.empty() && filter != suite->name())
		subTestsFilter = filter;

	mRunningSuites.push_back(suite->name());
	suite->doRun(this, subTestsFilter);
	mRunningSuites.resize(mRunningSuites.size() - 1);
}

struct AssertionFailed {
	explicit AssertionFailed(const Pire::ystring& e) : mExpr(e) {}
	Pire::ystring mExpr;
};

Pire::ystring TestRunner::testFullName()
{
	Pire::ystring name;
	for (Pire::yvector<Pire::ystring>::iterator sit = mRunningSuites.begin(); sit != mRunningSuites.end(); ++sit)
		name = name + *sit + "::";
	name = name + mRunningTest;
	return name;
}

void TestRunner::runCase(TestCase* testCase, const Pire::ystring& filter)
{	
	// Skip the test if the name doesn't match the filter
	if (!filter.empty() && filter != testCase->name())
		return;

	try {
		mRunningTest = testCase->name();
		testCase->runTest();
		mSuccessCount++;
		std::cout << ".";
	} catch (AssertionFailed& e) {
		std::cerr << testFullName() << " - Assertion failed: " << e.mExpr << std::endl;
		mFailCount++;
	} catch (std::exception& e) {
		std::cerr << testFullName() << " - ecxeption caught: " << e.what() << std::endl;
		mFailCount++;
	} catch (...) {
		std::cerr << testFullName() << " - unknown ecxeption caught: " << std::endl;
		mFailCount++;
	}	
	mRunningTest = "";
}

void TestRunner::checkAssertion(bool expr, const Pire::ystring& exprStr)
{
	if (!expr) {
		throw  AssertionFailed(exprStr);
	}
}

}


int main(int argc, char **argv)
{
	PireUnit::TestRunner runner;
	runner.addTest(PireUnit::Impl::globalSuite());
	return runner.run(Pire::ystring((argc >= 2) ? argv[1] : ""), false, true, true) ? 0 : 1;
}
