#include <stub/hacks.h>
#include <cppunit.h>
#include <pire.h>
#include <iostream>
#include <string.h>

SIMPLE_UNIT_TEST_SUITE(TestPireInline) {

template<class Scanner>
typename Scanner::State RunRegexp(const Scanner& scanner, const char* str, const Pire::Encoding& encoding = Pire::Encodings::Latin1())
{
	typename Scanner::State state;
	scanner.Initialize(state);
	Step(scanner, state, Pire::BeginMark);
	Run(scanner, state, str, str + strlen(str));
	Step(scanner, state, Pire::EndMark);
	return state;
}

template<class Scanner>
bool Matches(const Scanner& scanner, const char* str, const Pire::Encoding& encoding = Pire::Encodings::Latin1())
{
	return scanner.Final(RunRegexp(scanner, str, encoding));
}

template<class Scanner>
bool Matches2(const Scanner& scanner, const char* str)
{
	return Pire::Matches(scanner, str, str + strlen(str));
}

SIMPLE_UNIT_TEST(Inline)
{
	Pire::Scanner scanner = PIRE_REGEXP("http:\\/\\/([a-z0-9]+\\.)+[a-z]{2,4}\\/?", "is");
	UNIT_ASSERT(Matches(scanner, "http://domain.vasya.ru/"));
	UNIT_ASSERT(Matches(scanner, "prefix http://domain.vasya.ru/"));
	UNIT_ASSERT(!Matches(scanner, "http://127.0.0.1/"));

	Pire::Scanner scanner2 = PIRE_REGEXP("http:\\/\\/([a-z0-9]+\\.)+[a-z]{2,4}\\/?", "i");
	UNIT_ASSERT(Matches2(scanner2, "http://domain.vasya.ru/"));
	UNIT_ASSERT(!Matches2(scanner2, "prefix http://domain.vasya.ru/"));
	UNIT_ASSERT(!Matches2(scanner2, "http://127.0.0.1/"));
}

}
