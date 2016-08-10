# vim: ft=pyrex

from libcpp.string cimport string
from libcpp cimport bool


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    cdef cppclass Fsm:
        Fsm()

        void Swap(Fsm&)

        size_t Size()

        void Append(const string&)


    Fsm Fsm_MakeFalse "Pire::Fsm::MakeFalse"()


    cdef cppclass Lexer:
        Lexer()

        void Assign(const char* begin, const char* end)

        Fsm Parse() except +


    % for Scanner in scanners:
    cdef cppclass ${Scanner}:
        ${Scanner}()
        ${Scanner}(Fsm&)

        void Swap(${Scanner}&)

        size_t Size()
        bool Empty()

        size_t RegexpsCount()
        size_t LettersCount()


    bool Matches(const ${Scanner}&, const char* begin, const char* end)
    % endfor
