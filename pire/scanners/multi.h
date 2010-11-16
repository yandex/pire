/*
 * multi.h -- definition of the Scanner
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


#ifndef PIRE_SCANNERS_MULTI_H
#define PIRE_SCANNERS_MULTI_H

#include <string.h>
#include "common.h"
#include "../stub/stl.h"
#include "../fsm.h"
#include "../partition.h"
#include "../run.h"
#include "../static_assert.h"
#include "../stub/saveload.h"
#include "../stub/lexical_cast.h"

namespace Pire {

namespace Impl {

	inline static ssize_t SignExtend(i32 i) { return i; }
	template<class T>
	class ScannerGlueCommon;

	template<class T>
	class ScannerGlueTask;

	// This strategy allows to mmap() saved representation of a scanner. This is achieved by
	// storing shifts instead of addresses in the transition table.
	struct Relocatable {
		static const size_t Signature = 1;
		// Please note that Transition size is hardcoded as 32 bits.
		// This limits size of transition table to 4G, but compresses
		// it twice compared to 64-bit transitions. In future Transition
		// can be made a template parameter if this is a concern.
		typedef ui32 Transition;

		typedef const void* RetvalForMmap;

		static size_t Go(size_t state, Transition shift) { return state + SignExtend(shift); }
		static Transition Diff(size_t from, size_t to) { return static_cast<Transition>(to - from); }
	};

	// With this strategy the transition table stores addresses. This makes the scanner faster
	// compared to mmap()-ed
	struct Nonrelocatable {
		static const size_t Signature = 2;
		typedef size_t Transition;

		// Generates a compile-time error if Scanner<Nonrelocatable>::Mmap()
		// (which is unsupported) is mistakenly called
		typedef struct {} RetvalForMmap;

		static size_t Go(size_t /*state*/, Transition shift) { return shift; }
		static Transition Diff(size_t /*from*/, size_t to) { return to; }
	};

	/// Some properties of the particular state.
	struct ScannerRowHeader {
		enum { ExitMaskCount = 2 };

		/// If this state loops for all letters except particular set
		/// (common thing when matching something like /.*[Aa]/),
		/// each ExitMask contains that letter in each byte of size_t.
		///
		/// These masks are most commonly used for fast forwarding through parts
		/// of the string matching /.*/ somewhere in the middle regexp.
		///
		/// If bytes of mask hold different values, the mask is invalid
		/// and will not be used.
		Word ExitMasks[ExitMaskCount];

		enum {
			NO_SHORTCUT_MASK = 1, // the state doensn't have shortcuts
			NO_EXIT_MASK  =    2  // the state has only transtions to itself
		};

		size_t Flags; ///< Holds FinalFlag, DeadFlag, etc...

		ScannerRowHeader(): Flags(0)
		{
			for (size_t i = 0; i < ExitMaskCount; ++i)
				ExitMasks[i] = FromShort(NO_SHORTCUT_MASK);
		}
	};

// Scanner implementation parametrized by transition table representation strategy
template<class Relocation>
class Scanner {
protected:
	enum {
		 FinalFlag = 1,
		 DeadFlag  = 2,
		 Flags = FinalFlag | DeadFlag
	};

	static const size_t End = static_cast<size_t>(-1);

public:
	typedef typename Relocation::Transition Transition;

	typedef ui16		Letter;
	typedef ui32		Action;
	typedef ui8		Tag;

	enum ShortcutAction { Look, Shortcut, Exit };

	Scanner()
		: m_buffer(0)
		, m_finalEnd(0)
	{
		m.relocationSignature = Relocation::Signature;
		m.statesCount = 0;
		m.lettersCount = 0;
		m.regexpsCount = 0;
		m.finalTableSize = 0;
	}
	explicit Scanner(Fsm& fsm)
	{
		fsm.Canonize();
		Init(fsm.Size(), fsm.Letters(), fsm.Finals().size(), fsm.Initial(), 1);
		BuildScanner(fsm, *this);
	}


	size_t Size() const {
		return m.statesCount;
	}
	bool Empty() const {
		return !Size();
	}

