# Makefile for calc project with math and readline/libedit
# Works on Linux and macOS

UNAME_S := $(shell uname -s)

CC      = gcc
CFLAGS  = -g -Wall -Wextra -Werror -std=c17
LDFLAGS =
LIBS    = -lm
READLINE_LIBS =

# ----- Platform-specific readline detection -----
ifeq ($(UNAME_S),Darwin)                       # macOS
READLINE_PREFIX ?= $(shell brew --prefix readline 2>/dev/null)
ifneq ($(READLINE_PREFIX),)                    # Use Homebrew readline if present
  CFLAGS  += -I$(READLINE_PREFIX)/include
  LDFLAGS += -L$(READLINE_PREFIX)/lib
  READLINE_LIBS += -lreadline -lncurses
else                                           # Fallback to libedit
  READLINE_LIBS += -ledit -lncurses
endif
else                                           # Linux and others
READLINE_LIBS += -lreadline
endif
# ------------------------------------------------

TARGET = mm_14

SRC = main.c my_math.c stack_printing.c stack.c globals.c my_statistics.c \
      words.c help_functions.c registers.c util_func.c tab_complete.c \
      tokenizer.c interpreter.c
OBJ = $(SRC:.c=.o)

# ----- Installation paths (override on make cmdline if desired) -----
HOME ?= $(shell printf %s "$$HOME")
XDG_CONFIG_HOME ?= $(HOME)/.config
CONFIG_DIR ?= $(XDG_CONFIG_HOME)/mm_14
BINDIR ?= $(HOME)/.local/bin
CONFIG_FILES = registers.txt user_words.txt

# Copy behavior: default is non-destructive (-n). Use OVERWRITE=1 to force.
ifeq ($(OVERWRITE),1)
  CPFLAGS := -f
else
  CPFLAGS := -n
endif
# -------------------------------------------------------------------

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LIBS) $(READLINE_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	mkdir -p "$(CONFIG_DIR)"
	cp $(CPFLAGS) $(CONFIG_FILES) "$(CONFIG_DIR)/"
	mkdir -p "$(BINDIR)"
	ln -sf "$(abspath $(TARGET))" "$(BINDIR)/$(TARGET)"
	@echo "Config: $(CONFIG_FILES) -> $(CONFIG_DIR)"
	@echo "Symlink: $(BINDIR)/$(TARGET) -> $(abspath $(TARGET))"
	@command -v $(TARGET) >/dev/null 2>&1 || echo "Note: add '$(BINDIR)' to your PATH"

clean:
	rm -f $(TARGET) $(OBJ)
