# Program wide settings
EXE       := fircd
VERSION   := 0
SUBLEVEL  := 1
PATCH     := 0
VERSION_N := $(VERSION).$(SUBLEVEL).$(PATCH)

# Compiler settings
CC      ?= cc
CFLAGS  := -Wall -I'./include' -O2 -std=gnu99         \
           -DFIRCD_VERSION=$(VERSION)                 \
           -DFIRCD_SUBLEVEL=$(SUBLEVEL)               \
           -DFIRCD_PATCH=$(PATCH)                     \
           -DFIRCD_VERSION_N="$(VERSION_N)"           \
		   -D_FORTIFY_SOURCE=0
LDFLAGS ?=
LEX     := flex
LFLAGS  := -Pcfg_yy


# Install Paths
PREFIX  := /usr
BINDIR  := $(PREFIX)/bin
MANDIR  := $(PREFIX)/share/man
MAN1DIR := $(MANDIR)/man1
DOCDIR  := $(PREFIX)/share/doc/fircd

# Configuration -- Uncomment lines to enable option

# Enable debugging
# FIRCD_DEBUG := y

# Show all commands executed by the Makefile
# VERBOSE := y

