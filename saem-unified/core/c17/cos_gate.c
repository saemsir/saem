#include "cos_gate.h"

const char *cos_rule_name(CosRule r) {
  switch (r) {
    case COS_RULE_NONE: return "none";
    case COS_RULE_LENGTH: return "length";
    case COS_RULE_UTF8: return "utf8";
    case COS_RULE_NUL: return "nul";
    case COS_RULE_CONTROL: return "control";
    case COS_RULE_BIDI: return "bidi";
    case COS_RULE_INVISIBLE: return "invisible";
    case COS_RULE_JOINER: return "joiner";
    case COS_RULE_PRIVATE_USE: return "private_use";
    case COS_RULE_NONCHARACTER: return "noncharacter";
    case COS_RULE_DEPTH: return "depth";
    case COS_RULE_BRACKETS: return "brackets";
    case COS_RULE_METHOD: return "method";
    case COS_RULE_QUARANTINE: return "quarantine";
    case COS_RULE_FROZEN: return "frozen";
    case COS_RULE_SEAL: return "seal";
    case COS_RULE_ESCAPE: return "escape";
    case COS_RULE_SYNTAX: return "syntax";
    case COS_RULE_NUMBER: return "number";
    case COS_RULE_SHAPE: return "shape";
    case COS_RULE_DUPLICATE: return "duplicate";
    case COS_RULE_CAPACITY: return "capacity";
  }
  return "?";
}

/* ZWJ (U+200D) and ZWNJ (U+200C) are part of the spelling in Indic scripts (half forms, conjuncts) and in Arabic-script languages such as
 * Urdu and Persian. Rejecting them would reject Hindi. Everywhere else they hide text, so they are refused. */
static int joiner_ok(uint32_t prev) {
  return (prev >= 0x0900u && prev <= 0x0DFFu) || (prev >= 0xA8E0u && prev <= 0xA8FFu) ||                      /* Devanagari .. Sinhala, Devanagari Extended */
         (prev >= 0x0600u && prev <= 0x06FFu) || (prev >= 0x0750u && prev <= 0x077Fu) || (prev >= 0x0870u && prev <= 0x08FFu) || /* Arabic, Supplement, Extended-A and -B */
         (prev >= 0xFB50u && prev <= 0xFDFFu) || (prev >= 0xFE70u && prev <= 0xFEFEu);                                         /* Arabic presentation forms A and B */
}

CosRule cos_gate_classify(uint32_t cp, uint32_t prev) {
  if (cp == 0u) return COS_RULE_NUL;
  if (cp < 0x20u) return (cp == 9u || cp == 10u || cp == 13u) ? COS_RULE_NONE : COS_RULE_CONTROL;
  if (cp == 0x7Fu || (cp >= 0x80u && cp <= 0x9Fu)) return COS_RULE_CONTROL;
  if ((cp >= 0x202Au && cp <= 0x202Eu) || (cp >= 0x2066u && cp <= 0x2069u)) return COS_RULE_BIDI;
  if (cp == 0x200Bu || (cp >= 0x2060u && cp <= 0x2064u) || cp == 0xFEFFu || cp == 0x180Eu) return COS_RULE_INVISIBLE;
  if (cp == 0x200Cu || cp == 0x200Du) return joiner_ok(prev) ? COS_RULE_NONE : COS_RULE_JOINER;
  if ((cp >= 0xE0000u && cp <= 0xE007Fu) || (cp >= 0xE0100u && cp <= 0xE01EFu)) return COS_RULE_INVISIBLE;
  if ((cp >= 0xFDD0u && cp <= 0xFDEFu) || (cp & 0xFFFEu) == 0xFFFEu) return COS_RULE_NONCHARACTER; /* U+xFFFE and U+xFFFF in every plane */
  if (cp == 0xFFFDu || (cp >= 0xFFF9u && cp <= 0xFFFCu)) return COS_RULE_NONCHARACTER;
  if ((cp >= 0xE000u && cp <= 0xF8FFu) || (cp >= 0xF0000u && cp <= 0x10FFFDu)) return COS_RULE_PRIVATE_USE;
  return COS_RULE_NONE;
}

