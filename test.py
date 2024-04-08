import ctypes
import unittest
import random
import math

random.seed(12345)
TESTS = 50
BITS = 4096
lib = ctypes.CDLL("./bigint.so")
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
    return ~num & ((1 << bits) - 1)

def prepare_buffer(num):
    hex_ = hex(num)[2:].lstrip("0").encode()
    return ctypes.create_string_buffer(hex_)

def test_binary_op(t, python_op, lib_op):
    a = rand(BITS)
    b = rand(BITS)
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
            expected = hex(bit_not(some_number_))[2:].encode()
            got_hex = lib.bigint_get_hex(tmp, False)
            self.assertEqual(expected, got_hex)
            lib.bigint_free_limbs(bigint)

    def test_binary_bitwise(self):
        for i in range(TESTS):
            test_binary_op(self, lambda x,y: x^y, lib.bigint_bit_xor)
            test_binary_op(self, lambda x,y: x|y, lib.bigint_bit_or)
            test_binary_op(self, lambda x,y: x&y, lib.bigint_bit_and)

    def test_binary(self):
        for i in range(TESTS):
            test_binary_op(self, lambda x,y: x+y, lib.bigint_add)

if __name__ == '__main__':
    unittest.main()