# MM-14 2.0 — changes from 1.x

MM-14 2.0 is built on the MM-11 2.0 core (dispatch table, atomic errors,
compiled words, undo ring, batch mode, tests), so everything in MM-11's
CHANGES applies here too. This file lists what is specific to MM-14. Item
numbers refer to the MM-14 review.

## Bugs

| # | Fix |
|---|---|
| 1 | `IF` without `ELSE` ran its body for a false condition. Conditionals are now compiled to branches; `0 IF 5 THEN` pushes nothing. |
| 2 | Loop indices live on a per-word loop stack: nested loops work and `j` exists. |
| 3 | Functions are passed as execution tokens (`' f`) instead of a word index selected with `word_sel`; deleting words no longer silently changes which function `integrate` uses. The hardcoded `intg_function = 5` is gone. |
| 4 | A function that leaves the stack empty (or more than one value) is an error, not a read of `data[-1]`. |
| 5 | `fzero`, `solve`, `integrate` and friends fail as errors, with the stack rolled back, instead of pushing `a` or an unconverged value. |
| 6 | Bisection (100 iterations, absolute `|f| < tol`) is replaced by Brent's method with an x-tolerance; `solve` expands a bracket from one guess the way the HP-15C does. |
| 7 | Romberg kept (`romberg`), but `integrate` is now tanh-sinh with adaptive bisection: endpoint singularities, peaks and steps converge; words are compiled once rather than re-tokenized per evaluation. |
| 8 | `tol` must be in (0,1) and reports underflow; it replaces `int_prec` and `f0_prec`. |
| 9 | `.` honours the display mode. |
| 10 | Loop bodies and IF blocks are compiled, not copied into fixed buffers. |
| 11 | `;` outside a definition is an error. |
| 12 | Numbers that don't fit the FIX format fall back to SCI; the display is built with bounds-checked appends. |
| 13 | Registers autoload too; a missing file is silent. |
| 14 | `makefile_old`, `my_tokenizer.c` and the committed `registers.txt` are gone; `make install` copies the binary to `~/.local/bin` (not a symlink into the build tree) with the man page and docs. |
| 15 | Help, `see` and `COMMANDS.md` come from the dispatch table. |

## Additions

| # | Addition |
|---|---|
| 16 | `' name` pushes an execution token; built-ins and user words alike. `exec` runs one. |
| 17 | `integrate` (tanh-sinh, adaptive), `romberg`, `interr`, `nevals`. Checked against SciPy on 70 integrals including `x^-1/2`, `x ln x`, `1/sqrt(1-x^2)`, `exp(2 cos 5x)` and a width-1e-3 peak: all within 1e-9 except singularities at a non-zero endpoint (~1e-8, warned). |
| 18 | `fzero` (Brent), `solve` (one guess). |
| 19 | `deriv` (Ridders), `minimize` (Brent), `fsum`, `fprod`. |
| 20 | Compiled `do/loop/+loop/i/j`, `begin/until`, `begin/while/repeat`; `DO LOOP IF ELSE THEN I` accepted in upper case; control flow at top level compiles the rest of the line as one unit. |
| 21 | `stacksum`, `stacksumsq` kept; `stackmean`, `stackmax`, `stackmin`, `stack>stat` added; `depth` with `stacksize` as alias. |
| 22 | Everything from MM-11 2.0: atomic errors, 64-level undo, table-driven help and completion, batch mode (`-e`, `-f`, stdin), golden tests, sanitizer target, `~/.local` install, man page. |

## Behaviour changes that affect old word files

- `integrate` and `fzero` take three arguments: `a b ' f`.
- `word_sel`, `int_prec`, `f0_prec`, `p_prec` and `getstats` are gone
  (`' f`, `tol`, `p`, `statscalc`).
- `%` and `%chg` now leave y on the stack; `s+` swapped its x/y order.
- `sq`, `sqrt` and similar are built-ins now, so saved words with those names
  are rejected on load (with line numbers); the built-ins do the same job.
- `user_words.txt` is read when `words.txt` does not exist; the first
  `savewords` writes `words.txt`.
