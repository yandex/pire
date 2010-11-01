/*
 * defs.h -- common Pire definitions.
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


#ifndef PIRE_DEFS_H
#define PIRE_DEFS_H

namespace Pire {

#ifdef PIRE_DEBUG
#define PIRE_IFDEBUG(x) x
#else
#define PIRE_IFDEBUG(x)
#endif

	typedef unsigned short Char;
	
	namespace SpecialChar {
	enum {
		Epsilon = 257,
		BeginMark = 258,
		EndMark = 259,

		// Actual size of input alphabet
		MaxCharUnaligned = 260,

		// Size of letter transition tables, must be a multiple of the machine word size
		MaxChar = (MaxCharUnaligned + (sizeof(void*)-1)) & ~(sizeof(void*)-1)
	};
	}
	
	using namespace SpecialChar;

	namespace Impl {
#ifndef WORDS_BIGENDIAN
		inline size_t ToLittleEndian(size_t val) { return val; }
#else
#error TODO: Please implement Pire::Impl::ToLittleEndian()
#endif
	}
}

#endif
