#ifndef PIRE_SCANNERS_SIMPLE_H
#define PIRE_SCANNERS_SIMPLE_H


#include "../stl.h"
#include "../stub/defaults.h"
#include "../stub/saveload.h"

namespace Pire {
class SimpleScanner {
private:
	static const size_t STATE_ROW_SIZE = MaxChar + 1; // All characters + 1 element to store final state flag

public:
	typedef ui64        Transition;
	typedef ui16        Letter;
	typedef ui32        Action;
	typedef ui8         Tag;

	SimpleScanner()
		: m_buffer(0)
	{
		m.statesCount = 0;
	}
	
	explicit SimpleScanner(Fsm& fsm);

	size_t Size() const { return m.statesCount; }
	bool Empty() const { return !Size(); }

	typedef size_t State;

	size_t RegexpsCount() const { return 1; }
	size_t LettersCount() const { return MaxChar; }

	/// Checks whether specified state is in any of the final sets
	bool Final(const State& state) const { return *(((const Transition*) state) - 1) != 0; }

	/// returns an initial state for this scanner
	void Initialize(State& state) const { state = m.initial; }

	/// Handles one characters
	Action Next(State& state, Char c) const
	{
		i64 shift = reinterpret_cast<const Transition*>(state)[c];
		state += shift;
		return 0;
	}

	bool TakeAction(State&, Action) const { return false; }

	SimpleScanner(const SimpleScanner& s): m(s.m)
	{
		if (!s.m_buffer) {
			// Empty or mmap()-ed scanner, just copy pointers
			m_buffer = 0;
			m_transitions = s.m_transitions;
		} else {
			// In-memory scanner, perform deep copy
			m_buffer = new char[BufSize()];
			memcpy(m_buffer, s.m_buffer, BufSize());
			Markup(m_buffer);

			m.initial += (m_transitions - s.m_transitions) * sizeof(Transition);
		}
	}

	void Swap(SimpleScanner& s)
	{
		DoSwap(m_buffer, s.m_buffer);
		DoSwap(m.statesCount, s.m.statesCount);
		DoSwap(m.initial, s.m.initial);
		DoSwap(m_transitions, s.m_transitions);
	}

	SimpleScanner& operator = (const SimpleScanner& s) { SimpleScanner(s).Swap(*this); return *this; }

	~SimpleScanner()
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
		SimpleScanner s;

		const size_t* p = reinterpret_cast<const size_t*>(ptr);
		Impl::ValidateHeader(p, size, 2, sizeof(m));
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

	ui64 StateIndex(State s) const
	{
		return (s - reinterpret_cast<size_t>(m_transitions)) / (STATE_ROW_SIZE * sizeof(Transition));
	}

	// Returns the size of the memory buffer used (or required) by scanner.
	size_t BufSize() const
	{
		return STATE_ROW_SIZE * m.statesCount * sizeof(Transition); // Transitions table
	}

	void Save(OutputStream*) const;
	void Load(InputStream*);

protected:
	struct Locals {
		ui32 statesCount;
		ui64 initial;
	} m;

	char* m_buffer;

	Transition* m_transitions;

	/*
	* Initializes pointers depending on buffer start, letters and states count
	*/
	void Markup(void* ptr)
	{
		m_transitions = reinterpret_cast<Transition*>(ptr);
	}

	void SetJump(size_t oldState, Char c, size_t newState, unsigned long /*payload*/ = 0)
	{
		assert(m_buffer);
		assert(oldState < m.statesCount);
		assert(newState < m.statesCount);
		m_transitions[oldState * STATE_ROW_SIZE + 1 + c]
			= (((newState - oldState) * STATE_ROW_SIZE) * sizeof(Transition));
	}

	unsigned long RemapAction(unsigned long action) { return action; }

	void SetInitial(size_t state)
	{
		assert(m_buffer);
		m.initial = reinterpret_cast<size_t>(m_transitions + state * STATE_ROW_SIZE + 1);
	}

	void SetTag(size_t state, size_t tag)
	{
		assert(m_buffer);
		m_transitions[state * STATE_ROW_SIZE] = tag;
	}

};
inline SimpleScanner::SimpleScanner(Fsm& fsm)
{
	fsm.Canonize();
	
	m.statesCount = fsm.Size();
	m_buffer = new char[BufSize()];
	memset(m_buffer, 0, BufSize());
	Markup(m_buffer);
	m.initial = reinterpret_cast<size_t>(m_transitions + fsm.Initial() * STATE_ROW_SIZE + 1);
	for (size_t state = 0; state < fsm.Size(); ++state)
		SetTag(state, fsm.Tag(state) | (fsm.IsFinal(state) ? 1 : 0));

	for (size_t from = 0; from != fsm.Size(); ++from)
		for (Fsm::LettersTbl::ConstIterator i = fsm.Letters().Begin(), ie = fsm.Letters().End(); i != ie; ++i) {
			const Fsm::StatesSet& tos = fsm.Destinations(from, i->first);
			if (tos.empty())
				continue;
			for (yvector<Char>::const_iterator l = i->second.second.begin(), le = i->second.second.end(); l != le; ++l)
				for (Fsm::StatesSet::const_iterator to = tos.begin(), toEnd = tos.end(); to != toEnd; ++to)
					SetJump(from, *l, *to, RemapAction(fsm.Output(from, *to)));
		}
}

	
}

#endif
