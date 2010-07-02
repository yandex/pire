#ifndef PIRE_STUB_UTF8_H_INCLUDED
#define PIRE_STUB_UTF8_H_INCLUDED

#include <sys/types.h>
#include "defaults.h"

namespace Pire {
	enum RECODE_RESULT { RECODE_OK }; // A dirty evil hack breaking the ODR
	
	size_t utf8_rune_len(const unsigned char p);
	size_t utf8_rune_len_by_ucs(wchar32 rune);
	RECODE_RESULT utf8_read_rune(wchar32 &rune, size_t &rune_len, const unsigned char *s, const unsigned char *end);
	RECODE_RESULT utf8_put_rune(wchar32 rune, size_t &rune_len, unsigned char *s, const unsigned char *end);
	bool is_lower(wchar32 ch);
	bool is_upper(wchar32 ch);
	bool is_digit(wchar32 ch);
	
	wchar32 to_lower(wchar32 ch);
	wchar32 to_upper(wchar32 ch);
}

#endif
