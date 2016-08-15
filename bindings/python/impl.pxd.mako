# vim: ft=pyrex

from libcpp cimport bool

from stub cimport yvector, ypair, yset, ystring, yauto_ptr, yistream, yostream


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    ctypedef int wchar32


cdef extern from "encoding.h" namespace "PireBinding" nogil:
    yvector[wchar32] Utf8ToUcs4(const char* begin, const char* end)


cdef extern from "pire/pire.h" namespace "Pire" nogil:
    ctypedef unsigned short Char

    cdef enum SpecialChar:
        % for ch in SPECIAL_CHARS:
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


    cdef cppclass Feature:
        pass

cdef extern from "pire/extra.h" namespace "Pire::Features":
    Feature* Capture(size_t)


cdef extern from "pire/pire.h" namespace "Pire":
    cdef cppclass Lexer:
        Lexer()
        Lexer(const char* begin, const char* end)
        Lexer(const yvector[wchar32]&)

        Fsm Parse() except +

        void AddFeature(Feature*)


    cdef cppclass CapturingScannerState "Pire::CapturingScanner::State":
        bool Captured()
        size_t Begin()
        size_t End()


    % for Scanner, spec in SCANNERS.items():

    % if spec.state_t != "__nontrivial__":
    ctypedef ${spec.state_t} ${Scanner}State "Pire::${Scanner}::State"
    % endif

    cdef cppclass ${Scanner}:
        ${Scanner}()
        ${Scanner}(Fsm&)

        void Swap(${Scanner}&)

        % if "Size" not in spec.ignored_methods:
        size_t Size()
        % endif
        bool Empty()

        size_t RegexpsCount()
        % if "LettersCount" not in spec.ignored_methods:
        size_t LettersCount()
        % endif

        void Initialize(${Scanner}State&)
        bool Final(const ${Scanner}State&)
        bool Dead(const ${Scanner}State&)

        % if "AcceptedRegexps" not in spec.ignored_methods:
        ypair[const size_t*, const size_t*] AcceptedRegexps(const ${Scanner}State&)
        % endif

        void Save(yostream*)
        void Load(yistream*)


    % if "Glue" not in spec.ignored_methods:
    ${Scanner} Glue "Pire::${Scanner}::Glue"(const ${Scanner}&, const ${Scanner}&, size_t maxSize)
    % endif

    bool Matches(const ${Scanner}&, const char* begin, const char* end)

    void Step(const ${Scanner}&, ${Scanner}State&, Char)

    void Run(const ${Scanner}&, ${Scanner}State&, const char* begin, const char* end)

    const char* LongestPrefix(const ${Scanner}& scanner, const char* begin, const char* end)
    const char* ShortestPrefix(const ${Scanner}& scanner, const char* begin, const char* end)

    const char* LongestSuffix(const ${Scanner}& scanner, const char* rbegin, const char* rend)
    const char* ShortestSuffix(const ${Scanner}& scanner, const char* rbegin, const char* rend)
    % endfor


cdef extern from "pire/easy.h" namespace "Pire":
    cdef cppclass Options:
        void Apply(Lexer&)

    cdef cppclass Regexp:
        Regexp(const ystring&, const Options&)

        Regexp(Scanner)
        Regexp(SlowScanner)

        bool Matches(const char* begin, const char* end)


cdef extern from "options.h" namespace "PireBinding":
    cdef enum OptionFlag:
        % for option in OPTIONS:
        ${option}
        % endfor
    ctypedef yset[OptionFlag] FlagSet


    yauto_ptr[Options] ConvertFlagSetToOptions(const FlagSet&)