static CosScan fail(CosStatus st, CosRule rule, size_t off, uint32_t cps, uint32_t depth) {
  CosScan o = {st, rule, (uint32_t)off, cps, depth};
  return o;
}

/* Strict UTF-8 (Unicode Table 3-7). Returns the byte length, or 0 for a malformed sequence. */
static size_t decode(const uint8_t *p, size_t left, uint32_t *cp) {
  const uint8_t b0 = p[0];
  if (b0 < 0x80u) { *cp = b0; return 1; }
  size_t need;
  uint8_t lo = 0x80u, hi = 0xBFu;
  uint32_t v;
  if (b0 >= 0xC2u && b0 <= 0xDFu) { need = 2; v = b0 & 0x1Fu; }
  else if (b0 >= 0xE0u && b0 <= 0xEFu) {
    need = 3; v = b0 & 0x0Fu;
    if (b0 == 0xE0u) lo = 0xA0u;      /* no overlong */
    if (b0 == 0xEDu) hi = 0x9Fu;      /* no surrogates */
  } else if (b0 >= 0xF0u && b0 <= 0xF4u) {
    need = 4; v = b0 & 0x07u;
    if (b0 == 0xF0u) lo = 0x90u;      /* no overlong */
    if (b0 == 0xF4u) hi = 0x8Fu;      /* nothing above U+10FFFF */
  } else {
    return 0;
  }
  if (left < need) return 0;
  if (p[1] < lo || p[1] > hi) return 0;
  v = (v << 6) | (p[1] & 0x3Fu);
  for (size_t k = 2; k < need; ++k) {
    if ((p[k] & 0xC0u) != 0x80u) return 0;
    v = (v << 6) | (p[k] & 0x3Fu);
  }
  *cp = v;
  return need;
}

static int hexval(uint32_t c) {
  if (c >= '0' && c <= '9') return (int)(c - '0');
  if (c >= 'a' && c <= 'f') return (int)(c - 'a') + 10;
  if (c >= 'A' && c <= 'F') return (int)(c - 'A') + 10;
  return -1;
}

/* The letter after a backslash, as the character it stands for; 0xFFFFFFFF if it is not a JSON escape ('u' is handled separately). */
static uint32_t simple_escape(uint32_t c) {
  switch (c) {
    case '"': return 0x22u;
    case '\\': return 0x5Cu;
    case '/': return 0x2Fu;
    case 'b': return 0x08u;
    case 'f': return 0x0Cu;
    case 'n': return 0x0Au;
    case 'r': return 0x0Du;
    case 't': return 0x09u;
    default: return 0xFFFFFFFFu;
  }
}

/* State of the JSON string lexer. An escape is judged exactly like the raw character it stands for, so writing U+202E as "\u202E" does not
 * get past Zone 0. Lone surrogates are not text; a high surrogate must be followed at once by an escaped low surrogate. */
typedef struct JStr {
  uint8_t in, esc, uleft;
  uint32_t ucp;      /* the hex digits read so far */
  uint32_t hi;       /* a high surrogate waiting for its low half, else 0 */
  uint32_t pre_prev; /* the character before the current escape began */
  uint32_t hi_prev;  /* ... and before the high half of a pair */
  size_t esc_at, hi_at;
} JStr;

