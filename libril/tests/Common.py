# Ril.py

import enum
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
def read (self):
    return self._response.pop(0) if len(self._response) else 0

rsp_read_t = CFUNCTYPE(c_char, py_object)
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
                    f'buf_max:{self.buf_max}, buf:{self.buf}'\
                f') at {id(self):#x} -> {id(self.count):#x})'

class Error(enum.IntEnum):
        NONE                 = 0
        BAD_PARAMETER        = 1
        CMD_WRITE            = 2
        RSP_CHAR             = 3
        RSP_CHARP            = 4
        RSP_ECHO             = 5
        RSP_EOL              = 6
        RSP_FINAL_ABORT      = 7
        RSP_FINAL_CME        = 8
        RSP_FINAL_CMS        = 9
        RSP_FINAL_ERROR      = 10
        RSP_FINAL_UNKNOWN    = 11
        RSP_HEX              = 12
        RSP_INT              = 13
        RSP_LINE             = 14
        RSP_LINE_DUMP        = 15
        RSP_QUERY            = 16
        RSP_STR              = 17
        RSP_STRA_NONE        = 18
        RSP_STRA_OVERFLOW    = 19
        RSP_STRA_UNDERFLOW   = 20
        RSP_STRPN            = 21
        RSP_STRQQE_BEGIN     = 22
        RSP_STRQQE_END       = 23
        RSP_STRQQE_NO_QUOTES = 24
        RSP_UINT             = 25
