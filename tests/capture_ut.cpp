#include <stub/hacks.h>
#include <stub/saveload.h>
#include <stub/utf8.h>
#include <stub/memstreams.h>
#include <cppunit.h>
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

	std::string Captured(const State& state, const char* str)
	{
		if (state.Captured())
			return std::string(str + state.Begin() - 1, str + state.End() - 1);
		else
			return std::string();
	}

	SIMPLE_UNIT_TEST(Trivial)
	{
		CapturingScanner scanner = Compile("google_id\\s*=\\s*[\'\"]([a-z0-9]+)[\'\"]\\s*;", 1);
		State state;
		const char* str;

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("abcde"));

		str = "var google_id = 'abcde'; eval(google_id);";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("abcde"));

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
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("abcde"));

		str = "var google_id = 'abc de'; google_id = 'xyz';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("xyz"));
	}

	SIMPLE_UNIT_TEST(NegatedTerminator)
	{
		CapturingScanner scanner = Compile("=(\\d+)[^\\d]", 1);
		State state;
		const char* str;

		str = "=12345;";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("12345"));
	}

	SIMPLE_UNIT_TEST(Serialization)
	{
		CapturingScanner scanner2 = Compile("google_id\\s*=\\s*[\'\"]([a-z0-9]+)[\'\"]\\s*;", 1);

		BufferOutput wbuf;
		::Save(&wbuf, scanner2);

		MemoryInput rbuf(wbuf.Buf().Data(), wbuf.Buf().Size());
		CapturingScanner scanner;
		::Load(&rbuf, scanner);

		State state;
		const char* str;

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("abcde"));

		str = "google_id != 'abcde';";
		state = RunRegexp(scanner, str);
		UNIT_ASSERT(!state.Captured());

		CapturingScanner scanner3;
		const void* tail = scanner3.Mmap(&wbuf.Buf().Data()[0], wbuf.Buf().Size());
		UNIT_ASSERT_EQUAL(tail, (const void*) (&wbuf.Buf().Data()[0] + wbuf.Buf().Size()));

		str = "google_id = 'abcde';";
		state = RunRegexp(scanner3, str);
		UNIT_ASSERT(state.Captured());
		UNIT_ASSERT_EQUAL(Captured(state, str), std::string("abcde"));

		str = "google_id != 'abcde';";
		state = RunRegexp(scanner3, str);
		UNIT_ASSERT(!state.Captured());

	}

}
