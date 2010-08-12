#ifndef PIRE_COMPAT_H_INCLUDED
#define PIRE_COMPAT_H_INCLUDED

#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <bitset>
#include <utility>
#include <memory>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <functional>

typedef std::string ystring;
#define yvector std::vector
#define ydeque std::deque
#define ylist std::list
#define ymap std::map
#define yset std::set
#define yauto_ptr std::auto_ptr
#define ybitset std::bitset
#define yistream std::istream
#define yostream std::ostream
#define ypair std::pair
#define ymake_pair std::make_pair
#define ymax std::max
#define ymin std::min
#define ybinary_function std::binary_function
#define Cdbg std::clog
#define Endl std::endl

namespace Pire {
	template<class T>
	static inline void DoSwap(T& a, T& b) { std::swap(a, b); }

	template<class Iter, class T>
	static inline void Fill(Iter begin, Iter end, T t) { std::fill(begin, end, t); }
	
	template <class I, class T, class C>
	static inline I LowerBound(I f, I l, const T& v, C c) {
	    return std::lower_bound(f, l, v, c);
	}

	class Error: public std::runtime_error {
	public:
		Error(const char* msg): std::runtime_error(msg) {}
		Error(const ystring& msg): std::runtime_error(msg) {}
	};

}

#endif
