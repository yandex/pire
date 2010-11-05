/*
 * lexical_cast.h -- a replacement for boost::lexical_cast.
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
