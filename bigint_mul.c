#include "bigint.h"

static void supermul(Limb a, Limb b, Limb *lo, Limb *hi) {
#if (defined(__x86_64__) || defined(__i386__) || defined(__GNUC__))
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

BigIntError bigint_mul(const bigint *a, const bigint *b, bigint *result) {
  const size_t max_len = a->len + b->len;

  bigint_resize(result, max_len);

  for (size_t i = 0; i < a->len; i++) {
    Limb carry = 0;
    for (size_t j = 0; j < b->len; j++) {
      Limb lo, hi;
      supermul(a->limbs[i], b->limbs[j], &lo, &hi);

      if (lo + carry < lo) {
        hi++;
      }
      lo += carry;

      carry = (lo + result->limbs[i + j] < lo) + hi;
      result->limbs[i + j] += lo;
    }
    result->limbs[i + b->len] = carry;
  }

  return Ok;
}