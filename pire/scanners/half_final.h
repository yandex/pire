/*
 * half_final.h -- definition of the HalfFinalScanner
 *
 * Copyright (c) 2019, Philip Gribov <grphil@mail.ru>
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


#ifndef PIRE_SCANNERS_HALF_FINAL_H
#define PIRE_SCANNERS_HALF_FINAL_H

#include <string.h>
#include "common.h"
#include "multi.h"
#include "../fsm.h"
#include "../half_final_fsm.h"
#include "../stub/stl.h"

namespace Pire {

namespace Impl {


/*
 * A half final scanner -- the deterministic scanner having half-terminal states,
 * so it matches regexps in all terminal transitional states.
 *
 * The scanner can also count the number of substrings, that match each regexp. These substrings may intersect.
 *
 * Comparing it with scanner, it runs slower, but allows to glue significantly
 * larger number of scanners into one within the same size limits.
 *
 * The class is subclass of Scanner, having the same methods, but different state type.
 *
 * There are no restrictions for regexps and fsm's, for which it is built, but it
 * does not work properly if the matching text does not end with EndMark.
 *
 * For count to work correctly, the fsm should not be determined.
 */
template<typename Relocation, typename Shortcutting>
class HalfFinalScanner : public Scanner<Relocation, Shortcutting> {
public:
	typedef typename Impl::Scanner<Relocation, Shortcutting> Scanner;

	HalfFinalScanner() : Scanner() {}

	explicit HalfFinalScanner(const Fsm& fsm_, size_t distance = 0, bool count = false) {
		Fsm fsm = fsm_;
		if (distance) {
			fsm = CreateApproxFsm(fsm, distance);
		}
		if (fsm.IsDetermined()) {
			fsm = CreateHalfFinalFsm(fsm, true, count);
		} else {
			fsm = CreateHalfFinalFsm(fsm, false, count);
			fsm.Canonize();
		}
		Scanner::Init(fsm.Size(), fsm.Letters(), fsm.Finals().size(), fsm.Initial(), 1);
		BuildScanner(fsm, *this);
	}

	typedef typename Scanner::ScannerRowHeader ScannerRowHeader;
	typedef typename Scanner::Action Action;

	class State {
	public:
		typedef TVector<size_t>::const_iterator IdsIterator;

		void GetMatchedRegexpsIds() {
			MatchedRegexpsIds.clear();
			for (size_t i = 0; i < MatchedRegexps.size(); i++) {
				if (MatchedRegexps[i]) {
					MatchedRegexpsIds.push_back(i);
				}
			}
		}

		IdsIterator IdsBegin() const {
			return MatchedRegexpsIds.cbegin();
		}

		IdsIterator IdsEnd() const {
			return MatchedRegexpsIds.cend();
		}

		bool operator==(const State& other) const {
			return State == other.State && MatchedRegexps == other.MatchedRegexps;
		}

		bool operator!=(const State& other) const {
			return State != other.State || MatchedRegexps != other.MatchedRegexps;
		}

		size_t Result(size_t regexp_id) const {
			return MatchedRegexps[regexp_id];
		}

		void Save(yostream* s) const {
			SavePodType(s, Pire::Header(5, sizeof(size_t)));
			Impl::AlignSave(s, sizeof(Pire::Header));
			auto stateSizePair = ymake_pair(State, MatchedRegexps.size());
			SavePodType(s, stateSizePair);
			Impl::AlignSave(s, sizeof(ypair<size_t, size_t>));
			Y_ASSERT(0);
		}

		void Load(yistream* s) {
			Impl::ValidateHeader(s, 5, sizeof(size_t));
			ypair<size_t, size_t> stateSizePair;
			LoadPodType(s, stateSizePair);
			Impl::AlignLoad(s, sizeof(ypair<size_t, size_t>));
			State = stateSizePair.first;
			MatchedRegexps.clear();
			MatchedRegexps.resize(stateSizePair.second);
		}

	private:
		TVector<size_t> MatchedRegexpsIds;
		typename Scanner::State State;
		TVector<size_t> MatchedRegexps;

		friend class HalfFinalScanner<Relocation, Shortcutting>;
	};


	/// Checks whether specified state is in any of the final sets
	bool Final(const State& state) const { return Scanner::Final(state.State); }

