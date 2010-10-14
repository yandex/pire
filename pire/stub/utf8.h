/*
 * utf8.h -- an interface to borrowed Yandex internals
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
