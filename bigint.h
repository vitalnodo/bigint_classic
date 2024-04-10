#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum BigIntError {
  Ok,
  ResultMemoryTooSmall,
  MemoryError,
  NotImplemented,
  DivisionByZeroError,
} BigIntError;
extern const char *BigIntErrorStrings[];

#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
typedef uint64_t Limb;
typedef __uint128_t DoubleLimb;
#define LIMB_SIZE_BYTES 8
#define LIMB_SIZE_BITS (LIMB_SIZE_BYTES * 8)
#elif UINTPTR_MAX == 0xFFFFFFFF
typedef uint32_t Limb;
typedef uint64_t DoubleLimb;
#define LIMB_SIZE_BYTES 4
#define LIMB_SIZE_BITS (LIMB_SIZE_BYTES * 8)
#elif UINTPTR_MAX == 0xFFFF
typedef uint16_t Limb;
typedef uint32_t DoubleLimb;
#define LIMB_SIZE_BYTES 2
#define LIMB_SIZE_BITS (LIMB_SIZE_BYTES * 8)
#elif UINTPTR_MAX == 0xFF
typedef uint8_t Limb;
typedef uint16_t DoubleLimb;
#define LIMB_SIZE_BYTES 1
#define LIMB_SIZE_BITS (LIMB_SIZE_BYTES * 8)
#endif

#define MIN_LIMBS 4

typedef struct bigint {
  Limb *limbs;
  size_t capacity;
  size_t len;
} bigint;
#define BIGINT_ZERO ((bigint){0})

#define BINARY_BIT_OPERATION(FIRST, SECOND, OPERATOR, RESULT)                  \
  const bigint *a = FIRST;                                                     \
  const bigint *b = SECOND;                                                    \
  const size_t max_len = (a->len > b->len) ? a->len : b->len;                  \
  bigint *c = RESULT;                                                          \
  bigint_resize(c, max_len);                                                   \
  for (size_t i = 0; i < max_len; i++) {                                       \
    c->limbs[i] = (i < a->len ? a->limbs[i] : 0) OPERATOR                      \
                  (i < b->len ? b->limbs[i] : 0);                              \
  }                                                                            \
  c->len = max_len;                                                            \
  return Ok;


bigint *bigint_new_capacity(size_t capacity);
BigIntError bigint_resize(bigint *a, size_t len);
void bigint_free_limbs(bigint *bigint);
BigIntError bigint_set_hex(const char *hex, bigint *result);
char *bigint_get_hex(const bigint *bigint, bool upper);
BigIntError bigint_bit_not(const bigint *a, bigint *result);
BigIntError bigint_bit_xor(const bigint *first, const bigint *second,
                           bigint *result);
BigIntError bigint_bit_or(const bigint *first, const bigint *second,
                          bigint *result);
BigIntError bigint_bit_and(const bigint *first, const bigint *second,
                           bigint *result);
BigIntError bigint_bit_shiftl(const bigint *a, size_t n, bigint *result);
BigIntError bigint_bit_shiftr(const bigint *a, size_t n, bigint *result);
BigIntError bigint_add(const bigint *a, const bigint *b, bigint *result);
BigIntError bigint_sub(const bigint *a, const bigint *b, bigint *result);
bool bigint_greater_than(const bigint *a, const bigint *b);
bool bigint_less_than(const bigint *a, const bigint *b);
bool bigint_equal(const bigint *a, const bigint *b);
BigIntError bigint_mul(const bigint *a, const bigint *b, bigint *result);
BigIntError bigint_copy(const bigint *src, bigint *dst);
BigIntError bigint_div(const bigint *a, const bigint *b, bigint *q, bigint *r);
size_t bigint_bit_length(const bigint *a);
typedef struct Montgomery {
    bigint modulus;
    bigint rrm;
    size_t n;
} Montgomery;
BigIntError bigint_montgomery_init(const bigint* modulus, Montgomery *m);
BigIntError bigint_montgomery_reduce(const Montgomery *m, const bigint* a, bigint* result);
BigIntError bigint_montgomery_mul(const Montgomery *m, const bigint* r1, const bigint* r2, bigint* result);
size_t calc_needed_limbs_for_hex(size_t hex_len);
BigIntError bigint_mul_karatsuba(const bigint *a, const bigint *b, bigint *result);
BigIntError bigint_mul_classic(const bigint *a, const bigint *b, bigint *result);