	typedef size_t State;

	size_t RegexpsCount() const {
		return m.regexpsCount;
	}
	size_t LettersCount() const {
		return m.lettersCount;
	}

	/// Checks whether specified state is in any of the final sets
	bool Final(const State& state) const { return (Header(state).Flags & FinalFlag) != 0; }

	/// Checks whether specified state is 'dead' (i.e. scanner will never
	/// reach any final state from current one)
	bool Dead(const State& state) const { return (Header(state).Flags & DeadFlag) != 0; }

	ypair<const size_t*, const size_t*> AcceptedRegexps(const State& state) const
	{
		size_t idx = (state - reinterpret_cast<size_t>(m_transitions)) /
			(RowSize() * sizeof(Transition));
		const size_t* b = m_final + m_finalIndex[idx];
		const size_t* e = b;
		while (*e != End)
			++e;
		return ymake_pair(b, e);
	}

	/// Returns an initial state for this scanner
	void Initialize(State& state) const { state = m.initial; }

	/// Handles one characters
	Action Next(State& state, Char c) const
	{
		state = Relocation::Go(state, reinterpret_cast<const Transition*>(state)[m_letters[c]]);
		return 0;
	}

	void TakeAction(State&, Action) const {}

	Scanner(const Scanner& s): m(s.m)
	{
		if (!s.m_buffer) {
			// Empty or mmap()-ed scanner, just copy pointers
			m_buffer = 0;
			m_letters = s.m_letters;
			m_final = s.m_final;
			m_finalIndex = s.m_finalIndex;
			m_transitions = s.m_transitions;
		} else {
			// In-memory scanner
			DeepCopy(s);
		}
	}

	template<class AnotherRelocation>
	Scanner(const Scanner<AnotherRelocation>& s)
	{
		DeepCopy(s);
	}

	void Swap(Scanner& s)
	{
		DoSwap(m_buffer, s.m_buffer);
		DoSwap(m.statesCount, s.m.statesCount);
		DoSwap(m.lettersCount, s.m.lettersCount);
		DoSwap(m.regexpsCount, s.m.regexpsCount);
		DoSwap(m.initial, s.m.initial);
		DoSwap(m_letters, s.m_letters);
		DoSwap(m.finalTableSize, s.m.finalTableSize);
		DoSwap(m_final, s.m_final);
		DoSwap(m_finalEnd, s.m_finalEnd);
		DoSwap(m_finalIndex, s.m_finalIndex);
		DoSwap(m_transitions, s.m_transitions);
	}

	Scanner& operator = (const Scanner& s) { Scanner(s).Swap(*this); return *this; }

	~Scanner()
	{
		delete[] m_buffer;
	}

	/*
	 * Constructs the scanner from mmap()-ed memory range, returning a pointer
	 * to unconsumed part of the buffer.
	 */
	typename Relocation::RetvalForMmap Mmap(const void* ptr, size_t size)
	{
		Impl::CheckAlign(ptr, sizeof(Word));
		Scanner s;

		const size_t* p = reinterpret_cast<const size_t*>(ptr);
		Impl::ValidateHeader(p, size, 1, sizeof(m));
		if (size < sizeof(s.m))
			throw Error("EOF reached while mapping Pire::Scanner");

		memcpy(&s.m, p, sizeof(s.m));
		if (s.m.relocationSignature != Relocation::Signature)
			throw Error("Type mismatch while mmapping Pire::Scanner");
		Impl::AdvancePtr(p, size, sizeof(s.m));
		Impl::AlignPtr(p, size);
		
		Settings required;
		const Settings* actual;
		Impl::MapPtr(actual, 1, p, size);
		if (required != *actual)
			throw Error("This scanner was compiled for an incompatible platform");

		Impl::AlignPtr(p, size);
		if (size < s.BufSize())
			throw Error("EOF reached while mapping NPire::Scanner");
		s.Markup(const_cast<size_t*>(p));
		s.m.initial += reinterpret_cast<size_t>(s.m_transitions);

		Swap(s);
		Impl::AdvancePtr(p, size, BufSize());
		return Impl::AlignPtr(p, size);
	}

