#ifndef PIRE_SSE_H_INCLUDED
#define PIRE_SSE_H_INCLUDED

namespace Pire {
namespace Impl {

#if defined(__SSE2__) && !defined(PIRE_WORD_DEFINED)
#include <xmmintrin.h>

template<bool LargeSSEAvail> struct AvailSSEImpl {
	//typedef char Vector __attribute__((vector_size(16)));
	typedef __m128i Vector;
	
	static inline short ToShort(Vector v) { return _mm_extract_epi16(v, 0); }
	static inline Vector FromShort(short x) { return _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, x); }

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

template<>
struct AvailSSEImpl<false> {
	//typedef char Vector __attribute__((vector_size(8)));
	typedef __m64 Vector;
	static inline short ToShort(Vector v) { return _mm_extract_pi16(v, 0); }
	static inline Vector FromShort(short x) { return _m_from_int(x); }
	
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
		return _mm_movemask_pi8(mask);
	}
};

typedef AvailSSEImpl<sizeof(void*) >= 8> AvailSSE;

typedef AvailSSE::Vector Word;
inline short ToShort(Word v) { return AvailSSE::ToShort(v); }
inline Word FromShort(short x) { return AvailSSE::FromShort(x); }

inline Word FillWord(unsigned char c)
{
	union {
		size_t fields[sizeof(Word)/sizeof(size_t)];
		Word w;
	};
	fields[0] = c;
	for (size_t i = 8; i != sizeof(size_t)*8; i <<= 1)
		fields[0] = (fields[0] << i) | fields[0];
	for (size_t i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i)
		fields[i] = fields[0];
	return w;
}

inline Word CheckBytes(Word mask, Word chunk) { return AvailSSE::CheckBytes(mask, chunk); }

inline Word Or(Word mask1, Word mask2) { return AvailSSE::Or(mask1, mask2); }

inline bool IsAnySet(Word mask) { return AvailSSE::IsAnySet(mask); }


inline Word ToLittleEndian(Word x) { return x; }

#define PIRE_WORD_DEFINED
#endif /*defined(__SSE2__) && !defined(PIRE_WORD_DEFINED)*/

	
#ifndef PIRE_WORD_DEFINED

typedef size_t Word;
	
inline size_t FillWord(char c)
{
	size_t w = c;
	for (size_t i = 8; i != sizeof(size_t)*8; i <<= 1)
		w = (w << i) | w;
	return w;
}
	
inline size_t ToShort(short s) { return s; }
inline short FromShort(size_t x) { return (short) x; }

// Check bytes in the chunk against bytes in the mask
inline Word CheckBytes(Word mask, Word chunk)
{
	const size_t mask0x01 = (size_t)0x0101010101010101ull;
	const size_t mask0x80 = (size_t)0x8080808080808080ull;
	size_t mc = chunk ^ mask;
	return ((mc - mask0x01) & ~mc & mask0x80);
}

inline Word Or(Word mask1, Word mask2) { return (mask1 | mask2); }

inline bool IsAnySet(Word mask) { return (mask != 0); }

#define PIRE_SHORTCUTS_DEFINED
#endif

inline size_t GetSizeT(const Word* pVal)
{
	return *(const size_t*)pVal;
}

}}

#endif
