import unittest
from Common import Error
import Ublox

ril = Ublox.Ublox()

class TestRil (unittest.TestCase):

    def test_ublox_ubconf1 (self):
        ril.rsp_udconf_ok(1, 1)
        err, hex_mode = ril.at_udconf1()
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+UDCONF=1\r\n')
        self.assertEqual(hex_mode, 1)

    def test_ublox_ubconf1_set (self):
        ril.rsp_ok()
        err = ril.at_udconf1_set(0)
        self.assertEqual(err, -Error.NONE)
        self.assertEqual(ril.cmd(), 'AT+UDCONF=1,0\r\n')
