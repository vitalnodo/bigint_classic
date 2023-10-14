#include "bigint.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *BigIntErrorStrings[] = {
    "Ok",
    "ResultMemoryTooSmall",
    "MemoryError",
    "NotImplemented",
};

bigint *bigint_new_capacity(size_t capacity) {
  capacity = (capacity < MIN_LIMBS) ? MIN_LIMBS : capacity;
  struct bigint *bigint = malloc(sizeof(*bigint));
  if (bigint == NULL) {
    return NULL;
  }
  bigint->limbs = malloc(capacity * LIMB_SIZE_BYTES);
  if (bigint->limbs == NULL) {
    free(bigint);
    return NULL;
  }
  bigint->capacity = capacity;
  bigint->len = 0;
  return bigint;
}

BigIntError bigint_resize(bigint *a, size_t len) {
  if (a->capacity < len) {
    a->limbs = realloc(a->limbs, LIMB_SIZE_BYTES * len);
    if (a->limbs == NULL) {
      return MemoryError;
    }
    memset(a->limbs + a->len, 0, (len - a->len) * LIMB_SIZE_BYTES);
    a->capacity = len;
  }
  a->len = len;
  return Ok;
}

void bigint_free(bigint *bigint) {
  free(bigint->limbs);
  bigint->capacity = 0;
}

size_t calc_needed_limbs_for_hex(size_t hex_len) {
  size_t needed_limbs = (hex_len % (LIMB_SIZE_BYTES * 2) == 0
                             ? hex_len / (LIMB_SIZE_BYTES * 2)
                             : hex_len / (LIMB_SIZE_BYTES * 2) + 1);
  needed_limbs = needed_limbs > MIN_LIMBS ? needed_limbs : MIN_LIMBS;
  return needed_limbs;
}

BigIntError bigint_set_hex(const char *hex, bigint *result) {
  const size_t hex_len = strlen(hex);
  const size_t needed_limbs = calc_needed_limbs_for_hex(hex_len);

  bigint_resize(result, needed_limbs);

  size_t current_limb = 0;
  size_t nibble_count = 0;
  Limb limb = 0;

  for (size_t i = hex_len - 1; i + 1 > 0; i--) {
    char nibble = hex[i];
    char value = char2digit(nibble);

    if (value == -1) {
      result->len = 0;
      return false;
    }

    limb |= ((Limb)value) << (4 * nibble_count);

    if (nibble_count == (LIMB_SIZE_BYTES * 2 - 1)) {
      result->limbs[current_limb] = limb;
      current_limb++;
      nibble_count = 0;
      limb = 0;
    } else {
      nibble_count++;
    }
  }

  if (nibble_count > 0) {
    result->limbs[current_limb] = limb;
  }
  return true;
}

char *bigint_get_hex(const bigint *bigint, bool upper) {
  const char *OUT_HEX = upper ? "0123456789ABCDEF" : "0123456789abcdef";
  const size_t hex_len = bigint->len * LIMB_SIZE_BYTES * 2 + 1;

  char *hex = malloc(hex_len);
  if (hex == NULL) {
    return NULL;
  }

  size_t hex_index = 0;
  for (size_t i = bigint->len - 1; i + 1 > 0; i--) {
    Limb limb = bigint->limbs[i];

    for (size_t j = LIMB_SIZE_BYTES * 2 - 1; j + 1 > 0; j--) {
      unsigned char nibble = (limb >> (j * 4)) & 0xF;
      hex[hex_index] = OUT_HEX[nibble];
      hex_index++;
    }
  }

  hex[hex_index] = '\0';
  hex = ltrim(hex);
  if (hex[0] == '\0') {
    hex = realloc(hex, 2);
    if (hex == NULL) {
      return NULL;
    }
    hex[0] = '0';
    hex[1] = '\0';
  }
  return hex;
}

BigIntError bigint_bit_not(const bigint *a, bigint *result) {
  if (result->len != a->len) {
    bigint_resize(result, a->len);
  }
  for (size_t i = 0; i < a->len; i++) {
    result->limbs[i] = ~(a->limbs[i]);
  }
  result->len = a->len;
  return Ok;
}

BigIntError bigint_bit_xor(const bigint *first, const bigint *second,
                           bigint *result) {
  BINARY_BIT_OPERATION(first, second, ^, result);
}

BigIntError bigint_bit_or(const bigint *first, const bigint *second,
                          bigint *result) {
  BINARY_BIT_OPERATION(first, second, |, result);
}