	/// Checks whether specified state is 'dead' (i.e. scanner will never
	/// reach any final state from current one)
	bool Dead(const State& state) const { return Scanner::Dead(state.State); }

	typedef ypair<typename State::IdsIterator, typename State::IdsIterator> AcceptedRegexpsType;

	AcceptedRegexpsType AcceptedRegexps(State& state) const {
		state.GetMatchedRegexpsIds();
		return ymake_pair(state.IdsBegin(), state.IdsEnd());
	}

	/// Returns an initial state for this scanner
	void Initialize(State& state) const {
		state.State = Scanner::m.initial;
		state.MatchedRegexps.clear();
		state.MatchedRegexps.resize(Scanner::m.regexpsCount);
		TakeAction(state, 0);
	}

	Action NextTranslated(State& state, Char letter) const {
		return Scanner::NextTranslated(state.State, letter);
	}

	/// Handles one character
	Action Next(State& state, Char c) const {
		return Scanner::NextTranslated(state.State, Scanner::Translate(c));
	}

	void TakeAction(State& state, Action) const {
		if (Final(state)) {
			size_t idx = StateIndex(state);
			const size_t *it = Scanner::m_final + Scanner::m_finalIndex[idx];
			while (*it != Scanner::End) {
				state.MatchedRegexps[*it]++;
				++it;
			}
		}
	}

	HalfFinalScanner(const HalfFinalScanner& s) : Scanner(s) {}

	HalfFinalScanner(const Scanner& s) : Scanner(s) {}

	HalfFinalScanner(HalfFinalScanner&& s) : Scanner(s) {}

	HalfFinalScanner(Scanner&& s) : Scanner(s) {}

	template<class AnotherRelocation>
	HalfFinalScanner(const HalfFinalScanner<AnotherRelocation, Shortcutting>& s)
			: Scanner(s) {}

	template<class AnotherRelocation>
	HalfFinalScanner(const Impl::Scanner<AnotherRelocation, Shortcutting>& s) : Scanner(s) {}

	void Swap(HalfFinalScanner& s) {
		Scanner::Swap(s);
	}

	HalfFinalScanner& operator=(const HalfFinalScanner& s) {
		HalfFinalScanner(s).Swap(*this);
		return *this;
	}

	size_t StateIndex(const State& s) const {
		return Scanner::StateIndex(s.State);
	}

	/**
	 * Agglutinates two scanners together, producing a larger scanner.
	 * Checking a string against that scanner effectively checks them against both agglutinated regexps
	 * (detailed information about matched regexps can be obtained with AcceptedRegexps()).
	 *
	 * Returns default-constructed scanner in case of failure
	 * (consult Scanner::Empty() to find out whether the operation was successful).
	 */
	static HalfFinalScanner Glue(const HalfFinalScanner& a, const HalfFinalScanner& b, size_t maxSize = 0) {
		return Scanner::Glue(a, b, maxSize);
	}

	ScannerRowHeader& Header(const State& s) { return Scanner::Header(s.State); }

	const ScannerRowHeader& Header(const State& s) const { return Scanner::Header(s.State); }

private:
	template<class Scanner>
	friend void Pire::BuildScanner(const Fsm&, Scanner&);

	typedef State InternalState; // Needed for agglutination
};

}


typedef Impl::HalfFinalScanner<Impl::Relocatable, Impl::ExitMasks<2> > HalfFinalScanner;
typedef Impl::HalfFinalScanner<Impl::Relocatable, Impl::NoShortcuts> HalfFinalScannerNoMask;

/**
 * Same as above, but does not allow relocation or mmap()-ing.
 * On the other hand, runs faster than HalfFinal.
 */
typedef Impl::HalfFinalScanner<Impl::Nonrelocatable, Impl::ExitMasks<2> > NonrelocHalfFinalScanner;
typedef Impl::HalfFinalScanner<Impl::Nonrelocatable, Impl::NoShortcuts> NonrelocHalfFinalScannerNoMask;

}


namespace std {
	inline void swap(Pire::HalfFinalScanner& a, Pire::HalfFinalScanner& b) {
		a.Swap(b);
	}

	inline void swap(Pire::NonrelocHalfFinalScanner& a, Pire::NonrelocHalfFinalScanner& b) {
		a.Swap(b);
	}
}

#endif
