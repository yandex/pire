/*
 * common.h -- common declaration for Pire scanners
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

#ifndef PIRE_SCANNERS_COMMON_H_INCLUDED
#define PIRE_SCANNERS_COMMON_H_INCLUDED

#include <stdlib.h>
#include "../align.h"
#include "../stub/defaults.h"
#include "../stub/lexical_cast.h"
#include "../defs.h"
#include "../platform.h"

namespace Pire {

	struct Header {
		ui32 Magic;
		ui32 Version;
		ui32 PtrSize;
		ui32 MaxWordSize;
		ui32 Type;
		ui32 HdrSize;

		static const ui32 MAGIC = 0x45524950;   // "PIRE" on litte-endian
		static const ui32 RE_VERSION = 6;       // Should be incremented each time when the format of serialized scanner changes

		explicit Header(ui32 type, size_t hdrsize)
			: Magic(MAGIC)
			, Version(RE_VERSION)
			, PtrSize(sizeof(void*))
			, MaxWordSize(sizeof(Impl::MaxSizeWord))
			, Type(type)
			, HdrSize(hdrsize)
		{}

		void Validate(ui32 type, size_t hdrsize) const
		{
			// fix link errors:
			// ./.libs/libpire.so: undefined reference to `Pire::Header::MAGIC'
			ui32 MAGIC_ = MAGIC;
			ui32 RE_VERSION_ = RE_VERSION;
			if (Magic != MAGIC)
				throw Error("Bad Magic in serialized regexp. "
						"Expected: " + ToString(MAGIC_) + ". "
						"Found: " + ToString(Magic));
			if (PtrSize != sizeof(void*))
				throw Error("Bad PtrSize in serialized regexp. "
						"Expected: " + ToString(sizeof(void*)) + ". "
						"Found: " + ToString(PtrSize));
			if (MaxWordSize != sizeof(Impl::MaxSizeWord))
				throw Error("Bad MaxWordSize in serialized regexp. "
						"Expected: " + ToString(sizeof(Impl::MaxSizeWord)) + ". "
						"Found: " + ToString(MaxWordSize));
			if (Version != RE_VERSION)
				throw Error("Bad Version in serialized regexp. "
						"Expected: " + ToString(RE_VERSION_) + ". "
						"Found: " + ToString(Version));
			if (type != 0 && type != Type)
				throw Error("Bad type in serialized regexp. "
						"Expected: " + ToString(type) + ". "
						"Found: " + ToString(Type));
			if (hdrsize != 0 && HdrSize != hdrsize)
				throw Error("Bad HdrSize in serialized regexp. "
						"Expected: " + ToString(hdrsize) + ". "
						"Found: " + ToString(HdrSize));
		}
	};

	namespace Impl {
		inline const void* AdvancePtr(const size_t*& ptr, size_t& size, size_t delta)
		{
			ptr = (const size_t*) ((const char*) ptr + delta);
			size -= delta;
			return (const void*) ptr;
		}

		template<class T>
		inline void MapPtr(T*& field, size_t count, const size_t*& p, size_t& size)
		{
			if (size < count * sizeof(*field))
				throw Error("EOF reached while mapping Pire::SlowScanner");
			field = (T*) p;
			Impl::AdvancePtr(p, size, count * sizeof(*field));
			Impl::AlignPtr(p, size);
		}

		inline void CheckAlign(const void* ptr, size_t bound = sizeof(size_t))
		{
			if (!IsAligned(ptr, bound))
				throw Error("Tried to mmap scanner at misaligned address");
		}

		inline void ValidateHeader(const size_t*& ptr, size_t& size, ui32 type, size_t hdrsize)
		{
			const Header* hdr;
			MapPtr(hdr, 1, ptr, size);
			hdr->Validate(type, hdrsize);
		}

		inline void ValidateHeader(yistream* s, ui32 type, size_t hdrsize)
		{
			Header hdr(0, 0);
			LoadPodType(s, hdr);
			AlignLoad(s, sizeof(hdr));
			hdr.Validate(type, hdrsize);
		}
	}
}

#endif
