#!/bin/sh
# Golden-file tests for MM-14.
#   tests/run.sh ./mm_14            run all tests
#   UPDATE=1 tests/run.sh ./mm_14   regenerate the .out files (review the diff!)
BIN=${1:-./mm_14}
DIR=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM
pass=0
fail=0

check() {   # name, actual-file
  if [ -n "$UPDATE" ]; then
    cp "$2" "$DIR/$1.out"
  elif diff -u "$DIR/$1.out" "$2" > "$TMP/diff"; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $1"
    cat "$TMP/diff"
  fi
}

for t in "$DIR"/*.rpn; do
  name=$(basename "$t" .rpn)
  rm -rf "$TMP/cfg"
  XDG_CONFIG_HOME="$TMP/cfg" "$BIN" -n -f "$t" > "$TMP/$name.actual" 2>&1
  echo "[exit $?]" >> "$TMP/$name.actual"
  check "$name" "$TMP/$name.actual"
done

# Command-line interface: -e, stdin pipe, exit status, usage error.
{
  "$BIN" -n -e "3 4 +" ; echo "[exit $?]"
  echo "2 3 * 1 +" | "$BIN" -n ; echo "[exit $?]"
  printf ': cube dup dup * * ;\n7 cube\n' | "$BIN" -n ; echo "[exit $?]"
  "$BIN" -n -e "1 0 /" ; echo "[exit $?]"
  "$BIN" -n -e ": open 1 2" ; echo "[exit $?]"
  "$BIN" -n -f /nonexistent/file ; echo "[exit $?]"
  "$BIN" --bogus > /dev/null 2>&1 ; echo "[exit $?]"
} > "$TMP/cli.actual" 2>&1
check cli "$TMP/cli.actual"

# Autoload: words and registers saved in one run are present in the next.
{
  rm -rf "$TMP/cfg"
  XDG_CONFIG_HOME="$TMP/cfg" "$BIN" -n -e ": twice 2 * ; savewords 3 regidx 1.5 sto saveregs" > /dev/null
  XDG_CONFIG_HOME="$TMP/cfg" "$BIN" -e "3 regidx rcl twice" ; echo "[exit $?]"
  XDG_CONFIG_HOME="$TMP/cfg" "$BIN" -n -e "3 regidx rcl twice" ; echo "[exit $?]"
} > "$TMP/autoload.actual" 2>&1
check autoload "$TMP/autoload.actual"

# The generated command reference must be up to date.
if [ -f "$DIR/../COMMANDS.md" ] && [ -z "$UPDATE" ]; then
  if "$BIN" --commands | cmp -s - "$DIR/../COMMANDS.md"; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: COMMANDS.md is stale (run 'make docs')"
  fi
fi

[ -n "$UPDATE" ] && echo "golden files updated" && exit 0
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
