#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
struct float_
{
  uint32_t frac : 23;
  uint32_t exp : 8;
  uint32_t sign : 1;
};
FLOAT F_mul_F(FLOAT a, FLOAT b)
{
  // assert(0);
  // return 0;
  return ((int64_t)a * (int64_t)b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b)
{
  // assert(0);
  // return 0;
  assert(b != 0);
  FLOAT x = Fabs(a);
  FLOAT y = Fabs(b);
  FLOAT z = x / y;
  x = x % y;

  for (int i = 0; i < 16; i++)
  {
    x <<= 1;
    z <<= 1;
    if (x >= y)
    {
      x -= y;
      z++;
    }
  }
  if (((a ^ b) & 0x80000000) == 0x80000000)
  {
    z = -z;
  }
  return z;
}

FLOAT f2F(float a)
{
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  struct float_ *f = (struct float_ *)&a;
  uint32_t res;
  uint32_t frac;
  int exp;
  if ((f->exp & 0xff) == 0xff)
    assert(0);
  else if (f->exp == 0)
  {
    exp = 1 - 127;
    frac = (f->frac & 0x7fffff);
  }
  else
  {
    exp = f->exp - 127;
    frac = (f->frac & 0x7fffff) | (1 << 23);
  }
  if (exp >= 7 && exp < 22)
    res = frac << (exp - 7);
  else if (exp < 7 && exp > -32)
    res = frac >> 7 >> -exp;
  else
    assert(0);
  return (f->sign) ? -res : res;
}

FLOAT Fabs(FLOAT a)
{
  return (a > 0) ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x)
{
  FLOAT dt, t = int2F(2);

  do
  {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while (Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y)
{
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do
  {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while (Fabs(dt) > f2F(1e-4));

  return t;
}
