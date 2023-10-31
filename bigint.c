#include "bigint.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *BigIntErrorStrings[] = {"Ok", "ResultMemoryTooSmall", "MemoryError",
                                    "NotImplemented", "DivisionByZeroError"};

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
    a->capacity = len;
  }
  if (len > a->len) {
    memset(a->limbs + a->len, 0, (len - a->len) * LIMB_SIZE_BYTES);
  }
  a->len = len;
  return Ok;
}

void bigint_free_limbs(bigint *bigint) {
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

  if (limb_shifts >= a->len) {
    bigint_free_limbs(result);
    *result = BIGINT_ZERO;
    return Ok;
  }

  size_t new_len = a->len - limb_shifts;
  bigint_resize(result, new_len);

  Limb carry = 0;
  for (size_t i = new_len - 1; i + 1 > 0; i--) {
    Limb shifted = (a->limbs[i + limb_shifts] >> bit_shifts) | carry;
    carry = (a->limbs[i + limb_shifts] << (LIMB_SIZE_BITS - bit_shifts));
    if (bit_shifts == 0)
      carry = 0;
    result->limbs[i] = shifted;
  }
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

  if (a->len > b->len) {
    for (size_t i = min_len; i < a->len; i++) {
      if (a->limbs[i]) {
        return true;
      }
    }
  } else if (b->len > a->len) {
    for (size_t i = min_len; i < b->len; i++) {
      if (b->limbs[i]) {
        return false;
      }
    }
  }

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

  const size_t len = a->len;

  bigint_resize(result, len);

  Limb carry = 0;
  for (size_t i = 0; i < len; i++) {
    Limb a_ = (i < a->len) ? a->limbs[i] : 0;
    Limb b_ = (i < b->len) ? b->limbs[i] : 0;
    Limb difference = a_ - b_;
    Limb res = difference - carry;
    carry = (difference > a->limbs[i]) | (res > difference);
    result->limbs[i] = res;
  }
  // carry == 0

  result->len = len;

  return Ok;
}

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

static bool bigint_is_zero(const bigint *bi) {
  for (size_t i = 0; i < bi->len; i++) {
    if (bi->limbs[i]) {
      return false;
    }
  }
  return true;
}

BigIntError bigint_copy(const bigint *src, bigint *dst) {
  BigIntError resize_result = bigint_resize(dst, src->len);
  if (resize_result != Ok) {
    return resize_result;
  }
  memcpy(dst->limbs, src->limbs, src->len * LIMB_SIZE_BYTES);
  return Ok;
}

static void bigint_fit(bigint *bi) {
  while (bi->limbs[bi->len - 1] == 0) {
    bi->len--;
  }
}

