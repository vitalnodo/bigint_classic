#include "bigint.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct test_unary {
  char *a_hex;
  char *b_hex;
  char *c_hex;
  bool upper;
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
  char *result_hex = bigint_get_hex(&c, test.upper);
  check(test.c_hex, result_hex);
  printf("test passed\n");
  bigint_free(&a);
  bigint_free(&c);
  free(result_hex);
}

typedef struct test_binary {
  char *a_hex;
  char *b_hex;
  char *c_hex;
  bool upper;
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
  char *actual = bigint_get_hex(&c, test.upper);
  check(test.c_hex, actual);
  printf("test passed\n");
  bigint_free(&a);
  bigint_free(&b);
  bigint_free(&c);
  free(actual);
}

typedef struct test_binary_size_t {
  char *a_hex;
  size_t amount;
  char *c_hex;
  bool upper;
  BigIntError (*function)(const bigint *, size_t, bigint *);
  char *function_name;
} test_binary_size_t;
void test_binary_size_t_op(test_binary_size_t test) {
  printf("test %s... ", test.function_name);
  bigint a = BIGINT_ZERO;
  bigint c = BIGINT_ZERO;
  bigint_set_hex(test.a_hex, &a);
  BigIntError result = test.function(&a, test.amount, &c);
  if (result != Ok) {
    printf("%s\n", BigIntErrorStrings[result]);
    exit(EXIT_FAILURE);
  }
  char *actual = bigint_get_hex(&c, test.upper);
  check(test.c_hex, actual);
  printf("test passed\n");
  bigint_free(&a);
  bigint_free(&c);
  free(actual);
}

int main() {
  test_unary_op((test_unary){
      .a_hex =
          "c8f80adf8bc5e367bbb77eaac3cae1a12ff5c50411d705ef6df99f6154e3cbc"
          "958a2d3112c2c80214910fcea6b3f087ed1d96524e530b0fbf2f70750b3f7aec6",
      .c_hex =
          "3707f520743a1c98444881553c351e5ed00a3afbee28fa109206609eab1c3436a"
          "75d2ceed3d37fdeb6ef031594c0f7812e269adb1acf4f040d08f8af4c085139",
      .upper = false,
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
      .upper = false,
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
      .upper = false,
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
      .upper = false,
      .function = bigint_bit_and,
      .function_name = "and",
  });
  test_binary_size_t_op((test_binary_size_t){
      .a_hex = "ab",
      .amount = 32,
      .c_hex = "ab00000000",
      .upper = false,
      .function = bigint_bit_shiftl,
      .function_name = "shiftl",
  });
  test_binary_size_t_op((test_binary_size_t){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 32,
      .c_hex = "1df999549df4f3bcd95a",
      .upper = false,
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_binary_size_t_op((test_binary_size_t){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 112,
      .c_hex = "0",
      .upper = false,
      .function = bigint_bit_shiftr,
      .function_name = "shiftr",
  });
  test_binary_size_t_op((test_binary_size_t){
      .a_hex = "1df999549df4f3bcd95a01a2443a",
      .amount = 32,
      .c_hex = "1df999549df4f3bcd95a",
      .upper = false,
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
      .upper = false,
      .function = bigint_add,
      .function_name = "addition1",
  });
  test_binary_op((test_binary){
      .a_hex = "ffffffffffffffff",
      .b_hex = "11111111fffffffffffffffff",
      .c_hex = "111111120fffffffffffffffe",
      .upper = false,
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
      .upper = false,
      .function = bigint_sub,
      .function_name = "subtraction",
  });
  return EXIT_SUCCESS;
}