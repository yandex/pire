/*
 * pire_ut.cpp --
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


#include <stub/hacks.h>
#include <stub/defaults.h>
#include <stub/saveload.h>
#include <stub/memstreams.h>
#include "stub/cppunit.h"
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
			ystring str = ystring(size*2, 'b') + "x" + ystring(size, 'e');
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

	try {
		REGEXP("abc[def") {}
		UNIT_ASSERT(!"Should report syntax error");
	}
	catch (Pire::Error&) {}
}

SIMPLE_UNIT_TEST(Reverse)
{
	SCANNER(ParseRegexp("abcdef").Reverse()) {
		ACCEPTS("fedcba");
		DENIES ("abcdef");
	}
}

SIMPLE_UNIT_TEST(PrefixSuffix)
{
	static const char* pattern = "-->";
	Pire::Fsm fsm = ParseRegexp(pattern, "n");
	Pire::Scanner ngsc = (~Pire::Fsm::MakeFalse() + fsm).Compile<Pire::Scanner>();
	Pire::Scanner gsc  = (~fsm.Surrounded() + fsm).Compile<Pire::Scanner>();
	Pire::Scanner rsc  = fsm.Reverse().Compile<Pire::Scanner>();

	static const char* text = "1234567890 --> middle --> end";
	const char* end = Pire::LongestPrefix(gsc, text, text + strlen(text));
	UNIT_ASSERT_EQUAL(end, text + 14);
	const char* begin = Pire::LongestSuffix(rsc, end - 1, text - 1) + 1;
	UNIT_ASSERT_EQUAL(begin, text + 11);

	end = Pire::LongestPrefix(ngsc, text, text + strlen(text));
	UNIT_ASSERT_EQUAL(end, text + 25);
	begin = Pire::LongestSuffix(rsc, end - 1, text - 1) + 1;
	UNIT_ASSERT_EQUAL(begin, text + 22);

	end = Pire::ShortestPrefix(gsc, text, text + strlen(text));
	UNIT_ASSERT_EQUAL(end, text + 14);
	begin = Pire::ShortestSuffix(rsc, end - 1, text - 1) + 1;
	UNIT_ASSERT_EQUAL(begin, text + 11);

	end = Pire::ShortestPrefix(ngsc, text, text + strlen(text));
	UNIT_ASSERT_EQUAL(end, text + 14);
	begin = Pire::ShortestSuffix(rsc, end - 1, text - 1) + 1;
	UNIT_ASSERT_EQUAL(begin, text + 11);
}

namespace {
	ssize_t LongestPrefixLen(const char* pattern, const char* str)
	{
		Pire::Scanner sc = Pire::Lexer(pattern).Parse().Compile<Pire::Scanner>();
		const char* end = Pire::LongestPrefix(sc, str, str + strlen(str));
		return end ? end - str : -1;
	}

	ssize_t ShortestPrefixLen(const char* pattern, const char* str)
	{
		Pire::Scanner sc = Pire::Lexer(pattern).Parse().Compile<Pire::Scanner>();
		const char* end = Pire::ShortestPrefix(sc, str, str + strlen(str));
		return end ? end - str : -1;
	}
}

SIMPLE_UNIT_TEST(ScanBoundaries)
{
	UNIT_ASSERT_EQUAL(LongestPrefixLen("fixed", "fixed prefix"), ssize_t(5));
	UNIT_ASSERT_EQUAL(LongestPrefixLen("fixed", "a fixed nonexistent prefix"), ssize_t(-1));

	UNIT_ASSERT_EQUAL(LongestPrefixLen("a*", "aaabbb"), ssize_t(3));
	UNIT_ASSERT_EQUAL(LongestPrefixLen("a*", "bbbbbb"), ssize_t(0));
	UNIT_ASSERT_EQUAL(LongestPrefixLen("a*", "aaaaaa"), ssize_t(6));
	UNIT_ASSERT_EQUAL(LongestPrefixLen(".*",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-"), ssize_t(80));
	UNIT_ASSERT_EQUAL(LongestPrefixLen(".*b",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-"), ssize_t(-1));
	UNIT_ASSERT_EQUAL(LongestPrefixLen(".*a.*",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-b"), ssize_t(162));
	UNIT_ASSERT_EQUAL(LongestPrefixLen("1.*a.*",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-b"), ssize_t(162));
	UNIT_ASSERT_EQUAL(LongestPrefixLen("a+", "bbbbbb"), ssize_t(-1));

	UNIT_ASSERT_EQUAL(ShortestPrefixLen("fixed", "fixed prefix"), ssize_t(5));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen("fixed", "a fixed nonexistent prefix"), ssize_t(-1));

	UNIT_ASSERT_EQUAL(ShortestPrefixLen("a*", "aaabbb"), ssize_t(0));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen("aa*", "aaabbb"), ssize_t(1));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen("a*a", "aaaaaa"), ssize_t(1));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen(".*a", "bbbba"), ssize_t(5));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen(".*a",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"), ssize_t(81));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen(".*a",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"), ssize_t(81));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen(".*a.*b",
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-a"
		"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-b"), ssize_t(162));
	UNIT_ASSERT_EQUAL(ShortestPrefixLen("a+", "bbbbbb"), ssize_t(-1));
}

SIMPLE_UNIT_TEST(ScanTermination)
{
	Pire::Scanner sc = Pire::Lexer("aaa").Parse().Compile<Pire::Scanner>();
	// Scanning must terminate at first dead state. If it does not,
	// we will pass through the end of our string and end up with segfault.
	const char str[] = "aaab";
	const char* p = Pire::LongestPrefix(sc, &str[0], &str[0] + sizeof(str));
	UNIT_ASSERT(p == &str[0] + 3);
}

struct BasicMmapTest {
	template <class Scanner>
	static void Match(Scanner& sc, const void* ptr, size_t sz, const char* str)
	{
		try {
			sc.Mmap(ptr, sz);
			if (!Pire::Impl::IsAligned(ptr, sizeof(size_t))) {
				UNIT_ASSERT(!"Failed to check for misaligned mmaping");
			} else {
				UNIT_ASSERT(Matches(sc, str));
			}
		}
		catch (Pire::Error&) {}
	}
};

template <class Sc1, class Sc2>
void TestCopyingHelper()
{
	Pire::Fsm fsm = ParseRegexp("^r$", "");
	Sc1 sc1(Pire::Fsm(fsm).Compile<Sc1>());

	// Test copy ctor
	UNIT_ASSERT(Matches(Sc2(sc1), "r"));
	UNIT_ASSERT(!Matches(Sc2(sc1), "p"));

	// Test '=' operator
	Sc2 sc2;
	sc2 = sc1;
	UNIT_ASSERT(Matches(sc2, "r"));
	UNIT_ASSERT(!Matches(sc2, "p"));
}

template <class Sc1, class Sc2>
void TestCopying()
{
	TestCopyingHelper<Sc1, Sc2>();
	TestCopyingHelper<Sc2, Sc1>();
}

SIMPLE_UNIT_TEST(Copying)
{
	TestCopying<Pire::Scanner, Pire::NonrelocScanner>();
	TestCopying<Pire::ScannerNoMask, Pire::NonrelocScannerNoMask>();
}

SIMPLE_UNIT_TEST(Serialization)
{
	Scanners s("^regexp$");

	BufferOutput wbuf;
	Save(&wbuf, s.fast);
	Save(&wbuf, s.simple);
	Save(&wbuf, s.slow);
	Save(&wbuf, s.fastNoMask);
	Save(&wbuf, s.nonreloc);
	Save(&wbuf, s.nonrelocNoMask);

	Pire::Scanner fast;
	Pire::SimpleScanner simple;
	Pire::SlowScanner slow;
	Pire::ScannerNoMask fastNoMask;
	Pire::NonrelocScanner nonreloc;
	Pire::NonrelocScannerNoMask nonrelocNoMask;

	MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
	Load(&rbuf, fast);
	Load(&rbuf, simple);
	Load(&rbuf, slow);
	Load(&rbuf, fastNoMask);
	Load(&rbuf, nonreloc);
	Load(&rbuf, nonrelocNoMask);

	UNIT_ASSERT(Matches(fast, "regexp"));
	UNIT_ASSERT(Matches(simple, "regexp"));
	UNIT_ASSERT(Matches(slow, "regexp"));
	UNIT_ASSERT(Matches(fastNoMask, "regexp"));
	UNIT_ASSERT(Matches(nonreloc, "regexp"));
	UNIT_ASSERT(Matches(nonrelocNoMask, "regexp"));
	UNIT_ASSERT(!Matches(fast, "regxp"));
	UNIT_ASSERT(!Matches(simple, "regxp"));
	UNIT_ASSERT(!Matches(slow, "regxp"));
	UNIT_ASSERT(!Matches(fastNoMask, "regxp"));
	UNIT_ASSERT(!Matches(nonreloc, "regxp"));
	UNIT_ASSERT(!Matches(nonrelocNoMask, "regxp"));
	UNIT_ASSERT(!Matches(fast, "regexp t"));
	UNIT_ASSERT(!Matches(simple, "regexp t"));
	UNIT_ASSERT(!Matches(slow, "regexp t"));
	UNIT_ASSERT(!Matches(fastNoMask, "regexp t"));
	UNIT_ASSERT(!Matches(nonreloc, "regexp t"));
	UNIT_ASSERT(!Matches(nonrelocNoMask, "regexp t"));

	Pire::Scanner fast2;
	Pire::SimpleScanner simple2;
	Pire::SlowScanner slow2;
	Pire::ScannerNoMask fastNoMask2;
	Pire::Scanner fast3;
	Pire::ScannerNoMask fastNoMask3;
	const size_t MaxTestOffset = 2 * sizeof(Pire::Impl::MaxSizeWord);
	yvector<char> buf2(wbuf.Buffer().Size() + sizeof(size_t) + MaxTestOffset);
	const char* ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t));
	const char* end = ptr + wbuf.Buffer().Size();
	memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());

	const char* ptr2 = 0;
	ptr2 = (const char*)fast2.Mmap(ptr, end - ptr);
	size_t fastSize = ptr2 - ptr;
	ptr = ptr2;
	ptr2 = (const char*)simple2.Mmap(ptr, end - ptr);
	size_t simpleSize = ptr2 - ptr;
	ptr = ptr2;
	ptr2 = (const char*)slow2.Mmap(ptr, end - ptr);
	ptr = ptr2;
	ptr2 = (const char*)fastNoMask2.Mmap(ptr, end - ptr);
	ptr = ptr2;
	// Nonreloc-s are saved as Scaner-s, so read them again
	ptr2 = (const char*)fast3.Mmap(ptr, end - ptr);
	ptr = ptr2;
	ptr2 = (const char*)fastNoMask3.Mmap(ptr, end - ptr);
	ptr = ptr2;
	UNIT_ASSERT_EQUAL(ptr, end);

	UNIT_ASSERT(Matches(fast2, "regexp"));
	UNIT_ASSERT(!Matches(fast2, "regxp"));
	UNIT_ASSERT(!Matches(fast2, "regexp t"));
	UNIT_ASSERT(Matches(fastNoMask2, "regexp"));
	UNIT_ASSERT(!Matches(fastNoMask2, "regxp"));
	UNIT_ASSERT(!Matches(fastNoMask2, "regexp t"));
	UNIT_ASSERT(Matches(slow2, "regexp"));
	UNIT_ASSERT(!Matches(slow2, "regxp"));
	UNIT_ASSERT(!Matches(slow2, "regexp t"));
	UNIT_ASSERT(Matches(simple2, "regexp"));
	UNIT_ASSERT(!Matches(simple2, "regxp"));
	UNIT_ASSERT(!Matches(simple2, "regexp t"));
	UNIT_ASSERT(Matches(fast3, "regexp"));
	UNIT_ASSERT(!Matches(fast3, "regxp"));
	UNIT_ASSERT(!Matches(fast3, "regexp t"));
	UNIT_ASSERT(Matches(fastNoMask3, "regexp"));
	UNIT_ASSERT(!Matches(fastNoMask3, "regxp"));
	UNIT_ASSERT(!Matches(fastNoMask3, "regexp t"));

	for (size_t offset = 1; offset < MaxTestOffset; ++offset) {
		ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t)) + offset;
		end = ptr + wbuf.Buffer().Size();
		memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());
		BasicMmapTest::Match(fast2, ptr, end - ptr, "regexp");
		ptr = ptr + fastSize;
		BasicMmapTest::Match(simple2, ptr, end - ptr, "regexp");
		ptr = ptr + simpleSize;
		BasicMmapTest::Match(slow2, ptr, end - ptr, "regexp");
	}
}

SIMPLE_UNIT_TEST(TestShortcuts)
{
	REGEXP("aaa") {
		ACCEPTS("......................................aaa.............");
		DENIES ("......................................aab.............");
		DENIES ("......................................................");
	}
	REGEXP("[ab]{3}") {
		ACCEPTS("......................................aaa.............");
		ACCEPTS("......................................aab.............");
		ACCEPTS("......................................bbb.............");
		DENIES ("......................................................");
	}
	REGEXP2("\xD0\xB0", "u") {
		ACCEPTS("......................................\xD0\xB0...............");
		ACCEPTS("...................................\xD0\xB0..................");
		ACCEPTS("................................\xD0\xB0.....................");
	}
}

template<class Scanner>
void TestGlue()
{
	Scanner sc1 = ParseRegexp("aaa").Compile<Scanner>();
	Scanner sc2 = ParseRegexp("bbb").Compile<Scanner>();
	Scanner glued = Scanner::Glue(sc1, sc2);
	UNIT_ASSERT_EQUAL(glued.RegexpsCount(), size_t(2));

	ypair<const size_t*, const size_t*> res;

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

	Scanner sc3 = ParseRegexp("ccc").Compile<Scanner>();
	glued = Scanner::Glue(sc3, glued);
	UNIT_ASSERT_EQUAL(glued.RegexpsCount(), size_t(3));

	res = glued.AcceptedRegexps(RunRegexp(glued, "ccc"));
	UNIT_ASSERT_EQUAL(res.second - res.first, ssize_t(1));
	UNIT_ASSERT_EQUAL(res.first[0], size_t(0));
	Scanner sc4 = Scanner::Glue(
		ParseRegexp("a", "n").Compile<Scanner>(),
		ParseRegexp("c", "n").Compile<Scanner>()
	);
	res = sc4.AcceptedRegexps(RunRegexp(sc4, "ac"));
	UNIT_ASSERT(res.second == res.first);
	UNIT_ASSERT(!sc4.Final(RunRegexp(sc4, "ac")));
}

SIMPLE_UNIT_TEST(Glue)
{
	TestGlue<Pire::Scanner>();
	TestGlue<Pire::NonrelocScanner>();
	TestGlue<Pire::ScannerNoMask>();
	TestGlue<Pire::NonrelocScannerNoMask>();
}

SIMPLE_UNIT_TEST(Slow)
{
	Pire::SlowScanner sc = ParseRegexp("a.{30}$", "").Compile<Pire::SlowScanner>();
	//                             123456789012345678901234567890
	UNIT_ASSERT( Matches(sc, "....a.............................."));
	UNIT_ASSERT(!Matches(sc, "....a..............................."));
	UNIT_ASSERT(!Matches(sc, "....a............................."));
}

SIMPLE_UNIT_TEST(Aligned)
{
	UNIT_ASSERT(Pire::Impl::IsAligned(ystring("x").c_str(), sizeof(void*)));

	REGEXP("xy") {
		// Short string with aligned head
		ACCEPTS(ystring("xy").c_str());
		DENIES (ystring("yz").c_str());
		// Short string, unaligned
		ACCEPTS(ystring(".xy").c_str() + 1);
		DENIES (ystring(".yz").c_str() + 1);
		// Short string with aligned tail
		ACCEPTS((ystring(sizeof(void*) - 2, '.') + "xy").c_str() + sizeof(void*) - 2);
		DENIES ((ystring(sizeof(void*) - 2, '.') + "yz").c_str() + sizeof(void*) - 2);
	}

	REGEXP("abcde") {
		// Everything aligned, match occurs in the middle
		ACCEPTS(ystring("ZZZZZabcdeZZZZZZ").c_str());
		DENIES (ystring("ZZZZZabcdfZZZZZZ").c_str());
		// Unaligned head
		ACCEPTS(ystring(".ZabcdeZZZ").c_str() + 1);
		DENIES (ystring(".ZxbcdeZZZ").c_str() + 1);
		// Unaligned tail
		ACCEPTS(ystring("ZZZZZZZZZZZZZabcde").c_str());
		DENIES (ystring("ZZZZZZZZZZZZZabcdf").c_str());
	}
}

#undef Run

template <class Scanner>
void BasicTestEmptySaveLoadMmap()
{
	Scanner sc;
	UNIT_ASSERT(sc.Empty());
	UNIT_ASSERT_EQUAL(sc.RegexpsCount(), size_t(0));
	UNIT_CHECKPOINT(); Pire::Runner(sc).Begin().Run("a string", 7).End(); // should not crash

	BufferOutput wbuf;
	UNIT_CHECKPOINT(); Save(&wbuf, sc);
	
	MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
	Scanner sc3;
	/*UNIT_CHECKPOINT();*/ Load(&rbuf, sc3);
	UNIT_ASSERT(sc3.Empty());
	UNIT_CHECKPOINT(); Pire::Runner(sc3).Begin().Run("a string", 7).End();
	
	Scanner sc4;
	/*UNIT_CHECKPOINT();*/ const char* ptr = (const char*) sc4.Mmap(wbuf.Buffer().Data(), wbuf.Buffer().Size());
	UNIT_ASSERT(ptr == wbuf.Buffer().Data() + wbuf.Buffer().Size());
	UNIT_ASSERT(sc4.Empty());
	UNIT_CHECKPOINT(); Pire::Runner(sc4).Begin().Run("a string", 7).End();
}

