#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cos_rat.h"
#include "cos_status.h"

int saem_kernel_rational(const char *operation, const char *left_text,
                         const char *right_text, char *out, size_t out_capacity) {
  CosRat left;
  CosRat right;
  CosRat result;
  CosStatus status;
  char formatted[64];

  if (operation == NULL || left_text == NULL || right_text == NULL ||
      out == NULL || out_capacity == 0U) {
    return COS_E_INVALID;
  }

  status = cos_rat_parse((const uint8_t *)left_text, strlen(left_text), &left);
  if (status != COS_OK) return status;
  status = cos_rat_parse((const uint8_t *)right_text, strlen(right_text), &right);
  if (status != COS_OK) return status;

  if (strcmp(operation, "add") == 0) {
    status = cos_rat_add(left, right, &result);
  } else if (strcmp(operation, "subtract") == 0) {
    status = cos_rat_sub(left, right, &result);
  } else if (strcmp(operation, "multiply") == 0) {
    status = cos_rat_mul(left, right, &result);
  } else if (strcmp(operation, "divide") == 0) {
    status = cos_rat_div(left, right, &result);
  } else {
    return COS_E_UNKNOWN;
  }
  if (status != COS_OK) return status;

  const size_t length = cos_rat_format(result, formatted, sizeof formatted);
  if (length == 0U || length >= out_capacity) return COS_E_RANGE;
  memcpy(out, formatted, length + 1U);
  return COS_OK;
}
