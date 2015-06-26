#pragma once

#include "common.h"
#include "../stub/array.h"

namespace Pire {

namespace Impl
{
    template<class T, size_t S>
    using FixedSizeArray = yarray<T, S>;

    template<class Scanner, size_t S, size_t I>
    struct InitializeImpl
    {
        static void Do(FixedSizeArray<const Scanner*, S> scanners, FixedSizeArray<typename Scanner::State, S>& state)
        {
            InitializeImpl<Scanner, S, I - 1>::Do(scanners, state);
            scanners[I-1]->Initialize(state[I - 1]);
        }
    };
    template<class Scanner, size_t S>
    struct InitializeImpl<Scanner, S, 0>
    {
        static void Do(FixedSizeArray<const Scanner*, S>, FixedSizeArray<typename Scanner::State, S>&)
        {
        }
    };
    template<class Scanner, size_t S>
    using Initialize = InitializeImpl<Scanner, S, S>;

    template<class Scanner, size_t S, size_t I>
    struct NextImpl
    {
        static void Do(FixedSizeArray<const Scanner*, S> scanners, FixedSizeArray<typename Scanner::State, S>& state, Char ch, FixedSizeArray<typename Scanner::Action, S>& action)
        {
            NextImpl<Scanner, S, I - 1>::Do(scanners, state, ch, action);
            action[I - 1] = scanners[I - 1]->Next(state[I - 1], ch);
        }
    };
    template<class Scanner, size_t S>
    struct NextImpl<Scanner, S, 0>
    {
        static void Do(FixedSizeArray<const Scanner*, S>, FixedSizeArray<typename Scanner::State, S>&, Char, FixedSizeArray<typename Scanner::Action, S>&)
        {
        }
    };
    template<class Scanner, size_t S>
    using Next = NextImpl<Scanner, S, S>;

    template<class Scanner, size_t S, size_t I>
    struct TakeActionImpl
    {
        static void Do(FixedSizeArray<const Scanner*, S> scanners, FixedSizeArray<typename Scanner::State, S>& state, FixedSizeArray<typename Scanner::Action, S> action)
        {
            TakeActionImpl<Scanner, S, I - 1>::Do(scanners, state, action);
            scanners[I - 1]->TakeAction(state[I - 1], action[I - 1]);
        }
    };
    template<class Scanner, size_t S>
    struct TakeActionImpl<Scanner, S, 0>
    {
        static void Do(FixedSizeArray<const Scanner*, S>, FixedSizeArray<typename Scanner::State, S>&, FixedSizeArray<typename Scanner::Action, S>)
        {
        }
    };
    template<class Scanner, size_t S>
    using TakeAction = TakeActionImpl<Scanner, S, S>;

    template<class Scanner, size_t S, size_t I>
    struct FinalImpl
    {
        static bool Do(const FixedSizeArray<const Scanner*, S>& scanners, const FixedSizeArray<typename Scanner::State, S>& state)
        {
            return  FinalImpl<Scanner, S, I - 1>::Do(scanners, state) || scanners[I - 1]->Final(state[I - 1]);
        }
    };
    template<class Scanner, size_t S>
    struct FinalImpl<Scanner, S, 0>
    {
        static bool Do(const FixedSizeArray<const Scanner*, S>&, const FixedSizeArray<typename Scanner::State, S>&)
        {
            return false;
        }
    };
    template<class Scanner, size_t S>
    using Final = FinalImpl<Scanner, S, S>;

    template<class Scanner, size_t S, size_t I>
    struct DeadImpl
    {
        static bool Do(const FixedSizeArray<const Scanner*, S>& scanners, const FixedSizeArray<typename Scanner::State, S>& state)
        {
            return DeadImpl<Scanner, S, I - 1>::Do(scanners, state) && scanners[I - 1]->Dead(state[I - 1]);
        }
    };
    template<class Scanner, size_t S>
    struct DeadImpl<Scanner, S, 0>
    {
        static bool Do(const FixedSizeArray<const Scanner*, S>&, const FixedSizeArray<typename Scanner::State, S>&)
        {
            return true;
        }
    };
    template<class Scanner, size_t S>
    using Dead = DeadImpl<Scanner, S, S>;

    template<class Scanner, size_t S, size_t I>
    struct StateIndexImpl
    {
        static void Do(const FixedSizeArray<const Scanner*, S>& scanners, const FixedSizeArray<typename Scanner::State, S>& state, FixedSizeArray<size_t, S>& res)
        {
            StateIndexImpl<Scanner, S, I - 1>::Do(scanners, state, res);
            res[I - 1] = scanners[I - 1]->StateIndex(state[I - 1]);
        }
    };
    template<class Scanner, size_t S>
    struct StateIndexImpl<Scanner, S, 0>
    {
        static void Do(const FixedSizeArray<const Scanner*, S>&, const FixedSizeArray<typename Scanner::State, S>&, FixedSizeArray<size_t, S>&)
        {
        }
    };
    template<class Scanner, size_t S>
    using StateIndex = StateIndexImpl<Scanner, S, S>;
}

    template <class Scanner, size_t S>
    class ScannersGroup
    {
    public:
        using State  = Impl::FixedSizeArray<typename Scanner::State,  S>;
        using Action = Impl::FixedSizeArray<typename Scanner::Action, S>;

        void Initialize(State& state) const
        {
            Impl::Initialize<Scanner, S>::Do(m_scanners, state);
        }

        Action Next(State& state, Char ch) const
        {
            Action action;
            Impl::Next<Scanner, S>::Do(m_scanners, state, ch, action);
            return action;
        }

        void TakeAction(State& s, Action a) const
        {
            Impl::TakeAction<Scanner, S>::Do(m_scanners, s, a);
        }

        bool Final(const State& state) const
        {
            return Impl::Final<Scanner, S>::Do(m_scanners, state);
        }

        bool Dead(const State& state) const
        {
            return Impl::Dead<Scanner, S>::Do(m_scanners, state);
        }

        // Impl::FixedSizeArray<typename Scanner::State, S> StateIndex(const State& state) const
        Impl::FixedSizeArray<size_t, S> StateIndex(const State& state) const
        {
            Impl::FixedSizeArray<size_t, S> res;
            Impl::StateIndex<Scanner, S>::Do(m_scanners, state, res);
            return res;
        }

        void SetScanner(const Scanner* scanner, size_t i)
        {
            m_scanners[i] = scanner;
        }
        const Scanner* Get(size_t i) const
        {
            return m_scanners[i];
        }

    private:
        Impl::FixedSizeArray<const Scanner*, S> m_scanners;
    };
}

namespace NPire
{
    using Pire::ScannersGroup;
}
