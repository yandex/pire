/*
 * Copyright (C) 2000-2010, Yandex
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser Public License
 * along with Pire.  If not, see <http://www.gnu.org/licenses>.
 */


#ifndef PIRE_STUB_DEFAULTS_H_INCLUDED
#define PIRE_STUB_DEFAULTS_H_INCLUDED

/*
 * Some C++ implementations require __STDC_CONSTANT_MACROS
 * and __STDC_LIMIT_MACROS to be defined before inclusion of
 * stdint.h to make it possible to use macroses like UINT32_MAX
 * or UINT32_C. See centos6, for example.
 */
#if defined(__cplusplus) && !defined(__STDC_CONSTANT_MACROS)
#define __STDC_CONSTANT_MACROS 1
#endif
#if defined(__cplusplus) && !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif
#include <stdint.h>
#include <stddef.h>

namespace Pire {
	typedef int8_t i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;
	typedef uint8_t ui8;
	typedef uint16_t ui16;
	typedef uint32_t ui32;
	typedef uint64_t ui64;

	typedef ui32 wchar32;
	
	template<class T>
	inline uint64_t ULL(T t) { return (uint64_t) t; }
}

#endif
