#ifndef PIRE_STUB_SINGLETON_H_INCLUDED
#define PIRE_STUB_SINGLETON_H_INCLUDED

namespace Pire {
	
	// FIXME: not thread safe.
	template<class T>
	T* Singleton()
	{
		static T* p = 0;
		if (!p)
			p = new T;
		return p;
	}
	template<class T>
	const T& DefaultValue() { return *Singleton<T>(); }
	
};

#endif
