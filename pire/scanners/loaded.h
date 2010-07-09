#ifndef PIRE_SCANNERS_LOADED_H
#define PIRE_SCANNERS_LOADED_H


#include <assert.h>
#include "../partition.h"
#include "../re_scanner.h"
#include "../stub/saveload.h"

#ifdef PIRE_DEBUG
#include <iostream>
#endif

namespace Pire {

/**
* A loaded scanner -- the deterministic scanner having actions
*                     associated with states and transitions
*
* Not a complete scanner itself (hence abstract), this class provides
* infrastructure for regexp-based algorithms (e.g. counts or captures),
* supporting major part of scanner construction, (de)serialization,
* mmap()-ing, etc.
*
* It is a good idea to override copy ctor, operator= and swap()
* in subclasses to avoid mixing different scanner types in these methods.
* Also please note that subclasses should not have any data members of thier own.
*/
class LoadedScanner {
public:
	typedef ui64        Transition;
	typedef ui8         Letter;
	typedef ui32        Action;
	typedef ui8         Tag;

	// Override in subclass, if neccessary
	enum { 
		FinalFlag = 0,
		DeadFlag  = 0
	};

	size_t Size() const { return m.statesCount; }

	LoadedScanner()
		: m_buffer(0)
		, m_letters(0)
		, m_jumps(0)
		, m_tags(0)
	{
		m.statesCount = 0;
		m.lettersCount = 0;
		m.initial = 0;
	}

	LoadedScanner(const LoadedScanner& s): m(s.m)
	{
		if (s.m_buffer) {
			m_buffer = malloc(BufSize());
			memcpy(m_buffer, s.m_buffer, BufSize());
			Markup(m_buffer);
			m.initial = (InternalState)m_jumps + (s.m.initial - (InternalState)s.m_jumps);
		} else {
			m_buffer = 0;
			m_letters = s.m_letters;
			m_jumps = s.m_jumps;
			m_tags = s.m_tags;
			m_actions = s.m_actions;
		}
	}

	size_t RegexpsCount() const {return m.regexpsCount;}

	bool Empty() const { return !Size(); }

	void Swap(LoadedScanner& s)
	{
		DoSwap(m_buffer, s.m_buffer);
		DoSwap(m.statesCount, s.m.statesCount);
		DoSwap(m.lettersCount, s.m.lettersCount);
		DoSwap(m.regexpsCount, s.m.regexpsCount);
		DoSwap(m.initial, s.m.initial);
		DoSwap(m_letters, s.m_letters);
		DoSwap(m_jumps, s.m_jumps);
		DoSwap(m_actions, s.m_actions);
		DoSwap(m_tags, s.m_tags);
	}

	LoadedScanner& operator = (const LoadedScanner& s) { LoadedScanner(s).Swap(*this); return *this; }

	const void* Mmap(const void* ptr, size_t size)
	{
		Impl::CheckAlign(ptr);
		LoadedScanner s;
		const size_t* p = reinterpret_cast<const size_t*>(ptr);
		Impl::ValidateHeader(p, size, 4, sizeof(s.m));

		Locals* locals;
		Impl::MapPtr(locals, 1, p, size);
		memcpy(&s.m, locals, sizeof(s.m));
		
		Impl::MapPtr(s.m_letters, MaxChar, p, size);
		Impl::MapPtr(s.m_jumps, s.m.statesCount * s.m.lettersCount, p, size);
		Impl::MapPtr(s.m_actions, s.m.statesCount * s.m.lettersCount, p, size);
		Impl::MapPtr(s.m_tags, s.m.statesCount, p, size);
		
		s.m.initial += reinterpret_cast<size_t>(s.m_jumps);
		Swap(s);

		return (const void*) p;
	}

	void Save(OutputStream*) const;
	void Load(InputStream*);

protected:

	typedef size_t InternalState;

	static const ui64 MAX_RE_COUNT        = 16;

	static const Action IncrementMask     = 0x0f;
	static const Action ResetMask         = 0x0f << MAX_RE_COUNT;
	static const Transition ShiftMask     = 0xffffffffull & ~(sizeof(Transition) - 1);
	static const int ActionShift          = 32;

	// TODO: maybe, put fields in private section and provide data accessors

	struct Locals {
		ui32 statesCount;
		ui32 lettersCount;
		ui32 regexpsCount;
		ui64 initial;
	} m;
	void* m_buffer;

	Letter* m_letters;
	Transition* m_jumps;
	Action* m_actions;
	Tag* m_tags;

	virtual ~LoadedScanner() // Cannot be instantiated
	{
		if (m_buffer)
			free(m_buffer);
	}

	size_t BufSize() const
	{
		return
			MaxChar * sizeof(*m_letters)
			+ m.lettersCount * m.statesCount * sizeof(*m_jumps)
			+ m.statesCount * sizeof(*m_tags)
			+ m.lettersCount * m.statesCount * sizeof(*m_actions)
			;
	}

	void Markup(void* buf)
	{
		m_letters = reinterpret_cast<Letter*>(buf);
		m_jumps   = reinterpret_cast<Transition*>(m_letters + MaxChar);
		m_actions = reinterpret_cast<Action*>(m_jumps + m.statesCount * m.lettersCount);
		m_tags    = reinterpret_cast<Tag*>(m_actions + m.statesCount * m.lettersCount);
	}

	template<class Eq>
	void Init(size_t states, const Partition<Char, Eq>& letters, size_t startState, size_t regexpsCount = 1)
	{
		m.statesCount = states;
		m.lettersCount = letters.Size();
		m.regexpsCount = regexpsCount;
		m_buffer = malloc(BufSize());
		memset(m_buffer, 0, BufSize());
		Markup(m_buffer);

		m.initial = reinterpret_cast<size_t>(m_jumps + startState * m.lettersCount);

		// Build letter translation table
		fill(m_letters, m_letters + sizeof(m_letters)/sizeof(*m_letters), 0);
		for (typename Partition<Char, Eq>::ConstIterator it = letters.Begin(), ie = letters.End(); it != ie; ++it)
			for (yvector<Char>::const_iterator it2 = it->second.second.begin(), ie2 = it->second.second.end(); it2 != ie2; ++it2)
				m_letters[*it2] = it->second.first;
	}

	template<class Eq>
	LoadedScanner(size_t states, const Partition<Char, Eq>& letters, size_t startState, size_t regexpsCount = 1)
	{
		Init(states, letters, startState, regexpsCount);
	}

	void SetJump(size_t oldState, Char c, size_t newState, Action action)
	{
		assert(m_buffer);
		assert(oldState < m.statesCount);
		assert(newState < m.statesCount);

		size_t shift = (newState - oldState) * m.lettersCount * sizeof(*m_jumps);

		m_jumps[oldState * m.lettersCount + m_letters[c]] =
			(shift & ShiftMask) |
			(((Transition) action) << ActionShift);
		m_actions[oldState * m.lettersCount + m_letters[c]] = action;
	}

	Action RemapAction(Action action) { return action; }

	void SetInitial(size_t state) { assert(m_buffer); m.initial = reinterpret_cast<size_t>(m_jumps + state * m.lettersCount); }
	void SetTag(size_t state, Tag tag) { assert(m_buffer); m_tags[state] = tag; }

	size_t StateIdx(InternalState s) const
	{
		return (reinterpret_cast<Transition*>(s) - m_jumps) / m.lettersCount;
	}

	i64 SignExtend(i32 i) const { return i; }

	friend class Fsm;
};

}


#endif
