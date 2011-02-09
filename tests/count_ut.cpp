/*
 * count_ut.cpp --
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
#include <stub/saveload.h>
#include <stub/utf8.h>
#include <stub/memstreams.h>
#include "stub/cppunit.h"
#include <pire.h>
#include <extra.h>
#include <string.h>


SIMPLE_UNIT_TEST_SUITE(TestCount) {

	using Pire::CountingScanner;
	typedef Pire::CountingScanner::State State;

	Pire::Fsm MkFsm(const char* regexp, const Pire::Encoding& encoding)
	{
		Pire::Lexer lex;
		lex.SetEncoding(encoding);
		yvector<wchar32> ucs4;
		encoding.FromLocal(regexp, regexp + strlen(regexp), std::back_inserter(ucs4));
		lex.Assign(ucs4.begin(), ucs4.end());
		return lex.Parse();
	}
	
	Pire::CountingScanner::State Run(const Pire::CountingScanner& scanner, const char* text, size_t len =-1)
	{
		if (len == (size_t)-1) len = strlen(text);
		Pire::CountingScanner::State state;
		scanner.Initialize(state);
		Pire::Step(scanner, state, Pire::BeginMark);
		Pire::Run(scanner, state, text, text + len);
		Pire::Step(scanner, state, Pire::EndMark);
		return state;
	}

	size_t Count(const char* regexp, const char* separator, const char* text, size_t len = -1, const Pire::Encoding& encoding = Pire::Encodings::Utf8())
	{
		return Run(Pire::CountingScanner(MkFsm(regexp, encoding), MkFsm(separator, encoding)), text, len).Result(0);
	}

	SIMPLE_UNIT_TEST(Count)
	{
		UNIT_ASSERT_EQUAL(Count("[a-z]+", "\\s",  "abc def, abc def ghi, abc"), size_t(3));
		char aaa[] = "abc def\0 abc\0 def ghi, abc";
		UNIT_ASSERT_EQUAL(Count("[a-z]+", ".*", aaa, sizeof(aaa), Pire::Encodings::Latin1()), size_t(6));
		UNIT_ASSERT_EQUAL(Count("[a-z]+", ".*", aaa, sizeof(aaa)), size_t(6));
		UNIT_ASSERT_EQUAL(Count("\\w", "", "abc abcdef abcd abcdefgh ac"), size_t(8));
		UNIT_ASSERT_EQUAL(Count("http", ".*", "http://aaa, http://bbb, something in the middle, http://ccc, end"), size_t(3));
		UNIT_ASSERT_EQUAL(Count("[\320\220-\320\257\320\260-\321\217]+", "\\s+", " \320\257\320\275\320\264\320\265\320\272\321\201      "
			"\320\237\320\276\320\262\320\265\321\200\320\275\321\203\321\202\321\214  \320\222\320\276\320\271\321\202\320\270\302\240"
			"\320\262\302\240\320\277\320\276\321\207\321\202\321\203                       \302\251\302\240" "1997\342\200\224" "2008 "
			"\302\253\320\257\320\275\320\264\320\265\320\272\321\201\302\273    \320\224\320\270\320\267\320\260\320\271\320\275\302"
			"\240\342\200\224\302\240\320\241\321\202\321\203\320\264\320\270\321\217 \320\220\321\200\321\202\320\265\320\274\320\270"
			"\321\217\302\240\320\233\320\265\320\261\320\265\320\264\320\265\320\262\320\260\012\012"), size_t(5));
		UNIT_ASSERT_EQUAL(Count("\321\201\320\265\320\272\321\201", ".*",
			"\320\277\320\276\321\200\320\275\320\276, \320\273\320\265\321\202 10 \320\263\320\276\320\273\321\213\320\265 12 "
			"\320\264\320\265\321\202\320\270, \320\264\320\265\321\202\320\270 \320\277\320\276\321\200\320\275\320\276 "
			"\320\262\320\270\320\264\320\265\320\276 \320\261\320\265\321\201\320\277\320\273\320\260\321\202\320\275\320\276\320\265. "
			"\320\261\320\265\321\201\320\277\320\273\320\260\321\202\320\275\320\276\320\265 \320\262\320\270\320\264\320\265\320\276 "
			"\320\277\320\276\321\200\320\275\320\276 \320\264\320\265\321\202\320\270. \320\264\320\265\321\202\320\270 "
			"\320\277\320\276\321\200\320\275\320\276 \320\262\320\270\320\264\320\265\320\276 "
			"\320\261\320\265\321\201\320\277\320\273\320\260\321\202\320\275\320\276\320\265!<br> "
			"\320\264\320\265\320\262\321\203\321\210\320\272\321\203 \320\264\320\273\321\217 \320\277\320\276\320\264 "
			"\321\201\320\265\320\272\321\201\320\260 \320\277\320\260\321\200\320\276\320\271 "
			"\321\201\320\265\320\274\320\265\320\271\320\275\320\276\320\271 \321\201 \320\270\321\211\320\265\320\274 "
			"\320\272\320\260\320\271\321\204\320\276\320\274. \321\201\320\265\320\274\320\265\320\271\320\275\320\276\320\271 "
			"\320\277\320\276\320\264 \320\272\320\260\320\271\321\204\320\276\320\274 "
			"\320\264\320\265\320\262\321\203\321\210\320\272\321\203 \320\277\320\260\321\200\320\276\320\271 "
			"\320\270\321\211\320\265\320\274 \321\201 \320\264\320\273\321\217 \321\201\320\265\320\272\321\201\320\260!<br> "
			"\321\202\320\270\321\202\321\214\320\272\320\270 \320\261\320\276\320\273\321\214\321\210\320\270\320\265. "
			"\320\273\320\265\321\202 10 \320\263\320\276\320\273\321\213\320\265 12 \320\264\320\265\321\202\320\270!<br> "
			"\320\270\321\211\320\265\320\274 \321\201 \320\277\320\276\320\264 \320\272\320\260\320\271\321\204\320\276\320\274 "
			"\321\201\320\265\320\272\321\201\320\260\320\277\320\260\321\200\320\276\320\271 \320\264\320\273\321\217 "
			"\320\264\320\265\320\262\321\203\321\210\320\272\321\203 \321\201\320\265\320\274\320\265\320\271\320\275\320\276\320\271! "
			"\320\261\320\276\320\273\321\214\321\210\320\270\320\265 \321\202\320\270\321\202\321\214\320\272\320\270, "
			"\320\273\320\265\320\272\320\260\321\200\321\201\321\202\320\262\320\260 \321\201\320\270\321\201\321\202\320\265\320\274\320\260 "
			"\320\264\320\273\321\217 \320\276\320\277\320\276\321\200\320\275\320\276-\320\264\320\262\320\270\320\263\320\260\321\202"
			"\320\265\320\273\321\214\320\275\320\260\321\217 \320\266\320\270\320\262\320\276\321\202\320\275\321\213\321\205, \320\264"
			"\320\273\321\217 \320\270\321\211\320\265\320\274 \321\201\320\265\320\272\321\201\320\260 \320\272\320\260\320\271\321\204"
			"\320\276\320\274 \320\264\320\265\320\262\321\203\321\210\320\272\321\203 \321\201\320\265\320\274\320\265\320\271\320\275"
			"\320\276\320\271 \320\277\320\276\320\264 \320\277\320\260\321\200\320\276\320\271 \321\201. \320\276\320\277\320\276\321"
			"\200\320\275\320\276-\320\264\320\262\320\270\320\263\320\260\321\202\320\265\320\273\321\214\320\275\320\260\321\217 \321"
			"\201\320\270\321\201\321\202\320\265\320\274\320\260 \320\273\320\265\320\272\320\260\321\200\321\201\321\202\320\262\320\260 "
			"\320\264\320\273\321\217 \320\266\320\270\320\262\320\276\321\202\320\275\321\213\321\205, \320\261\320\265\321\201\320\277"
			"\320\273\320\260\321\202\320\275\320\276\320\265 \320\277\320\276\321\200\320\275\320\276 \320\262\320\270\320\264\320\265"
			"\320\276 \320\264\320\265\321\202\320\270. \320\276\321\204\320\270\321\206\320\265\321\200\321\213 \320\277\320\276\321"
			"\200\320\275\320\276 \321\204\320\276\321\202\320\276 \320\263\320\265\320\270, \320\270\321\211\320\265\320\274 \321\201"
			"\320\265\320\274\320\265\320\271\320\275\320\276\320\271 \320\264\320\265\320\262\321\203\321\210\320\272\321\203 \320\277"
			"\320\276 \320\277\320\260\321\200\320\276\320\271 \321\201\320\265\320\272\321\201\320\260 \320\264\320\273\321\217 \321\201 "
			"\320\272\320\260\320\271\321\204\320\276\320\274. \320\277\320\276\320\264 \320\264\320\273\321\217 \320\272\320\260\320\271"
			"\321\204\320\276\320\274 \321\201\320\265\320\274\320\265\320\271\320\275\320\276\320\271 \321\201\320\265\320\272\321\201"
			"\320\260 \320\277\320\260\321\200\320\276\320\271 \321\201 \320\264\320\265\320\262\321\203\321\210\320\272\321\203 \320\270"
			"\321\211\320\265\320\274? \320\262\320\270\320\264\320\265\320\276 \320\261\320\265\321\201\320\277\320\273\320\260\321\202"
			"\320\275\320\276\320\265 \320\277\320\276\321\200\320\275\320\276 \320\264\320\265\321\202\320\270, \320\264\320\265\321\202"
			"\320\270 \320\261\320\265\321\201\320\277\320\273\320\260\321\202\320\275\320\276\320\265"),
			size_t(6));
		UNIT_ASSERT_EQUAL(Count("<a[^>]*>[^<]*</a>", "([^<]|<br\\s?/?>)*", "\321\200\320\275\320\276</a><br />"
			"<a href=\"http://wapspzk.1sweethost.com//22.html\">\320\264\320\265\321\210\320\265\320\262\321\213\320\265 \320\277\320\276"
			"\321\200\320\275\320\276 \321\204\320\270\320\273\321\214\320\274\321\213</a><br /><a href=\"http://wapspzk.1sweethost.com//23.html\">"
			"\321\201\320\265\320\272\321\201 \321\210\320\276\320\277 \321\200\320\276\321\201\320\270\321\202\320\260</a><br />"
			"<a href=\"http://wapspzk.1sweethost.com//24.html\">\320\263\320\276\320\273\321\213\320\265 \320\264\320\265\320\262\321\203"
			"\321\210\320\272\320\270 \321\203\320\273\320\270\321\206\320\260</a><br /><a href=\"http://wapspzk.1sweethost.com//25.html\">"
			"\321\202\321\200\320\260\321\205\320\275\321\203\321\202\321\214 \320\274\320\260\320\274\320\260\321\210\320\270</a><br />"
			"<a href=\"http://wapspzk.1sweethost.com//26.html\">\320\277\320\270\320\267\320\264\320\260 \321\204\321\200\320\270\321\201"
			"\320\272\320\265</a><br /><a href=\"http://wapspzk.1sweethost.com//27.html\">\320\261\320\265\321\201\320\277\320\273\320\260"
			"\321\202\320\275\320\276</a><br /><a href=\"http://wapspzk.1sweethost.com//33.html\">\321\201\320\276\321\206\320\270\320\276"
			"\320\273\320\276\320\263\320\270\321\207\320\265\321\201\320\272\320\270\320\271 \320\260\320\275\320\260\320\273\320\270\320"
			"\267 \320\274\320\276\320\264\320\265\320\273\320\265\320\271 \321\201\320\265\320\272\321\201\321\203\320\260\320\273\321\214"
			"\320\275\320\276\320\263\320\276 \320\277\320\276\320\262\320\265\320\264\320\265\320\275\320\270\321\217</a>\321\217"), size_t(7));
		UNIT_ASSERT(Count("a", "b", "aaa") != size_t(3));
	}

	SIMPLE_UNIT_TEST(CountGlue)
	{
		const Pire::Encoding& enc = Pire::Encodings::Utf8();
		Pire::CountingScanner sc1 = Pire::CountingScanner(MkFsm("[a-z]+", enc), MkFsm(".*", enc));
		Pire::CountingScanner sc2 = Pire::CountingScanner(MkFsm("[0-9]+", enc), MkFsm(".*", enc));
		Pire::CountingScanner sc  = Pire::CountingScanner::Glue(sc1, sc2);
		Pire::CountingScanner::State st = Run(sc, "abc defg 123 jklmn 4567 opqrst");
		UNIT_ASSERT_EQUAL(st.Result(0), size_t(4));
		UNIT_ASSERT_EQUAL(st.Result(1), size_t(2));
	}
	
	SIMPLE_UNIT_TEST(CountBoundaries)
	{
		const Pire::Encoding& enc = Pire::Encodings::Utf8();
		Pire::CountingScanner sc(MkFsm("^[a-z]+$", enc), MkFsm("(.|^|$)*", enc));
		const char* strings[] = { "abcdef", "abc def", "defcba", "xyz abc", "a", "123" };
		Pire::CountingScanner::State st;
		sc.Initialize(st);
		for (size_t i = 0; i < sizeof(strings) / sizeof(*strings); ++i) {
			Pire::Step(sc, st, Pire::BeginMark);
			Pire::Run(sc, st, strings[i], strings[i] + strlen(strings[i]));
			Pire::Step(sc, st, Pire::EndMark);
		}
		UNIT_ASSERT_EQUAL(st.Result(0), size_t(3));
	}

	SIMPLE_UNIT_TEST(Serialization)
	{
		const Pire::Encoding& enc = Pire::Encodings::Latin1();
		Pire::CountingScanner sc1 = Pire::CountingScanner(MkFsm("[a-z]+", enc), MkFsm(".*", enc));
		Pire::CountingScanner sc2 = Pire::CountingScanner(MkFsm("[0-9]+", enc), MkFsm(".*", enc));
		Pire::CountingScanner sc  = Pire::CountingScanner::Glue(sc1, sc2);

		BufferOutput wbuf;
		::Save(&wbuf, sc);

		MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
		Pire::CountingScanner sc3;
		::Load(&rbuf, sc3);

		Pire::CountingScanner::State st = Run(sc3, "abc defg 123 jklmn 4567 opqrst");
		UNIT_ASSERT_EQUAL(st.Result(0), size_t(4));
		UNIT_ASSERT_EQUAL(st.Result(1), size_t(2));

		const size_t MaxTestOffset = 2 * sizeof(Pire::Impl::MaxSizeWord);
		yvector<char> buf2(wbuf.Buffer().Size() + sizeof(size_t) + MaxTestOffset);

		// Test mmap-ing at various alignments
		for (size_t offset = 0; offset < MaxTestOffset; ++offset) {
			const void* ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t)) + offset;
			memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());
			try {
				Pire::CountingScanner sc4;
				const void* tail = sc4.Mmap(ptr, wbuf.Buffer().Size());

				if (offset % sizeof(size_t) != 0) {
					UNIT_ASSERT(!"CountingScanner failed to check for misaligned mmaping");
				} else {
					UNIT_ASSERT_EQUAL(tail, (const void*) ((size_t)ptr + wbuf.Buffer().Size()));

					st = Run(sc4, "abc defg 123 jklmn 4567 opqrst");
					UNIT_ASSERT_EQUAL(st.Result(0), size_t(4));
					UNIT_ASSERT_EQUAL(st.Result(1), size_t(2));
				}
			}
			catch (Pire::Error&) {}
		}
	}

	SIMPLE_UNIT_TEST(Empty)
	{
		Pire::CountingScanner sc;
		UNIT_ASSERT(sc.Empty());

		UNIT_CHECKPOINT(); Run(sc, "a string"); // Just should not crash

		// Test glueing empty
		const Pire::Encoding& enc = Pire::Encodings::Latin1();
		Pire::CountingScanner sc1 = Pire::CountingScanner(MkFsm("[a-z]+", enc), MkFsm(".*", enc));
		Pire::CountingScanner sc2  = Pire::CountingScanner::Glue(sc, Pire::CountingScanner::Glue(sc, sc1));
		Pire::CountingScanner::State st = Run(sc2, "abc defg 123 jklmn 4567 opqrst");
		UNIT_ASSERT_EQUAL(st.Result(0), size_t(4));

		// Test Save/Load/Mmap
		BufferOutput wbuf;
		::Save(&wbuf, sc);

		MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
		Pire::CountingScanner sc3;
		::Load(&rbuf, sc3);
		UNIT_CHECKPOINT(); Run(sc3, "a string");

		const size_t MaxTestOffset = 2 * sizeof(Pire::Impl::MaxSizeWord);
		yvector<char> buf2(wbuf.Buffer().Size() + sizeof(size_t) + MaxTestOffset);
		const void* ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t));
		memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());

		Pire::CountingScanner sc4;
		const void* tail = sc4.Mmap(ptr, wbuf.Buffer().Size());
		UNIT_ASSERT_EQUAL(tail, (const void*) ((size_t)ptr + wbuf.Buffer().Size()));
		UNIT_CHECKPOINT(); Run(sc4, "a string");
	}

}