CosScan cos_gate_scan(const uint8_t *in, size_t n, uint32_t max_len, uint32_t flags) {
  const uint32_t limit = (max_len == 0u || max_len > COS_MAX_INPUT) ? COS_MAX_INPUT : max_len;
  if (n > limit) return fail(COS_E_TOO_BIG, COS_RULE_LENGTH, limit, 0, 0);
  if (n > 0u && in == NULL) return fail(COS_E_INVALID, COS_RULE_UTF8, 0, 0, 0);
  const int json = (flags & COS_SCAN_JSON) != 0u;
  uint32_t cps = 0, prev = 0, stack = 0, depth = 0, depth_max = 0;
  JStr j = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  size_t i = 0;
  while (i < n) {
    uint32_t cp = 0;
    const size_t len = decode(in + i, n - i, &cp);
    if (len == 0u) return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, i, cps, depth_max);
    const CosRule bad = cos_gate_classify(cp, prev);
    if (bad != COS_RULE_NONE) return fail(COS_E_FORBIDDEN, bad, i, cps, depth_max);
    uint32_t next_prev = cp;
    if (json) {
      if (j.in) {
        if (j.uleft != 0u) { /* inside \uXXXX */
          const int hv = cp < 0x80u ? hexval(cp) : -1;
          if (hv < 0) return fail(COS_E_FORBIDDEN, COS_RULE_ESCAPE, j.esc_at, cps, depth_max);
          j.ucp = (j.ucp << 4) | (uint32_t)hv;
          if (--j.uleft == 0u) {
            const uint32_t v = j.ucp;
            if (v >= 0xD800u && v <= 0xDBFFu) { /* a high half: wait for its low half */
              if (j.hi != 0u) return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, j.hi_at, cps, depth_max);
              j.hi = v;
              j.hi_at = j.esc_at;
              j.hi_prev = j.pre_prev;
            } else {
              uint32_t dec = v, before = j.pre_prev;
              size_t at = j.esc_at;
              if (v >= 0xDC00u && v <= 0xDFFFu) { /* a low half: it must complete a pair */
                if (j.hi == 0u) return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, j.esc_at, cps, depth_max);
                dec = 0x10000u + ((j.hi - 0xD800u) << 10) + (v - 0xDC00u);
                before = j.hi_prev;
                at = j.hi_at;
                j.hi = 0;
              } else if (j.hi != 0u) {
                return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, j.hi_at, cps, depth_max);
              }
              const CosRule r = cos_gate_classify(dec, before);
              if (r != COS_RULE_NONE) return fail(COS_E_FORBIDDEN, r, at, cps, depth_max);
              next_prev = dec;
            }
          }
        } else if (j.esc) { /* the character after a backslash */
          j.esc = 0;
          if (cp == 'u') {
            j.uleft = 4u;
            j.ucp = 0;
          } else {
            const uint32_t dec = simple_escape(cp);
            if (dec == 0xFFFFFFFFu) return fail(COS_E_FORBIDDEN, COS_RULE_ESCAPE, j.esc_at, cps, depth_max);
            if (j.hi != 0u) return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, j.hi_at, cps, depth_max);
            const CosRule r = cos_gate_classify(dec, j.pre_prev);
            if (r != COS_RULE_NONE) return fail(COS_E_FORBIDDEN, r, j.esc_at, cps, depth_max);
            next_prev = dec;
          }
        } else if (cp == '\\') {
          j.esc = 1;
          j.esc_at = i;
          if (j.hi == 0u) j.pre_prev = prev; /* after a high half, the pair is judged against what came before the high half */
        } else {
          if (j.hi != 0u) return fail(COS_E_FORBIDDEN, COS_RULE_UTF8, j.hi_at, cps, depth_max); /* a raw character or the closing quote right after a high half */
          if (cp == '"') j.in = 0;
        }
      } else if (cp >= 0x80u) {
        /* non-ASCII outside a string cannot open or close anything */
      } else if (cp == '"') {
        j.in = 1;
      } else if (cp == '{' || cp == '[') {
        if (depth >= COS_MAX_DEPTH) return fail(COS_E_FORBIDDEN, COS_RULE_DEPTH, i, cps, depth_max);
        stack = (stack << 1) | (cp == '[' ? 1u : 0u);
        depth++;
        if (depth > depth_max) depth_max = depth;
      } else if (cp == '}' || cp == ']') {
        if (depth == 0u || ((stack & 1u) != 0u) != (cp == ']')) return fail(COS_E_FORBIDDEN, COS_RULE_BRACKETS, i, cps, depth_max);
        stack >>= 1;
        depth--;
      }
    }
    prev = next_prev;
    i += len;
    cps++;
  }
  if (json && (j.in || depth != 0u)) return fail(COS_E_FORBIDDEN, COS_RULE_BRACKETS, n, cps, depth_max);
  CosScan ok = {COS_OK, COS_RULE_NONE, 0u, cps, depth_max};
  return ok;
}
