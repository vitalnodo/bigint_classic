#include "bigint.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#define UPPER false

typedef struct test_unary {
  char *a_hex;
  char *b_hex;
  char *c_hex;
  BigIntError (*function)(const bigint *, bigint *);
  char *function_name;
} test_unary;

void test_unary_op(test_unary test) {
  printf("test %s... ", test.function_name);
  bigint a = BIGINT_ZERO;
  bigint_set_hex(test.a_hex, &a);
  bigint c = BIGINT_ZERO;
  BigIntError result = test.function(&a, &c);
  if (result != Ok) {
    printf("%s\n", BigIntErrorStrings[result]);
    exit(EXIT_FAILURE);
  }
  char *result_hex = bigint_get_hex(&c, UPPER);
  check(test.c_hex, result_hex);
  printf("test passed\n");
  bigint_free_limbs(&a);
  bigint_free_limbs(&c);
  free(result_hex);
}

typedef struct test_binary {
  char *a_hex;
  char *b_hex;
  char *c_hex;
  BigIntError (*function)(const bigint *, const bigint *, bigint *);
  char *function_name;
} test_binary;

void test_binary_op(test_binary test) {
  printf("test %s... ", test.function_name);
  bigint a = BIGINT_ZERO, b = BIGINT_ZERO, c = BIGINT_ZERO;
  bigint_set_hex(test.a_hex, &a);
  bigint_set_hex(test.b_hex, &b);
  BigIntError result = test.function(&a, &b, &c);
  if (result != Ok) {
    printf("%s\n", BigIntErrorStrings[result]);
    exit(EXIT_FAILURE);
  }
  char *actual = bigint_get_hex(&c, UPPER);
  check(test.c_hex, actual);
  printf("test passed\n");
  bigint_free_limbs(&a);
  bigint_free_limbs(&b);
  bigint_free_limbs(&c);
  free(actual);
}

typedef struct test_shift {
  char *a_hex;
  size_t amount;
  char *c_hex;
  BigIntError (*function)(const bigint *, size_t, bigint *);
  char *function_name;
} test_shift;
void test_shift_op(test_shift test) {
  printf("test %s... ", test.function_name);
  bigint *a = bigint_new_capacity(MIN_LIMBS);
  bigint c = BIGINT_ZERO;
  bigint_set_hex(test.a_hex, a);
  BigIntError result = test.function(a, test.amount, &c);
  if (result != Ok) {
    printf("%s\n", BigIntErrorStrings[result]);
    exit(EXIT_FAILURE);
  }
  char *actual = bigint_get_hex(&c, UPPER);
  check(test.c_hex, actual);
  printf("test passed\n");
  bigint_free_limbs(a);
  free(a);
  bigint_free_limbs(&c);
  free(actual);
}

typedef struct test_division {
  char *a_hex;
  char *b_hex;
  char *q_hex;
  char *r_hex;
  BigIntError (*function)(const bigint *, const bigint *, bigint *, bigint *);
  char *function_name;
} test_division;
void test_division_op(test_division test) {
  printf("test %s... ", test.function_name);
  bigint a = BIGINT_ZERO;
  bigint b = BIGINT_ZERO;
  bigint q = BIGINT_ZERO;
  bigint r = BIGINT_ZERO;
  bigint_set_hex(test.a_hex, &a);
  bigint_set_hex(test.b_hex, &b);
  BigIntError result = test.function(&a, &b, &q, &r);
  if (result != Ok) {
    printf("%s\n", BigIntErrorStrings[result]);
    exit(EXIT_FAILURE);
  }
  char *actual_q = bigint_get_hex(&q, UPPER);
  check(test.q_hex, actual_q);
  char *actual_r = bigint_get_hex(&r, UPPER);
  check(test.r_hex, actual_r);
  printf("test passed\n");
  bigint_free_limbs(&a);
  bigint_free_limbs(&b);
  bigint_free_limbs(&q);
  bigint_free_limbs(&r);
  free(actual_q);
  free(actual_r);
}

void test_bit_length() {
  printf("test bit_length... ");
  bigint a = BIGINT_ZERO;
  bigint_set_hex("c8f80adf8bc5e367bbb77eaac3cae1a12ff5c50411d705ef6df99f6154e3cbc", &a);
  if (bigint_bit_length(&a) != 252) {
    printf("%lu\n", bigint_bit_length(&a));
    exit(EXIT_FAILURE);
  }
  printf("test passed\n");
  bigint_free_limbs(&a);
}

bool bigint_equal_hex(const bigint* a, char* hex) {
  bigint bigint_from_hex = BIGINT_ZERO;
  bigint_set_hex(hex, &bigint_from_hex);
  if (!bigint_equal(a, &bigint_from_hex)) {
    return false;
  }
  bigint_free_limbs(&bigint_from_hex);
  return true;
}

