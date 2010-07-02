#ifndef PIRE_STUB_LEXICAL_CAST_H_INCLUDED
#define PIRE_STUB_LEXICAL_CAST_H_INCLUDED

#include <stdexcept>
#include <typeinfo>
#include <sstream>
#include <string>

namespace Pire {

class BadLexicalCast: public std::invalid_argument {
public:
	BadLexicalCast(const std::string& what): std::invalid_argument(what) {}

	template<class From, class To> static void Raise()
	{
		throw BadLexicalCast(
			std::string("Cannot convert from ") + typeid(From).name()
			+ " to " + typeid(To).name()
		);
	}
};

template<class T, class F> struct LexicalCaster {
	static T lexical_cast(const F& f) {
		std::stringstream s;
		T t;
		if (s << f && s >> t)
			return t;
		else {
			BadLexicalCast::Raise<F, T>();
			return T(); // Make compiler happy
		}
	}
};

template<class T, class F> T LexicalCast(const F& f) { return LexicalCaster<T, F>::lexical_cast(f); }
	
template<class F> std::string ToString(const F& f) { return LexicalCast<std::string>(f); }
template<class T> T FromString(const std::string& s) { return LexicalCast<T>(s); }

template<class Iter>
inline std::string Join(Iter begin, Iter end, const std::string& separator)
{
if (begin == end)
return std::string();
std::stringstream s;
s << *begin;
for (++begin; begin != end; ++begin)
s << separator << *begin;
return s.str();
}

}

#endif
