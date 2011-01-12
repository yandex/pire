#include <stub/hacks.h>
#include <stub/defaults.h>
#include "stub/cppunit.h"
#include <stdexcept>
#include "common.h"
#include <easy.h>

SIMPLE_UNIT_TEST_SUITE(TestPireEasy) {
	
SIMPLE_UNIT_TEST(Match)
{
	Pire::Regexp re("(foo|bar)+", Pire::I);
	UNIT_ASSERT("prefix fOoBaR suffix" ==~ re);
	UNIT_ASSERT(!("bla bla bla" ==~ re));
}

SIMPLE_UNIT_TEST(Utf8)
{
	Pire::Regexp re("^.$", Pire::I | Pire::UTF8);
	UNIT_ASSERT("\x41" ==~ re);
	UNIT_ASSERT(!("\x81" ==~ re));
}

SIMPLE_UNIT_TEST(TwoFeatures)
{
	Pire::Regexp re("^(a.c&.b.)$", Pire::I | Pire::ANDNOT);
	UNIT_ASSERT("abc" ==~ re);
	UNIT_ASSERT("ABC" ==~ re);
	UNIT_ASSERT(!("adc" ==~ re));
}
	
}
