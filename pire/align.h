#ifndef PIRE_ALIGN_H
#define PIRE_ALIGN_H

#include "stl.h"
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
