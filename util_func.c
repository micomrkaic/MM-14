/*
 * This file is part of MM-14.
 *
 * MM-14 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MM-14 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MM-14 If not, see <https://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "state.h"
#include "util_func.h"

NumParse parse_number(const char *s, double *out) {
  if (!s || !*s) return NUM_NO;
  if (strspn(s, "0123456789+-.eE") != strlen(s)) return NUM_NO;
  if (!strpbrk(s, "0123456789")) return NUM_NO;
  char *end;
  double v = strtod(s, &end);
  if (end == s || *end != '\0') return NUM_NO;
  if (!isfinite(v)) return NUM_RANGE;
  if (out) *out = v;
  return NUM_OK;
}

static int mkdir_if_missing(const char *p) {
  if (mkdir(p, 0700) != 0 && errno != EEXIST)
    return rpn_err("Cannot create %s: %s", p, strerror(errno));
  return RPN_OK;
}

int config_path(const char *file, char *out, size_t outsz, bool create) {
  char base[1024];
  const char *xdg = getenv("XDG_CONFIG_HOME");
  if (xdg && *xdg) {
    snprintf(base, sizeof base, "%s", xdg);
  } else {
    const char *home = getenv("HOME");
    if (!home || !*home) return rpn_err("HOME is not set");
    snprintf(base, sizeof base, "%s/.config", home);
  }
  if (create) {
    char dir[1100];
    snprintf(dir, sizeof dir, "%s/mm_14", base);
    TRY(mkdir_if_missing(base));
    TRY(mkdir_if_missing(dir));
  }
  if ((size_t)snprintf(out, outsz, "%s/mm_14/%s", base, file) >= outsz)
    return rpn_err("Config path too long");
  return RPN_OK;
}
