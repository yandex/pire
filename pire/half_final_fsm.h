/*
 * half_final_fsm.h -- function for creating fsm with half-final states, which means
 *                      that it matches regexp if it passes through transitional state
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

#include "fsm.h"
#include "defs.h"

namespace Pire {
	Fsm CreateHalfFinalFsm(Fsm fsm, bool addFinalTransitions = false, bool count = false);
}
