#include <stub/hacks.h>
#include <stub/defaults.h>
#include <stub/saveload.h>
#include <stub/memstreams.h>
#include <cppunit.h>
#include <stdexcept>
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(TestPire) {

/*****************************************************************************
* Tests themselves
*****************************************************************************/

SIMPLE_UNIT_TEST(String)
{
	REGEXP("abc") {
		ACCEPTS("def abc ghi");
		ACCEPTS("abc");
		DENIES ("def abd ghi");
	}
}

SIMPLE_UNIT_TEST(Boundaries)
{
	REGEXP("^abc") {
		ACCEPTS("abc ghi");
		DENIES ("def abc");
	}

	REGEXP("abc$") {
		DENIES ("abc ghi");
		ACCEPTS("def abc");
	}
}

SIMPLE_UNIT_TEST(Primitives)
{
	REGEXP("abc|def") {
		ACCEPTS("def");
		ACCEPTS("abc");
		DENIES ("deb");
	}

	REGEXP("ad*e") {
		ACCEPTS("xaez");
		ACCEPTS("xadez");
		ACCEPTS("xaddez");
		ACCEPTS("xadddddddddddddddddddddddez");
		DENIES ("xafez");
	}

	REGEXP("ad+e") {
		DENIES ("xaez");
		ACCEPTS("xadez");
		ACCEPTS("xaddez");
		ACCEPTS("xadddddddddddddddddddddddez");
		DENIES ("xafez");
	}

	REGEXP("ad?e") {
		ACCEPTS("xaez");
		ACCEPTS("xadez");
		DENIES ("xaddez");
		DENIES ("xafez");
	}

	REGEXP("a.{1}e") {
		ACCEPTS("axe");
		DENIES ("ae");
		DENIES ("axye");
	}
}
 
SIMPLE_UNIT_TEST(MassAlternatives)
{
	REGEXP("((abc|def)|ghi)|klm") {
		ACCEPTS("abc");
		ACCEPTS("def");
		ACCEPTS("ghi");
		ACCEPTS("klm");
		DENIES ("aei");
		DENIES ("klc");
	}

	REGEXP("(abc|def)|(ghi|klm)") {
		ACCEPTS("abc");
		ACCEPTS("def");
		ACCEPTS("ghi");
		ACCEPTS("klm");
		DENIES ("aei");
		DENIES ("klc");
	}

	REGEXP("abc|(def|(ghi|klm))") {
		ACCEPTS("abc");
		ACCEPTS("def");
		ACCEPTS("ghi");
		ACCEPTS("klm");
		DENIES ("aei");
		DENIES ("klc");
	}

	REGEXP("abc|(def|ghi)|klm") {
		ACCEPTS("abc");
		ACCEPTS("def");
		ACCEPTS("ghi");
		ACCEPTS("klm");
		DENIES ("aei");
		DENIES ("klc");
	}
}

SIMPLE_UNIT_TEST(Composition)
{
	REGEXP("^/([^\\\\/]|\\\\.)*/[a-z]*$") {
		ACCEPTS("/regexp/i");
		ACCEPTS("/regexp2/");
		DENIES ("regexp");

		ACCEPTS("/dir\\/file/");
		DENIES ("/dir/file/");

		ACCEPTS("/dir\\\\/");
		DENIES ("/dir\\\\/file/");
	}

	REGEXP("Head(Inner)*Tail") {
		ACCEPTS("HeadInnerTail");
		ACCEPTS("HeadInnerInnerTail");
		DENIES ("HeadInneInnerTail");
		ACCEPTS("HeadTail");
	}
}

SIMPLE_UNIT_TEST(Repetition)
{
	REGEXP("^x{3,6}$") {
		DENIES ("xx");
		ACCEPTS("xxx");
		ACCEPTS("xxxx");
		ACCEPTS("xxxxx");
		ACCEPTS("xxxxxx");
		DENIES ("xxxxxxx");
	}

	REGEXP("^x{3,}$") {
		DENIES ("xx");
		ACCEPTS("xxx");
		ACCEPTS("xxxx");
		ACCEPTS("xxxxxxxxxxx");
		ACCEPTS("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	}

	REGEXP("^x{3}$") {
		DENIES ("x");
		DENIES ("xx");
		ACCEPTS("xxx");
		DENIES ("xxxx");
		DENIES ("xxxxx");
		DENIES ("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	}

	REGEXP("x.{3,10}$") {
		for (size_t size = 0; size < 20; ++size) {
			std::string str = std::string(size*2, 'b') + "x" + std::string(size, 'e');
			if (size >= 3 && size <= 10)
				ACCEPTS(str.c_str());
			else
				DENIES(str.c_str());
		}
	}
}

SIMPLE_UNIT_TEST(UTF8)
{
	REGEXP2("^.$", "u") {
		// A single-byte sequence 0xxx xxxx
		ACCEPTS("\x41");
		DENIES ("\x81");

		// A two-byte sequence: 110x xxxx | 10xx xxxx
		ACCEPTS("\xC1\x81");
		DENIES ("\xC1");
		DENIES ("\xC1\x41");
		DENIES ("\xC1\xC2");
		DENIES ("\xC1\x81\x82");

		// A three-byte sequence: 1110 xxxx | 10xx xxxx | 10xx xxxx
		ACCEPTS("\xE1\x81\x82");
		DENIES ("\xE1");
		DENIES ("\xE1\x42");
		DENIES ("\xE1\x42\x43");
		DENIES ("\xE1\xC2\xC3");
		DENIES ("\xE1\x82");
		DENIES ("\xE1\x82\x83\x84");

		// A four-byte sequence: 1111 0xxx | 10xx xxxx | 10xx xxxx | 10xx xxxx
		ACCEPTS("\xF1\x81\x82\x83");
	}

	REGEXP2("x\xD0\xA4y", "u") ACCEPTS("x\xD0\xA4y");
}

SIMPLE_UNIT_TEST(AndNot)
{
	REGEXP2("<([0-9]+&~123&~456)>", "a") {
		ACCEPTS("<111>");
		ACCEPTS("<124>");
		DENIES ("<123>");
		DENIES ("<456>");
		DENIES ("<abc>");
	}
	
	REGEXP2("[0-9]+\\&1+", "a") {
		DENIES("111");
		ACCEPTS("123&111");
	}
}

SIMPLE_UNIT_TEST(Empty)
{
	Scanners s("\\s*", "n");
	Pire::Scanner::State state;
	s.fast.Initialize(state);
	UNIT_ASSERT(s.fast.Final(state));
	Pire::SimpleScanner::State stateSF;
	s.simple.Initialize(stateSF);
	UNIT_ASSERT(s.simple.Final(stateSF));
}

SIMPLE_UNIT_TEST(Misc)
{
	REGEXP2("^[^\\s=/>]*$", "n") ACCEPTS("a");
	REGEXP("\\t") ACCEPTS("\t");

	SCANNER(ParseRegexp(".*") & ~ParseRegexp(".*http.*")) {
		ACCEPTS("str");
		DENIES("str_http");
	}

	SCANNER(~Pire::Fsm()) ACCEPTS("str");
}

SIMPLE_UNIT_TEST(Ranges)
{
	REGEXP("a\\W") {
		ACCEPTS("a,");
		DENIES("ab");
	}
}
 
SIMPLE_UNIT_TEST(Reverse)
{
	SCANNER(ParseRegexp("abcdef").Reverse()) {
		ACCEPTS("fedcba");
		DENIES ("abcdef");
	}
}
 
SIMPLE_UNIT_TEST(Scan)
{
	static const char* pattern = "-->";
	Pire::Fsm fsm = ParseRegexp(pattern, "n");
	Pire::Scanner ngsc = (~Pire::Fsm::MakeFalse() + fsm).Compile<Pire::Scanner>();
	Pire::Scanner gsc  = (~fsm.Surrounded() + fsm).Compile<Pire::Scanner>();
	Pire::Scanner rsc  = fsm.Reverse().Compile<Pire::Scanner>();
	
	static const char* text = "1234567890 --> middle --> end";
	const char* end = Pire::Scan(gsc, text, text + strlen(text));
	const char* begin = Pire::ReversedScan(rsc, end - 1, text - 1);
	++begin;

	UNIT_ASSERT_EQUAL(end, text + 14);
	UNIT_ASSERT_EQUAL(begin, text + 11);
	
	end = Pire::Scan(ngsc, text, text + strlen(text));
	begin = Pire::ReversedScan(rsc, end - 1, text - 1);
	++begin;
	
	UNIT_ASSERT_EQUAL(end, text + 25);
	UNIT_ASSERT_EQUAL(begin, text + 22);
}

namespace {
	ssize_t PrefixLen(const char* pattern, const char* str)
	{
		Pire::Scanner sc = Pire::Lexer(pattern).Parse().Compile<Pire::Scanner>();
		const char* end = Pire::Scan(sc, str, str + strlen(str));
		return end ? end - str : -1;
	}
}

SIMPLE_UNIT_TEST(ScanBoundaries)
{
	UNIT_ASSERT_EQUAL(PrefixLen("fixed", "fixed prefix"), ssize_t(5));
	UNIT_ASSERT_EQUAL(PrefixLen("fixed", "a fixed nonexistent prefix"), ssize_t(-1));
	
	UNIT_ASSERT_EQUAL(PrefixLen("a*", "aaabbb"), ssize_t(3));
	UNIT_ASSERT_EQUAL(PrefixLen("a*", "bbbbbb"), ssize_t(0));
	UNIT_ASSERT_EQUAL(PrefixLen("a*", "aaaaaa"), ssize_t(6));
	UNIT_ASSERT_EQUAL(PrefixLen("a+", "bbbbbb"), ssize_t(-1));
}

SIMPLE_UNIT_TEST(ScanTermination)
{
	Pire::Scanner sc = Pire::Lexer("aaa").Parse().Compile<Pire::Scanner>();
	// Scanning must terminate at first dead state. If it does not,
	// we will pass through the end of our string and end up with segfault.
	Pire::Scan(sc, "aaab", reinterpret_cast<const char*>(size_t(-1)));
}

SIMPLE_UNIT_TEST(Serialization)
{
	Scanners s("^regexp$");

	BufferOutput wbuf;
	Save(&wbuf, s.fast);
	Save(&wbuf, s.simple);
	Save(&wbuf, s.slow);

	Pire::Scanner fast;
	Pire::SimpleScanner simple;
	Pire::SlowScanner slow;

	MemoryInput rbuf(wbuf.Buf().Data(), wbuf.Buf().Size());
	Load(&rbuf, fast);
	Load(&rbuf, simple);
	Load(&rbuf, slow);

	UNIT_ASSERT(Matches(fast, "regexp"));
	UNIT_ASSERT(Matches(simple, "regexp"));
	UNIT_ASSERT(Matches(slow, "regexp"));
	UNIT_ASSERT(!Matches(fast, "regxp"));
	UNIT_ASSERT(!Matches(simple, "regxp"));
	UNIT_ASSERT(!Matches(slow, "regxp"));
	UNIT_ASSERT(!Matches(fast, "regexp t"));
	UNIT_ASSERT(!Matches(simple, "regexp t"));
	UNIT_ASSERT(!Matches(slow, "regexp t"));

	Pire::Scanner fast2;
	Pire::SimpleScanner simple2;
	Pire::SlowScanner slow2;
	const void* ptr = wbuf.Buf().Data();
	const void* end = (const void*) ((const char*) ptr + wbuf.Buf().Size());
	
	ptr = fast2.Mmap(ptr, (const char*) end - (const char*) ptr);
	ptr = simple2.Mmap(ptr, (const char*) end - (const char*) ptr);
	ptr = slow2.Mmap(ptr, (const char*) end - (const char*) ptr);
	UNIT_ASSERT_EQUAL(ptr, end);
	
	UNIT_ASSERT(Matches(fast2, "regexp"));
	UNIT_ASSERT(!Matches(fast2, "regxp"));
	UNIT_ASSERT(!Matches(fast2, "regexp t"));
	UNIT_ASSERT(Matches(slow2, "regexp"));
	UNIT_ASSERT(!Matches(slow2, "regxp"));
	UNIT_ASSERT(!Matches(slow2, "regexp t"));
	UNIT_ASSERT(Matches(simple2, "regexp"));
	UNIT_ASSERT(!Matches(simple2, "regxp"));
	UNIT_ASSERT(!Matches(simple2, "regexp t"));
	
	ptr = (const void*) ((const char*) wbuf.Buf().Data() + 1);
	try {
		fast2.Mmap(ptr, wbuf.Buf().Size());
		UNIT_ASSERT(!"FastScanner failed to check for misaligned mmaping");
	}
	catch (Pire::Error&) {}
	try {
		simple2.Mmap(ptr, wbuf.Buf().Size());
		UNIT_ASSERT(!"SimpleScanner failed to check for misaligned mmaping");
	}
	catch (Pire::Error&) {}
	try {
		slow2.Mmap(ptr, wbuf.Buf().Size());
		UNIT_ASSERT(!"SlowScanner failed to check for misaligned mmaping");
	}
	catch (Pire::Error&) {}

}

SIMPLE_UNIT_TEST(Glue)
{
	Pire::Scanner sc1 = ParseRegexp("aaa").Compile<Pire::Scanner>();
	Pire::Scanner sc2 = ParseRegexp("bbb").Compile<Pire::Scanner>();
	Pire::Scanner glued = Pire::Scanner::Glue(sc1, sc2);
	UNIT_ASSERT_EQUAL(glued.RegexpsCount(), size_t(2));

	std::pair<const size_t*, const size_t*> res;

	res = glued.AcceptedRegexps(RunRegexp(glued, "aaa"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(1));
	UNIT_ASSERT_EQUAL(*res.first, size_t(0));

	res = glued.AcceptedRegexps(RunRegexp(glued, "bbb"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(1));
	UNIT_ASSERT_EQUAL(*res.first, size_t(1));

	res = glued.AcceptedRegexps(RunRegexp(glued, "aaabbb"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(2));
	UNIT_ASSERT_EQUAL(res.first[0], size_t(0));
	UNIT_ASSERT_EQUAL(res.first[1], size_t(1));

	res = glued.AcceptedRegexps(RunRegexp(glued, "ccc"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(0));

	Pire::Scanner sc3 = ParseRegexp("ccc").Compile<Pire::Scanner>();
	glued = Pire::Scanner::Glue(sc3, glued);
	UNIT_ASSERT_EQUAL(glued.RegexpsCount(), size_t(3));

	res = glued.AcceptedRegexps(RunRegexp(glued, "ccc"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(1));
	UNIT_ASSERT_EQUAL(res.first[0], size_t(0));
	Pire::Scanner sc4 = Pire::Scanner::Glue(
		ParseRegexp("a", "n").Compile<Pire::Scanner>(),
		ParseRegexp("c", "n").Compile<Pire::Scanner>()
	);
	res = sc4.AcceptedRegexps(RunRegexp(sc4, "ac"));
	UNIT_ASSERT(res.second == res.first);
	UNIT_ASSERT(!sc4.Final(RunRegexp(sc4, "ac")));
}

}
