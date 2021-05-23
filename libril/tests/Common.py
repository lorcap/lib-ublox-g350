# Ril.py

import enum
import time
from ctypes import *

BUF_MAX = 32

# write callback function
def write (self, c):
    assert len(c) == 1
    self._command.append(c[0])
    return 1

cmd_write_t = CFUNCTYPE(c_int, py_object, c_char)
cmd_write = cmd_write_t(write)


# read callback function
def read (self, timeout):
    if timeout:
        self.timeout = timeout
        self._timestamp = time.monotonic_ns()
    elif self.timeout <= (time.monotonic_ns() - self._timestamp)//1_000_000:
        return -2
    if not len(self._response):
        return -2
    return self._response.pop(0)

rsp_read_t = CFUNCTYPE(c_int, py_object, c_uint)
rsp_read = rsp_read_t(read)


# internal state
class ril_state_t (Structure):
    _fields_ = [('error'    , c_int),
                ('cm_err'   , c_int),
                ('write'    , cmd_write_t),
                ('write_obj', c_void_p),
                ('read'     , rsp_read_t),
                ('read_obj' , c_void_p),
                ('count'    , c_size_t),
                ('index'    , c_size_t),
                ('timeout'  , c_uint),
                ('buf_max'  , c_size_t),
                ('buf'      , c_char*BUF_MAX)]

    def __repr__ (self):
        write_obj = int(self.write_obj) if self.write_obj else 0
        read_obj = int(self.read_obj) if self.read_obj else 0
        return 'ril_state_t('\
                    f'error:{self.error}, cm_err:{self.cm_err}, '\
                    f'write:{self.write}, write_obj:{write_obj:#x}, '\
                    f'read:{self.read}, read_obj:{read_obj:#x}, '\
                    f'count:{self.count}, index:{self.index}, '\
                    f'timeout:{self.timeout}, '\
                    f'buf_max:{self.buf_max}, buf:{self.buf}'\
                f') at {id(self):#x} -> {id(self.count):#x})'

class Error(enum.IntEnum):
        NONE                 = 0
        BAD_PARAMETER        = 1
        CMD_WRITE            = 2
        READ_OVERFLOW        = 3
        READ_TIMEOUT         = 4
        READ_GENERAL         = 5
        RSP_CHAR             = 6
        RSP_CHARP            = 7
        RSP_ECHO             = 8
        RSP_EOL              = 9
        RSP_FINAL_ABORT      = 10
        RSP_FINAL_CME        = 11
        RSP_FINAL_CMS        = 12
        RSP_FINAL_ERROR      = 13
        RSP_FINAL_UNKNOWN    = 14
        RSP_HEX              = 15
        RSP_INT              = 16
        RSP_LINE             = 17
        RSP_LINE_DUMP        = 18
        RSP_QUERY            = 19
        RSP_STR              = 20
        RSP_STRA_NONE        = 21
        RSP_STRA_OVERFLOW    = 22
        RSP_STRA_UNDERFLOW   = 23
        RSP_STRPN            = 24
        RSP_STRQQE_BEGIN     = 25
        RSP_STRQQE_END       = 26
        RSP_STRQQE_NO_QUOTES = 27
        RSP_UINT             = 28
