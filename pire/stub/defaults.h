#ifndef PIRE_STUB_DEFAULTS_H_INCLUDED
#define PIRE_STUB_DEFAULTS_H_INCLUDED

#include <stdint.h>

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

#ifndef FORCED_INLINE
#define FORCED_INLINE inline
#endif

#endif
