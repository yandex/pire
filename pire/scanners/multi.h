#ifndef PIRE_SCANNERS_MULTI_H
#define PIRE_SCANNERS_MULTI_H


#include "../stl.h"
#include "../fsm.h"
#include "../partition.h"
#include "../stub/saveload.h"

namespace Pire {

	inline static ssize_t SignExtend(i32 i) { return i; }
	template<class T>
	class ScannerGlueCommon;
	
	namespace Impl { class ScannerGlueTask; }

/**
* A compiled multiregexp.
* Can only find out whether a string matches the regexps or not,
* but takes O( str.length() ) time.
*
* In addition, multiple scanners can be agglutinated together,
* producting a scanner which can be used for checking
* strings against several regexps in a single pass.
*/
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
	typedef ui32        Transition;
	// Please note that Transition size is hardcoded as 32 bits.
	// This limits size of transition table to 4G, but compresses
	// it twice compared to 64-bit transitions. In future Transition
	// can be made a template parameter if this is a concern.
	
	typedef ui16        Letter;
	typedef ui32        Action;
	typedef ui8         Tag;

	Scanner()
		: m_buffer(0)
		, m_finalEnd(0)
	{
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
	bool Final(const State& state) const { return (state & FinalFlag) != 0;	}
	
	/// Checks whether specified state is 'dead' (i.e. scanner will never
	/// reach any final state from current one)
	bool Dead(const State& state) const { return (state & DeadFlag) != 0; }

	ypair<const size_t*, const size_t*> AcceptedRegexps(const State& state) const
	{
		size_t idx = ((state & ~Flags) - reinterpret_cast<size_t>(m_transitions)) /
			(m.lettersCount * sizeof(Transition));
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
		state &= ~Flags;
		size_t letterClass = m_letters[c];
		ssize_t shift = SignExtend(reinterpret_cast<const Transition*>(state)[letterClass]);
		state += shift;

		return 0;
	}

	void TakeAction(State&, Action) const {}

	Scanner(const Scanner& s): m(s.m), m_tags(s.m_tags)
	{
		if (!s.m_buffer) {
			// Empty or mmap()-ed scanner, just copy pointers
			m_buffer = 0;
			m_letters = s.m_letters;
			m_final = s.m_final;
			m_finalIndex = s.m_finalIndex;
			m_transitions = s.m_transitions;
		} else {
			// In-memory scanner, perform deep copy
			m_buffer = new char[BufSize()];
			memcpy(m_buffer, s.m_buffer, BufSize());
			Markup(m_buffer);

			m.initial += (m_transitions - s.m_transitions) * sizeof(Transition);
			m_finalEnd = m_final + (s.m_finalEnd - s.m_final);
		}
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
		DoSwap(m_tags, s.m_tags);
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
	const void* Mmap(const void* ptr, size_t size)
	{
		Impl::CheckAlign(ptr);
		Scanner s;

		const size_t* p = reinterpret_cast<const size_t*>(ptr);
		Impl::ValidateHeader(p, size, 1, sizeof(m));
		if (size < sizeof(s.m))
			throw Error("EOF reached while mapping NPire::Scanner");

		memcpy(&s.m, p, sizeof(s.m));
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
		return ((s & ~Flags) - reinterpret_cast<size_t>(m_transitions)) / (m.lettersCount * sizeof(Transition));
	}

	static Scanner Glue(const Scanner& a, const Scanner& b, size_t maxSize = 0);

	// Returns the size of the memory buffer used (or required) by scanner.
	size_t BufSize() const
	{
		// TODO: need to fix according to letter table size
		return
			MaxChar * sizeof(Letter)                               // Letters translation table
			+ m.finalTableSize * sizeof(size_t)                    // Final table
			+ m.statesCount * sizeof(size_t)                       // Final index
			+ m.lettersCount * m.statesCount * sizeof(Transition); // Transitions table
	}

	void Save(yostream*) const;
	void Load(yistream*);

protected:
	struct Locals {
		ui32 statesCount;
		ui32 lettersCount;
		ui32 regexpsCount;
		size_t initial;
		ui32 finalTableSize;
	} m;

	char* m_buffer;
	Letter* m_letters;

	size_t* m_final;
	size_t* m_finalEnd;
	size_t* m_finalIndex;

	Transition* m_transitions;
	
	yvector<Tag> m_tags; // Not used during match. Actually tags are stored in lower bits of the state.

	template<class Eq>
	void Init(size_t states, const Partition<Char, Eq>& letters, size_t finalStatesCount, size_t startState, size_t regexpsCount = 1)
	{
		m.statesCount = states;
		m.lettersCount = letters.Size();
		m.regexpsCount = regexpsCount;
		m.finalTableSize = finalStatesCount + states;
		m_tags.resize(states, 0);

		m_buffer = new char[BufSize()];
		memset(m_buffer, 0, BufSize());
		Markup(m_buffer);
		m_finalEnd = m_final;

		m.initial = reinterpret_cast<size_t>(m_transitions + startState * m.lettersCount);

		// Build letter translation table
		for (typename Partition<Char, Eq>::ConstIterator it = letters.Begin(), ie = letters.End(); it != ie; ++it)
			for (yvector<Char>::const_iterator it2 = it->second.second.begin(), ie2 = it->second.second.end(); it2 != ie2; ++it2)
				m_letters[*it2] = it->second.first;
	}

	/*
	* Initializes pointers depending on buffer start, letters and states count
	*/
	void Markup(void* ptr)
	{
		m_letters     = reinterpret_cast<Letter*>(ptr);
		m_final       = reinterpret_cast<size_t*>(m_letters + MaxChar);
		m_finalIndex  = reinterpret_cast<size_t*>(m_final + m.finalTableSize);
		m_transitions = reinterpret_cast<Transition*>(m_finalIndex + m.statesCount);
	}

	void SetJump(size_t oldState, Char c, size_t newState, unsigned long /*payload*/ = 0)
	{
		assert(m_buffer);
		assert(oldState < m.statesCount);
		assert(newState < m.statesCount);
		m_transitions[oldState * m.lettersCount + m_letters[c]]
			= ((newState - oldState) * m.lettersCount * sizeof(Transition))
			| (m_tags[newState] & Flags);
	}

	unsigned long RemapAction(unsigned long action) { return action; }

	void SetInitial(size_t state)
	{
		assert(m_buffer);
		m.initial = reinterpret_cast<size_t>(m_transitions + state * m.lettersCount) | (m_tags[state] & Flags);
	}

	void SetTag(size_t state, size_t tag)
	{
		assert(m_buffer);
		
		if (!(m_tags[state] & TagSet)) {
			m_finalIndex[state] = m_finalEnd - m_final;
			if (tag & FinalFlag)
				*m_finalEnd++ = 0;
			*m_finalEnd++ = static_cast<size_t>(-1);
		}
		m_tags[state] = (tag & Flags) | TagSet;	

		if (((m.initial & ~Flags) - reinterpret_cast<size_t>(m_transitions)) / (m.lettersCount * sizeof(Transition)) == state)
			m.initial = (m.initial & ~Flags) | (tag & Flags);
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
	friend class Impl::ScannerGlueTask;
};


}


#endif