	size_t StateIndex(State s) const
	{
		return (s - reinterpret_cast<size_t>(m_transitions)) / (RowSize() * sizeof(Transition));
	}

	static Scanner Glue(const Scanner& a, const Scanner& b, size_t maxSize = 0);

	// Returns the size of the memory buffer used (or required) by scanner.
	size_t BufSize() const
	{
		return AlignUp(
			MaxChar * sizeof(Letter)                           // Letters translation table
			+ m.finalTableSize * sizeof(size_t)                // Final table
			+ m.statesCount * sizeof(size_t)                   // Final index
			+ sizeof(Word)                                     // Additional space, so transition table can be aligned
			+ RowSize() * m.statesCount * sizeof(Transition),  // Transitions table
		sizeof(Word));
	}

	void Save(yostream*) const;
	void Load(yistream*);

private:
	struct Settings {
		size_t ExitMaskCount;
		size_t ExitMaskSize;
		
		Settings():
			ExitMaskCount(ScannerRowHeader::ExitMaskCount),
			ExitMaskSize(sizeof(Word))
		{}
		bool operator == (const Settings& rhs) const { return ExitMaskCount == rhs.ExitMaskCount && ExitMaskSize == rhs.ExitMaskSize; }
		bool operator != (const Settings& rhs) const { return !(*this == rhs); }
	};
	
	struct Locals {
		ui32 statesCount;
		ui32 lettersCount;
		ui32 regexpsCount;
		size_t initial;
		ui32 finalTableSize;
		size_t relocationSignature;
	} m;

	char* m_buffer;
	Letter* m_letters;

	size_t* m_final;
	size_t* m_finalEnd;
	size_t* m_finalIndex;

	Transition* m_transitions;
	
	size_t RowSize() const { return AlignUp(m.lettersCount + HEADER_SIZE, sizeof(Word)); }

	static const size_t HEADER_SIZE = sizeof(ScannerRowHeader) / sizeof(Transition);
	PIRE_STATIC_ASSERT(sizeof(ScannerRowHeader) % sizeof(Transition) == 0);

	template<class Eq>
	void Init(size_t states, const Partition<Char, Eq>& letters, size_t finalStatesCount, size_t startState, size_t regexpsCount = 1)
	{
		m.relocationSignature = Relocation::Signature;
		m.statesCount = states;
		m.lettersCount = letters.Size();
		m.regexpsCount = regexpsCount;
		m.finalTableSize = finalStatesCount + states;

		m_buffer = new char[BufSize() + sizeof(Word)];
		memset(m_buffer, 0, BufSize() + sizeof(Word));
		Markup(AlignUp(m_buffer, sizeof(Word)));
		m_finalEnd = m_final;

		for (size_t i = 0; i != Size(); ++i)
			Header(IndexToState(i)) = ScannerRowHeader();

		m.initial = reinterpret_cast<size_t>(m_transitions + startState * RowSize());

		// Build letter translation table
		for (typename Partition<Char, Eq>::ConstIterator it = letters.Begin(), ie = letters.End(); it != ie; ++it)
			for (yvector<Char>::const_iterator it2 = it->second.second.begin(), ie2 = it->second.second.end(); it2 != ie2; ++it2)
				m_letters[*it2] = it->second.first + HEADER_SIZE;
	}

	/*
	 * Initializes pointers depending on buffer start, letters and states count
	 */
	void Markup(void* ptr)
	{
		Impl::CheckAlign(ptr, sizeof(Word));
		m_letters     = reinterpret_cast<Letter*>(ptr);
		m_final	      = reinterpret_cast<size_t*>(m_letters + MaxChar);
		m_finalIndex  = reinterpret_cast<size_t*>(m_final + m.finalTableSize);
		m_transitions = AlignUp(reinterpret_cast<Transition*>(m_finalIndex + m.statesCount), sizeof(Word));
	}

