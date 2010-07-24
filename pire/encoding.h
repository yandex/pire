#ifndef PIRE_ENCODING_H
#define PIRE_ENCODING_H


#include "stub/defaults.h"
#include "stl.h"

namespace Pire {

class Fsm;

class Encoding {
public:
	virtual ~Encoding() {}

	virtual wchar32 FromLocal(const char*& begin, const char* end) const = 0;
	virtual ystring ToLocal(wchar32 c) const = 0;
	virtual void AppendDot(Fsm&) const = 0;

	template<class OutputIter>
	OutputIter FromLocal(const char* begin, const char* end, OutputIter iter) const
	{
		while (begin != end) {
			*iter = FromLocal(begin, end);
			++iter;
		}
		return iter;
	}
};

namespace Encodings {
	const Encoding& Latin1();
	const Encoding& Utf8();

};


};

#endif
