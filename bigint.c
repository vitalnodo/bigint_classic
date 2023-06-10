#include "bigint.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void bigint_free(bigint *bigint) {
  free(bigint->limbs);
  free(bigint);
}

size_t calc_needed_limbs_for_hex(size_t hex_len) {
  size_t needed_limbs = (hex_len % (LIMB_SIZE_BYTES * 2) == 0
                             ? hex_len / (LIMB_SIZE_BYTES * 2)
                             : hex_len / (LIMB_SIZE_BYTES * 2) + 1);
  needed_limbs = needed_limbs > MIN_LIMBS ? needed_limbs : MIN_LIMBS;
  return needed_limbs;
}

bool bigint_set_hex(bigint *bigint, const char *hex) {
  const size_t hex_len = strlen(hex);
  const size_t needed_limbs = calc_needed_limbs_for_hex(hex_len);

  bigint->capacity = needed_limbs;
  bigint->len = needed_limbs;
  bigint->limbs = realloc(bigint->limbs, needed_limbs * sizeof(Limb));
  if (bigint->limbs == NULL) {
    bigint->len = 0;
    return false;
  }

  size_t current_limb = 0;
  size_t nibble_count = 0;
  Limb limb = 0;

  for (size_t i = hex_len - 1; i + 1 > 0; i--) {
    char nibble = hex[i];
    char value = char2digit(nibble);

    if (value == -1) {
      bigint->len = 0;
      return false;
    }

    limb |= ((Limb)value) << (4 * nibble_count);

    if (nibble_count == (LIMB_SIZE_BYTES * 2 - 1)) {
      bigint->limbs[current_limb] = limb;
      current_limb++;
      nibble_count = 0;
      limb = 0;
    } else {
      nibble_count++;
    }
  }

  if (nibble_count > 0) {
    bigint->limbs[current_limb] = limb;
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

bigint *bigint_init_hex(const char *hex) {
  bigint *bigint = bigint_new_capacity(MIN_LIMBS);
  if (bigint == NULL) {
    return NULL;
  }
  bool is_ok = bigint_set_hex(bigint, hex);
  if (!is_ok) {
    bigint_free(bigint);
    return NULL;
  }
  return bigint;
}

bigint *bigint_bit_not(const bigint *bigint_) {
  bigint *c = bigint_new_capacity(bigint_->len);
  if (c == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < bigint_->len; i++) {
    c->limbs[i] = ~(bigint_->limbs[i]);
  }
  c->len = bigint_->len;
  return c;
}

bigint *bigint_bit_xor(const bigint *first, const bigint *second) {
  BINARY_BIT_OPERATION(first, second, ^);
}

bigint *bigint_bit_or(const bigint *first, const bigint *second) {
  BINARY_BIT_OPERATION(first, second, |);
}

bigint *bigint_bit_and(const bigint *first, const bigint *second) {
  BINARY_BIT_OPERATION(first, second, &);
}

bigint *bigint_bit_shiftl(const bigint *bigint_, size_t n) {
  size_t limb_shifts = n / (LIMB_SIZE_BITS);
  size_t bit_shifts = n % (LIMB_SIZE_BITS);

  size_t new_len = bigint_->len + limb_shifts;
  bigint *result = bigint_new_capacity(new_len);
  if (result == NULL) {
    return NULL;
  }

  Limb carry = 0;
  for (size_t i = 0; i < bigint_->len; i++) {
    Limb shifted = (bigint_->limbs[i] << bit_shifts) | carry;
    carry = (bigint_->limbs[i] >> (LIMB_SIZE_BITS - bit_shifts)) &
            ((1 << bit_shifts) - 1);
    result->limbs[i + limb_shifts] = shifted;
  }
  if (carry > 0) {
    result->limbs[bigint_->len + limb_shifts] = carry;
    new_len++;
  }

  result->len = new_len;
  return result;
}

bigint *bigint_bit_shiftr(const bigint *bigint_, size_t n) {
  size_t limb_shifts = n / (LIMB_SIZE_BITS);
  size_t bit_shifts = n % (LIMB_SIZE_BITS);

  if (limb_shifts >= bigint_->len) {
    return bigint_init_hex("0");
  }

  size_t new_len = bigint_->len - limb_shifts;
  bigint *result = bigint_new_capacity(new_len);
  if (result == NULL) {
    return NULL;
  }

  Limb carry = 0;
  for (size_t i = new_len - 1; i + 1 > 0; i--) {
    Limb shifted = (bigint_->limbs[i + limb_shifts] >> bit_shifts) | carry;
    carry = (bigint_->limbs[i + limb_shifts] << ((LIMB_SIZE_BITS)-bit_shifts));
    if (LIMB_SIZE_BITS == n)
      carry &= ((1 << bit_shifts) - 1);
    result->limbs[i] = shifted;
  }
  result->len = new_len;
  return result;
}

bigint *bigint_add(const bigint *a, const bigint *b) {
  const size_t len_a = a->len;
  const size_t len_b = b->len;
  const size_t max_len = (len_a > len_b) ? len_a : len_b;

  bigint *result = bigint_new_capacity(max_len + 1);
  if (result == NULL) {
    return NULL;
  }

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
    result->limbs[max_len] = carry;
  }
  result->len = max_len + 1;
  return result;
}

bool bigint_greater_than(const bigint *a, const bigint *b) {
  bool res = true;
  if (bigint_equal(a, b)) {
    res = false;
  } else {
    for (int i = a->len - 1; i >= 0; i--) {
      if (a->limbs[i] < b->limbs[i]) {
        res = false;
        break;
      } else if (a->limbs[i] > b->limbs[i]) {
        res = true;
        break;
      }
    }
  }

  return res;
}

bool bigint_less_than(const bigint *a, const bigint *b) {
  return !bigint_greater_than(a, b);
}

bool bigint_equal(const bigint *a, const bigint *b) {
  const size_t len_a = a->len;
  const size_t len_b = b->len;
  const size_t max_len = (len_a > len_b) ? len_a : len_b;
  size_t x = 0;
  for (size_t i = 0; i < max_len; i++) {
    Limb a_ = (i < len_a) ? a->limbs[i] : 0;
    Limb b_ = (i < len_b) ? b->limbs[i] : 0;
    x += (a_ != b_);
  }
  return x == 0;
}

bigint *bigint_sub(const bigint *a, const bigint *b) {
  if (bigint_equal(a, b)) {
    return bigint_init_hex("0");
  }
  if (bigint_less_than(a, b)) {
    return NULL;
  }

  const size_t len_a = a->len;
  const size_t len_b = b->len;
  const size_t len = len_a;

  bigint *result = bigint_new_capacity(len + 1);
  if (result == NULL) {
    return NULL;
  }

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

  return result;
}