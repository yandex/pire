#ifndef PIRE_EXTRA_COUNT_H
#define PIRE_EXTRA_COUNT_H

#include "../scanners/loaded.h"

namespace Pire {
class Fsm;
	
template<class T>
class ScannerGlueCommon;
namespace Impl {
	class CountingScannerGlueTask;
};

class CountingScanner: public LoadedScanner {
public:
	enum {
		IncrementAction = 1,
		ResetAction = 2,
	
		FinalFlag = 0,
		DeadFlag = 1,
		Matched = 2
	};

	static const size_t OPTIMAL_RE_COUNT = 4;

	class State {
	public:
		size_t Result(int i) const { return ymax(m_current[i], m_total[i]); }
	private:
		InternalState m_state;
		ui32 m_current[OPTIMAL_RE_COUNT];
		ui32 m_total[OPTIMAL_RE_COUNT];
		size_t m_updatedMask;
		friend class CountingScanner;

#ifdef PIRE_DEBUG
		friend yostream& operator << (yostream& s, const State& state)
		{
			s << state.m_state << " ( ";
			for (size_t i = 0; i < OPTIMAL_RE_COUNT; ++i)
				s << state.m_current[i] << '/' << state.m_total[i] << ' ';
			return s << ')';
		}
#endif
	};

	static CountingScanner Glue(const CountingScanner& a, const CountingScanner& b, size_t maxSize = 0);

	void Initialize(State& state) const
	{
		state.m_state = m.initial;
		memset(&state.m_current, 0, sizeof(state.m_current));
		memset(&state.m_total, 0, sizeof(state.m_total));
		state.m_updatedMask = 0;
	}

	void TakeAction(State& s, Action a) const
	{
		if (a & IncrementMask)
			PerformIncrement(s, a);
		if (a & ResetMask)
			PerformReset(s, a);
	}

	bool CanStop(const State&) const { return false; }

	Action Next(State& s, Char c) const
	{
		return NextTranslated(s, m_letters[c]);
	}

	Action Next(const State& current, State& n, Char c) const
	{
		n = current;
		return Next(n, c);
	}

	bool Final(const State& /*state*/) const { return false; }

	CountingScanner() {}
	CountingScanner(const CountingScanner& s): LoadedScanner(s) {}
	CountingScanner(const Fsm& re, const Fsm& sep);
	
	void Swap(CountingScanner& s) { LoadedScanner::Swap(s); }
	CountingScanner& operator = (const CountingScanner& s) { CountingScanner(s).Swap(*this); return *this; }

#ifdef PIRE_DEBUG
	const State& StateIndex(const State& s) const { return s; }
#endif

private:
	using LoadedScanner::Init;
	void PerformIncrement(State& s, Action mask) const
	{
		if (mask) {
			if (mask & 1) ++s.m_current[0];
			if (mask & 2) ++s.m_current[1];
			if (mask & 4) ++s.m_current[2];
			if (mask & 8) ++s.m_current[3];
			s.m_updatedMask |= ((size_t)mask) << MAX_RE_COUNT;
		}

	}

	void Reset(State &s, size_t i) const
	{
		if (s.m_current[i]) {
			s.m_total[i] = ymax(s.m_total[i], s.m_current[i]);
			s.m_current[i] = 0;
		}
	}

	void PerformReset(State& s, Action mask) const
	{
		mask &= s.m_updatedMask;
		if (mask) {
			if (mask & (1 << MAX_RE_COUNT)) Reset(s, 0);
			if (mask & (2 << MAX_RE_COUNT)) Reset(s, 1);
			if (mask & (4 << MAX_RE_COUNT)) Reset(s, 2);
			if (mask & (8 << MAX_RE_COUNT)) Reset(s, 3);
			s.m_updatedMask &= ~mask;
		}
	}

	Action NextTranslated(State& s, Char c) const
	{
		Transition x = reinterpret_cast<const Transition*>(s.m_state)[c];
		s.m_state += SignExtend(x /*& ShiftMask*/);

		return x >> ActionShift;
	}

	void Next(InternalState& s, Char c) const
	{
		Transition x = reinterpret_cast<const Transition*>(s)[m_letters[c]];
		s += SignExtend(x);
	}

	Action RemapAction(Action action)
	{
		if (action == (Matched | DeadFlag))
			return 1;
		else if (action == DeadFlag)
			return 1 << MAX_RE_COUNT;
		else
			return 0;
	}
	
	typedef LoadedScanner::InternalState InternalState;
	friend void BuildScanner<CountingScanner>(const Fsm&, CountingScanner&);
	friend class ScannerGlueCommon<CountingScanner>;
	friend class Impl::CountingScannerGlueTask;
};

}


#endif