BigIntError bigint_div(const bigint *A, const bigint *B, bigint *q, bigint *r) {
  if (bigint_is_zero(B)) {
    return DivisionByZeroError;
  }

  if (bigint_less_than(A, B)) {
    *q = BIGINT_ZERO;
    bigint_copy(A, r);
    return Ok;
  }

  bigint a = BIGINT_ZERO;
  bigint b = BIGINT_ZERO;

  bigint_copy(A, &a);
  bigint_copy(B, &b);

  bigint_fit(&a);
  bigint_fit(&b);

  if (b.len < 2) {
    DoubleLimb partial = 0;

    bigint_resize(q, a.len);
    for (intmax_t i = a.len - 1; i >= 0; i--) {
      partial = (partial << LIMB_SIZE_BITS) + a.limbs[i];
      q->limbs[i] = partial / b.limbs[0];
      partial %= b.limbs[0];
    }

    bigint_resize(r, 1);
    r->limbs[0] = partial;

    return Ok;
  }

  bigint_resize(q, a.len);
  bigint_resize(r, a.len);

  intmax_t shifts = 0;

  Limb vn = b.limbs[b.len - 1];
  while (vn < (1UL << (LIMB_SIZE_BITS - 1))) {
    vn <<= 1;
    shifts++;
  }

  for (intmax_t i = b.len - 1; i > 0; i--) {
    b.limbs[i] = (b.limbs[i] << shifts) |
                 (((DoubleLimb)b.limbs[i - 1]) >> (LIMB_SIZE_BITS - shifts));
  }
  b.limbs[0] = b.limbs[0] << shifts;

  bigint_resize(&a, a.len + 1);

  a.limbs[a.len - 1] =
      ((DoubleLimb)a.limbs[a.len - 2]) >> (LIMB_SIZE_BITS - shifts);
  for (intmax_t i = a.len - 2; i > 0; i--) {
    a.limbs[i] = (a.limbs[i] << shifts) |
                 (((DoubleLimb)a.limbs[i - 1]) >> (LIMB_SIZE_BITS - shifts));
  }
  a.limbs[0] = a.limbs[0] << shifts;

  DoubleLimb uno = 1;
  uno <<= LIMB_SIZE_BITS;

  for (intmax_t k = a.len - b.len - 1; k >= 0; k--) {
    DoubleLimb rhat = a.limbs[k + b.len];
    rhat <<= LIMB_SIZE_BITS;
    rhat += a.limbs[k + b.len - 1];

    DoubleLimb qhat = rhat / b.limbs[b.len - 1];

    rhat %= b.limbs[b.len - 1];

    if (qhat == uno) {
      qhat -= 1;
      rhat += b.limbs[b.len - 1];
    }

    while (rhat < uno &&
           (qhat * b.limbs[b.len - 2] > uno * rhat + a.limbs[k + b.len - 2])) {
      qhat -= 1;
      rhat += b.limbs[b.len - 1];
    }

    __int128_t carry = 0;
    __int128_t widedigit;
    for (uintmax_t i = 0; i < b.len; i++) {
      DoubleLimb product = qhat * b.limbs[i];

      widedigit = (a.limbs[k + i] + carry) - (product & (uno - 1));

      a.limbs[k + i] = widedigit;

      carry = (widedigit >> LIMB_SIZE_BITS) - (product >> LIMB_SIZE_BITS);
    }

    widedigit = a.limbs[k + b.len] + carry;
    a.limbs[k + b.len] = widedigit;

    q->limbs[k] = qhat;

    if (widedigit < 0) {
      q->limbs[k] -= 1;
      widedigit = 0;
      for (uintmax_t i = 0; i < b.len; i++) {
        widedigit += a.limbs[k + i] + b.limbs[i];
        a.limbs[k + i] = widedigit;
        widedigit >>= LIMB_SIZE_BITS;
      }
      a.limbs[k + b.len] += carry;
    }
  }

  for (uintmax_t i = 0; i < b.len - 1; i++) {
    r->limbs[i] = (a.limbs[i] >> shifts) |
                  (((DoubleLimb)a.limbs[i + 1]) << (LIMB_SIZE_BITS - shifts));
  }
  r->limbs[b.len - 1] = a.limbs[b.len - 1] >> shifts;

  bigint_free_limbs(&a);
  bigint_free_limbs(&b);

  return Ok;
}

size_t bigint_bit_length(const bigint *a) {
  for (size_t i = a->len - 1; i + 1 > 0; i--) {
    if (a->limbs[i] != 0) {
      for (size_t bit = LIMB_SIZE_BITS - 1; bit + 1 > 0; bit--) {
        if (a->limbs[i] & (1ul << bit)) {
          return i * LIMB_SIZE_BITS + bit + 1;
        }
      }
    }
  }
  return 0;
}

BigIntError bigint_montgomery_init(const bigint *modulus, Montgomery *m) {
  if (bigint_is_zero(modulus)) {
    return DivisionByZeroError;
  }
  if ((modulus->limbs[0] & 1) == 0) {
    return NotImplemented;
  }
  m->modulus = *modulus;
  m->n = bigint_bit_length(modulus);
  bigint dividend = BIGINT_ZERO;
  bigint_resize(&dividend, m->n * 2 / LIMB_SIZE_BITS + 1);
  dividend.limbs[m->n * 2 / LIMB_SIZE_BITS] = (1ul << ((m->n * 2) % LIMB_SIZE_BITS));

  bigint q = BIGINT_ZERO;
  bigint_div(&dividend, modulus, &q, &m->rrm);
  return Ok;
}