#ifndef PIRE_ASSERT_H_INCLUDED
#define PIRE_ASSERT_H_INCLUDED

namespace Pire { namespace Impl {
        
    // A static (compile-tile) assertion.
    // The idea was shamelessly borrowed from Boost.
    template<bool x> struct StaticAssertion;
    template<> struct StaticAssertion<true> {};
#define PIRE_STATIC_ASSERT(x) \
    enum { PireStaticAssertion ## __LINE__ = sizeof(Pire::Impl::StaticAssertion<(bool) (x)>) }
}}

#endif