	ScannerRowHeader& Header(State s) { return *(ScannerRowHeader*) s; }
	const ScannerRowHeader& Header(State s) const { return *(const ScannerRowHeader*) s; }

	template<class AnotherRelocation>
	void DeepCopy(const Scanner<AnotherRelocation>& s)
	{
		// Ensure that specializations of Scanner across different Relocations do not touch its Locals
		YASSERT(sizeof(m) == sizeof(s.m));
		memcpy(&m, &s.m, sizeof(s.m));
		m.relocationSignature = Relocation::Signature;
		m_buffer = new char[BufSize() + sizeof(Word)];
		Markup(AlignUp(m_buffer, sizeof(Word)));

		memcpy(m_letters, s.m_letters, MaxChar * sizeof(*m_letters));
		memcpy(m_final, s.m_final, m.finalTableSize * sizeof(*m_final));
		memcpy(m_finalIndex, s.m_finalIndex, m.statesCount * sizeof(*m_finalIndex));

		m.initial = IndexToState(s.StateIndex(s.m.initial));
		m_finalEnd = m_final + (s.m_finalEnd - s.m_final);

		for (size_t st = 0; st != m.statesCount; ++st) {
			size_t oldstate = s.IndexToState(st);
			size_t newstate = IndexToState(st);
			Header(newstate) = s.Header(oldstate);
			const typename Scanner<AnotherRelocation>::Transition* os
				= reinterpret_cast<const typename Scanner<AnotherRelocation>::Transition*>(oldstate);
			Transition* ns = reinterpret_cast<Transition*>(newstate);

			for (size_t let = 0; let != LettersCount(); ++let)
				ns[let + HEADER_SIZE] = Relocation::Diff(newstate,
					IndexToState(s.StateIndex(AnotherRelocation::Go(oldstate, os[let + s.HEADER_SIZE]))));
		}
	}


	size_t IndexToState(size_t stateIndex) const
	{
		return reinterpret_cast<size_t>(m_transitions + stateIndex * RowSize());
	}

	void SetJump(size_t oldState, Char c, size_t newState, unsigned long /*payload*/ = 0)
	{
		YASSERT(m_buffer);
		YASSERT(oldState < m.statesCount);
		YASSERT(newState < m.statesCount);

		m_transitions[oldState * RowSize() + m_letters[c]]
			= Relocation::Diff(IndexToState(oldState), IndexToState(newState));
	}

	unsigned long RemapAction(unsigned long action) { return action; }

	void SetInitial(size_t state)
	{
		YASSERT(m_buffer);
		m.initial = IndexToState(state);
	}

	void SetTag(size_t state, size_t value)
	{
		YASSERT(m_buffer);
		Header(IndexToState(state)).Flags = value;
	}

	// Fill shortcut masks for all the states
	void BuildShortcuts()
	{
		YASSERT(m_buffer);

		// Build the mapping from letter classes to characters
		yvector< yvector<char> > letters(RowSize());
		for (unsigned ch = 0; ch != 1 << (sizeof(char)*8); ++ch)
			letters[m_letters[ch]].push_back(ch);

		// Loop through all states in the transition table and
		// check if it is possible to setup shortcuts
		for (size_t i = 0; i != Size(); ++i) {
			State st = IndexToState(i);
			Word* ptr = Header(st).ExitMasks;
			Word* end = ptr + ScannerRowHeader::ExitMaskCount;
			Word lastMask = FromShort(ScannerRowHeader::NO_EXIT_MASK);
			size_t let = HEADER_SIZE;
			for (; let != LettersCount() + HEADER_SIZE; ++let) {
				// Check if the transition is not the same state
				if (Relocation::Go(st, reinterpret_cast<const Transition*>(st)[let]) != st) {
					if (ptr + letters[let].size() > end)
						break;
					// For each character setup a mask
					for (yvector<char>::const_iterator chit = letters[let].begin(), chie = letters[let].end(); chit != chie; ++chit)
						*ptr++ = lastMask = FillWord(*chit);
				}
			}

			if (let != LettersCount() + HEADER_SIZE) {
				// Not enough space in ExitMasks, so reset all masks (which leads to bypassing the optimization)
				lastMask = FromShort(ScannerRowHeader::NO_SHORTCUT_MASK);
				ptr = Header(st).ExitMasks;
			}
			// Fill the rest of the shortcut masks with the last used mask
			while (ptr != end)
				*ptr++ = lastMask;
		}
	}

