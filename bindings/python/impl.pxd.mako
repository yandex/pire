# vim: ft=pyrex

from libcpp cimport bool

from stub cimport yvector, ypair, ystring, yauto_ptr, yistream, yostream


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    cdef cppclass Fsm:
        Fsm()

        void Swap(Fsm&)

        size_t Size()

        void Append(char)
        void Append(const ystring&)

        void AppendStrings(const yvector[ystring]&)

        % for unary in FSM_INPLACE_UNARIES:
        void ${unary}()
        % endfor

        % for sign, operation, rhs_type, _ in FSM_BINARIES:
        void __i${operation}__ "operator ${sign}=" (${rhs_type})
        Fsm __${operation}__ "operator ${sign}" (${rhs_type})
        % endfor

        Fsm Surrounded()

        Fsm operator * ()
        Fsm operator ~ ()

        TScanner Compile[TScanner]()

        bool Determine(size_t maxSize)
        bool IsDetermined()


    Fsm Fsm_MakeFalse "Pire::Fsm::MakeFalse"()


    cdef cppclass Lexer:
        Lexer()

        void Assign(const char* begin, const char* end)

        void AddFeature(Feature*)

        Fsm Parse() except +


    % for Scanner in SCANNERS:
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


    cdef cppclass Feature:
        pass


cdef extern from "pire/pire.h" namespace "Pire::Features" nogil:
    % for feature in FEATURES:
    Feature* ${feature}()
    % endfor