void test_montgomery() {
  printf("test montgomery... ");
  bigint modulus = BIGINT_ZERO;
  bigint_set_hex("979efd66ad4419169c5b34413", &modulus);
  bigint expected_rrm = BIGINT_ZERO;
  bigint_set_hex("78da46e5dbfd3aef0613394da", &expected_rrm);
  Montgomery m;
  m.modulus = BIGINT_ZERO;
  m.rrm = BIGINT_ZERO;
  bigint_montgomery_init(&modulus, &m);
  if (!bigint_equal(&expected_rrm, &m.rrm)) {
    printf("Error. rrm not equals\n");
    exit(EXIT_FAILURE);
  }
  if (m.n != 100) {
    printf("Error. n should be equal 100\n");
    exit(EXIT_FAILURE);
  }

  bigint x1 = BIGINT_ZERO;
  bigint_set_hex("6d0e5e4b23a854021124f3dbe", &x1);
  bigint t1 = BIGINT_ZERO;
  bigint_mul(&x1, &m.rrm, &t1);
  bigint r1 = BIGINT_ZERO;
  bigint_montgomery_reduce(&m, &t1, &r1);
  bigint x2 = BIGINT_ZERO;
  bigint_set_hex("6824a837539e97a07d95963a1", &x2);
  bigint t2 = BIGINT_ZERO;
  bigint_mul(&x2, &m.rrm, &t2);
  bigint r2 = BIGINT_ZERO;
  bigint_montgomery_reduce(&m, &t2, &r2);
  if (!bigint_equal_hex(&r1, "58e3bd6d7580be7cc4a3686f5") 
    || !bigint_equal_hex(&r2, "57ebb868592ea9305b1464237")) {
    printf("Error in reduce\n");
    exit(EXIT_FAILURE);
  }

  bigint montgomery_mul = BIGINT_ZERO;
  bigint_montgomery_mul(&m, &r1, &r2, &montgomery_mul);
  bigint_montgomery_reduce(&m, &montgomery_mul, &montgomery_mul);
  if (!bigint_equal_hex(&montgomery_mul, "93cfd5fd5b63b5db26b65aa6d")) {
    printf("Error in multiplication\n");
    exit(EXIT_FAILURE);
  }
  printf("test passed\n");
  bigint_free_limbs(&modulus);
  bigint_free_limbs(&m.rrm);
  bigint_free_limbs(&x1);
  bigint_free_limbs(&t1);
  bigint_free_limbs(&r1);
  bigint_free_limbs(&x2);
  bigint_free_limbs(&t2);
  bigint_free_limbs(&r2);
}

