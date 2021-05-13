import unittest
from ctypes import c_char, c_char_p, c_int, c_uint, sizeof, Structure
from Common import Error
import Rsp

class element_t (Structure):
    _fields_ = [('prefix', c_int),
                ('string', c_char*5)]

rsp = Rsp.Rsp()

class TestRsp (unittest.TestCase):

    def setUp (self):
        rsp.flush()

    def test_rsp01_init (self):
        self.assertEqual(rsp._state.count, 0)
        self.assertEqual(rsp._state.index, 0)
        self.assertEqual(rsp._state.read_obj, id(rsp))
        self.assertEqual(rsp._state.buf_max, Rsp.BUF_MAX)

    def test_rsp02_flush (self):
        s = b'garbage'
        rsp.response(s)
        count = rsp.flush()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(len(rsp._response), 0)

    def test_rsp03_match_char (self):
        c = b'c'
        rsp.response(c)
        count = rsp.match_char(c)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp04_match_charp (self):
        c = b'c'
        rsp.response(c)
        count = rsp.match_charp(b'c')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp05_match_charp_not (self):
        b = b'ctail'
        rsp.response(b)
        count = rsp.match_charp(b'C')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.index, 0)

    def test_rsp06_match_charp_tail (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.match_charp(b'c')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp07_match_charp_neg (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.match_charp(b'^C')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp08_match_charp_range (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.match_charp(b'a-z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp09_match_charp_range_not (self):
        b = b'ctail'
        rsp.response(b)
        count = rsp.match_charp(b'A-Z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.index, 0)

    def test_rsp10_match_charp_range_neg (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.match_charp(b'^A-Z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.index, len(c))

    def test_rsp11_seek_char (self):
        s = b'ab'
        t = b'cacbc'
        rsp.response(s+t)
        count = rsp.seek_char('c')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.tail(), s+t)

    def test_rsp12_char (self):
        c = b'c'
        rsp.response(c)
        count = rsp.char(c)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), b'')

    def test_rsp13_charp (self):
        c = b'c'
        rsp.response(c)
        count = rsp.charp(b'c')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), b'')

    def test_rsp14_charp_not (self):
        b = b'ctail'
        rsp.response(b)
        count = rsp.charp(b'C')
        self.assertEqual(rsp.error, -Error.RSP_CHARP)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.tail(), b)

    def test_rsp15_charp_tail (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.charp(b'c')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), t)

    def test_rsp16_charp_neg (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.charp(b'^C')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), t)

    def test_rsp17_charp_range (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.charp(b'a-z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), t)

    def test_rsp18_charp_range_not (self):
        b = b'ctail'
        rsp.response(b)
        count = rsp.charp(b'A-Z')
        self.assertEqual(rsp.error, -Error.RSP_CHARP)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.tail(), b)

    def test_rsp19_charp_range_neg (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.charp(b'^A-Z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), t)

    def test_rsp20_match_eol (self):
        e = b'\r\n'
        t = b'tail'
        rsp.response(e+t)
        count = rsp.match_eol()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(e))
        self.assertEqual(rsp.tail(), e+t)

    def test_rsp21_eol (self):
        e = b'\r\n'
        t = b'tail'
        rsp.response(e+t)
        count = rsp.eol()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(e))
        self.assertEqual(rsp.tail(), t)

    def test_rsp22_uint (self):
        u = 0xdeadbeef
        b = str(u).encode('ascii')
        rsp.response(b)
        count, val = rsp.uint()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, u)

    def test_rsp23_uint_trail (self):
        u = 123
        b = str(u).encode('ascii')
        t = b'tail'
        rsp.response(b+t)
        count, val = rsp.uint()
        self.assertEqual(count, len(b))
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(val, u)
        self.assertEqual(rsp.tail(), t)

    def test_rsp24_int (self):
        i = 24680
        b = str(i).encode('ascii')
        rsp.response(b)
        count, val = rsp.int()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, i)

    def test_rsp25_int_neg (self):
        i = -24680
        b = str(i).encode('ascii')
        rsp.response(b)
        count, val = rsp.int()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, i)

    def test_rsp26_int_pos (self):
        i = 24680
        b = b'+' + str(i).encode('ascii')
        rsp.response(b)
        count, val = rsp.int()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, i)

    def test_rsp27_hex (self):
        b = b'deadBEEF'
        i = int(b, 16)
        t = b'tail'
        rsp.response(b+t)
        count, val = rsp.hex()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, i)
        self.assertEqual(rsp.tail(), t)

    def test_rsp28_charn (self):
        s = b'byten'
        t = b'tail'
        rsp.response(s+t)
        rsp.int() # move one char from Python's to C's buffer
        rsp._state.error = 0
        count, val = rsp.charn(len(s))
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp29_match_strp (self):
        s = b'STRING'
        rsp.response(s)
        count = rsp.match_strp(b'A-Z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.index, len(s))

    def test_rsp30_strp (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.strp(b'A-Z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp31_match_str (self):
        s = b'STRING'
        rsp.response(s)
        count = rsp.match_str(s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.index, len(s))

    def test_rsp32_match_str_str (self):
        s = b'STRING'
        rsp.response(s*2)
        count = rsp.match_str(s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.index, len(s))
        count = rsp.match_str(s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.index, len(s)*2)

    def test_rsp33_match_str_not (self):
        S = b'STRING'
        s = S[:-1]
        rsp.response(s)
        count = rsp.match_str(S)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.index, 0)

    def test_rsp34_str (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count = rsp.str(s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.tail(), t)

    def test_rsp35_str_not (self):
        S = b'STRING'
        s = S[:-1]
        t = b'tail'
        b = s+t
        rsp.response(b)
        count = rsp.str(S)
        self.assertEqual(rsp.error, -Error.RSP_STR)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.tail(), b)

    def test_rsp36_match_strpn_neg (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count = rsp.match_strpn(b'^a-z', 10)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))

    def test_rsp37_strpn_neg (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.strpn(b'^a-z', 10)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp38_strqqe (self):
        s = b'<string\\>>'
        assert len(s) == 10
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.strqqe(b'<', b'>', b'\\')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp39_strqq (self):
        s = b'<>'
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.strqq(b'<', b'>')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp40_strqe (self):
        s = b'"string\\""'
        assert len(s) == 10
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.strqe(b'"', b'\\')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp41_strq (self):
        s = b'"string"'
        t = b'"tail'
        rsp.response(s+t)
        count, val = rsp.strq(b'"')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp42_strq_not (self):
        s = b'"string'
        rsp.response(s)
        count, val = rsp.strq(b'"')
        self.assertEqual(count, 0)
        self.assertEqual(rsp.error, -Error.RSP_STRQQE_END)

    def _test_stra (self, array, res = None):
        for i in range(len(array)):
            with self.subTest(i=i):
                rsp.response(array[i].string)
                target = rsp.stra(b'A-Z', array, sizeof(array), sizeof(element_t), sizeof(c_int))
                self.assertEqual(target, i if res == None else res)

    def test_rsp43_stra_a_b_c_d (self):
        self._test_stra((element_t*4)((0, b'A'), (1, b'B'), (2, b'C'), (3, b'D')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp44_stra_aa_ab_ac_ad (self):
        self._test_stra((element_t*4)((0, b'AA'), (1, b'AB'), (2, b'AC'), (3, b'AD')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp45_stra_aa_ab_ba_bb (self):
        self._test_stra((element_t*4)((0, b'AA'), (1, b'AB'), (2, b'BA'), (3, b'BB')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp46_stra_a(self):
        self._test_stra((element_t*1)((0, b'A')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp47_stra_a_aa (self):
        self._test_stra((element_t*2)((0, b'A'), (1, b'AA')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp48_stra_a_aa_aaa(self):
        self._test_stra((element_t*3)((0, b'A'), (1, b'AA'), (2, b'AAA')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp49_stra_a_aa_aaa_aaaa (self):
        self._test_stra((element_t*4)((0, b'A'), (1, b'AA'), (2, b'AAA'), (3, b'AAAA')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp50_stra_aaaa_bbb_cc_d (self):
        self._test_stra((element_t*4)((0, b'AAAA'), (1, b'BBB'), (2, b'CC'), (3, b'D')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp51_stra_a_aa_b_bb (self):
        self._test_stra((element_t*4)((0, b'A'), (1, b'AA'), (2, b'B'), (3, b'BB')))
        self.assertEqual(rsp.error, -Error.NONE)

    def test_rsp52_stra_not (self):
        self._test_stra((element_t*2)((0, b'a'), (1, b'aa')), -1)
        self.assertEqual(rsp.error, -Error.RSP_STRA_NONE)

    def test_rsp53_line (self):
        s = b'this is a line'
        l = s + b'\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.line(s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp54_line_abort (self):
        l = b'ABORT\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.line_abort()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp55_line_error (self):
        l = b'ERROR\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.line_error()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp56_line_ok (self):
        l = b'OK\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.line_ok()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp57_line_dump (self):
        l = b'foo bar\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.line_dump()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp58_echo (self):
        l = b'ATCMD\r\n'
        rsp.response(l)
        count = rsp.echo()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))

    def test_rsp59_echo_empty (self):
        l = b'\r\n'
        rsp.response(l)
        count = rsp.echo()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))

    def test_rsp60_echo_not (self):
        l = b'foo\r\n'
        rsp.response(l)
        count = rsp.echo()
        self.assertEqual(rsp.error, -Error.RSP_ECHO)
        self.assertEqual(count, 0)

    def test_rsp61_query (self):
        c = b'CMD'
        b = c + b': 1'
        t = b'tail'
        rsp.response(b+t)
        count, val = rsp.query(c)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, 1)
        self.assertEqual(rsp.tail(), t)

    def test_rsp62_scanf_percentage (self):
        c = b'%'
        rsp.response(c)
        b = rsp.scanf(b'%%', c)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertTrue(b)
        self.assertEqual(rsp.tail(), b'')

    def test_rsp63_scanf_eol (self):
        e = b'\r\n'
        t = b'tail'
        rsp.response(e+t)
        count = rsp.scanf(b'%$')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(e))
        self.assertEqual(rsp.tail(), t)

    def test_rsp64_scanf_char (self):
        c = b'c'
        rsp.response(c)
        count = rsp.scanf(b'%c', c)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), b'')

    def test_rsp65_scanf_charp_range (self):
        c = b'c'
        t = b'tail'
        rsp.response(c+t)
        count = rsp.scanf(b'%/c', b'a-z')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(rsp.tail(), t)

    def test_rsp66_scanf_charn_hash (self):
        s = b'byten'
        t = b'tail'
        rsp.response(s+t)
        rsp.int() # move one char from Python's to C's buffer
        rsp._state.error = 0
        count, val = rsp.scanf(b'%#c', len(s), c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp67_scanf_charn_len (self):
        s = b'byten'
        t = b'tail'
        rsp.response(s+t)
        rsp.int() # move one char from Python's to C's buffer
        rsp._state.error = 0
        count, val = rsp.scanf(f'%{len(s)}c'.encode('ascii'), c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp68_scanf_uint (self):
        u = 0x7eadbeef
        b = str(u).encode('ascii')
        rsp.response(b)
        count, val = rsp.scanf(b'%u', c_int)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, u)

    def test_rsp69_scanf_int (self):
        i = 24680
        b = str(i).encode('ascii')
        rsp.response(b)
        count, val = rsp.scanf(b'%d', c_int)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, i)

    def test_rsp70_scanf_hex (self):
        b = b'deadBEEF'
        x = int(b, 16)
        t = b'tail'
        rsp.response(b+t)
        count, val = rsp.scanf(b'%x', c_uint)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(val, x)
        self.assertEqual(rsp.tail(), t)

    def test_rsp71_scanf_str (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count = rsp.scanf(b'%s', s)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(rsp.tail(), t)

    def test_rsp72_scanf_strpn_neg (self):
        s = b'STRING'
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%/#s', b'^a-z', 10, c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s)
        self.assertEqual(rsp.tail(), t)

    def test_rsp73_scanf_strqqe (self):
        s = b'<string\\>>'
        assert len(s) == 10
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%<>|s', b'<', b'>', b'\\', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp74_scanf_strqq (self):
        s = b'<>'
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%<>s', b'<', b'>', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp75_scanf_strqe (self):
        s = b'"string\\""'
        assert len(s) == 10
        t = b'tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%\'|s', b'"', b'\\', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp76_scanf_strq (self):
        s = b'"string"'
        t = b'"tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%\'s', b'"', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp77_scanf_str_dquote (self):
        s = b'"string"'
        t = b'"tail'
        rsp.response(s+t)
        count, val = rsp.scanf(b'%"s', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(val, s[1:-1])
        self.assertEqual(rsp.tail(), t)

    def test_rsp78_scanf (self):
        i = 24680
        s = 'string'
        r = f'AT: {i},{s}\r\n'.encode('ascii')
        rsp.response(r)
        count, vi, vs = rsp.scanf(b'AT: %d,%/s%$', c_int, b'^,\r', c_char_p)
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(r))
        self.assertEqual(vi, i)
        self.assertEqual(vs, s.encode('ascii'))

    def test_rsp79_scanf_discard (self):
        i = 24680
        s = 'string'
        r = f'AT: {i},{s}\r\n'.encode('ascii')
        rsp.response(r)
        count = rsp.scanf(b'AT: %*d,%*/s%$', b'^,\r')
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(r))

    def test_rsp80_final_ok (self):
        l = b'OK\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.NONE)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp81_final_error (self):
        l = b'ERROR\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.RSP_FINAL_ERROR)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp82_final_abort (self):
        l = b'ABORT\r\n'
        t = b'tail'
        rsp.response(l+t)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.RSP_FINAL_ABORT)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp83_final_cme (self):
        err = 123
        l = f'+CME ERROR: {err}\r\n'.encode('ascii')
        t = b'tail'
        rsp.response(l+t)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.RSP_FINAL_CME)
        self.assertEqual(rsp.cm_err, err)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp84_final_cms (self):
        err = 123
        l = f'+CMS ERROR: {err}\r\n'.encode('ascii')
        t = b'tail'
        rsp.response(l+t)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.RSP_FINAL_CMS)
        self.assertEqual(rsp.cm_err, err)
        self.assertEqual(count, len(l))
        self.assertEqual(rsp.tail(), t)

    def test_rsp85_final_unknown (self):
        err = 123
        l = b'unknown\r\ntail'
        rsp.response(l)
        count = rsp.final()
        self.assertEqual(rsp.error, -Error.RSP_FINAL_UNKNOWN)
        self.assertEqual(count, 0)
        self.assertEqual(rsp.tail(), l)

if __name__ == '__main__':
    unittest.main()
