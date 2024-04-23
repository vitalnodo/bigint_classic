#include "bigint.h"
#include <stdint.h>
#include <string.h>

static void supermul(Limb a, Limb b, Limb *lo, Limb *hi) {
#if ((defined(__x86_64__) || defined(__i386__) || defined(__GNUC__))) && LIMB_SIZE_BITS == 64
  __asm__("mul %%rbx" : "=a"(*lo), "=d"(*hi) : "a"(a), "b"(b), "d"(0) :);
#else
  Limb aLo = a & ((1UL << (LIMB_SIZE_BITS / 2)) - 1);
  Limb bLo = b & ((1UL << (LIMB_SIZE_BITS / 2)) - 1);

  Limb aHi = a >> (LIMB_SIZE_BITS / 2);
  Limb bHi = b >> (LIMB_SIZE_BITS / 2);

  Limb axb1 = aHi * bLo;
  Limb bxa1 = bHi * aLo;

  *lo = a * b;

  Limb carry = ((axb1 & ((1UL << (LIMB_SIZE_BITS / 2)) - 1)) +
                (bxa1 & ((1UL << (LIMB_SIZE_BITS / 2)) - 1)) +
                ((aLo * bLo) >> (LIMB_SIZE_BITS / 2))) >>
               (LIMB_SIZE_BITS / 2);

  *hi = aHi * bHi + (axb1 >> (LIMB_SIZE_BITS / 2)) +
        (bxa1 >> (LIMB_SIZE_BITS / 2)) + carry;
#endif
}

BigIntError bigint_mul_classic(const bigint *a, const bigint *b, bigint *result) {
  const size_t max_len = a->len + b->len;

  bigint_resize(result, max_len);

  for (size_t i = 0; i < a->len; i++) {
    Limb carry = 0;
    for (size_t j = 0; j < b->len; j++) {
      Limb lo, hi;
      supermul(a->limbs[i], b->limbs[j], &lo, &hi);

      if ((Limb)(lo + carry) < lo) {
        hi++;
      }
      lo += carry;

      carry = ((Limb)(lo + result->limbs[i + j]) < lo) + hi;
      result->limbs[i + j] += lo;
    }
    result->limbs[i + b->len] = carry;
  }

  return Ok;
}

BigIntError bigint_mul_karatsuba_internal(const bigint *a, 
  const bigint *b, bigint *result) {
  if (a->len <= MIN_LIMBS && b->len <= MIN_LIMBS) {
    return bigint_mul_classic(a, b, result);
  }

  const size_t max_len = (a->len > b->len) ? a->len : b->len;
  const size_t middle = (max_len + 1)/2;
  const size_t middle_bytes = (max_len * LIMB_SIZE_BYTES) / 2;

  bigint *a_low = bigint_new_capacity(0);
  bigint_resize(a_low, middle);
  memset(a_low->limbs, 0, a_low->capacity * LIMB_SIZE_BYTES);
  memcpy(a_low->limbs, a->limbs, middle_bytes);

  bigint *a_high = bigint_new_capacity(0);
  bigint_resize(a_high, middle);
  memset(a_high->limbs, 0, a_high->capacity * LIMB_SIZE_BYTES);
  memcpy(a_high->limbs, (uint8_t*)a->limbs + middle_bytes,middle_bytes);

  bigint *b_low = bigint_new_capacity(0);
  bigint_resize(b_low, middle);
  memset(b_low->limbs, 0, b_low->capacity * LIMB_SIZE_BYTES);
  memcpy(b_low->limbs, b->limbs, middle_bytes);

  bigint *b_high = bigint_new_capacity(0);
  bigint_resize(b_high, middle);
  memset(b_high->limbs, 0, b_high->capacity * LIMB_SIZE_BYTES);
  memcpy(b_high->limbs, (uint8_t*)b->limbs + middle_bytes,middle_bytes);

  bigint *c0 = bigint_new_capacity(0);
  bigint_mul_karatsuba(a_low, b_low, c0);
 
  bigint *c2 = bigint_new_capacity(0);
  bigint_mul_karatsuba(a_high, b_high, c2);

  bigint *c1 = bigint_new_capacity(0);
  bigint *c1_0 = bigint_new_capacity(0);
  bigint_add(a_high, a_low, c1_0);
  bigint *c1_1 = bigint_new_capacity(0);
  bigint_add(b_low, b_high, c1_1);
  bigint_mul_karatsuba(c1_0, c1_1, c1);
  bigint_sub(c1, c2, c1_0);
  bigint_sub(c1_0, c0, c1_1);
  bigint_copy(c1_1, c1);
  
  bigint_copy(c0, result);
  
  bigint *tmp = bigint_new_capacity(0);
  memset(tmp->limbs, 0, LIMB_SIZE_BYTES * tmp->capacity);
  bigint_bit_shiftl(c1, middle_bytes*8, tmp);
  bigint_add(result, tmp, c1);
  bigint_copy(c1, result);

  memset(tmp->limbs, 0, LIMB_SIZE_BYTES * tmp->capacity);
  bigint_bit_shiftl(c2, 2*middle_bytes*8, tmp);
  bigint_add(result, tmp, c2);
  bigint_copy(c2, result);

  bigint_free_limbs(a_low);
  bigint_free_limbs(a_high);
  bigint_free_limbs(b_low);
  bigint_free_limbs(b_high);
  bigint_free_limbs(c0);
  bigint_free_limbs(c2);
  bigint_free_limbs(c1);
  bigint_free_limbs(c1_0);
  bigint_free_limbs(c1_1);
  bigint_free_limbs(tmp);
  return Ok;
}

BigIntError bigint_mul_karatsuba(const bigint *a, 
  const bigint *b, bigint *result) {
    bigint *a_copy = bigint_new_capacity(0);
    bigint *b_copy = bigint_new_capacity(0);
    bigint_copy(a, a_copy);
    bigint_copy(b, b_copy);
    const size_t max_len = (a->len > b->len) ? a->len : b->len;
    bigint_resize(a_copy, max_len);
    bigint_resize(b_copy, max_len);
    bigint_mul_karatsuba_internal(a_copy, b_copy, result);
    return Ok;
}

BigIntError bigint_mul(const bigint *a, const bigint *b, bigint *result) {
  return bigint_mul_karatsuba(a, b, result);
}
