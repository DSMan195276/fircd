
include ./config.mk

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)


# Set configuration options
ifdef VERBOSE
	Q :=
else
	Q := @
endif

ifdef FIRCD_DEBUG
	CFLAGS += -DFIRCD_DEBUG -g
endif

.PHONY: all install clean doc dist install_$(EXE) install_doc test

all: $(EXE) doc

include ./test/test.mk

dist: clean
	$(Q)mkdir -p $(EXE)-$(VERSION_N)
	$(Q)cp -R Makefile README.md config.mk LICENSE ./doc ./include ./src $(EXE)-$(VERSION_N)
	$(Q)tar -cf $(EXE)-$(VERSION_N).tar $(EXE)-$(VERSION_N)
	$(Q)gzip $(EXE)-$(VERSION_N).tar
	$(Q)rm -fr $(EXE)-$(VERSION_N)
	@echo " Created $(EXE)-$(VERSION_N).tar.gz"

install: install_$(EXE) install_doc

install_$(EXE): $(EXE)
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(DOCDIR)
	$(Q)install -d $(BINDIR) $(DOCDIR)
	@echo " INSTALL README.md LICENSE doc/fircdrc.example"
	$(Q)install -m 644 README.md LICENSE doc/fircdrc.example $(DOCDIR)
	@echo " INSTALL fircd"
	$(Q)install -m 775 fircd $(BINDIR)
	@echo " $(EXE) Installation done"

install_doc: doc
	$(Q)mkdir -p $(MAN1DIR)
	$(Q)mkdir -p $(MAN5DIR)
	$(Q)install -d $(MAN5DIR) $(MAN1DIR)
	@echo " INSTALL doc/fircd.1"
	$(Q)install -m 444 doc/fircd.1 $(MAN1DIR)
	@echo " Doc Installation done"

clean: clean_tests
	@echo " RM      $(OBJS)"
	$(Q)rm -f $(OBJS)
	@echo " RM      $(EXE)"
	$(Q)rm -f $(EXE)

$(EXE): $(OBJS)
	@echo " CCLD    $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) -o $@

src/%.o: src/%.c
	@echo " CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

test: run_tests

doc:

