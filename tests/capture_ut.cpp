/*
 * capture_ut.cpp --
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

SIMPLE_UNIT_TEST_SUITE(TestPireCapture) {

	using Pire::CapturingScanner;
	typedef Pire::CapturingScanner::State State;

	CapturingScanner Compile(const char* regexp, int index)
	{
		Pire::Lexer lexer;

		lexer.Assign(regexp, regexp + strlen(regexp));
		lexer.AddFeature(Pire::Features::CaseInsensitive());
		lexer.AddFeature(Pire::Features::Capture((size_t) index));

		Pire::Fsm fsm = lexer.Parse();

		fsm.Surround();
		fsm.Determine();
		return fsm.Compile<Pire::CapturingScanner>();
	}

	State RunRegexp(const CapturingScanner& scanner, const char* str)
	{
		State state;
		scanner.Initialize(state);
		Step(scanner, state, Pire::BeginMark);
		Run(scanner, state, str, str + strlen(str));
		Step(scanner, state, Pire::EndMark);
		return state;
	}

	ystring Captured(const State& state, const char* str)
	{
		if (state.Captured())
			return ystring(str + state.Begin() - 1, str + state.End() - 1);
		else
			return ystring();
	}

	SIMPLE_UNIT_TEST(Trivial)
	{
		CapturingScanner scanner = Compile("google_id\\s*=\\s*[\'\"]([a-z0-9]+)[\'\"]\\s*;", 1);
		State state;
		const char* str;

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("abcde"));

		str = "var google_id = 'abcde'; eval(google_id);";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("abcde"));

		str = "google_id != 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(!state.Captured());
	}

	SIMPLE_UNIT_TEST(Sequential)
	{
		CapturingScanner scanner = Compile("google_id\\s*=\\s*[\'\"]([a-z0-9]+)[\'\"]\\s*;", 1);
		State state;
		const char* str;

		str = "google_id = 'abcde'; google_id = 'xyz';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("abcde"));

		str = "var google_id = 'abc de'; google_id = 'xyz';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("xyz"));
	}

	SIMPLE_UNIT_TEST(NegatedTerminator)
	{
		CapturingScanner scanner = Compile("=(\\d+)[^\\d]", 1);
		State state;
		const char* str;

		str = "=12345;";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("12345"));
	}

	SIMPLE_UNIT_TEST(Serialization)
	{
		CapturingScanner scanner2 = Compile("google_id\\s*=\\s*[\'\"]([a-z0-9]+)[\'\"]\\s*;", 1);

		BufferOutput wbuf;
		::Save(&wbuf, scanner2);

		MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
		CapturingScanner scanner;
		::Load(&rbuf, scanner);

		State state;
		const char* str;

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("abcde"));

		str = "google_id != 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(!state.Captured());

		CapturingScanner scanner3;
		const size_t MaxTestOffset = 2 * sizeof(Pire::Impl::MaxSizeWord);
		yvector<char> buf2(wbuf.Buffer().Size() + sizeof(size_t) + MaxTestOffset);
		const void* ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t));
		memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());
		const void* tail = scanner3.Mmap(ptr, wbuf.Buffer().Size());
		UNIT_ASSERT_EQUAL(tail, (const void*) ((size_t)ptr + wbuf.Buffer().Size()));

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner3, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), ystring("abcde"));

		str = "google_id != 'abcde';";
		state = RunRegexp(scanner3, str);
		UNIT_ASSERT(!state.Captured());

		ptr = (const void*) ((const char*) wbuf.Buffer().Data() + 1);
		try {
			scanner3.Mmap(ptr, wbuf.Buffer().Size());
			UNIT_ASSERT(!"CapturingScanner failed to check for misaligned mmaping");
		}
		catch (Pire::Error&) {}

		for (size_t offset = 1; offset < MaxTestOffset; ++offset) {
			ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t)) + offset;
			memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());
			try {
				scanner3.Mmap(ptr, wbuf.Buffer().Size());
				if (offset % sizeof(size_t) != 0) {
					UNIT_ASSERT(!"CapturingScanner failed to check for misaligned mmaping");
				} else {
					str = "google_id = 'abcde';";
					state = RunRegexp(scanner3, str);
					UNIT_ASSERT(state.Captured());
				}
			}
			catch (Pire::Error&) {}
		}
	}

	SIMPLE_UNIT_TEST(Empty)
	{
		Pire::CapturingScanner sc;
		UNIT_ASSERT(sc.Empty());
		
		UNIT_CHECKPOINT(); RunRegexp(sc, "a string"); // Just should not crash

		// Test Save/Load/Mmap
		BufferOutput wbuf;
		::Save(&wbuf, sc);

		MemoryInput rbuf(wbuf.Buffer().Data(), wbuf.Buffer().Size());
		Pire::CapturingScanner sc3;
		::Load(&rbuf, sc3);
		UNIT_CHECKPOINT(); RunRegexp(sc3, "a string");

		const size_t MaxTestOffset = 2 * sizeof(Pire::Impl::MaxSizeWord);
		yvector<char> buf2(wbuf.Buffer().Size() + sizeof(size_t) + MaxTestOffset);
		const void* ptr = Pire::Impl::AlignUp(&buf2[0], sizeof(size_t));
		memcpy((void*) ptr, wbuf.Buffer().Data(), wbuf.Buffer().Size());

		Pire::CapturingScanner sc4;
		const void* tail = sc4.Mmap(ptr, wbuf.Buffer().Size());
		UNIT_ASSERT_EQUAL(tail, (const void*) ((size_t)ptr + wbuf.Buffer().Size()));
		UNIT_CHECKPOINT(); RunRegexp(sc4, "a string");
	}

}
