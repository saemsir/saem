/* Zone 0: the hard, deterministic filter every raw input passes before anything else looks at it.
 * Pure function: bytes in, verdict out. No allocation, no state, no clock, no floating point.
 * It checks REPRESENTATION only (is this well-formed, visible, bounded text?); what the text MEANS is decided later.
 * Adapted from xv6-riscv (MIT): kernel/syscall.c (one door, bounds on every argument) and kernel/vm.c (copyin/copyinstr never trust a caller's
 * pointer or length), plus the invisible-text rule of the Open Math Harness protocol. Rewritten in C17 for the Saem sovereign kernel. */
#ifndef COS_GATE_H
#define COS_GATE_H
#include <stddef.h>
#include <stdint.h>

#include "cos_limits.h"
#include "cos_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CosRule {
  COS_RULE_NONE = 0,
  COS_RULE_LENGTH = 1,       /* more bytes than the method allows */
  COS_RULE_UTF8 = 2,         /* malformed UTF-8: bad lead byte, truncated, overlong, surrogate, beyond U+10FFFF */
  COS_RULE_NUL = 3,          /* a NUL byte inside text */
  COS_RULE_CONTROL = 4,      /* a control character other than tab, line feed, carriage return */
  COS_RULE_BIDI = 5,         /* a direction override, embedding or isolate (the "Trojan Source" family) */
  COS_RULE_INVISIBLE = 6,    /* zero-width space, word joiner, invisible operators, BOM inside text, tag characters, VS17-256 */
  COS_RULE_JOINER = 7,       /* ZWJ or ZWNJ where the script does not use them */
  COS_RULE_PRIVATE_USE = 8,  /* private-use code points: usually text from a legacy font that is not real Unicode */
  COS_RULE_NONCHARACTER = 9, /* noncharacters, the replacement character, annotation marks: text that was already damaged */
  COS_RULE_DEPTH = 10,       /* brackets nested deeper than COS_MAX_DEPTH */
  COS_RULE_BRACKETS = 11,    /* unbalanced or crossed brackets, or an unterminated string */
  COS_RULE_METHOD = 12,      /* a method this core does not serve */
  COS_RULE_QUARANTINE = 13,  /* the method is quarantined until a person lifts it */
  COS_RULE_FROZEN = 14,      /* the core is frozen */
  COS_RULE_SEAL = 15,        /* the constitution seal no longer matches */
  COS_RULE_ESCAPE = 16,      /* a malformed escape inside a JSON string (bad letter after a backslash, bad hex digits) */
  COS_RULE_SYNTAX = 17,      /* the Semantic IR parser: not valid JSON (cos_sir.h) */
  COS_RULE_NUMBER = 18,      /* a number that cannot be held exactly: too large, too small, or too precise (cos_rat.h) */
  COS_RULE_SHAPE = 19,       /* the structure is not the one the method expects: unknown key, missing key, wrong kind */
  COS_RULE_DUPLICATE = 20,   /* the same key twice in one object: two readers could pick different values */
  COS_RULE_CAPACITY = 21     /* more nodes than the fixed arena holds */
} CosRule;

#define COS_SCAN_JSON 1u /* also check that { } [ ] balance outside strings, to COS_MAX_DEPTH, and judge every \\uXXXX and \\n-style escape inside
                          * strings as the character it stands for: "\\u202E" is as forbidden as the raw character, "\\ud800" alone is not text */

typedef struct CosScan {
  CosStatus status;    /* COS_OK, COS_E_TOO_BIG, COS_E_FORBIDDEN or COS_E_INVALID */
  CosRule rule;        /* which rule fired */
  uint32_t offset;     /* byte offset of the first violation */
  uint32_t codepoints; /* code points scanned before stopping */
  uint32_t depth_max;  /* deepest bracket nesting seen (COS_SCAN_JSON only) */
} CosScan;

/* max_len 0 means COS_MAX_INPUT; a larger value is clamped to COS_MAX_INPUT. */
CosScan cos_gate_scan(const uint8_t *in, size_t n, uint32_t max_len, uint32_t flags);
const char *cos_rule_name(CosRule r);
/* The character-class rule alone, for one code point given what came before it: COS_RULE_NONE if allowed. The same function is used by the
 * scan, by the escape decoder and by the Semantic IR, so there is exactly one definition of "forbidden". */
CosRule cos_gate_classify(uint32_t cp, uint32_t prev);

#ifdef __cplusplus
}
#endif
#endif
