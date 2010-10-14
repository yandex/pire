/*
 * utf8.cpp -- the wrapper for compiling Unicode-handling routines
 *             borrowed from Yandex internals
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


#include <string.h>
#include "defaults.h"
#include "singleton.h"
#include "stl.h"

#define DECLARE_NOCOPY(klass) private: klass(const klass&); klass& operator = (const klass&);

namespace Pire {

#include "doccodes_h.h"
#include "unidata_h.h"
#include "codepage_h.h"
#include "unidata_cpp.h"

	const wchar32 BROKEN_RUNE = (wchar32) -1;

	const Recoder rcdr_to_lower[1] = {};
	const Recoder rcdr_to_upper[1] = {};
	const Recoder rcdr_to_title[1] = {};
	const CodePage *codepage_by_doccodes[1] = {};
	const CodePage *CodePageByName(const char*) { return 0; }

	NCodepagePrivate::TCodepagesMap::TCodepagesMap() {}
}