	// Fills final states table and builds shortcuts if possible
	void FinishBuild()
	{
		YASSERT(m_buffer);
		for (size_t state = 0; state != Size(); ++state) {
			m_finalIndex[state] = m_finalEnd - m_final;
			if (Header(IndexToState(state)).Flags & FinalFlag)
				*m_finalEnd++ = 0;
			*m_finalEnd++ = static_cast<size_t>(-1);
		}
		BuildShortcuts();
	}

	size_t AcceptedRegexpsCount(size_t idx) const
	{
		const size_t* b = m_final + m_finalIndex[idx];
		const size_t* e = b;
		while (*e != End)
			++e;
		return e - b;
	}

	friend void BuildScanner<Scanner>(const Fsm&, Scanner&);

	typedef State InternalState; // Needed for agglutination
	friend class ScannerGlueCommon<Scanner>;
	friend class ScannerGlueTask<Scanner>;
	template<class AnotherRelocation> friend class Scanner;
#ifndef PIRE_DEBUG
	friend struct AlignedRunner< Scanner<Relocation> >;
#endif
};

#ifndef PIRE_DEBUG


template<unsigned N>
struct MaskChecker {	
	MaskChecker(const Word* mend)
        : prev(mend-1)
        , mask(mend[-1])
        , runMe(GetSizeT(mend - 2) != GetSizeT(mend - 1)) {}
	
	inline bool Check(Word chunk) const { return prev.Check(chunk) && CmpBytes(mask, chunk); }
	
	inline const Word* DoRun(const Word* begin, const Word* end) const
	{
		for (; begin != end && Check(ToLittleEndian(*begin)); ++begin);
		return begin;
	}
	
	inline const Word* Run(const Word* begin, const Word* end) const
	{
		if (runMe)
			return DoRun(begin, end);
		else
			return prev.Run(begin, end);
	}
	
	MaskChecker<N-1> prev;
	Word mask;
	bool runMe;
};
	
template<>
struct MaskChecker<1> {
	MaskChecker(const Word* mend): mask(mend[-1]) {}
	
	inline bool Check(Word chunk) const { return CmpBytes(mask, chunk); }
	
	inline const Word* Run(const Word* begin, const Word* end) const
	{
		for (; begin != end && Check(ToLittleEndian(*begin)); ++begin);
		return begin;
	}
	
	Word mask;
};

// Processes a size_t-sized chunk
template<class Relocation>
static inline typename Scanner<Relocation>::State
ProcessChunk(const Scanner<Relocation>& scanner, typename Scanner<Relocation>::State state, size_t chunk)
{
	// Comparing loop variable to 0 saves inctructions becuase "sub 1, reg" will set zero flag
	// while in case of "for (i = 0; i < 8; ++i)" loop there will be an extra "cmp 8, reg" on each step
	for (unsigned i = sizeof(void*); i != 0; --i) {
		Step(scanner, state, chunk & 0xFF);
		chunk >>= 8;
	}
	return state;
}

// The purpose of this template is to produce a number of ProcessChunk() calls
// instead of writing for(...){ProcessChunk()} loop that GCC refuses to unroll.
// Manually unrolled code proves to be faster
template <class Relocation, unsigned Count>
struct MultiChunk {
	// Process Word-sized chunk which consist of >=1 size_t-sized chunks
	static inline typename Scanner<Relocation>::State
	Process(const Scanner<Relocation>& scanner, typename Scanner<Relocation>::State state, const size_t* p)
	{
		state = ProcessChunk(scanner, state, ToLittleEndian(*p++));
		state = MultiChunk<Relocation, Count-1>::Process(scanner, state, p);
		return state;
	}
};

