#include "re_scanner.h"
#include "count.h"
#include "fsm.h"
#include "../determine.h"
#include "../glue.h"

namespace Pire {

CountingScanner::CountingScanner(const Fsm& re, const Fsm& sep)
{
	Fsm res = re;
	res.Surround();
	Fsm sep_re = ((sep & ~res) /* | Fsm()*/) + re;
	sep_re.Determine();

	Fsm dup = sep_re;
	for (size_t i = 0; i < dup.Size(); ++i)
		dup.SetTag(i, Matched);
	size_t oldsize = sep_re.Size();
	sep_re.Import(dup);
	for (Fsm::FinalTable::iterator i = sep_re.Finals().begin(), ie = sep_re.Finals().end(); i != ie; ++i)
		if (*i < oldsize)
			sep_re.Connect(*i, oldsize + *i);

	Fsm dot;
	dot.AppendDot();
	sep_re |= dot;

	// Make a full Cartesian product of two sep_res
	sep_re.Determine();
	sep_re.Unsparse();
	std::set<size_t> dead = sep_re.DeadStates();

	PIRE_IFDEBUG(std::clog << "=== Original FSM ===" << std::endl << sep_re << ">>> " << sep_re.Size() << " states, dead: [" << Join(dead.begin(), dead.end(), ", ") << "]" << std::endl);

	Fsm sq;

	typedef std::pair<size_t, size_t> NewState;
	std::vector<NewState> states;
	std::map<NewState, size_t> invstates;

	states.push_back(NewState(sep_re.Initial(), sep_re.Initial()));
	invstates.insert(std::make_pair(states.back(), states.size() - 1));

	// TODO: this loop reminds me a general determination task...
	for (size_t curstate = 0; curstate < states.size(); ++curstate) {

		unsigned long tag = sep_re.Tag(states[curstate].first);
		if (tag)
			sq.SetTag(curstate, tag);
		sq.SetFinal(curstate, sep_re.IsFinal(states[curstate].first));

		PIRE_IFDEBUG(std::clog << "State " << curstate << " = (" << states[curstate].first << ", " << states[curstate].second << ")" << std::endl);
		for (Fsm::LettersTbl::ConstIterator lit = sep_re.Letters().Begin(), lie = sep_re.Letters().End(); lit != lie; ++lit) {

			unsigned char letter = static_cast<unsigned char>(lit->first);

			const Fsm::StatesSet& mr = sep_re.Destinations(states[curstate].first, letter);
			const Fsm::StatesSet& br = sep_re.Destinations(states[curstate].second, letter);

			if (mr.size() != 1)
				assert(!"Wrong transition size for main");
			if (br.size() != 1)
				assert(!"Wrong transition size for backup");

			NewState ns(*mr.begin(), *br.begin());
			NewState savedNs = ns;
			unsigned long outputs = 0;

			PIRE_IFDEBUG(std::string dbgout);
			if (dead.find(ns.first) != dead.end()) {
				PIRE_IFDEBUG(dbgout = ((sep_re.Tag(ns.first) & Matched) ? ", ++cur" : ", max <- cur"));
				outputs = DeadFlag | (sep_re.Tag(ns.first) & Matched);
				ns.first = ns.second;
			}
			if (sep_re.IsFinal(ns.first) || (sep_re.IsFinal(ns.second) && !(sep_re.Tag(ns.first) & Matched)))
				ns.second = sep_re.Initial();

			PIRE_IFDEBUG(if (ns != savedNs) std::clog << "Diverted transition to (" << savedNs.first << ", " << savedNs.second << ") on " << (char) letter << " to (" << ns.first << ", " << ns.second << ")" << dbgout << std::endl);

			std::map<NewState, size_t>::iterator nsi = invstates.find(ns);
			if (nsi == invstates.end()) {
				PIRE_IFDEBUG(std::clog << "New state " << states.size() << " = (" << ns.first << ", " << ns.second << ")" << std::endl);
				states.push_back(ns);
				nsi = invstates.insert(std::make_pair(states.back(), states.size() - 1)).first;
				sq.Resize(states.size());
			}

			for (std::vector<Char>::const_iterator li = lit->second.second.begin(), le = lit->second.second.end(); li != le; ++li)
			sq.Connect(curstate, nsi->second, *li);
			if (outputs)
				sq.SetOutput(curstate, nsi->second, outputs);
		}
	}

	sq.Determine();

	PIRE_IFDEBUG(std::clog << "=== FSM ===" << std::endl << sq << std::endl);
	Init(sq.Size(), sq.Letters(), sq.Initial(), 1);
	BuildScanner(sq, *this);
}


namespace Impl {

class CountingScannerGlueTask: public ScannerGlueCommon<CountingScanner> {
public:
	typedef std::map<State, size_t> InvStates;
	
	CountingScannerGlueTask(const CountingScanner& lhs, const CountingScanner& rhs)
		: ScannerGlueCommon<CountingScanner>(lhs, rhs, LettersEquality<CountingScanner>(lhs.m_letters, rhs.m_letters))
	{
	}
	
	void AcceptStates(const std::vector<State>& states)
	{
		States = states;
		SetSc(new CountingScanner);
		Sc().Init(states.size(), Letters(), 0, Lhs().RegexpsCount() + Rhs().RegexpsCount());
		
		for (size_t i = 0; i < states.size(); ++i)
			Sc().SetTag(i, Lhs().m_tags[Lhs().StateIdx(states[i].first)] | (Rhs().m_tags[Rhs().StateIdx(states[i].second)] << 3));
	}
	
	void Connect(size_t from, size_t to, Char letter)
	{
		Sc().SetJump(from, letter, to,
			Action(Lhs(), States[from].first, letter) | (Action(Rhs(), States[from].second, letter) << Lhs().RegexpsCount()));
	}
			
private:
	std::vector<State> States;
	CountingScanner::Action Action(const CountingScanner& sc, CountingScanner::InternalState state, Char letter) const
	{
		return sc.m_actions[sc.StateIdx(state) * sc.m.lettersCount + sc.m_letters[letter]];
	}
};

}
	
CountingScanner CountingScanner::Glue(const CountingScanner& lhs, const CountingScanner& rhs, size_t maxSize /* = 0 */)
{
	static const size_t DefMaxSize = 250000;    
	Impl::CountingScannerGlueTask task(lhs, rhs);
	return Impl::Determine(task, maxSize ? maxSize : DefMaxSize);
}

}
