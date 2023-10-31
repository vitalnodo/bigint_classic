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
#elif UINTPTR_MAX == 0xFFFFFFFF
typedef uint32_t Limb;
typedef uint64_t DoubleLimb;
#elif UINTPTR_MAX == 0xFFFF
typedef uint16_t Limb;
typedef uint32_t DoubleLimb;
#elif UINTPTR_MAX == 0xFF
typedef uint8_t Limb;
typedef uint16_t DoubleLimb;
#endif

#define LIMB_SIZE_BYTES sizeof(Limb)
#define LIMB_SIZE_BITS (LIMB_SIZE_BYTES * 8)
#define MIN_LIMBS 4

typedef struct bigint {
  Limb *limbs;
  size_t capacity;
  size_t len;
} bigint;
#define BIGINT_ZERO ((bigint){0})

#define BINARY_BIT_OPERATION(FIRST, SECOND, OPERATOR, RESULT)                  \
  const bigint *a = first;                                                     \
  const bigint *b = second;                                                    \
  bigint *c = result;                                                          \
  bigint_resize(c, a->len);                                                    \
  if (second->len > first->len) {                                              \
    a = second;                                                                \
    b = first;                                                                 \
  }                                                                            \
  memset(c->limbs, 0, c->capacity);                                            \
  for (size_t i = 0; i < b->len; i++) {                                        \
    c->limbs[i] = a->limbs[i] OPERATOR b->limbs[i];                            \
  }                                                                            \
  c->len = a->len;                                                             \
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