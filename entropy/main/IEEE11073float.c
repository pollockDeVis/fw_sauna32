/*
 * IEEE11073float.c
 *
 *  Created on: May 30, 2020
 *      Author: Pollock
 */

#include "IEEE11073float.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

uint32_t float2IEEE11073(double data, uint8_t output[4])
{
  uint32_t result = MDER_NaN;


  if (isnan(data)) {
    goto finally;
  }

  double sgn; sgn = data > 0 ? +1 : -1;
  double mantissa; mantissa = fabs(data);
  int32_t exponent; exponent = 0; // Note: 10**x exponent, not 2**x

  // scale up if number is too big
  while (mantissa > MDER_FLOAT_MANTISSA_MAX) {
    mantissa /= 10.0;
    ++exponent;
    if (exponent > MDER_FLOAT_EXPONENT_MAX) {
      // argh, should not happen
      if (sgn > 0) {
        result = MDER_POSITIVE_INFINITY;
      } else {
        result = MDER_NEGATIVE_INFINITY;
      }
      goto finally;
    }
  }

  // scale down if number is too small
  while (mantissa < 1) {
    mantissa *= 10;
    --exponent;
    if (exponent < MDER_FLOAT_EXPONENT_MIN) {
      // argh, should not happen
      result = 0;
      goto finally;
    }
  }

  // scale down if number needs more precision
  double smantissa; smantissa = round(mantissa * MDER_FLOAT_PRECISION);
  double rmantissa; rmantissa = round(mantissa) * MDER_FLOAT_PRECISION;
  double mdiff; mdiff = abs(smantissa - rmantissa);
  while (mdiff > 0.5 && exponent > MDER_FLOAT_EXPONENT_MIN &&
      (mantissa * 10) <= MDER_FLOAT_MANTISSA_MAX) {
    mantissa *= 10;
    --exponent;
    smantissa = round(mantissa * MDER_FLOAT_PRECISION);
    rmantissa = round(mantissa) * MDER_FLOAT_PRECISION;
    mdiff = abs(smantissa - rmantissa);
  }

  uint32_t int_mantissa; int_mantissa = (int) round(sgn * mantissa);
  result = (exponent << 24) | (int_mantissa & 0xFFFFFF);

finally:
  if ( output ) memcpy(output, &result, 4);
  return result;
}
