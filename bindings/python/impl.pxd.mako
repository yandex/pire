# vim: ft=pyrex

from libcpp cimport bool

from .stub cimport yvector, ypair, ystring, yauto_ptr, yistream, yostream


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    ctypedef unsigned short Char

    cdef enum SpecialChar:
        % for ch in special_chars:
        ${ch}
        % endfor

    cdef cppclass Fsm:
        Fsm()

        void Swap(Fsm&)

        size_t Size()

        void Append(char)
        void Append(const ystring&)
        void AppendSpecial(Char)

        void AppendStrings(const yvector[ystring]&)

        % for unary in fsm_inplace_unaries:
        void ${unary}()
        % endfor

        % for sign, operation, rhs_type, _ in fsm_binaries:
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


    % for Scanner in scanners:
    cdef cppclass ${Scanner}:
        ${Scanner}()
        ${Scanner}(Fsm&)

        void Swap(${Scanner}&)

        size_t Size()
        bool Empty()

        size_t RegexpsCount()
        size_t LettersCount()

        void Save(yostream*)
        void Load(yistream*)


    bool Matches(const ${Scanner}&, const char* begin, const char* end)

    const char* LongestPrefix(const ${Scanner}& scanner, const char* begin, const char* end)
    const char* ShortestPrefix(const ${Scanner}& scanner, const char* begin, const char* end)

    const char* LongestSuffix(const ${Scanner}& scanner, const char* rbegin, const char* rend)
    const char* ShortestSuffix(const ${Scanner}& scanner, const char* rbegin, const char* rend)
    % endfor


    cdef cppclass Feature:
        pass


cdef extern from "pire/pire.h" namespace "Pire::Features":
    % for feature in features:
    Feature* ${feature}()
    % endfor
