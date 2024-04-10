import ctypes
import unittest
import random
import math

random.seed(12345)
TESTS = 25
BITS = 4096
assert BITS >= 64
lib = ctypes.CDLL("./bigint.so")
LIMB_SIZE_BITS = 64
Limb = ctypes.c_uint64

class Bigint(ctypes.Structure):
    _fields_ = [("limbs", ctypes.POINTER(Limb)),
                ("capacity", ctypes.c_size_t),
                ("len", ctypes.c_size_t)]

lib.bigint_new_capacity.restype = ctypes.POINTER(Bigint)
lib.bigint_set_hex.argtypes = [ctypes.c_char_p, ctypes.POINTER(Bigint)]
lib.bigint_get_hex.args = [ctypes.c_char_p, ctypes.c_bool]
lib.bigint_get_hex.restype = ctypes.c_char_p
lib.bigint_bit_not.argtypes = [ctypes.POINTER(Bigint), ctypes.POINTER(Bigint)]
lib.bigint_bit_xor.argtypes = [ctypes.POINTER(Bigint), ctypes.POINTER(Bigint),
    ctypes.POINTER(Bigint)]

def rand(bits):
    return random.getrandbits(bits)

def bit_not(num, bits=BITS):
    return num ^ ((1 << bits) - 1)

def prepare_buffer(num):
    hex_ = hex(num)[2:].lstrip("0").encode()
    return ctypes.create_string_buffer(hex_)

def test_binary_op(t, python_op, lib_op):
    a = rand(BITS)
    b = rand(BITS)
    if b > a:
        a, b = b, a
    expected = python_op(a, b)
    bigint_a = lib.bigint_new_capacity(0)
    bigint_b = lib.bigint_new_capacity(0)
    bigint_expected = lib.bigint_new_capacity(0)

    lib.bigint_set_hex(prepare_buffer(a), bigint_a)
    lib.bigint_set_hex(prepare_buffer(b), bigint_b)
    lib.bigint_set_hex(prepare_buffer(expected), bigint_expected)

    bigint_res = lib.bigint_new_capacity(0)
    lib_op(bigint_a, bigint_b, bigint_res)
    actual_hex = lib.bigint_get_hex(bigint_res, False)
    t.assertEqual(prepare_buffer(expected).value, actual_hex)

    lib.bigint_free_limbs(bigint_a)
    lib.bigint_free_limbs(bigint_b)
    lib.bigint_free_limbs(bigint_expected)

class TestLib(unittest.TestCase):
    def test_unary_not(self):
        for i in range(TESTS):
            some_number_ = rand(BITS)
            bigint = lib.bigint_new_capacity(0)
            lib.bigint_set_hex(prepare_buffer(some_number_), bigint)
            got_hex = lib.bigint_get_hex(bigint, False)
            self.assertEqual(prepare_buffer(some_number_).value, got_hex)

            tmp = lib.bigint_new_capacity(0)
            lib.bigint_bit_not(bigint, tmp)
            b = bigint.contents.len * LIMB_SIZE_BITS
            expected = hex(bit_not(some_number_, b))[2:].encode()
            got_hex = lib.bigint_get_hex(tmp, False)
            self.assertEqual(expected, got_hex)
            lib.bigint_free_limbs(bigint)

    def test_binary(self):
        for i in range(TESTS):
            test_binary_op(self, lambda x,y: x^y, lib.bigint_bit_xor)
            test_binary_op(self, lambda x,y: x|y, lib.bigint_bit_or)
            test_binary_op(self, lambda x,y: x&y, lib.bigint_bit_and)
            test_binary_op(self, lambda x,y: x+y, lib.bigint_add)
            test_binary_op(self, lambda x,y: x-y, lib.bigint_sub)
            test_binary_op(self, lambda x,y: x*y, lib.bigint_mul_classic)
            test_binary_op(self, lambda x,y: x*y, lib.bigint_mul_karatsuba)
    
    def test_division(self):
        for i in range(TESTS):
            a = rand(BITS)
            b = rand(BITS)
            if b > a:
                a, b = b, a
            bigint_a = lib.bigint_new_capacity(0)
            bigint_b = lib.bigint_new_capacity(0)
            lib.bigint_set_hex(prepare_buffer(a), bigint_a)
            lib.bigint_set_hex(prepare_buffer(b), bigint_b)
            bigint_q = lib.bigint_new_capacity(0)
            bigint_r = lib.bigint_new_capacity(0)

            lib.bigint_div(bigint_a, bigint_b, bigint_q, bigint_r)
            expected_q = hex(a // b)[2:].encode()
            expected_r = hex(a % b)[2:].encode()
            actual_q = lib.bigint_get_hex(bigint_q)
            actual_r = lib.bigint_get_hex(bigint_r)
            self.assertEqual(expected_q, actual_q)

            lib.bigint_free_limbs(bigint_a)
            lib.bigint_free_limbs(bigint_b)
            lib.bigint_free_limbs(bigint_q)
            lib.bigint_free_limbs(bigint_r)
            

    def test_shifts(self):
        for i in range(TESTS):
            a = rand(BITS)
            for i in range(0,a.bit_length()+1):
                bigint_a = lib.bigint_new_capacity(0)
                bigint_res = lib.bigint_new_capacity(0)
                lib.bigint_set_hex(prepare_buffer(a), bigint_a)
                lib.bigint_bit_shiftr(bigint_a, i, bigint_res)
                expected = hex(a >> i)[2:].encode()
                actual = lib.bigint_get_hex(bigint_res, False)
                self.assertEqual(expected, actual)
                lib.bigint_free_limbs(bigint_a)
                lib.bigint_free_limbs(bigint_res)

                bigint_a = lib.bigint_new_capacity(0)
                bigint_res = lib.bigint_new_capacity(0)
                lib.bigint_set_hex(prepare_buffer(a), bigint_a)
                lib.bigint_bit_shiftl(bigint_a, i, bigint_res)
                expected = hex(a << i)[2:].encode()
                actual = lib.bigint_get_hex(bigint_res, False)
                self.assertEqual(expected, actual)
                lib.bigint_free_limbs(bigint_a)
                lib.bigint_free_limbs(bigint_res)


if __name__ == '__main__':
    unittest.main()
