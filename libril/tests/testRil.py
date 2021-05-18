import unittest
from Common import Error
import Ril

ril = Ril.Ril()

class TestRil (unittest.TestCase):

    def _at_read (self, cmd, ril_at, state):
        ril.rsp_ok(f'{cmd}: {state}')
        err, state_ = ril_at()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT{cmd}?\r\n')
        self.assertEqual(state, state_)

    def _at_set (self, cmd, ril_at_set, state):
        ril.rsp_ok()
        err = ril_at_set(state)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT{cmd}={state}\r\n')


    #--- General operation -------------------------------------------------#

    def test_ril_at_cgmr_read (self):
        version_ = '11.40'
        ril.rsp_ok(version_)
        err, version = ril.at_cgmr_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CGMR\r\n')
        self.assertEqual(version_, version)

    def test_ril_at_ccid_read (self):
        ccid_ = '8939107800023416395'
        ril.rsp_ok(f'+CCID: {ccid_}')
        err, ccid = ril.at_ccid_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CCID\r\n')
        self.assertEqual(ccid_, ccid)


    #--- General -----------------------------------------------------------#

    def test_ril_at_cscs_read (self):
        ril.rsp_ok('+CSCS: "IRA"')
        err, chset = ril.at_cscs_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CSCS?\r\n')
        self.assertEqual(chset, 4)

    def test_ril_at_cscs_set (self):
        ril.rsp_ok()
        err = ril.at_cscs_set(4)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CSCS="IRA"\r\n')


    #--- Mobile equipment control and status -------------------------------#

    def test_ril_at_cmer_read (self):
        ril.rsp_ok('+CMER: 1,0,0,0,1')
        err, mode, ind, bfr = ril.at_cmer_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CMER?\r\n')
        self.assertEqual(mode, 1)
        self.assertEqual(ind , 0)
        self.assertEqual(bfr , 1)

    def test_ril_at_cmer_set (self):
        ril.rsp_ok()
        err = ril.at_cmer_set(1, 2, 1)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CMER=1,0,0,2,1\r\n')

    def test_ril_at_cclk_read (self):
        ril.rsp_ok(f'+CCLK: "14/07/01,15:00:00+01"')
        err, year, month, day, hours, minutes, seconds, timezone = ril.at_cclk_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CCLK?\r\n')
        self.assertEqual(year    , 2014)
        self.assertEqual(month   ,    7)
        self.assertEqual(day     ,    1)
        self.assertEqual(hours   ,   15)
        self.assertEqual(minutes ,    0)
        self.assertEqual(seconds ,    0)
        self.assertEqual(timezone,   15)

    def test_ril_at_cclk_set (self):
        ril.rsp_ok()
        err = ril.at_cclk_set(2014, 7, 1, 15, 0, 0, 15)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CCLK="14/07/01,15:00:00+01"\r\n')

    def test_ril_at_cmee_read (self):
        self._at_read('+CMEE', ril.at_cmee_read, 2)

    def test_ril_at_cged_set (self):
        self._at_set('+CMEE', ril.at_cmee_set, 2)


    #--- Network service ---------------------------------------------------#

    def test_ril_at_cged_read (self):
        self._at_read('+CGED', ril.at_cged_read, 3)

    def test_ril_at_cged_set (self):
        self._at_set('+CGED', ril.at_cged_set, 3)


    #--- Short Messages Service --------------------------------------------#

    def test_ril_at_cmgf_read (self):
        self._at_read('+CMGF', ril.at_cmgf_read, 1)

    def test_ril_at_cmgf_set (self):
        self._at_set('+CMGF', ril.at_cmgf_set, 1)

    def test_ril_at_csdh_read (self):
        self._at_read('+CMGF', ril.at_csdh_read, 0)

    def test_ril_at_csdh_set (self):
        self._at_set('+CMGF', ril.at_csdh_set, 1)

    def test_ril_at_cnmi_read (self):
        ril.rsp_ok('+CNMI: 0,0,0,0,0')
        err, mode, mt, bm, ds, bfr = ril.at_cnmi_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CNMI?\r\n')
        self.assertEqual(mode, 0)
        self.assertEqual(mt, 0)
        self.assertEqual(bm, 0)
        self.assertEqual(ds, 0)
        self.assertEqual(bfr, 0)

    def test_ril_at_cnmi_set (self):
        ril.rsp_ok()
        err = ril.at_cnmi_set(1, 1, 0, 0, 0)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CNMI=1,1,0,0,0\r\n')

    def test_ril_at_csca_read (self):
        ril.rsp_ok('+CSCA: "",129')
        err, csa, tocsa = ril.at_csca_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CSCA?\r\n')
        self.assertEqual(csa, '')
        self.assertEqual(tocsa, 129)

    def test_ril_at_csca_set (self):
        csa = '0170111000'
        ril.rsp_ok()
        err = ril.at_csca_set(csa)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), f'AT+CSCA="{csa}"\r\n')


    #--- V24 control and V25ter --------------------------------------------#

    def test_ril_ate_set (self):
        ril.rsp_ok()
        err = ril.ate_set(1)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'ATE1\r\n')


    #--- Packet switched data servicies ------------------------------------#

    def test_ril_at_cgatt_read (self):
        self._at_read('+CGATT', ril.at_cgatt_read, 1)

    def test_ril_at_cgatt_set (self):
        self._at_set('+CGATT', ril.at_cgatt_set, 1)

    def test_ril_at_cgreg0_read (self):
        ril.rsp_ok(f'+CGREG: 0,4')
        err, n, stat, lac, ci = ril.at_cgreg_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CGREG?\r\n')
        self.assertEqual(n   , 0)
        self.assertEqual(stat, 4)

    def test_ril_at_cgreg2_read (self):
        ril.rsp_ok(f'+CGREG: 2,1,"61EF","7D58A3"')
        err, n, stat, lac, ci = ril.at_cgreg_read()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+CGREG?\r\n')
        self.assertEqual(n   ,        2)
        self.assertEqual(stat,        1)
        self.assertEqual(lac ,   0x61EF)
        self.assertEqual(ci  , 0x7D58A3)

    def test_ril_at_cgreg_set (self):
        self._at_set('+CGREG', ril.at_cgreg_set, 1)

if __name__ == '__main__':
    unittest.main()
