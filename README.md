# MM-14 — an RPN calculator with numerical methods

A reverse Polish notation calculator in C17, in the spirit of the HP-15C, with a
Forth-like word system. It shares its core with [MM-11](https://github.com/micomrkaic/MM-11)
and adds the numerical layer:

- integration (tanh-sinh with adaptive bisection, and Romberg), root finding
  (Brent, and HP-15C-style `solve` from one guess), minimisation, numerical
  derivatives, finite sums and products — all over user-defined words
- real arithmetic, scientific and probability functions
- 100 registers
- HP-style Σ statistics with linear regression
- user-defined words with `if`/`do`/`begin` control flow, usable at top level too
- 64-level undo
- a scriptable batch mode

## Build and install

```
make                 # ./mm_14 (needs GNU readline; macOS uses Homebrew readline or libedit)
make test            # golden-file tests
make debug           # ./mm_14-debug with AddressSanitizer + UBSan
make test-debug      # tests under the sanitizers
make docs            # regenerate COMMANDS.md
make install         # to ~/.local: bin/mm_14, share/man/man1/mm_14.1, share/doc/mm_14/
make uninstall       # remove those
```

`PREFIX` defaults to `~/.local`, so no root is needed; `sudo make install
PREFIX=/usr/local` installs system-wide. Make sure `~/.local/bin` is in your
`PATH`. `man mm_14` works once `~/.local/share/man` is on the man path, which
most distributions handle automatically when the directory exists.

The build uses `-Werror`, so a new compiler release can turn a fresh warning
into a build failure. If that happens, `make WARN=-Wall` builds without it.

## Use

```
$ ./mm_14                      # interactive
> 3 4 +
> deg 30 sin
> : hyp ( a b -- c ) sq swap sq + sqrt ;
> 3 4 hyp
> : f dup * 2 - ;
> 0 3 ' f fzero               \ 1.414213562373095
> 0 pi ' sin integrate        \ 2

$ ./mm_14 -e "2 10 ^"          # one-shot
1024.000000
$ echo "all 1 3 /" | ./mm_14   # pipe
0.333333333333333
$ ./mm_14 -f script.rpn        # run a file
```

Batch runs (`-e`, `-f`, or stdin that is not a terminal) print the final stack
one value per line. They exit with status 1 if any error occurred. Pass `-n` to
skip autoloading saved words and registers.

The full command list is in [COMMANDS.md](COMMANDS.md), which is generated from
the dispatch table, and `man mm_14` covers the same ground. In the calculator,
`help` lists every command and `see <name>` describes one.

## Behaviour worth knowing

- **Stack labels.** The bottom four levels show as `X Y Z T`, and deeper levels
  by their depth. `pick` and `roll` use the same depth numbers: `0 pick` is
  `dup`, `1 roll` is `swap`, `2 roll` is `rot`.
- **Errors are atomic.** A failing token leaves everything as it was before
  that token, the same way an HP shows "Error". This covers the stack, last
  x/y, the registers and the statistics. The rest of the line is skipped. A
  user word counts as one token.
- **Undo.** `u` steps back through up to 64 changes. Settings, such as the
  display mode, are not part of undo. Neither are word definitions.
- **Registers.** `n regidx` selects a register, and then `sto`, `sto+`,
  `sto-`, `sto*`, `sto/` and `rcl` act on it. As on the HP, `sto` leaves X in
  place.
- **Statistics.** `s+` takes x from X and y from Y (HP order). The standard
  deviations are sample (n−1) values. The accumulators use Welford's method, so
  `s-` is exact and large offsets don't destroy precision. The side panel
  updates live: N and the sums on one side; x̄, sₓ, ȳ, sᵧ, r and the
  regression a, b on the other.
- **Percent (HP semantics).** `%` gives y·x/100, `%t` gives 100·x/y and `%chg`
  gives 100·(x−y)/y. All three leave y in Y.
- **Angles.** `deg`, `rad` and `grad` set the mode. In degrees and grads,
  multiples of 90°/100g give exact results, so `180 sin` is 0. `>p`/`>r`
  convert between rectangular and polar coordinates, and `>hms`/`>h` between
  decimal hours and H.MMSS.
- **Display.** The modes are `fix`, `sci`, `eng` (exponent a multiple of 3) and
  `all` (15 significant digits). `n p` sets the number of digits. FIX switches
  to SCI when a number is too large or too small to show.

## User words

```
: name body ;            \ may span several lines
( comment )   \ comment  \ parentheses need surrounding spaces
see name                 \ show the definition
listwords  n delword  clearwords  savewords  loadwords
```

- **Definition-time checks.** Each word is compiled when it is defined, so an
  unknown name is reported then rather than at run time. A word may call
  itself.
- **Late binding.** Calls are resolved by name, so redefining a word also
  changes what its callers do.
- **Limits.** Recursion stops at 256 levels. Ctrl-C interrupts a running word,
  and the stack rolls back.

Control flow works inside definitions only:

```
flag if ... [else ...] then
begin ... flag until
begin ... flag while ... repeat
limit start do ... i ... loop        \ skipped when start = limit
limit start do ... n +loop           \ negative n counts down to limit inclusive
```

```
: fac  dup 1 > if dup 1 - fac * then ;
: sum  ( n -- 1+..+n ) 0 swap 1 + 1 do i + loop ;
```

## Numerical methods

A function is any word (or built-in) that maps `x` on the stack to `f(x)`:
`( x -- f(x) )`. `' name` pushes its execution token, and the numerical words
consume that token:

```
a b ' f integrate      \ ∫ f over [a,b]: tanh-sinh, bisecting where it must
a b ' f romberg        \ the same by Romberg extrapolation, for comparison
a b ' f fzero          \ root in a bracket where f changes sign (Brent)
g   ' f solve          \ root near one guess: finds a bracket, then Brent
a b ' f minimize       \ argument of the minimum in [a,b] (Brent)
x   ' f deriv          \ f'(x) by Ridders' extrapolated central differences
lo hi ' f fsum         \ Σ f(i), i = lo..hi (integers; Kahan summation)
lo hi ' f fprod        \ Π f(i)
' f exec               \ run f
```

- **Tolerance.** `1e-6 tol` sets the convergence tolerance for all of them
  (default 1e-10, relative to `max(1, |result|)`). `nevals` tells how many
  function evaluations the last call used; `interr` is `integrate`'s error
  estimate. `internals` shows the current settings.
- **Functions run on the live stack**, so they may use registers and call other
  words. They must leave exactly one value; anything else is an error. An error
  inside the function aborts the call and the stack rolls back. Ctrl-C
  interrupts a long run.
- **Singularities.** tanh-sinh handles integrable endpoint singularities such as
  `1/sqrt(x)` on [0,1] to full precision when the singular endpoint is at 0.
  At another endpoint the nodes nearest to it round onto the endpoint, and the
  result is good to about 1e-8; a warning says so. Shift the variable so the
  singularity sits at 0 if you need more. Interior peaks and steps are handled
  by bisection.
- **Built-ins are functions too:** `0 pi ' sin integrate`, `2 ' exp deriv`.
- **For a maximum**, minimise the negation: `: negf f chs ;`.

```
: f dup * 2 - ;          0 3 ' f fzero       \ sqrt 2
1 ' f solve  -1 ' f solve                     \ either root, by guess
: g 1 + inv ;            0 1 ' g integrate   \ ln 2
: invsq dup * inv ;      1 1000 ' invsq fsum \ 1.6439345666...
```

## Compatibility with MM-14 1.x

- `DO LOOP IF ELSE THEN I` in upper case are accepted everywhere, as are the
  lower-case forms, and control flow works outside definitions: the rest of the
  line is compiled and run as one unit (`10 0 DO I . LOOP`).
- `integrate` and `fzero` now take the function as a third argument (`' f`),
  replacing `word_sel`; `int_prec`/`f0_prec` are replaced by `tol`; `p_prec` is
  `p`; `getstats` is `statscalc`; `stacksize` is `depth` (the old name is kept).
- A `~/.config/mm_14/user_words.txt` from 1.x is read when there is no
  `words.txt`; save once to convert. Words named after new built-ins (`sq`,
  `sqrt`) are rejected with their line number, since the built-in now does it.
- `%` and `%chg` follow the HP and keep y; `s+` takes x from X and y from Y.

## Files

Words, registers and readline history live in `$XDG_CONFIG_HOME/mm_14/`, which
defaults to `~/.config/mm_14/`. Saved words and registers load automatically at
startup. Saving writes to a temporary file and then renames it, so an
interrupted save can't corrupt the existing file. Words saved by 1.x in `user_words.txt` are picked up automatically.

| File | Contents |
|---|---|
| `main.c` | argument handling, REPL, batch mode |
| `interp.c` | line interpreter: comments, definitions, atomic tokens, undo |
| `ops.c` | **dispatch table** and all built-in operations |
| `words.c` | word compiler, executor, persistence |
| `state.c` | `State` (everything undoable), settings, undo ring, messages |
| `stack.c` | push/pop/peek |
| `registers.c` | registers and their file |
| `numerics.c` | tanh-sinh and Romberg integration, Brent root/minimum, solve, deriv, fsum |
| `my_math.c` | distributions, combinatorics, angles, H.MS, RNG |
| `my_statistics.c` | Welford accumulators and derived statistics |
| `stack_printing.c` | number formatting and the stack display |
| `help_functions.c` | help, `see`, COMMANDS.md — all from the table |
| `tab_complete.c` | readline completion over commands and user words |
| `tokenizer.c` | reentrant tokenizer |
| `util_func.c` | number parsing, config paths |
| `mm_14.1` | man page |
| `tests/` | `*.rpn` scripts with expected `*.out`; `run.sh` |

## Adding a command

Write `static int op_x(State *s)` in `ops.c` and add one row to `ops[]`, giving
its name, minimum stack depth, category, stack picture and description. The
dispatcher checks arity, and help, completion and `see` pick the command up
automatically. Run `make docs` and `UPDATE=1 tests/run.sh ./mm_14` afterwards,
then review the diff, and add the name to the category list in `mm_14.1`.

## Still to come in later calculators

- **MM-15:** complex numbers and matrices.
- **MM-48:** terminal plotting, symbolic algebra, strings, lists, units.

## Licence

GPL-3.0-or-later.
