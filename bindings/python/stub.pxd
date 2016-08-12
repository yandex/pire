from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.string cimport string


cdef extern from "sstream" namespace "std":
    cdef cppclass istringstream:
        istringstream(const string&)

    cdef cppclass ostringstream:
        ostringstream()

        string GetStr "str"()

    cdef cppclass auto_ptr[T]:
        auto_ptr()

        T* get()
        T& operator * ()

        void reset(T*)


cdef extern from "pire/pire.h" namespace "Pire":
    ctypedef vector yvector
    ctypedef pair ypair
    ctypedef string ystring

    ctypedef auto_ptr yauto_ptr


ctypedef istringstream yistream
ctypedef ostringstream yostream
