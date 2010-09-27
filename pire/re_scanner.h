#ifndef PIRE_RE_SCANNER_H
#define PIRE_RE_SCANNER_H


#ifndef NDEBUG
// PIRE_DEBUG is disabled by default even in debug mode because its too slow.
// uncomment the line below to enable it.

//#define PIRE_DEBUG
#endif

#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "stub/stl.h"
#include "align.h"
#include "stub/defaults.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace Pire {

	struct Header {
		ui32 Magic;
		ui32 Version;
		ui32 PtrSize;
		ui32 Type;
		size_t HdrSize;

		static const ui32 MAGIC = 0x45524950;   // "PIRE" on litte-endian
		static const ui32 RE_VERSION = 1;       // Should be incremented each time when the format of serialized scanner changes

		explicit Header(ui32 type, size_t hdrsize)
			: Magic(MAGIC)
			, Version(RE_VERSION)
			, PtrSize(sizeof(void*))
			, Type(type)
			, HdrSize(hdrsize)
		{}

		void Validate(ui32 type, size_t hdrsize) const
		{
			if (Magic != MAGIC || PtrSize != sizeof(void*))
				throw Error("Serialized regexp incompatible with your system");
			if (Version != RE_VERSION)
				throw Error("You are trying to used an incompatible version of a serialized regexp");
			if ((type != 0 && type != Type) || (hdrsize != 0 && HdrSize != hdrsize))
				throw Error("Serialized regexp incompatible with your system");
		}
	};

	namespace Impl {
		inline const void* AdvancePtr(const size_t*& ptr, size_t& size, size_t delta)
		{
			ptr = (const size_t*) ((const char*) ptr + delta);
			size -= delta;
			return (const void*) ptr;
		}

		template<class T>
		inline void MapPtr(T*& field, size_t count, const size_t*& p, size_t& size)
		{
			if (size < count * sizeof(*field))
				throw Error("EOF reached while mapping Pire::SlowScanner");
			field = (T*) p;
			Impl::AdvancePtr(p, size, count * sizeof(*field));
			Impl::AlignPtr(p, size);
		}

		inline void CheckAlign(const void* ptr)
		{
			if ((size_t) ptr & (sizeof(void*)-1))
				throw Error("Tried to mmap scanner at misaligned address");
		}

		inline void ValidateHeader(const size_t*& ptr, size_t& size, ui32 type, size_t hdrsize)
		{
			const Header* hdr;
			MapPtr(hdr, 1, ptr, size);
			hdr->Validate(type, hdrsize);
		}

		inline void ValidateHeader(yistream* s, ui32 type, size_t hdrsize)
		{
			Header hdr(0, 0);
			LoadPodType(s, hdr);
			AlignLoad(s, sizeof(hdr));
			hdr.Validate(type, hdrsize);
		}
	}
}

#include "scanners/multi.h"
#include "scanners/slow.h"
#include "scanners/simple.h"

#ifdef PIRE_DEBUG
#include "stub/lexical_cast.h"

namespace Pire {

	template<class Scanner>
	struct StDumper {
		StDumper(const Scanner& sc, typename Scanner::State st): m_sc(&sc), m_st(st) {}
		void Dump(yostream& stream) const { stream << m_sc->StateIndex(m_st) << (m_sc->Final(m_st) ? " [final]" : ""); }
	private:
		const Scanner* m_sc;
		typename Scanner::State m_st;
	};

	template<>
	struct StDumper<Scanner> {
		StDumper(const Scanner& sc, Scanner::State st): m_sc(&sc), m_st(st) {}

		void Dump(yostream& stream) const
		{
			stream << m_sc->StateIndex(m_st);
			if (m_sc->Final(m_st))
				stream << " [final]";
			if (m_sc->Dead(m_st))
				stream << " [dead]";
		}
	private:
		const Scanner* m_sc;
		Scanner::State m_st;
	};
	template<class Scanner> StDumper<Scanner> StDump(const Scanner& sc, typename Scanner::State st) { return StDumper<Scanner>(sc, st); }
	template<class Scanner> yostream& operator << (yostream& stream, const StDumper<Scanner>& stdump) { stdump.Dump(stream); return stream; }
}

#endif

namespace Pire {

template<class Scanner>
FORCED_INLINE
void Step(const Scanner& scanner, typename Scanner::State& state, Char ch)
{
	YASSERT(ch < MaxCharUnaligned);
	typename Scanner::Action a = scanner.Next(state, ch);
	scanner.TakeAction(state, a);
}

#ifndef PIRE_DEBUG

namespace Impl {
#ifndef WORDS_BIGENDIAN
	inline size_t ToLittleEndian(size_t val) { return val; }
#else
#error TODO: Please implement Pire::Impl::ToLittleEndian()
#endif

	template<class Scanner1, class Scanner2>
	class ScannerPair {
	public:
		typedef ypair<typename Scanner1::State, typename Scanner2::State> State;
		typedef ypair<typename Scanner1::Action, typename Scanner2::Action> Action;

		ScannerPair()
			: m_scanner1()
			, m_scanner2()
		{
		}
		ScannerPair(const Scanner1& s1, const Scanner2& s2)
			: m_scanner1(&s1)
			, m_scanner2(&s2)
		{
		}