BigIntError bigint_bit_and(const bigint *first, const bigint *second,
                           bigint *result) {
  BINARY_BIT_OPERATION(first, second, &, result);
}

BigIntError bigint_bit_shiftl(const bigint *a, size_t n, bigint *result) {
  size_t limb_shifts = n / LIMB_SIZE_BITS;
  size_t bit_shifts = n % LIMB_SIZE_BITS;

  size_t new_len = a->len + limb_shifts;

  bigint_resize(result, new_len);

  Limb carry = 0;
  for (size_t i = 0; i < a->len; i++) {
    Limb shifted = (a->limbs[i] << bit_shifts) | carry;
    carry = (a->limbs[i] >> (LIMB_SIZE_BITS - bit_shifts)) &
            ((1 << bit_shifts) - 1);
    result->limbs[i + limb_shifts] = shifted;
  }
  if (carry > 0) {
    result->limbs[a->len + limb_shifts] = carry;
    new_len++;
  }
  result->len = new_len;
  return Ok;
}

BigIntError bigint_bit_shiftr(const bigint *a, size_t n, bigint *result) {
  size_t limb_shifts = n / LIMB_SIZE_BITS;
  size_t bit_shifts = n % LIMB_SIZE_BITS;

  size_t new_len = a->len - limb_shifts;
  bigint_resize(result, new_len);

  if (limb_shifts >= result->len) {
    bigint_free(result);
    *result = BIGINT_ZERO;
    return Ok;
  }

  Limb carry = 0;
  for (size_t i = new_len - 1; i + 1 > 0; i--) {
    Limb shifted = (a->limbs[i + limb_shifts] >> bit_shifts) | carry;
    carry = (a->limbs[i + limb_shifts] << (LIMB_SIZE_BITS - bit_shifts));
    if (LIMB_SIZE_BITS == n)
      carry &= ((1 << bit_shifts) - 1);
    result->limbs[i] = shifted;
  }
  result->len = new_len;
  return Ok;
}

BigIntError bigint_add(const bigint *a, const bigint *b, bigint *result) {
  const size_t len_a = a->len;
  const size_t len_b = b->len;
  const size_t max_len = (len_a > len_b) ? len_a : len_b;

  bigint_resize(result, max_len);

  Limb carry = 0;
  for (size_t i = 0; i < max_len; i++) {
    Limb a_ = (i < len_a) ? a->limbs[i] : 0;
    Limb b_ = (i < len_b) ? b->limbs[i] : 0;
    Limb sum = a_ + b_;
    Limb res = sum + carry;
    result->limbs[i] = res;
    carry = (sum < a_) | (res < sum);
  }

  if (carry) {
    bigint_resize(result, result->len + 1);
    result->limbs[max_len] = carry;
  }
  return Ok;
}

bool bigint_greater_than(const bigint *a, const bigint *b) {
  if (bigint_equal(a, b)) {
    return false;
  }

  const size_t min_len = (a->len > b->len) ? b->len : a->len;

  for (int i = min_len - 1; i >= 0; i--) {
    if (a->limbs[i] > b->limbs[i]) {
      return true;
    }
    if (a->limbs[i] < b->limbs[i]) {
      return false;
    }
  }

  return false;
}

bool bigint_less_than(const bigint *a, const bigint *b) {
  return bigint_greater_than(b, a);
}

bool bigint_equal(const bigint *a, const bigint *b) {
  const size_t min_len = (a->len > b->len) ? b->len : a->len;
  size_t x = 0;
  for (size_t i = 0; i < min_len; i++) {
    x |= a->limbs[i] ^ b->limbs[i];
  }
  return x == 0;
}

BigIntError bigint_sub(const bigint *a, const bigint *b, bigint *result) {

  if (bigint_less_than(a, b)) {
    return NotImplemented;
  }

  const size_t len_a = a->len;
  const size_t len_b = b->len;
  const size_t len = len_a;

  bigint_resize(result, len);

  Limb carry = 0;
  for (size_t i = 0; i < len; i++) {
    Limb a_ = (i < len_a) ? a->limbs[i] : 0;
    Limb b_ = (i < len_b) ? b->limbs[i] : 0;
    Limb difference = a_ - b_;
    Limb res = difference - carry;
    result->limbs[i] = res;
    carry = (difference > a->limbs[i]) | (res > difference);
  }
  result->limbs[len] = carry;

  result->len = len;

  return Ok;
}

static void supermul(Limb a, Limb b, Limb *lo, Limb *hi) {
#if defined(__x86_64__) || defined(__i386__)
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