int main() {
  test_unary_op((test_unary){
      .a_hex =
          "c8f80adf8bc5e367bbb77eaac3cae1a12ff5c50411d705ef6df99f6154e3cbc"
          "958a2d3112c2c80214910fcea6b3f087ed1d96524e530b0fbf2f70750b3f7aec6",
      .c_hex =
          "3707f520743a1c98444881553c351e5ed00a3afbee28fa109206609eab1c3436a"
          "75d2ceed3d37fdeb6ef031594c0f7812e269adb1acf4f040d08f8af4c085139",
      .function = bigint_bit_not,
      .function_name = "not",
  });
  test_binary_op((test_binary){
      .a_hex =
          "51bf608414ad5726a3c1bec098f77b1b54ffb2787f8d528a74c1d7fde6470ea4",
      .b_hex =
          "403db8ad88a3932a0b7e8189aed9eeffb8121dfac05c3512fdb396dd73f6331c",
      .c_hex =
          "1182d8299c0ec40ca8bf3f49362e95e4ecedaf82bfd167988972412095b13db8",
      .function = bigint_bit_xor,
      .function_name = "xor",
  });
  test_binary_op((test_binary){
      .a_hex =
          "1351a29eb2c64ac8c52795d75ae56512a35b78793b3a1570ffdd95fef4fd329e",
      .b_hex =
          "56a14ac569dca4b362a041a15393aa6ef58f5d34ab9c3041a1f0e15261b5d688",
      .c_hex =
          "57f1eadffbdeeefbe7a7d5f75bf7ef7ef7df7d7dbbbe3571fffdf5fef5fdf69e",
      .function = bigint_bit_or,
      .function_name = "or",
  });
  test_binary_op((test_binary){
      .a_hex =
          "a6f883b5b15333f928bb359039a8ef0d17186d3ed8c63c3c22ce72054e3f70ab",
      .b_hex =
          "93a1b6e2e410f23d6ac9fce7413fd7f920ce95bce3ff393cbfdc2941b593b301",
      .c_hex =
          "82a082a0a0103239288934800128c7090008053cc0c6383c22cc200104133001",
      .function = bigint_bit_and,
      .function_name = "and",
  });
  test_shift_op((test_shift){
      .a_hex = "ab",
      .amount = 32,
      .c_hex = "ab00000000",
      .function = bigint_bit_shiftl,
      .function_name = "shiftl",
  });
  test_shift_op((test_shift){
      .a_hex = "6f7c4912e56d4c7dfbcadea6e4da60",
      .amount = 141,
      .c_hex = "def89225cada98fbf795bd4dc9b4c000000000000000000000000000000000000",
      .function = bigint_bit_shiftl,
      .function_name = "shiftl",
  });
  test_shift_op((test_shift){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 32,
      .c_hex = "1df999549df4f3bcd95a",
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_shift_op((test_shift){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 112,
      .c_hex = "0",
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_shift_op((test_shift){
      .a_hex =
          "6f7c4912e56d4c7dfbcadea6e4da60fe6d2d3f52cfa5e60b5547e1fb737d870d",
      .amount = 64,
      .c_hex = "6f7c4912e56d4c7dfbcadea6e4da60fe6d2d3f52cfa5e60b",
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_shift_op((test_shift){
      .a_hex =
          "6f7c4912e56d4c7dfbcadea6e4da60fe6d2d3f52cfa5e60b5547e1fb737d870d",
      .amount = 128,
      .c_hex = "6f7c4912e56d4c7dfbcadea6e4da60fe",
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_shift_op((test_shift){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 32,
      .c_hex = "1df999549df4f3bcd95a",
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_binary_op((test_binary){
      .a_hex =
          "36f028580bb02cc8272a9a020f4200e346e276ae664e45ee80745574e2f5ab80",
      .b_hex =
          "70983d692f648185febe6d6fa607630ae68649f7e6fc45b94680096c06e4fadb",
      .c_hex =
          "a78865c13b14ae4e25e90771b54963ee2d68c0a64d4a8ba7c6f45ee0e9daa65b",
      .function = bigint_add,
      .function_name = "addition1",
  });
  test_binary_op((test_binary){
      .a_hex = "ffffffffffffffff",
      .b_hex = "11111111fffffffffffffffff",
      .c_hex = "111111120fffffffffffffffe",
      .function = bigint_add,
      .function_name = "addition2",
  });
  test_binary_op((test_binary){
      .a_hex =
          "33ced2c76b26cae94e162c4c0d2c0ff7c13094b0185a3c122e732d5ba77efebc",
      .b_hex =
          "22e962951cb6cd2ce279ab0e2095825c141d48ef3ca9dabf253e38760b57fe03",
      .c_hex =
          "10e570324e6ffdbc6b9c813dec968d9bad134bc0dbb061530934f4e59c2700b9",
      .function = bigint_sub,
      .function_name = "subtraction",
  });
  test_binary_op((test_binary){
      .a_hex =
          "33ced2c76b26cae94e162c4c0d2c0ff7c13094b0185a3c122e732d5ba77efebc",
      .b_hex =
          "22e962951cb6cd2ce279ab0e2095825c141d48ef3ca9dabf253e38760b57fe03",
      .c_hex =
          "710b32fad9c4ff53fa1fa0d4bf11cd2df77dafe6cd9c097c3858d1c8f0fd1fceca1f"
          "71fa8907df44ba9ffc4837bbcd37fa919893008811e067d64059d1f8434",
      .function = bigint_mul,
      .function_name = "multiplication",
  });
  test_division_op((test_division){
      .a_hex =
          "84bcae70bc527cd3ad3c31cd062e642234abcd5abe6aa66e065ca7270169a39d3a36"
          "ccd6ddb3fe5c14fff8a206da4e716c63eb4c1e4575ba1e56cc8f10aa6331",
      .b_hex = "84ea0cac4f7ec5fd2b8f06c5d8573f6f5907849327187656",
      .q_hex = "ffa89e5d1597f208d3de6c95d05568a3c47ea66888f92436940231c72aa55fe"
               "d2a89bf52ddffba64",
      .r_hex = "6fc77d9a4d59e75ed28a6f8609daf32e9399f799e777ad99",
      .function = bigint_div,
      .function_name = "division",
  });
  test_division_op((test_division){
      .a_hex = "74dec379409ad5e600e5c30f98b166b0c2eb69a991fc6efcb8e45e5e4786ef51",
      .b_hex = "d0387fb184f98bf9cd4da7105925a8269b4d5f6029893e",
      .q_hex = "8fb0105ab2cf786e4e",
      .r_hex = "6c161380d20c61b77184d51a257a192bdbed6faad67a6d",
      .function = bigint_div,
      .function_name = "division",
  });
  test_bit_length();
  test_montgomery();
  return EXIT_SUCCESS;
}