		void Initialize(State& state) const
		{
			m_scanner1->Initialize(state.first);
			m_scanner2->Initialize(state.second);
		}

		Action Next(State& state, Char ch) const
		{
			return ymake_pair(
				m_scanner1->Next(state.first, ch),
				m_scanner2->Next(state.second, ch)
			);
		}

		void TakeAction(State& s, Action a) const
		{
			m_scanner1->TakeAction(s.first, a.first);
			m_scanner2->TakeAction(s.second, a.second);
		}

	private:
		const Scanner1* m_scanner1;
		const Scanner2* m_scanner2;
	};


	/// Effectively runs a scanner on a short data chunk, fit completely into one machine word.
	template<class Scanner>
	inline void RunChunk(const Scanner& scanner, typename Scanner::State& state, size_t chunk, size_t size)
	{
		YASSERT(size < 8);

		if (size & 4) {
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
		}
		if (size & 2) {
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
			Step(scanner, state, chunk & 0xFF); chunk >>= 8;
		}
		if (size & 1) {
			Step(scanner, state, chunk & 0xFF);
		}
	}
}

/// The main function: runs a scanner through given memory range.
template<class Scanner>
inline void Run(const Scanner& scanner, typename Scanner::State& state, const char* begin, const char* end)
{
	YASSERT(sizeof(void*) <= 8);

	const size_t* head = reinterpret_cast<const size_t*>((reinterpret_cast<size_t>(begin)) & ~(sizeof(void*)-1));
	const size_t* tail = reinterpret_cast<const size_t*>((reinterpret_cast<size_t>(end)) & ~(sizeof(void*)-1));

	size_t headSize = ((const char*) head - begin + sizeof(void*)) & (sizeof(void*) - 1);
	size_t tailSize = end - (const char*) tail;

	if (head == tail) {
		Impl::RunChunk(scanner, state, Impl::ToLittleEndian(*head) >> 8*(sizeof(void*) - headSize), end - begin);
		return;
	}

	if (headSize) {
		Impl::RunChunk(scanner, state, Impl::ToLittleEndian(*head) >> 8*(sizeof(void*) - headSize), headSize);
		++head;
	}

	for (; head < tail; ++head) {
		size_t chunk = Impl::ToLittleEndian(*head);
		// TODO: manually unroll this loop
		for (unsigned i = 0; i < sizeof(void*); ++i) {
			Step(scanner, state, chunk & 0xFF);
			chunk >>= 8;
		}
	}

	if (tailSize)
		Impl::RunChunk(scanner, state, Impl::ToLittleEndian(*tail), tailSize);
}

/// Runs two scanners through given memory range simultaneously.
/// This is several percent faster than running them independently.
template<class Scanner1, class Scanner2>
inline void Run(const Scanner1& scanner1, const Scanner2& scanner2, typename Scanner1::State& state1, typename Scanner2::State& state2, const char* begin, const char* end)
{
	typedef Impl::ScannerPair<Scanner1, Scanner2> Scanners;
	Scanners pair(scanner1, scanner2);
	typename Scanners::State states(state1, state2);
	Run(pair, states, begin, end);
	state1 = states.first;
	state2 = states.second;
}

/// A specialization for SlowScanner, since its state is much heavier than other ones
/// and we thus want to avoid copying states.
template<>
inline void Run<SlowScanner>(const SlowScanner& scanner, SlowScanner::State& state, const char* begin, const char* end)
{
	SlowScanner::State temp;
	scanner.Initialize(temp);

	SlowScanner::State* src = &state;
	SlowScanner::State* dest = &temp;

	for (; begin != end; ++begin) {
		scanner.Next(*src, *dest, static_cast<unsigned char>(*begin));
		DoSwap(src, dest);
	}
	if (src != &state)
		state = *src;
}

#else

/// A debug version of all Run() methods.
template<class Scanner>
inline void Run(const Scanner& scanner, typename Scanner::State& state, const char* begin, const char* end)
{
	Cdbg << "Running regexp on string " << ystring(begin, ymin(end - begin, static_cast<ptrdiff_t>(100u))) << Endl;
	Cdbg << "Initial state " << StDump(scanner, state) << Endl;

	for (; begin != end; ++begin) {
		Step(scanner, state, (unsigned char)*begin);
		Cdbg << *begin << " => state " << StDump(scanner, state) << Endl;
	}
}

#endif

/// Find a longest acceptable prefix of a given string.
/// Returns its right boundary or NULL if there exists no acceptable prefix (even the empty string is rejected).
inline const char* LongestPrefix(const Scanner& scanner, const char* begin, const char* end)
{
	Scanner::State state;
	scanner.Initialize(state);

 	PIRE_IFDEBUG(Cdbg << "Running LongestPrefix on string " << ystring(begin, ymin(end - begin, static_cast<ptrdiff_t>(100u))) << Endl);
	PIRE_IFDEBUG(Cdbg << "Initial state " << StDump(scanner, state) << Endl);

	const char* pos = 0;
	while (begin != end && !scanner.Dead(state)) {
		if (scanner.Final(state))
			pos = begin;
		Step(scanner, state, (unsigned char)*begin);
		PIRE_IFDEBUG(Cdbg << *begin << " => state " << StDump(scanner, state) << Endl);
		++begin;
	}
	if (scanner.Final(state))
		pos = begin;
	return pos;
}

/// The same as above, but scans string in reverse direction
/// (consider using Fsm::Reverse() for using in this function).
inline const char* LongestSuffix(const Scanner& scanner, const char* rbegin, const char* rend)
{
	Scanner::State state;
	scanner.Initialize(state);

	PIRE_IFDEBUG(Cdbg << "Running LongestSuffix on string " << ystring(rbegin - ymin(rbegin - rend, static_cast<ptrdiff_t>(100u)) + 1, rbegin + 1) << Endl);
	PIRE_IFDEBUG(Cdbg << "Initial state " << StDump(scanner, state) << Endl);

	const char* pos = 0;
	while (rbegin != rend && !scanner.Dead(state)) {
		if (scanner.Final(state))
			pos = rbegin;
		Step(scanner, state, (unsigned char)*rbegin);
		PIRE_IFDEBUG(Cdbg << *rbegin << " => state " << StDump(scanner, state) << Endl);
		--rbegin;
	}
	if (scanner.Final(state))
		pos = rbegin;
	return pos;
}

/// Finds a first position where FSM jumps from a non-final state into a final
/// one (i.e. finds shortest acceptable prefixx)
inline const char* ShortestPrefix(const Scanner& scanner, const char* begin, const char* end)
{
	Pire::Scanner::State state;
	scanner.Initialize(state);

	PIRE_IFDEBUG(Cdbg << "Running ShortestPrefix on string " << ystring(begin, ymin(end - begin, static_cast<ptrdiff_t>(100u))) << Endl);
	PIRE_IFDEBUG(Cdbg << "Initial state " << StDump(scanner, state) << Endl);

	for (; begin != end && !scanner.Final(state); ++begin) {
		scanner.Next(state, (unsigned char)*begin);
		PIRE_IFDEBUG(Cdbg << *begin << " => state " << StDump(scanner, state) << Endl);
	}
	return scanner.Final(state) ? begin : 0;
}

/// The same as above, but scans string in reverse direction
inline const char* ShortestSuffix(const Scanner& scanner, const char* rbegin, const char* rend)
{
	Pire::Scanner::State state;
	scanner.Initialize(state);

	PIRE_IFDEBUG(Cdbg << "Running ShortestSuffix on string " << ystring(rbegin - ymin(rbegin - rend, static_cast<ptrdiff_t>(100u)) + 1, rbegin + 1) << Endl);
	PIRE_IFDEBUG(Cdbg << "Initial state " << StDump(scanner, state) << Endl);

	for (; rbegin != rend && !scanner.Final(state); --rbegin) {
		scanner.Next(state, (unsigned char)*rbegin);
		PIRE_IFDEBUG(Cdbg << *rbegin << " => state " << StDump(scanner, state) << Endl);
	}
	return scanner.Final(state) ? rbegin : 0;
}


template<class Scanner>
class RunHelper {
public:
	RunHelper(const Scanner& sc, typename Scanner::State st): Sc(&sc), St(st) {}
	explicit RunHelper(const Scanner& sc): Sc(&sc) { Sc->Initialize(St); }

