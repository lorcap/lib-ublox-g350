import unittest
from ctypes import c_int, c_char, sizeof, Structure
from Common import Error
import Cmd

cmd = Cmd.Cmd()

class TestCmd (unittest.TestCase):

    def test_cmd01_init (self):
        self.assertEqual(cmd._state.write_obj, id(cmd))

    def test_cmd02_char (self):
        c = b'c'
        count = cmd.char(c)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(cmd.command(), c)

    def test_cmd03_charn (self):
        b = b'string'
        count = cmd.charn(b)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd04_charn_none (self):
        b = b''
        count = cmd.charn(b)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd05_eol (self):
        count = cmd.eol()
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, 2)
        self.assertEqual(cmd.command(), b'\r\n')

    def test_cmd06_int (self):
        i = 0x012345678
        v = f'{i:d}'.encode('ascii')
        count = cmd.int(i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(v))
        self.assertEqual(cmd.command(), v)

    def test_cmd07_hex (self):
        i = 0x012345678
        v = f'{i:x}'.encode('ascii')
        count = cmd.hex(i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(v))
        self.assertEqual(cmd.command(), v)

    def test_cmd08_hexw (self):
        i = 0x1234
        w = 8
        v = f'{i:0{w}x}'.encode('ascii')
        count = cmd.hexw(i, w)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(v))
        self.assertEqual(cmd.command(), v)

    def test_cmd09_str (self):
        s = b'string'
        t = b'tail'
        count = cmd.str(s + b'\x00' + t)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd10_strn (self):
        s = b'string'
        t = b'tail'
        for i in range(2):
            with self.subTest(i=i):
                count = cmd.strn(s + b'\x00' + t, len(s) + i)
                self.assertEqual(cmd.error, -Error.NONE)
                self.assertEqual(count, len(s))
                self.assertEqual(cmd.command(), s)

    def test_cmd11_strq (self):
        s = b'string'
        q = b'"'
        qsq = q + s + q
        count = cmd.strq(s, q)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd12_strqq (self):
        s = b'string'
        qb = b'<'
        qe = b'>'
        qsq = qb + s + qe
        count = cmd.strqq(s, qb, qe)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd13_strqe (self):
        s = b'|str/ing|'
        q = b'|'
        e = b'/'
        qsq = q + s.replace(e, e+e)\
                   .replace(q, e+q)\
            + q
        count = cmd.strqe(s, q, e)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd14_strqqe (self):
        s = b'<str/ing>'
        qb = b'<'
        qe = b'>'
        e = b'/'
        qsq = qb + s.replace(e , e+e )\
                    .replace(qb, e+qb)\
                    .replace(qe, e+qe)\
            + qe
        count = cmd.strqqe(s, qb, qe, e)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd15_at (self):
        s = b'AT'
        count = cmd.at()
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd16_atc (self):
        c = b'CMD'
        s = b'AT' + c
        count = cmd.atc(c)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd17_atco (self):
        c = b'CMD'
        o = b'?'
        s = b'AT' + c + o
        count = cmd.atco(c, o)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd18_query (self):
        c = b'CMD'
        s = b'AT' + c + b'?\r\n'
        count = cmd.query(c)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd19_set (self):
        c = b'CMD'
        s = b'AT' + c + b'='
        count = cmd.set(c)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd20_printf_percentage (self):
        count = cmd.printf(b'%%')
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, 1)
        self.assertEqual(cmd.command(), b'%')

    def test_cmd21_printf_eol (self):
        count = cmd.printf(b'%$')
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, 2)
        self.assertEqual(cmd.command(), b'\r\n')

    def test_cmd22_printf_char (self):
        count = cmd.printf(b'%c', b'c')
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, 1)
        self.assertEqual(cmd.command(), b'c')

    def test_cmd23_printf_charn (self):
        c = b'abc'
        t = b'tail'
        count = cmd.printf(f'%{len(c)}c'.encode('ascii'), c+t, len(c))
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(c))
        self.assertEqual(cmd.command(), c)

    def test_cmd24_printf_int (self):
        i = 24680
        b = f'{i}'.encode('ascii')
        count = cmd.printf(b'%d', i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd25_printf_intp (self):
        i = 24680
        b = f'{i:+}'.encode('ascii')
        count = cmd.printf(b'%+d', i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd26_printf_intw (self):
        i = -24680
        w = 8
        b = f'{i:0{w}}'.encode('ascii')
        count = cmd.printf(f'%{w}d'.encode('ascii'), i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd27_printf_intpw (self):
        i = 24680
        w = 8
        b = f'{i:+0{w}}'.encode('ascii')
        count = cmd.printf(f'%+{w}d'.encode('ascii'), i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

    def test_cmd28_printf_str (self):
        s = b'string'
        count = cmd.printf(b'%s', s+b'\00')
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd29_printf_strn (self):
        s = b'string'
        n = len(s) + 1
        count = cmd.printf(b'%*s', n, s+b'\00')
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(s))
        self.assertEqual(cmd.command(), s)

    def test_cmd30_printf_str_dquote (self):
        s = b'string'
        q = b'"'
        qsq = q + s + q
        count = cmd.printf(b'%"s', s)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd31_printf_strq (self):
        s = b'string'
        q = b"'"
        qsq = q + s + q
        count = cmd.printf(b"%'s", q, s)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd32_printf_strqe (self):
        s = b'|str/ing|'
        q = b'|'
        e = b'/'
        qsq = q + s.replace(e, e+e)\
                   .replace(q, e+q)\
                + q
        count = cmd.printf(b"%'|s", q, e, s)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd33_printf_strqq (self):
        s = b'<str/ing>'
        qb = b'<'
        qe = b'>'
        qsq = qb + s.replace(qb, qb)\
                    .replace(qe, qe)\
            + qe
        count = cmd.printf(b"%><s", qe, qb, s)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd34_printf_strqqe (self):
        s = b'<str/ing>'
        qb = b'<'
        qe = b'>'
        e = b'/'
        qsq = qb + s.replace(e , e+e )\
                    .replace(qb, e+qb)\
                    .replace(qe, e+qe)\
            + qe
        count = cmd.printf(b"%><|s", qe, qb, e, s)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(qsq))
        self.assertEqual(cmd.command(), qsq)

    def test_cmd35_printf_uint (self):
        i = 24680
        b = f'{i}'.encode('ascii')
        count = cmd.printf(f'%u'.encode('ascii'), i)
        self.assertEqual(cmd.error, -Error.NONE)
        self.assertEqual(count, len(b))
        self.assertEqual(cmd.command(), b)

# Tip for renumbering tests under vim:
# :let i=1 | g/test_\(cmd\|rsp\)\zs\d\+\ze/ s//\=printf("%02d", i)/ | let i+=1

if __name__ == '__main__':
    unittest.main()
