#include "cppunit.h"
#include <cppunit/ui/text/TestRunner.h>
#include <stub/stl.h>

int main(int argc, char **argv)
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(Impl::globalSuite());
	return runner.run(Pire::ystring((argc >= 2) ? argv[1] : ""), false, true, true) ? 0 : 1;
}
