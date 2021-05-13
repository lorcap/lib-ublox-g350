# Ublox.py

from ctypes import *
from Ril import Ril, dll

class Ublox (Ril):

    def rsp_udconf_ok (self, op_code, *params):
        self.rsp_ok('+UDCONF: ' + ','.join(str(i) for i in (op_code, *params)))

    def at_udconf1 (self):
        hex_mode = c_int()
        err = dll.ril_at_udconf1(self._state, byref(hex_mode))
        return err, hex_mode.value

    def at_udconf1_set (self, hex_mode):
        return dll.ril_at_udconf1_set(self._state, hex_mode)
