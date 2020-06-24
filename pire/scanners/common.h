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
#include "../stub/msgpuck.h"
#include "../defs.h"
#include "../platform.h"

namespace Pire {
	namespace ScannerIOTypes {
		enum {
			NoScanner = 0,
			Scanner = 1,
			SimpleScanner = 2,
			SlowScanner = 3,
			LoadedScanner = 4,
			NoGlueLimitCountingScanner = 5,
		};
	}

	struct Header {
		ui32 Magic;
		ui32 Version;
		ui32 PtrSize;
		ui32 MaxWordSize;
		ui32 Type;
		ui32 HdrSize;

		static const ui32 MAGIC = 0x45524950;   // "PIRE" on litte-endian
		static const ui32 RE_VERSION = 7;       // Should be incremented each time when the format of serialized scanner changes
		static const ui32 RE_VERSION_WITH_MACTIONS = 6;  // LoadedScanner with m_actions, which is ignored
		static const ui32 PACKED_VERSION = 1;
		static const ui32 PACKED_HEADERS = 5;

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
			if (Magic != MAGIC || PtrSize != sizeof(void*) || MaxWordSize != sizeof(Impl::MaxSizeWord))
				throw Error("Serialized regexp incompatible with your system");
			if (Version != RE_VERSION && Version != RE_VERSION_WITH_MACTIONS)
				throw Error("You are trying to used an incompatible version of a serialized regexp");
			if (type != ScannerIOTypes::NoScanner && type != Type &&
			   !(type == ScannerIOTypes::LoadedScanner && Type == ScannerIOTypes::NoGlueLimitCountingScanner)) {
				throw Error("Serialized regexp incompatible with your system");
			}
			if (hdrsize != 0 && HdrSize != hdrsize)
				throw Error("Serialized regexp incompatible with your system");
		}

		void Pack(yostream* s)
		{
			char buf[MsgpuckSizeofArrayMax + PACKED_HEADERS * MsgpuckSizeofUintMax];
			char *ptr = buf;

			ptr = mp_encode_array(ptr, PACKED_HEADERS);
			ptr = mp_encode_uint(ptr, Magic);
			ptr = mp_encode_uint(ptr, PACKED_VERSION);
			ptr = mp_encode_uint(ptr, MaxCharUnaligned);
			ptr = mp_encode_uint(ptr, MaxWordSize);
			ptr = mp_encode_uint(ptr, Type);
			s->write(buf, ptr - buf);
		}

		void Unpack(yistream* s) const
		{
			if (MsgpuckReadArray(s) != PACKED_HEADERS) {
				throw Error("Headers number is invalid");
			}
			if (MsgpuckReadUint(s) != Magic) {
				throw Error("Unknown magic");
			}
			if (MsgpuckReadUint(s) != PACKED_VERSION) {
				throw Error("You are trying to use an incompatible version of a serialized regexp");
			}
			if (MsgpuckReadUint(s) != MaxCharUnaligned) {
				throw Error("MaxCharUnaligned mismatch");
			}
			if (MsgpuckReadUint(s) != MaxWordSize) {
				throw Error("MaxWordSize mismatch");
			}
			if (MsgpuckReadUint(s) != Type) {
				throw Error("Incompatible regexp type");
			}
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

		inline Header ValidateHeader(const size_t*& ptr, size_t& size, ui32 type, size_t hdrsize)
		{
			const Header* hdr;
			MapPtr(hdr, 1, ptr, size);
			hdr->Validate(type, hdrsize);
			return *hdr;
		}

		inline Header ValidateHeader(yistream* s, ui32 type, size_t hdrsize)
		{
			Header hdr(ScannerIOTypes::NoScanner, 0);
			LoadPodType(s, hdr);
			AlignLoad(s, sizeof(hdr));
			hdr.Validate(type, hdrsize);
			return hdr;
		}
	}
}

#endif
