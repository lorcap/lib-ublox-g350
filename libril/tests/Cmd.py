# Cmd.py

from ctypes import *
from Common import *

dll = CDLL('./ril.so')

dll.ril_cmd_at    .argtypes = [POINTER(ril_state_t)]
dll.ril_cmd_atc   .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_cmd_atco  .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_char]
dll.ril_cmd_char  .argtypes = [POINTER(ril_state_t), c_char]
dll.ril_cmd_charn .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_size_t]
dll.ril_cmd_deinit.argtypes = [POINTER(ril_state_t)]
dll.ril_cmd_eol   .argtypes = [POINTER(ril_state_t)]
dll.ril_cmd_hex   .argtypes = [POINTER(ril_state_t), c_uint]
dll.ril_cmd_hexw  .argtypes = [POINTER(ril_state_t), c_uint, c_int]
dll.ril_cmd_init  .argtypes = [POINTER(ril_state_t), cmd_write_t, py_object]
dll.ril_cmd_int   .argtypes = [POINTER(ril_state_t), c_int]
dll.ril_cmd_query .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_cmd_set   .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_cmd_str   .argtypes = [POINTER(ril_state_t), POINTER(c_char)]
dll.ril_cmd_strn  .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_size_t]
dll.ril_cmd_strq  .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_char]
dll.ril_cmd_strqe .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_char, c_char]
dll.ril_cmd_strqq .argtypes = [POINTER(ril_state_t), POINTER(c_char), c_char, c_char]
dll.ril_cmd_strqqe.argtypes = [POINTER(ril_state_t), POINTER(c_char), c_char, c_char, c_char]
dll.ril_cmd_uint  .argtypes = [POINTER(ril_state_t), c_uint]

class Cmd:

    def __init__ (self):
        self._command = bytearray() # command buffer
        self._state = ril_state_t()
        dll.ril_cmd_init(byref(self._state), cmd_write, py_object(self))

    def __del__ (self):
        dll.ril_cmd_deinit(self._state)

    def __repr__ (self):
        return f'Cmd({self._command}, {self._state})'

    @property
    def error (self):
        return self._state.error

    def command (self):
        cmd = self._command
        self._command = bytearray()
        return cmd

    #--- Main --------------------------------------------------------------#

    def printf (self, fmt, *args):
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

        count = dll.ril_cmd_printf(byref(self._state), fmt, *c_args)

        if c_outs:
            for i in range(len(c_outs)):
                c_outs[i] = c_outs[i].value
            return count, *c_outs
        else:
            return count

    def query (self, cmd):
        return dll.ril_cmd_query(self._state, cmd)

    def set (self, cmd):
        return dll.ril_cmd_set(self._state, cmd)


    #--- Chars -------------------------------------------------------------#

    def char (self, c):
        return dll.ril_cmd_char(self._state, c)

    def charn (self, buf):
        return dll.ril_cmd_charn(self._state, buf, len(buf))

    def eol (self):
        return dll.ril_cmd_eol(self._state)


    #--- Strings -----------------------------------------------------------#

    def str (self, string):
        return dll.ril_cmd_str(self._state, string)

    def strn (self, string, n):
        return dll.ril_cmd_strn(self._state, string, n)

    def strq (self, string, quote):
        return dll.ril_cmd_strq(self._state, string, quote)

    def strqe (self, string, quote, escape):
        return dll.ril_cmd_strqe(self._state, string, quote, escape)

    def strqq (self, string, quote_begin, quote_end):
        return dll.ril_cmd_strqq(self._state, string, quote_begin, quote_end)

    def strqqe (self, string, quote_begin, quote_end, escape):
        return dll.ril_cmd_strqqe(self._state, string, quote_begin, quote_end, escape)

    def at (self):
        return dll.ril_cmd_at(self._state)

    def atc (self, cmd):
        return dll.ril_cmd_atc(self._state, cmd)

    def atco (self, cmd, op):
        return dll.ril_cmd_atco(self._state, cmd, op)


    #--- Numbers -----------------------------------------------------------#

    def int (self, i):
        return dll.ril_cmd_int(self._state, i)

    def uint (self, u):
        return dll.ril_cmd_uint(self._state, u)

    def hex (self, x):
        return dll.ril_cmd_hex(self._state, x)

    def hexw (self, x, width):
        return dll.ril_cmd_hexw(self._state, x, width)