	RunHelper<Scanner>& Step(Char letter) { Pire::Step(*Sc, St, letter); return *this; }
	RunHelper<Scanner>& Run(const char* begin, const char* end) { Pire::Run(*Sc, St, begin, end); return *this; }
	RunHelper<Scanner>& Run(const char* str, size_t size) { return Run(str, str + size); }
	RunHelper<Scanner>& Run(const ystring& str) { return Run(str.c_str(), str.c_str() + str.size()); }
	RunHelper<Scanner>& Begin() { return Step(BeginMark); }
	RunHelper<Scanner>& End() { return Step(EndMark); }

	const typename Scanner::State& State() const { return St; }
	struct Tag {};
	operator const Tag*() const { return Sc->Final(St) ? (const Tag*) this : 0; }
	bool operator ! () const { return !Sc->Final(St); }

private:
	const Scanner* Sc;
	typename Scanner::State St;
};

template<class Scanner>
RunHelper<Scanner> Runner(const Scanner& sc) { return RunHelper<Scanner>(sc); }

template<class Scanner>
RunHelper<Scanner> Runner(const Scanner& sc, typename Scanner::State st) { return RunHelper<Scanner>(sc, st); }


/// Provided for testing purposes and convinience
template<class Scanner>
bool Matches(const Scanner& scanner, const char* begin, const char* end)
{
	return Runner(scanner).Run(begin, end);
}

/// Constructs an inline scanner in one statement
template<class Scanner>
Scanner MmappedScanner(const void* ptr, size_t size)
{
	Scanner s;
	s.Mmap(ptr, size);
	return s;
}

}

namespace std {
	inline void swap(Pire::Scanner& a, Pire::Scanner b) {
		a.Swap(b);
	}
}


#endif
