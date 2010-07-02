#ifndef PIRE_DEFS_H
#define PIRE_DEFS_H

namespace Pire {

	typedef unsigned short Char;
	
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

#endif
