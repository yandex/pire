#ifndef PIRE_EXTRA_CAPTURE_H
#define PIRE_EXTRA_CAPTURE_H


#include "../scanners/loaded.h"
#include "../fsm.h"
#include "../re_lexer.h"

namespace Pire {

/**
* A capturing scanner.
* Requires source FSM to be deterministic, matches input string
* against a single regexp (taking O(strlen(str)) time) and
* captures a substring between a single pair of parentheses.
*
* Requires regexp pattern to satisfy certain conditions
* (I still do not know exactly what they are :) )
*/
class CapturingScanner: public LoadedScanner {
public:
	static const ui8 FinalFlag = 1;

	enum { NoAction = 0};

	static const Action BeginCapture =    0x00000001;
	static const Action EndCapture =      0x00000002;

	class State {
	public:
		bool Captured() const { return (m_begin != npos) && (m_end != npos); }
		ui64 Begin() const { return m_begin; }
		ui64 End() const { return m_end; }
	private:
		static const ui64 npos = static_cast<ui64>(-1);
		ui64 m_state;
		ui64 m_begin;
		ui64 m_end;
		ui64 m_counter;
		friend class CapturingScanner;

#ifdef PIRE_DEBUG
		friend std::ostream& operator << (std::ostream& s, const State& state)
		{
			s << state.m_state;
			if (state.m_begin != State::npos || state.m_end != npos) {
				s << " [";
				if (state.m_begin != State::npos)
					s << 'b';
				if (state.m_end != State::npos)
					s << 'e';
				s << "]";
			}
			return s;
		}
#endif
	};

	void Initialize(State& state) const
	{
		state.m_state = m.initial;
		state.m_begin = state.m_end = State::npos;
		state.m_counter = 0;
	}

	void TakeAction(State& s, Action a) const
	{
		if ((a & BeginCapture) && !s.Captured())
			s.m_begin = s.m_counter - 1;
		else if ((a & EndCapture) && !s.Captured())
			s.m_end = s.m_counter - 1;
	}

	Action Next(State& s, Char c) const
	{
		return NextTranslated(s, m_letters[c]);
	}

	Action Next(const State& current, State& n, Char c) const
	{
		n = current;
		return Next(n, c);
	}

	bool CanStop(const State& s) const
	{
		return Final(s);
	}

	bool Final(const State& s) const { return m_tags[(reinterpret_cast<Transition*>(s.m_state) - m_jumps) / m.lettersCount] & FinalFlag; }

	CapturingScanner() {}
	CapturingScanner(const CapturingScanner& s): LoadedScanner(s) {}
	explicit CapturingScanner(Fsm& fsm)
	{
		fsm.Canonize();
		Init(fsm.Size(), fsm.Letters(), fsm.Initial());
		BuildScanner(fsm, *this);
	}
	
	void Swap(CapturingScanner& s) { LoadedScanner::Swap(s); }
	CapturingScanner& operator = (const CapturingScanner& s) { CapturingScanner(s).Swap(*this); return *this; }

#ifdef PIRE_DEBUG
	const State& StateIndex(const State& s) const { return s; }
#endif

private:

	Action NextTranslated(State& s, unsigned char c) const
	{
		Transition x = reinterpret_cast<const Transition*>(s.m_state)[c];
		s.m_state += SignExtend(x);
		++s.m_counter;

		return x >> ActionShift;
	}

	friend void BuildScanner<CapturingScanner>(const Fsm&, CapturingScanner&);
};
namespace Features {
	Feature* Capture(size_t pos);
}

}


#endif
