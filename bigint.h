#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#if UINTPTR_MAX == 0xffffffffffffffff
typedef uint64_t Limb;
#elif UINTPTR_MAX == 0xffffffff
typedef uint32_t Limb;
#elif UINTPTR_MAX == 0xffff
typedef uint16_t Limb;
#else
typedef uint8_t Limb;
#endif
#define LIMB_SIZE_BYTES sizeof(Limb)
#define LIMB_SIZE_BITS LIMB_SIZE_BYTES * 8
#define MIN_LIMBS 4

typedef struct bigint {
  Limb *limbs;
  size_t capacity;
  size_t len;
} bigint;

#define BINARY_BIT_OPERATION(FIRST, SECOND, OPERATOR)                          \
  const bigint *a = first;                                                     \
  const bigint *b = second;                                                    \
  if (second->len > first->len) {                                              \
    a = second;                                                                \
    b = first;                                                                 \
  }                                                                            \
  bigint *c = bigint_new_capacity(a->capacity);                                \
  memset(c->limbs, 0, c->capacity);                                            \
  for (size_t i = 0; i < b->len; i++) {                                        \
    c->limbs[i] = a->limbs[i] OPERATOR b->limbs[i];                            \
  }                                                                            \
  c->len = a->len;                                                             \
  return c;

bigint *bigint_new_capacity(size_t capacity);
void bigint_free(bigint *bigint);
bool bigint_set_hex(bigint *bigint, const char *hex);
char *bigint_get_hex(const bigint *bigint, bool upper);
bigint *bigint_init_hex(const char *hex);
bigint *bigint_bit_not(const bigint *bigint);
bigint *bigint_bit_xor(const bigint *first, const bigint *second);
bigint *bigint_bit_or(const bigint *first, const bigint *second);
bigint *bigint_bit_and(const bigint *first, const bigint *second);
bigint *bigint_bit_shiftl(const bigint *bigint, size_t n);
bigint *bigint_bit_shiftr(const bigint *bigint, size_t n);
bigint *bigint_add(const bigint *a, const bigint *b);
bigint *bigint_sub(const bigint *a, const bigint *b);
bool bigint_greater_than(const bigint *a, const bigint *b);
bool bigint_less_than(const bigint *a, const bigint *b);
bool bigint_equal(const bigint *a, const bigint *b);