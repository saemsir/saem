/* Link probe: references the whole kernel API so --gc-sections keeps every module and --no-undefined proves nothing else is missing. */
#include "cos_core.h"
#include "cos_rat.h"
#include "cos_sha256.h"

extern unsigned char __stack_bottom[];
volatile unsigned long long probe_result;

void qcos_main(void) {
  uint8_t d[32];
  cos_sha256((const uint8_t *)"abc", 3, d);
  CosRat a, b, c;
  (void)cos_rat_make(1, 3, &a);
  (void)cos_rat_make(1, 6, &b);
  (void)cos_rat_add(a, b, &c);
  CosScan sc = cos_gate_scan((const uint8_t *)"ok", 2, 0, 0);
  CosCore *core = NULL;
  CosConfig cfg = {0};
  (void)cos_core_open(__stack_bottom, 0, &cfg, &core);
  probe_result = (unsigned long long)d[0] + (unsigned long long)c.num + (unsigned long long)sc.status + (core != NULL ? 1u : 0u);
  for (;;) {
  }
}
