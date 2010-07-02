#ifndef PIRE_STUB_NONCOPYABLE_H_INCLUDED
#define PIRE_STUB_NONCOPYABLE_H_INCLUDED

namespace Pire {
	
	class NonCopyable {
	public:
		NonCopyable() {}
	private:
		NonCopyable(const NonCopyable&);
		NonCopyable& operator = (const NonCopyable&);
	};
}

#endif
