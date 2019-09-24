/*
 * half_final_fsm.cpp -- an implementation of CreateHalfFinalFsm
 *
 * Copyright (c) 2019 YANDEX LLC, Philipp Gribov <grphil@mail.ru>
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

#include "half_final_fsm.h"

namespace Pire {
	Fsm CreateHalfFinalFsm(Fsm fsm, bool addFinalTransitions, bool count) {
		bool allowHalfFinals = true;
		if (!fsm.IsDetermined()) {
			fsm.RemoveEpsilons();
		}
		for (size_t state = 0; state < fsm.Size(); ++state) {
			if (fsm.IsFinal(state)) {
				for (const auto& let : fsm.Letters()) {
					bool hasFinalTransition = fsm.Destinations(state, let.first).empty();
					for (const auto& to : fsm.Destinations(state, let.first)) {
						if (fsm.IsFinal(to)) {
							hasFinalTransition = true;
						}
					}
					allowHalfFinals = allowHalfFinals && hasFinalTransition;
				}
			}
		}
		fsm.Unsparse();
		if (!allowHalfFinals && !count) {
			const auto newFinal = fsm.Size();
			fsm.Resize(newFinal + 1);
			for (unsigned letter = 0; letter < MaxChar; ++letter) {
				if (letter != Epsilon) {
					fsm.Connect(newFinal, newFinal, letter);
				}
			}
			for (size_t state = 0; state < fsm.Size(); ++state) {
				bool hasFinalTransitions = false;
				for (const auto& to : fsm.Destinations(state, EndMark)) {
					if (fsm.IsFinal(to)) {
						hasFinalTransitions = true;
						break;
					}
				}
				if (hasFinalTransitions) {
					Fsm::StatesSet destinations = fsm.Destinations(state, EndMark);
					for (const auto& to : destinations) {
						fsm.Disconnect(state, to, EndMark);
					}
					fsm.Connect(state, newFinal, EndMark);
				}
			}
			fsm.ClearFinal();
			fsm.SetFinal(newFinal, true);
		}
		for (size_t state = 0; state != fsm.Size(); ++state) {
			if (fsm.IsFinal(state)) {
				for (unsigned letter = 0; letter < MaxChar; ++letter) {
					Fsm::StatesSet destinations = fsm.Destinations(state, letter);
					for (const auto& to : destinations) {
						fsm.Disconnect(state, to, letter);
					}
					if (addFinalTransitions) {
						fsm.Connect(state, fsm.Initial(), letter);
					}
				}
			}
		}
		fsm.Sparse();
		if (fsm.IsDetermined()) {
			fsm.Minimize();
		}
		return fsm;
	}
}
