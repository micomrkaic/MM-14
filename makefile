# Makefile for MM-14
#
#   make              optimized build       -> ./mm_14
#   make debug        ASan + UBSan build    -> ./mm_14-debug
#   make test         golden-file tests against ./mm_14
#   make test-debug   the same tests under the sanitizers
#   make docs         regenerate COMMANDS.md from the dispatch table
#   make install      install to $(PREFIX)  (default ~/.local)
#   make uninstall    remove what install put there
#   make clean
#
# Installed files:
#   $(PREFIX)/bin/mm_14
#   $(PREFIX)/share/man/man1/mm_14.1
#   $(PREFIX)/share/doc/mm_14/{README.md,COMMANDS.md,CHANGES.md,LICENSE}
#
# System-wide: sudo make install PREFIX=/usr/local

CC       ?= cc
CSTD      = -std=c17
WARN      = -Wall -Wextra -Werror -Wshadow -Wformat=2
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -MMD -MP
CFLAGS   ?= -O2 -g
LDLIBS    = -lreadline -lm

PREFIX   ?= $(HOME)/.local
BINDIR   ?= $(PREFIX)/bin
MANDIR   ?= $(PREFIX)/share/man/man1
DOCDIR   ?= $(PREFIX)/share/doc/mm_14
INSTALL  ?= install

TARGET   ?= mm_14
BUILD    ?= build

SRC = main.c interp.c ops.c words.c state.c stack.c registers.c my_math.c \
      my_statistics.c stack_printing.c help_functions.c tab_complete.c    \
      tokenizer.c util_func.c numerics.c
OBJ = $(SRC:%.c=$(BUILD)/%.o)
DEP = $(OBJ:.o=.d)
DOCS = README.md COMMANDS.md CHANGES.md LICENSE

# macOS ships libedit as "readline"; prefer Homebrew's GNU readline if present.
ifeq ($(shell uname -s),Darwin)
  RL_PREFIX ?= $(shell brew --prefix readline 2>/dev/null)
  ifneq ($(RL_PREFIX),)
    CPPFLAGS += -I$(RL_PREFIX)/include
    LDFLAGS  += -L$(RL_PREFIX)/lib
  else
    LDLIBS = -ledit -lm
  endif
endif

SAN = -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all

.PHONY: all debug test test-debug docs install uninstall clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CSTD) $(WARN) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

debug:
	$(MAKE) TARGET=mm_14-debug BUILD=build-debug CFLAGS="$(SAN)" LDFLAGS="$(LDFLAGS) -fsanitize=address,undefined"

test: $(TARGET)
	./tests/run.sh ./$(TARGET)

test-debug: debug
	ASAN_OPTIONS=detect_leaks=1 ./tests/run.sh ./mm_14-debug

docs: $(TARGET)
	./$(TARGET) --commands > COMMANDS.md

install: $(TARGET)
	$(INSTALL) -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR) $(DESTDIR)$(DOCDIR)
	$(INSTALL) -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/mm_14
	$(INSTALL) -m 644 mm_14.1 $(DESTDIR)$(MANDIR)/mm_14.1
	$(INSTALL) -m 644 $(DOCS) $(DESTDIR)$(DOCDIR)/
	@case ":$$PATH:" in *":$(BINDIR):"*) ;; \
	  *) echo "note: $(BINDIR) is not in your PATH" ;; esac
	@case ":$${MANPATH:-}:" in *":$(PREFIX)/share/man:"*) ;; \
	  *) command -v manpath >/dev/null 2>&1 && manpath 2>/dev/null | tr : '\n' | grep -qx "$(PREFIX)/share/man" \
	     || echo "note: 'man mm_14' may need MANPATH to include $(PREFIX)/share/man" ;; esac

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/mm_14
	rm -f $(DESTDIR)$(MANDIR)/mm_14.1
	rm -rf $(DESTDIR)$(DOCDIR)

clean:
	rm -rf build build-debug mm_14 mm_14-debug

-include $(DEP)
