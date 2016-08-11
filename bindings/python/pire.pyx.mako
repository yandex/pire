# vim: ft=pyrex
import copy_reg

cimport cython

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
        self.fsm_impl.Append(<impl.ystring>line)
        return self

    def AppendStrings(self, strings):
        self.fsm_impl.AppendStrings(<impl.yvector[impl.ystring]>strings)
        return self

    % for unary in fsm_inplace_unaries:
    def ${unary}(self):
        self.fsm_impl.${unary}()
        return self
    % endfor

    % for _, operation, _, rhs_type in fsm_binaries:
    <%
        unwrapped_rhs = "rhs.fsm_impl" if rhs_type == "Fsm" else "rhs"
        not_none = "not None" if rhs_type != "size_t" else ""
        inplace_op = "__i{}__".format(operation)
        explace_op = "__{}__".format(operation)
    %>
    def ${inplace_op}(self, ${rhs_type} rhs ${not_none}):
        self.fsm_impl.${inplace_op}(${unwrapped_rhs})
        return self

    def _${operation}(self, ${rhs_type} rhs ${not_none}):
        return wrap_fsm(self.fsm_impl.${explace_op}(${unwrapped_rhs}))

    def ${explace_op}(self, rhs):
        if isinstance(self, Fsm):
            return self._${operation}(rhs)
        return rhs.${explace_op}(self)  # __rmul__ mode, rhs is Fsm
    %endfor

    def Surrounded(self):
        return wrap_fsm(self.fsm_impl.Surrounded())

    def Iterated(self):
        return wrap_fsm(cython.operator.dereference(self.fsm_impl))

    def __invert__(self):
        return wrap_fsm(~self.fsm_impl)

    def Determine(self, size_t max_size=0):
        return self.fsm_impl.Determine(max_size)

    def IsDetermined(self):
        return self.fsm_impl.IsDetermined()

    def Compile(self, object scanner_class=None):
        if scanner_class is None:
            scanner_class = Scanner
        return scanner_class(self)



cdef inline wrap_feature(impl.Feature* feature_impl):
    ret = Feature()
    ret.feature_impl = feature_impl
    return ret


cdef class Feature:
    cdef impl.Feature* feature_impl


% for feature in features:
def ${feature}():
    return wrap_feature(impl.${feature}())
% endfor



cdef class Lexer:
    cdef impl.Lexer lexer_impl

    def __cinit__(self, bytes line=None):
        if line is not None:
            self.lexer_impl.Assign(begin(line), end(line))

    def AddFeature(self, Feature feature):
        if feature.feature_impl == NULL:
            raise ValueError("Empty feature wrapper. Features cannot be reused.")
        self.lexer_impl.AddFeature(feature.feature_impl)
        feature.feature_impl = NULL

    def Parse(self):
        return wrap_fsm(self.lexer_impl.Parse())



cdef class BaseScanner:
    pass


% for Scanner in scanners:
cdef inline object wrap_${Scanner}(impl.${Scanner} scanner_impl):
    ret = ${Scanner}()
    ret.scanner_impl.Swap(scanner_impl)
    return ret


def _${Scanner}_Load(bytes data not None):
    cdef:
        impl.yauto_ptr[impl.yistream] stream
        impl.${Scanner} loaded_scanner
    stream.reset(new impl.yistream(data))
    loaded_scanner.Load(stream.get())
    return wrap_${Scanner}(loaded_scanner)


def _reduce_${Scanner}(instance):
    return _${Scanner}_Load, (instance.Save(),)
copy_reg.pickle(${Scanner}, _reduce_${Scanner})


cdef class ${Scanner}(BaseScanner):
    cdef impl.${Scanner} scanner_impl

    def __cinit__(self, Fsm fsm=None):
        if fsm is not None:
            self.scanner_impl = impl.${Scanner}(fsm.fsm_impl)

    def Save(self):
        cdef impl.yostream stream
        self.scanner_impl.Save(&stream)
        return stream.GetStr()

    Load = _${Scanner}_Load

    % for method in ["Size", "Empty", "RegexpsCount", "LettersCount"]:
    def ${method}(self):
        return self.scanner_impl.${method}()
    % endfor

    def Matches(self, bytes line not None):
        return impl.Matches(self. scanner_impl, begin(line), end(line))
% endfor
