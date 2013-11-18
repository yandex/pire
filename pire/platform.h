/*
 * platform.h -- hardware and OS specific stuff
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

#ifndef PIRE_PLATFORM_H_INCLUDED
#define PIRE_PLATFORM_H_INCLUDED

#include "stub/defaults.h"
#include "static_assert.h"

#ifndef FORCED_INLINE
#ifdef __GNUC__
#define FORCED_INLINE __attribute__((always_inline))
#elif _MSC_VER
#define FORCED_INLINE __forceinline
#else
#define FORCED_INLINE inline
#endif
#endif

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 2))
#define PIRE_HOT_FUNCTION __attribute__ ((hot))
#else
#define PIRE_HOT_FUNCTION
#endif

#ifndef PIRE_LIKELY
#ifdef __GNUC__
#define PIRE_LIKELY(x) (__builtin_expect((x), 1))
#else
#define PIRE_LIKELY(x) (x)
#endif
#endif

#ifndef PIRE_UNLIKELY
#ifdef __GNUC__
#define PIRE_UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define PIRE_UNLIKELY(x) (x)
#endif
#endif

#ifdef _MSC_VER
#include <stdio.h>
#include <stdarg.h>

namespace Pire {

#ifdef _WIN64
typedef i64 ssize_t;
#else
typedef i32 ssize_t;
#endif

inline int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int i = _vsnprintf(str, size-1, format, argptr);
	va_end(argptr);

	// A workaround for some bug
	if (i < 0) {
		str[size - 1] = '\x00';
		i = (int)size;
	} else if (i < (int)size) {
		str[i] = '\x00';
	}
	return i;
}

}
#endif

namespace Pire {
namespace Impl {

// A portable way to define a constant like `(size_t)0101010101010101ull'
// without any compiler warnings.
template<unsigned Pos, unsigned char Byte>
struct DoGenerateMask {
	static const size_t Value = DoGenerateMask<Pos-1, Byte>::Value << 8 | (size_t) Byte;
};

template<unsigned char Byte>
struct DoGenerateMask<0, Byte> {
	static const size_t Value = 0;
};

template<unsigned char Byte>
struct GenerateMask {
	static const size_t Value = DoGenerateMask<sizeof(size_t), Byte>::Value;
};


// Common implementation of mask comparison logic suitable for
// any instruction set
struct BasicInstructionSet {
	typedef size_t Vector;

	// Check bytes in the chunk against bytes in the mask
	static inline Vector CheckBytes(Vector mask, Vector chunk)
	{
		size_t mc = chunk ^ mask;
		return ((mc - GenerateMask<0x01>::Value) & ~mc & GenerateMask<0x80>::Value);
	}

	static inline Vector Or(Vector mask1, Vector mask2) { return (mask1 | mask2); }

	static inline bool IsAnySet(Vector mask) { return (mask != 0); }
};

}}

#if defined(__SSE2__)
#include <emmintrin.h>

namespace Pire {
namespace Impl {

// SSE2-optimized mask comparison logic
struct AvailSSE2 {
	typedef __m128i Vector;

	static inline Vector CheckBytes(Vector mask, Vector chunk)
	{
		return _mm_cmpeq_epi8(mask, chunk);
	}

	static inline Vector Or(Vector mask1, Vector mask2)
	{
		return _mm_or_si128(mask1, mask2);
	}

	static inline bool IsAnySet(Vector mask)
	{
		return _mm_movemask_epi8(mask);
	}
};

typedef AvailSSE2 AvailInstructionSet;

inline AvailSSE2::Vector ToLittleEndian(AvailSSE2::Vector x) { return x; }

}}

#elif defined(__MMX__)
#include <mmintrin.h>

namespace Pire {
namespace Impl {

// MMX-optimized mask comparison logic
struct AvailMMX {
	typedef __m64 Vector;

	static inline Vector CheckBytes(Vector mask, Vector chunk)
	{
		return _mm_cmpeq_pi8(mask, chunk);
	}

	static inline Vector Or(Vector mask1, Vector mask2)
	{
		return _mm_or_si64(mask1, mask2);
	}

	static inline bool IsAnySet(Vector mask)
	{
		union {
			Vector mmxMask;
			ui64 ui64Mask;
		};
		mmxMask = mask;
		return ui64Mask;
	}
};

typedef AvailMMX AvailInstructionSet;

inline AvailMMX::Vector ToLittleEndian(AvailMMX::Vector x) { return x; }

}}

#else // no SSE and MMX

namespace Pire {
namespace Impl {

typedef BasicInstructionSet AvailInstructionSet;

}}

#endif

namespace Pire {
namespace Impl {

typedef AvailInstructionSet::Vector Word;

inline Word CheckBytes(Word mask, Word chunk) { return AvailInstructionSet::CheckBytes(mask, chunk); }

inline Word Or(Word mask1, Word mask2) { return AvailInstructionSet::Or(mask1, mask2); }

inline bool IsAnySet(Word mask) { return AvailInstructionSet::IsAnySet(mask); }

// MaxSizeWord type is largest integer type supported by the plaform including
// all possible SSE extensions that are are known for this platform (even if these
// extensions are not available at compile time)
// It is used for alignments and save/load data structures to produce data format
// compatible between all platforms with the same endianness and pointer size
template <size_t Size> struct MaxWordSizeHelper;

// Maximum size of SSE register is 128 bit on x86 and x86_64
template <>
struct MaxWordSizeHelper<16> {
	struct MaxSizeWord {
		char val[16];
	};
};

typedef MaxWordSizeHelper<16>::MaxSizeWord MaxSizeWord;

// MaxSizeWord size should be a multiple of size_t size and a multipe of Word size
PIRE_STATIC_ASSERT(
	(sizeof(MaxSizeWord) % sizeof(size_t) == 0) &&
	(sizeof(MaxSizeWord) % sizeof(Word) == 0));

inline size_t FillSizeT(char c)
{
	size_t w = c;
	w &= 0x0ff;
	for (size_t i = 8; i != sizeof(size_t)*8; i <<= 1)
		w = (w << i) | w;
	return w;
}

}}

#endif

