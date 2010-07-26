#ifndef PIRE_ANY_H
#define PIRE_ANY_H


#include <typeinfo>

#include "stl.h"

namespace Pire {

class Any {

public:
	Any()
		: h(0)
	{
	}

	Any(const Any& any)
		: h(0)
	{
		if (any.h)
			h = any.h->Duplicate();
	}

	Any& operator= (Any any)
	{
		any.Swap(*this);
		return *this;
	}

	~Any() {
		delete h;
	}

	template <class T>
	Any(const T& t)
		: h(new Holder<T>(t))
	{
	}

	bool Empty() const {
		return !h;
	}
	template <class T>
	bool IsA() const {
		return h && h->IsA(typeid(T));
	}

	template <class T>
	T& As()
	{
		if (h && IsA<T>())
			return *reinterpret_cast<T*>(h->Ptr());
		else
			throw Pire::Error("type mismatch");
	}

	template <class T>
	const T& As() const
	{
		if (h && IsA<T>())
			return *reinterpret_cast<const T*>(h->Ptr());
		else
			throw Pire::Error("type mismatch");
	}

	void Swap(Any& a) throw () {
		DoSwap(h, a.h);
	}

private:

	struct AbstractHolder {
		virtual ~AbstractHolder() {
		}
		virtual AbstractHolder* Duplicate() const = 0;
		virtual bool IsA(const std::type_info& id) const = 0;
		virtual void* Ptr() = 0;
		virtual const void* Ptr() const = 0;
	};

	template <class T>
	struct Holder: public AbstractHolder {
		Holder(T t)
			: d(t)
		{
		}
		AbstractHolder* Duplicate() const {
			return new Holder<T>(d);
		}
		bool IsA(const std::type_info& id) const {
			return id == typeid(T);
		}
		void* Ptr() {
			return &d;
		}
		const void* Ptr() const {
			return &d;
		}
	private:
		T d;
	};

	AbstractHolder* h;
};

}

namespace std {
	inline void swap(Pire::Any& a, Pire::Any& b) {
		a.Swap(b);
	}
}

#endif
