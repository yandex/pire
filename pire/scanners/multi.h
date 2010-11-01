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


#include "../stub/stl.h"
#include "../fsm.h"
#include "../partition.h"
#include "../static_assert.h"
#include "../stub/saveload.h"

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
		/// If this state loops for all letters except particular one,
		/// ExitMask contains that letter in each byte.
		/// If bytes of mask hold different values, the mask is invalid
		/// and will not be used.
		size_t ExitMask;

		size_t Flags; ///< Holds FinalFlag, DeadFlag, etc...

		ScannerRowHeader(): ExitMask(1), Flags(0) {}		
	};

// Scanner implementation parametrized by transition table representation strategy 
template<class Relocation>
class Scanner {
protected:
	enum {
		 FinalFlag = 1,
		 DeadFlag  = 2,
		 Flags = FinalFlag | DeadFlag,
		 TagSet = 0x80
	};

	static const size_t End = static_cast<size_t>(-1);

public:
	typedef typename Relocation::Transition Transition;
		
	typedef ui16		Letter;
	typedef ui32		Action;
	typedef ui8		 Tag;

	Scanner()
		: m_buffer(0)
		, m_finalEnd(0)
	{
		m.relocationSignature = Relocation::Signature;
		m.statesCount = 0;
		m.rowSize = 0;
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
		return m.rowSize - HEADER_SIZE;
	}

	/// Checks whether specified state is in any of the final sets
	bool Final(const State& state) const { return (Header(state).Flags & FinalFlag) != 0; }
	
	/// Checks whether specified state is 'dead' (i.e. scanner will never
	/// reach any final state from current one)
	bool Dead(const State& state) const { return (Header(state).Flags & DeadFlag) != 0; }

	ypair<const size_t*, const size_t*> AcceptedRegexps(const State& state) const
	{
		size_t idx = (state - reinterpret_cast<size_t>(m_transitions)) /
			(m.rowSize * sizeof(Transition));
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
		DoSwap(m.rowSize, s.m.rowSize);
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
		Impl::CheckAlign(ptr);
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
		return (s - reinterpret_cast<size_t>(m_transitions)) / (m.rowSize * sizeof(Transition));
	}

	static Scanner Glue(const Scanner& a, const Scanner& b, size_t maxSize = 0);

	// Returns the size of the memory buffer used (or required) by scanner.
	size_t BufSize() const
	{
		// TODO: need to fix according to letter table size
		return
			MaxChar * sizeof(Letter)                           // Letters translation table
			+ m.finalTableSize * sizeof(size_t)                // Final table
			+ m.statesCount * sizeof(size_t)                   // Final index
			+ m.rowSize * m.statesCount * sizeof(Transition);  // Transitions table
	}

	void Save(yostream*) const;
	void Load(yistream*);

private:
	struct Locals {
		ui32 statesCount;
		ui32 rowSize;
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
	
	static const size_t HEADER_SIZE = sizeof(ScannerRowHeader) / sizeof(Transition);
	PIRE_STATIC_ASSERT(sizeof(ScannerRowHeader) % sizeof(Transition) == 0);
	
	template<class Eq>
	void Init(size_t states, const Partition<Char, Eq>& letters, size_t finalStatesCount, size_t startState, size_t regexpsCount = 1)
	{
		m.relocationSignature = Relocation::Signature;
		m.statesCount = states;
		m.rowSize = letters.Size() + HEADER_SIZE;
		m.regexpsCount = regexpsCount;
		m.finalTableSize = finalStatesCount + states;

		m_buffer = new char[BufSize()];
		memset(m_buffer, 0, BufSize());
		Markup(m_buffer);
		m_finalEnd = m_final;
		
		for (size_t i = 0; i != Size(); ++i)
			Header(IndexToState(i)) = ScannerRowHeader();

		m.initial = reinterpret_cast<size_t>(m_transitions + startState * m.rowSize);

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
		m_letters	 = reinterpret_cast<Letter*>(ptr);
		m_final	   = reinterpret_cast<size_t*>(m_letters + MaxChar);
		m_finalIndex  = reinterpret_cast<size_t*>(m_final + m.finalTableSize);
		m_transitions = reinterpret_cast<Transition*>(m_finalIndex + m.statesCount);
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
		m_buffer = new char[BufSize()];
		Markup(m_buffer);
		
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
		
			for (size_t let = HEADER_SIZE; let != m.rowSize; ++let)
				ns[let] = Relocation::Diff(newstate, IndexToState(s.StateIndex(AnotherRelocation::Go(oldstate, os[let]))));
		}
	}

	
	size_t IndexToState(size_t stateIndex) const
	{
		return reinterpret_cast<size_t>(m_transitions + stateIndex * m.rowSize);
	}


	void SetJump(size_t oldState, Char c, size_t newState, unsigned long /*payload*/ = 0)
	{
		YASSERT(m_buffer);
		YASSERT(oldState < m.statesCount);
		YASSERT(newState < m.statesCount);
		
		m_transitions[oldState * m.rowSize + m_letters[c]]
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
		// FIXME: SetTag() needs to be called for _each_ state.
		YASSERT(m_buffer);
		size_t& tag = Header(IndexToState(state)).Flags;
		
		if (!(tag & TagSet)) {
			m_finalIndex[state] = m_finalEnd - m_final;
			if (value & FinalFlag)
				*m_finalEnd++ = 0;
			*m_finalEnd++ = static_cast<size_t>(-1);
		}
		tag = (value & Flags) | TagSet;
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
};

}

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

#endif
