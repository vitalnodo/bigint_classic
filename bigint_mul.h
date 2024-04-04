#include "bigint.h"
static void supermul(Limb a, Limb b, Limb *lo, Limb *hi);
BigIntError bigint_mul(const bigint *a, const bigint *b, bigint *result);