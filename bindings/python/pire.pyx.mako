# vim: ft=pyrex
cimport impl


cdef inline const char* begin(bytes line):
    return line


cdef inline const char* end(bytes line):
    return (<const char*>line) + len(line)


cdef inline wrap_fsm(impl.Fsm fsm_impl):
    ret = Fsm()
    ret.fsm_impl.Swap(fsm_impl)
    return ret


cdef class Fsm:
    cdef impl.Fsm fsm_impl

    def __cinit__(self, Fsm copy=None):
        if copy is not None:
            self.fsm_impl = copy.fsm_impl

    @staticmethod
    def MakeFalse():
        return wrap_fsm(impl.Fsm_MakeFalse())

    def Size(self):
        return self.fsm_impl.Size()

    def Append(self, bytes line):
        self.fsm_impl.Append(<impl.string>line)
        return self

    def Compile(self, object scanner_class=None):
        if scanner_class is None:
            scanner_class = Scanner
        return scanner_class(self)


cdef class Lexer:
    cdef impl.Lexer lexer_impl

    def __cinit__(self, bytes line=None):
        if line is not None:
            self.lexer_impl.Assign(begin(line), end(line))

    def Parse(self):
        return wrap_fsm(self.lexer_impl.Parse())



cdef class BaseScanner:
    pass


% for Scanner in scanners:
cdef inline object wrap_${Scanner}(impl.${Scanner} scanner_impl):
    ret = ${Scanner}()
    ret.scanner_impl.Swap(scanner_impl)
    return ret


cdef class ${Scanner}(BaseScanner):
    cdef impl.${Scanner} scanner_impl

    def __cinit__(self, Fsm fsm=None):
        if fsm is not None:
            self.scanner_impl = impl.${Scanner}(fsm.fsm_impl)

    % for method in ["Size", "Empty", "RegexpsCount", "LettersCount"]:
    def ${method}(self):
        return self.scanner_impl.${method}()
    % endfor

    def Matches(self, bytes line not None):
        return impl.Matches(self. scanner_impl, begin(line), end(line))
% endfor
