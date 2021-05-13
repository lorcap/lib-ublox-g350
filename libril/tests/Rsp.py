# AtRsp.py

from ctypes import *
from Common import *

dll = CDLL('./ril.so')

dll.ril_rsp_char            .argtypes = [POINTER(ril_state_t), c_char]
dll.ril_rsp_charn           .argtypes = [POINTER(ril_state_t), c_size_t, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_charp           .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_deinit          .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_echo            .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_eol             .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_final           .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_flush           .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_hex             .argtypes = [POINTER(ril_state_t), POINTER(c_uint)]
dll.ril_rsp_init            .argtypes = [POINTER(ril_state_t), rsp_read_t, py_object, c_size_t]
dll.ril_rsp_int             .argtypes = [POINTER(ril_state_t), POINTER(c_int)]
dll.ril_rsp_line            .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_line_abort      .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_line_dump       .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_line_error      .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_line_ok         .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_match_char      .argtypes = [POINTER(ril_state_t), c_char]
dll.ril_rsp_match_charp     .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_match_eol       .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_match_line_query.argtypes = [POINTER(ril_state_t), POINTER(c_char), POINTER(c_int)]
dll.ril_rsp_match_query     .argtypes = [POINTER(ril_state_t), POINTER(c_char), POINTER(c_int)]
dll.ril_rsp_match_str       .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_match_strp      .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_match_strpn     .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_size_t]
dll.ril_rsp_query           .argtypes = [POINTER(ril_state_t), POINTER(c_char), POINTER(c_int)]
dll.ril_rsp_res             .argtypes = [POINTER(ril_state_t), c_bool]
dll.ril_rsp_res_abort       .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_res_ok          .argtypes = [POINTER(ril_state_t)]
dll.ril_rsp_res_ok_str      .argtypes = [POINTER(ril_state_t), POINTER(c_char*BUF_MAX)]
dll.ril_rsp_res_str         .argtypes = [POINTER(ril_state_t), c_bool, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_seek_char       .argtypes = [POINTER(ril_state_t), c_char]
dll.ril_rsp_str             .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_rsp_stra            .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_void_p, c_size_t, c_size_t, c_size_t]
dll.ril_rsp_strp            .argtypes = [POINTER(ril_state_t), POINTER(c_char), POINTER(c_char*BUF_MAX)]
dll.ril_rsp_strpn           .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_size_t, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_strq            .argtypes = [POINTER(ril_state_t), c_char, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_strqe           .argtypes = [POINTER(ril_state_t), c_char, c_char, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_strqq           .argtypes = [POINTER(ril_state_t), c_char, c_char, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_strqqe          .argtypes = [POINTER(ril_state_t), c_char, c_char, c_char, POINTER(c_char*BUF_MAX)]
dll.ril_rsp_uint            .argtypes = [POINTER(ril_state_t), POINTER(c_uint)]

class Rsp:

    def __init__ (self):
        self._response = bytearray() # response buffer
        self._buf = c_char*BUF_MAX
        self._state = ril_state_t()
        dll.ril_rsp_init(byref(self._state), rsp_read, py_object(self), BUF_MAX)

    def __del__ (self):
        dll.ril_rsp_deinit(self._state)

    def __repr__ (self):
        return f'Rsp({self._response}, {self._state})'

    @property
    def error (self):
        return self._state.error

    @property
    def cm_err (self):
        return self._state.cm_err

    @property
    def index (self):
        return self._state.index

    def response (self, rsp):
        self._response.extend(rsp)

    def tail (self):
        return self._state.buf[:self._state.count] + self._response


    #--- Main --------------------------------------------------------------#

    def echo (self):
        return dll.ril_rsp_echo(self._state)

    def final (self):
        return dll.ril_rsp_final(self._state)

    def scanf (self, fmt, *args):
        c_args = []
        c_outs = []

        for a in args:
            l = len(c_outs)
            if type(a) == int:
                c_args.append(c_int(a))
            elif type(a) == bytes and len(a) == 1:
                c_args.append(c_char(ord(a)))
            elif type(a) == bytes and len(a) > 1:
                c_args.append(a)
            elif issubclass(a, c_char_p):
                c_outs.append((c_char*BUF_MAX)())
            elif issubclass(a, c_int):
                c_outs.append(c_int())
            elif issubclass(a, c_uint):
                c_outs.append(c_uint())
            else:
                print(type(a))
                raise TypeError
            if (l < len(c_outs)):
                c_args.append(byref(c_outs[-1]))

        count = dll.ril_rsp_scanf(byref(self._state), fmt, *c_args)

        if c_outs:
            for i in range(len(c_outs)):
                c_outs[i] = c_outs[i].value
            return count, *c_outs
        else:
            return count

    def query (self, cmd):
        val = c_int()
        return dll.ril_rsp_query(self._state, cmd, byref(val)), val.value

    def stra (self, pattern, array, array_size, element_size, offset):
        return dll.ril_rsp_stra(self._state, pattern, byref(array), array_size, element_size, offset)

    def flush (self):
        return dll.ril_rsp_flush(self._state)


    #--- General -----------------------------------------------------------#


    #--- Numbers -----------------------------------------------------------#

    def int (self):
        val = c_int()
        return dll.ril_rsp_int(self._state, byref(val)), val.value

    def uint (self):
        val = c_uint()
        return dll.ril_rsp_uint(self._state, byref(val)), val.value

    def hex (self):
        val = c_uint()
        return dll.ril_rsp_hex(self._state, byref(val)), val.value


    #--- Chars -------------------------------------------------------------#

    def match_char (self, c):
        return dll.ril_rsp_match_char(self._state, c_char(ord(c)))

    def match_charp (self, pattern):
        return dll.ril_rsp_match_charp(self._state, pattern)

    def seek_char (self, c):
        return dll.ril_rsp_seek_char(self._state, c_char(ord(c)))

    def char (self, c):
        return dll.ril_rsp_char(self._state, c_char(ord(c)))

    def charn (self, n):
        assert n <= BUF_MAX
        buf = (c_char*BUF_MAX)()
        return (dll.ril_rsp_charn(self._state, n, byref(buf)), buf.value)

    def charp (self, pattern):
        return dll.ril_rsp_charp(self._state, pattern)


    #--- Strings -----------------------------------------------------------#

    def match_str (self, string):
        return dll.ril_rsp_match_str(self._state, string)

    def match_strp (self, pattern):
        count = dll.ril_rsp_match_strp(self._state, pattern)
        assert count <= BUF_MAX
        return count

    def match_strpn (self, pattern, n):
        assert n <= BUF_MAX
        return dll.ril_rsp_match_strpn(self._state, pattern, n)

    def match_eol (self):
        return dll.ril_rsp_match_eol(self._state)

    def str (self, string):
        return dll.ril_rsp_str(self._state, string)

    def strp (self, pattern):
        buf = (c_char*BUF_MAX)()
        count = dll.ril_rsp_strp(self._state, pattern, byref(buf))
        assert count <= BUF_MAX
        return (count, buf.value)

    def strpn (self, pattern, n):
        assert n <= BUF_MAX
        buf = (c_char*BUF_MAX)()
        return (dll.ril_rsp_strpn(self._state, pattern, n, byref(buf)), buf.value)

    def strq (self, quote):
        buf = (c_char*BUF_MAX)()
        count = dll.ril_rsp_strq(self._state, quote, byref(buf))
        assert count <= BUF_MAX
        return (count, buf.value)

    def strqe (self, quote, escape):
        buf = (c_char*BUF_MAX)()
        count = dll.ril_rsp_strqe(self._state, quote, escape, byref(buf))
        assert count <= BUF_MAX
        return (count, buf.value)

    def strqq (self, quote_begin, quote_end):
        buf = (c_char*BUF_MAX)()
        count = dll.ril_rsp_strqq(self._state, quote_begin, quote_end, byref(buf))
        assert count <= BUF_MAX
        return (count, buf.value)

    def strqqe (self, quote_begin, quote_end, escape):
        buf = (c_char*BUF_MAX)()
        count = dll.ril_rsp_strqqe(self._state, quote_begin, quote_end, escape, byref(buf))
        return (count, buf.value)

    def eol (self):
        return dll.ril_rsp_eol(self._state)


    #--- Lines -------------------------------------------------------------#

    def line (self, string):
        return dll.ril_rsp_line(self._state, string)

    def line_abort (self):
        return dll.ril_rsp_line_abort(self._state)

    def line_error (self):
        return dll.ril_rsp_line_error(self._state)

    def line_ok (self):
        return dll.ril_rsp_line_ok(self._state)

    def line_dump (self):
        return dll.ril_rsp_line_dump(self._state)
