/*
 * align.h -- functions for positioning streams and memory pointers
 *            to word boundaries
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


#ifndef PIRE_ALIGN_H
#define PIRE_ALIGN_H

#include "stub/stl.h"
#include "stub/saveload.h"

namespace Pire {
	
	namespace Impl {
		
		inline void AlignSave(yostream* s, size_t size)
		{
			size_t tail = ((size + (sizeof(void*)-1)) & ~(sizeof(void*)-1)) - size;
			if (tail) {
				static const char buf[sizeof(void*)] = {0};
				SaveArray(s, buf, tail);
			}
		}

		inline void AlignLoad(yistream* s, size_t size)
		{
			size_t tail = ((size + (sizeof(void*)-1)) & ~(sizeof(void*)-1)) - size;
			if (tail) {
				char buf[sizeof(void*)];
				LoadArray(s, buf, tail);
			}
		}
		
		template<class T>
		inline void AlignedSaveArray(yostream* s, const T* array, size_t count)
		{
			SaveArray(s, array, count);
			AlignSave(s, sizeof(*array) * count);
		}

		template<class T>
		inline void AlignedLoadArray(yistream* s, T* array, size_t count)
		{
			LoadArray(s, array, count);
			AlignLoad(s, sizeof(*array) * count);
		}

		template<class T>
		inline T AlignPtr(T t)
		{
			return (T) (((size_t) t + (sizeof(void*)-1)) & ~(sizeof(void*)-1));
		}
		
		inline const void* AlignPtr(const size_t*& p, size_t& size)
		{
			if ((size_t) p & (sizeof(void*) - 1)) {
				size_t l = sizeof(void*) - ((size_t) p & (sizeof(void*) - 1));
				if (size < l)
					throw Error("EOF reached in NPire::Impl::align");
				p = (const size_t*) ((const char*) p + l);
				size -= l;
			}
			return (const void*) p;
		}

	}
	
}

#endif
