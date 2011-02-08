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
#include <stdlib.h>
#include <signal.h>

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
	
TestRunner::TestRunner():
	mChkptLine(0), mSuccessCount(0), mFailCount(0)
{
#ifndef _WIN32
	signal(SIGSEGV, &TestRunner::onSignal);
	signal(SIGBUS,  &TestRunner::onSignal);
	signal(SIGILL,  &TestRunner::onSignal);
	signal(SIGABRT, &TestRunner::onSignal);
#endif
}
	
void TestRunner::setCheckpoint(const Pire::ystring& file, int line)
{
	mChkptFile = file;
	mChkptLine = line;
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
	AssertionFailed(const Pire::ystring& e, const Pire::ystring& file, int line) 
		: mExpr(e)
		, mFile(file)
		, mLine(line)
	{}
	Pire::ystring mExpr;
	Pire::ystring mFile;
	int mLine;
};

Pire::ystring TestRunner::testFullName()
{
	Pire::ystring name;
	for (Pire::yvector<Pire::ystring>::iterator sit = mRunningSuites.begin(); sit != mRunningSuites.end(); ++sit)
		name = name + *sit + "::";
	name = name + mRunningTest;
	return name;
}

void TestRunner::onSignal(int signame)
{
#ifndef _WIN32
	Pire::ystring testName = "(no active test)";
	TestRunner* self = Impl::globalSuite()->runner();
	if (self)
		testName = self->testFullName();
	std::cerr << std::endl << testName << ": ";
	if (signame == SIGSEGV)
		std::cerr << "SIGSEGV";
	else if (signame == SIGBUS)
		std::cerr << "SIGBUS";
	else if (signame == SIGILL)
		std::cerr << "SIGILL";
	else if (signame == SIGABRT)
		std::cerr << "SIGABRT";
	else
		std::cerr << "signal " << signame;
	
	if (self)
		std::cerr << " (last checkpoint: " << self->mChkptFile << ":" << self->mChkptLine << "), aborting" << std::endl;
	
	_Exit(128 + signame);
#endif
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
		std::cerr << std::endl << testFullName() << " at " << e.mFile << ":" << e.mLine 
			<< ": Assertion failed: " << e.mExpr << std::endl;
		mFailCount++;
	} catch (std::exception& e) {
		std::cerr << std::endl << testFullName() << ": ecxeption caught: " << e.what()
		          << " (last checkpoint: " << mChkptFile << ":" << mChkptLine << ")" << std::endl;
		mFailCount++;
	} catch (...) {
		std::cerr << std::endl << testFullName() << ": unknown ecxeption caught: "
		          << " (last checkpoint: " << mChkptFile << ":" << mChkptLine << ")" << std::endl;
		mFailCount++;
	}	
	mRunningTest = "";
}

void TestRunner::checkAssertion(bool expr, const Pire::ystring& exprStr, const Pire::ystring& file, int line)
{
	if (!expr) {
		throw  AssertionFailed(exprStr, file, line);
	}
}

}


int main(int argc, char **argv)
{	
	PireUnit::TestRunner runner;
	runner.addTest(PireUnit::Impl::globalSuite());
	return runner.run(Pire::ystring((argc >= 2) ? argv[1] : ""), false, true, true) ? 0 : 1;
}