SIMPLE_UNIT_TEST(EmptyScanner)
{
	// Tests for Scanner
	BasicTestEmptySaveLoadMmap<Pire::Scanner>();
	BasicTestEmptySaveLoadMmap<Pire::ScannerNoMask>();

	Pire::Scanner sc;
	Pire::Scanner scsc = Pire::Scanner::Glue(sc, sc);
	UNIT_ASSERT(scsc.Empty());
	UNIT_ASSERT_EQUAL(scsc.RegexpsCount(), size_t(0));
	UNIT_CHECKPOINT(); Pire::Runner(scsc).Begin().Run("a string", 7).End();

	Pire::Scanner sc2 = Pire::Lexer("regex").Parse().Compile<Pire::Scanner>();
	UNIT_ASSERT_EQUAL(Pire::Scanner::Glue(sc, sc2).RegexpsCount(), size_t(1));
	UNIT_CHECKPOINT(); Pire::Runner(Pire::Scanner::Glue(sc, sc2)).Begin().Run("a string", 7).End();
	UNIT_ASSERT_EQUAL(Pire::Scanner::Glue(scsc, sc2).RegexpsCount(), size_t(1));
	UNIT_CHECKPOINT(); Pire::Runner(Pire::Scanner::Glue(scsc, sc2)).Begin().Run("a string", 7).End();
	UNIT_ASSERT_EQUAL(Pire::Scanner::Glue(Pire::Scanner::Glue(scsc, sc2), sc).RegexpsCount(), size_t(1));
	UNIT_CHECKPOINT(); Pire::Runner(Pire::Scanner::Glue(Pire::Scanner::Glue(scsc, sc2), sc)).Begin().Run("a string", 7).End();

	// Tests for NonrelocScanner
	Pire::NonrelocScanner nsc;
	UNIT_ASSERT(nsc.Empty());
	UNIT_ASSERT_EQUAL(nsc.RegexpsCount(), size_t(0));
	UNIT_CHECKPOINT(); Pire::Runner(nsc).Begin().Run("a string", 7).End();

	Pire::NonrelocScanner nsc2 = Pire::Lexer("regex").Parse().Compile<Pire::Scanner>();
	UNIT_ASSERT_EQUAL(Pire::Scanner::Glue(sc, sc2).RegexpsCount(), size_t(1));
	UNIT_CHECKPOINT(); Pire::Runner(Pire::Scanner::Glue(sc, sc2)).Begin().Run("a string", 7).End();

	{
		BufferOutput wbuf;
		UNIT_CHECKPOINT(); Save(&wbuf, nsc);
		
		MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
		Pire::NonrelocScanner nsc3;
		/*UNIT_CHECKPOINT();*/ Load(&rbuf, nsc3);
		UNIT_ASSERT(nsc3.Empty());
		UNIT_CHECKPOINT(); Pire::Runner(nsc3).Begin().Run("a string", 7).End();
	}

	BasicTestEmptySaveLoadMmap<Pire::SimpleScanner>();

	BasicTestEmptySaveLoadMmap<Pire::SlowScanner>();
}

}
