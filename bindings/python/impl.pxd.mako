# vim: ft=pyrex

from libcpp cimport bool

from .stub cimport yvector, ypair, ystring, yauto_ptr, yistream, yostream


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    cdef cppclass Fsm:
        Fsm()

        void Swap(Fsm&)

        size_t Size()

        void Append(const ystring&)


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