template <class Relocation>
struct MultiChunk<Relocation, 0> {
	// Process Word-sized chunk which consist of >=1 size_t-sized chunks
	static inline typename Scanner<Relocation>::State
	Process(const Scanner<Relocation>&, typename Scanner<Relocation>::State state, const size_t*)
	{
		return state;
	}
};


template<class Relocation>
struct AlignedRunner< Scanner<Relocation> > {
private:
	// Compares the ExitMask[0] value without SSE reads which seems to be more optimal
	static inline bool CheckFirstMask(const Scanner<Relocation>& scanner, typename Scanner<Relocation>::State state, size_t val)
	{
		return (GetSizeT(scanner.Header(state).ExitMasks) == val);
	}
	
public:
	
	static typename Scanner<Relocation>::State
	RunAligned(const Scanner<Relocation>& scanner, typename Scanner<Relocation>::State state, const size_t* begin, const size_t* end)
	{
		if (CheckFirstMask(scanner, state, ScannerRowHeader::NO_EXIT_MASK) || begin == end)
			return state;
		const Word* head = AlignUp((const Word*) begin, sizeof(Word));
		const Word* tail = AlignDown((const Word*) end, sizeof(Word));
		for (; begin != (const size_t*) head && begin != end; ++begin)
			state = ProcessChunk(scanner, state, *begin);
		if (begin == end)
			return state;

		if (CheckFirstMask(scanner, state, ScannerRowHeader::NO_EXIT_MASK))
			return state;

		bool noShortcut = CheckFirstMask(scanner, state, ScannerRowHeader::NO_SHORTCUT_MASK);

		while (head != tail) {
			while (noShortcut) {
				state = MultiChunk<Relocation, sizeof(Word)/sizeof(size_t)>::Process(scanner, state, (const size_t*)head);
				++head;
				if (head == tail)
					break;
				noShortcut = CheckFirstMask(scanner, state, ScannerRowHeader::NO_SHORTCUT_MASK);
			}
			if (head == tail)
				break;

			if (CheckFirstMask(scanner, state, ScannerRowHeader::NO_EXIT_MASK))
    			return state;

			head = MaskChecker<ScannerRowHeader::ExitMaskCount>(scanner.Header(state).ExitMasks + ScannerRowHeader::ExitMaskCount)
				.Run(head, tail);
			if (head == tail)
				break;
			noShortcut = true;
		}
		
		for (size_t* p = (size_t*) tail; p != end; ++p)
			state = ProcessChunk(scanner, state, *p);
		return state;
	}
};

#endif

}


template<class Relocation>
struct StDumper<Impl::Scanner<Relocation> > {
	StDumper(const Impl::Scanner<Relocation>& sc, typename Impl::Scanner<Relocation>::State st): m_sc(&sc), m_st(st) {}

	void Dump(yostream& stream) const
	{
		stream << m_sc->StateIndex(m_st);
		if (m_sc->Final(m_st))
			stream << " [final]";
		if (m_sc->Dead(m_st))
			stream << " [dead]";
	}
private:
	const Impl::Scanner<Relocation>* m_sc;
	typename Impl::Scanner<Relocation>::State m_st;
};


/**
 * A compiled multiregexp.
 * Can only find out whether a string matches the regexps or not,
 * but takes O( str.length() ) time.
 *
 * In addition, multiple scanners can be agglutinated together,
 * producting a scanner which can be used for checking
 * strings against several regexps in a single pass.
 */
typedef Impl::Scanner<Impl::Relocatable> Scanner;

/**
 * Same as above, but does not allow relocation or mmap()-ing.
 * On the other hand, runs almost twice as fast as the Scanner.
 */
typedef Impl::Scanner<Impl::Nonrelocatable> NonrelocScanner;

}

namespace std {
	inline void swap(Pire::Scanner& a, Pire::Scanner b) {
		a.Swap(b);
	}

	inline void swap(Pire::NonrelocScanner& a, Pire::NonrelocScanner b) {
		a.Swap(b);
	}
}


#endif
