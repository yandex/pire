/*
 * run.h -- routines for running scanners on strings.
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


#ifndef PIRE_RE_SCANNER_H
#define PIRE_RE_SCANNER_H

#include "defs.h"
#include "stub/stl.h"
#include "stub/memstreams.h"
#include "scanners/pair.h"

namespace Pire {

	template<class Scanner>
	struct StDumper {
		StDumper(const Scanner& sc, typename Scanner::State st): m_sc(&sc), m_st(st) {}
		void Dump(yostream& stream) const { stream << m_sc->StateIndex(m_st) << (m_sc->Final(m_st) ? " [final]" : ""); }
	private:
		const Scanner* m_sc;
		typename Scanner::State m_st;
	};

	template<class Scanner> StDumper<Scanner> StDump(const Scanner& sc, typename Scanner::State st) { return StDumper<Scanner>(sc, st); }
	template<class Scanner> yostream& operator << (yostream& stream, const StDumper<Scanner>& stdump) { stdump.Dump(stream); return stream; }
}

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
	
	template<class Scanner>
	struct AlignedRunner {
		
		static inline typename Scanner::State
		RunAligned(const Scanner& scanner, typename Scanner::State state, const size_t* begin, const size_t* end)
		{
			for (; begin != end; ++begin) {
				size_t chunk = ToLittleEndian(*begin);

				// Comparing loop variable to 0 saves inctructions becuase "sub 1, reg" will set zero flag
				// while in case of "for (i = 0; i < 8; ++i)" loop there will be an extra "cmp 8, reg" on each step
				for (unsigned i = sizeof(void*); i != 0; --i) {
					Step(scanner, state, chunk & 0xFF);
					chunk >>= 8;
				}
			}
			return state;
		}
	};
}

/// The main function: runs a scanner through given memory range.
template<class Scanner>
inline void Run(const Scanner& scanner, typename Scanner::State& st, const char* begin, const char* end)
{
	YASSERT(sizeof(void*) <= 8);

	const size_t* head = reinterpret_cast<const size_t*>((reinterpret_cast<size_t>(begin)) & ~(sizeof(void*)-1));
	const size_t* tail = reinterpret_cast<const size_t*>((reinterpret_cast<size_t>(end)) & ~(sizeof(void*)-1));

	size_t headSize = ((const char*) head - begin + sizeof(void*)) & (sizeof(void*) - 1);
	size_t tailSize = end - (const char*) tail;

	if (head == tail) {
		Impl::RunChunk(scanner, st, Impl::ToLittleEndian(*head) >> 8*(sizeof(void*) - headSize), end - begin);
		return;
	}

	// st is passed by reference to this function. If we use it directly on each step the compiler will have to
	// update it in memory because of pointer aliasing assumptions. Copying it into a local var allows the
	// compiler to store it in a register. This saves some instructions and cycles
	typename Scanner::State state = st;

	if (headSize) {
		Impl::RunChunk(scanner, state, Impl::ToLittleEndian(*head) >> 8*(sizeof(void*) - headSize), headSize);
		++head;
	}

	state = Impl::AlignedRunner<Scanner>::RunAligned(scanner, state, head, tail);

	if (tailSize)
		Impl::RunChunk(scanner, state, Impl::ToLittleEndian(*tail), tailSize);

	st = state;
}

/// Runs two scanners through given memory range simultaneously.
/// This is several percent faster than running them independently.
template<class Scanner1, class Scanner2>
inline void Run(const Scanner1& scanner1, const Scanner2& scanner2, typename Scanner1::State& state1, typename Scanner2::State& state2, const char* begin, const char* end)
{
	typedef ScannerPair<Scanner1, Scanner2> Scanners;
	Scanners pair(scanner1, scanner2);
	typename Scanners::State states(state1, state2);
	Run(pair, states, begin, end);
	state1 = states.first;
	state2 = states.second;
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
template<class Scanner>
inline const char* LongestPrefix(const Scanner& scanner, const char* begin, const char* end)
{
	typename Scanner::State state;
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
template<class Scanner>
inline const char* LongestSuffix(const Scanner& scanner, const char* rbegin, const char* rend)
{
	typename Scanner::State state;
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
template<class Scanner>
inline const char* ShortestPrefix(const Scanner& scanner, const char* begin, const char* end)
{
	typename Scanner::State state;
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
template<class Scanner>
inline const char* ShortestSuffix(const Scanner& scanner, const char* rbegin, const char* rend)
{
	typename Scanner::State state;
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
Scanner MmappedScanner(const char* ptr, size_t size)
{
	MemoryInput inp(ptr, size);
	Scanner s;
	s.Load(&inp);
	return s;
}

}

#endif
