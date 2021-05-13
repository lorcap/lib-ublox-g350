# Ril.py

from ctypes import *
from Common import *

dll = CDLL('./ril.so')

dll.ril_init.restype = POINTER(ril_state_t)

class Ril:

    def __init__ (self):
        self._command = bytearray() # command buffer
        self._response = bytearray() # response buffer
        self._state = dll.ril_init(cmd_write, py_object(self),
                                   rsp_read , py_object(self), BUF_MAX)

    def __del__ (self):
        dll.ril_deinit(self._state)

    def __repr__ (self):
        return f'Ril({self._command}, {self._response}, {self._state[0]}) at {id(self):#x}'

    def cmd (self):
        cmd = self._command
        self._command = bytearray()
        return cmd.decode('ascii')

    def rsp_ok (self, *lines):
        self.line('') # echo
        for l in lines:
            self.line(l)
        self.line('OK')

    def line (self, string):
        self._response.extend(string.encode('ascii'))
        self._response.extend(b'\r\n')

    def _at_read (self, cmd):
        state = c_int()
        err = cmd(self._state, byref(state))
        return err, state.value


    #--- General operation -------------------------------------------------#

    def at_cgmr (self):
        version = (c_char*BUF_MAX)()
        err = dll.ril_at_cgmr(self._state, byref(version))
        return err, version.value.decode('ascii')

    def at_ccid (self):
        ccid = (c_char*BUF_MAX)()
        err = dll.ril_at_ccid(self._state, byref(ccid))
        return err, ccid.value.decode('ascii')


    #--- General -----------------------------------------------------------#

    def at_cscs (self):
        chset = c_int()
        err = dll.ril_at_cscs(self._state, byref(chset))
        return err, chset.value

    def at_cscs_set (self, chset):
        return dll.ril_at_cscs_set(self._state, chset)


    #--- Mobile equipment control and status -------------------------------#

    def at_cmer (self):
        mode = c_int()
        ind  = c_int()
        bfr  = c_int()
        err = dll.ril_at_cmer(self._state, byref(mode), byref(ind), byref(bfr))
        return err, mode.value, ind.value, bfr.value

    def at_cmer_set (self, mode, ind, bfr):
        return dll.ril_at_cmer_set(self._state, mode, ind, bfr)

    def at_cclk (self):
        year     = c_int()
        month    = c_int()
        day      = c_int()
        hours    = c_int()
        minutes  = c_int()
        seconds  = c_int()
        timezone = c_int()
        err = dll.ril_at_cclk(self._state,
                    byref(year), byref(month), byref(day),
                    byref(hours), byref(minutes), byref(seconds),
                    byref(timezone))
        return err, year.value, month.value, day.value,\
            hours.value, minutes.value, seconds.value, timezone.value

    def at_cclk_set (self,\
                     year, month, day,\
                     hours, minutes, seconds,
                     timezone):
        return dll.ril_at_cclk_set(self._state,
                    year, month, day,
                    hours, minutes, seconds,
                    timezone)

    def at_cmee (self):
        return self._at_read(dll.ril_at_cmee)

    def at_cmee_set (self, n):
        return dll.ril_at_cmee_set(self._state, n)


    #--- Network service ---------------------------------------------------#

    def at_cged (self):
        return self._at_read(dll.ril_at_cged)

    def at_cged_set (self, state):
        return dll.ril_at_cged_set(self._state, state)


    #--- Short Messages Service --------------------------------------------#

    def at_cmgf (self):
        return self._at_read(dll.ril_at_cmgf)

    def at_cmgf_set (self, mode):
        return dll.ril_at_cmgf_set(self._state, mode)

    def at_csdh (self):
        return self._at_read(dll.ril_at_csdh)

    def at_csdh_set (self, mode):
        return dll.ril_at_csdh_set(self._state, mode)


    #--- V24 control and V25ter --------------------------------------------#

    def ate_set (self, value):
        return dll.ril_ate_set(self._state, value)


    #--- Packet switched data servicies ------------------------------------#

    def at_cgatt (self):
        return self._at_read(dll.ril_at_cgatt)

    def at_cgatt_set (self, state):
        return dll.ril_at_cgatt_set(self._state, state)

    def at_cgreg (self):
        n    = c_int()
        stat = c_int()
        lac  = c_int()
        ci   = c_int()
        err = dll.ril_at_cgreg(self._state, byref(n), byref(stat), byref(lac), byref(ci));
        return err, n.value, stat.value, lac.value, ci.value

    def at_cgreg_set (self, n):
        return dll.ril_at_cgreg_set(self._state, n)
