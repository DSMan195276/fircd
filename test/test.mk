
# This defines the names of all the tests we should run
TESTS := confuse
TESTS += confuse_dup_suite
TESTS += confuse_validate_suite
#TESTS += confuse_list_suite # Currently not run, confuse has some seg fault
                             # issues with it

# Definitions of the tests listed above
# Each test is defined by a list of files that make it up
#
# In general, there is a corespondence between one .c file in ./src and one .c
# file in ./test. The ./test file contains the definition of main() as well as
# the test functions to run (And this is also compiled in with ./test/test.c,
# which contains various functions for testing accessable via ./test/test.h)
#
# More complex tests may require the use of more then just one .c file from
# ./src, in which case all of them should be listed. Also possible is having
# more the one test program per ./src file.
confuse.SRC := ./test/confuse_test.c ./src/confuse.c ./src/lex/lexer.c
confuse_dup_suite.SRC := ./test/confuse_dup_test.c ./src/confuse.c ./src/lex/lexer.c
confuse_validate_suite.SRC := ./test/confuse_validate_test.c ./src/confuse.c ./src/lex/lexer.c
confuse_list_suite.SRC := ./test/confuse_list_test.c ./src/confuse.c ./src/lex/lexer.c

# This template generates a list of the outputted test executables, as well as
# rules for compiling them.
define TEST_template
$(1).OBJ := $($(1).SRC:%.c=%.o)
TEST_TESTS += ./test/bin/$(1)_test
./test/bin/$(1)_test: ./test/test.o $$($(1).OBJ) | ./test/bin
	@echo " CCLD    ./test/bin/$(1)_test"
	$(Q)$(CC) $(LDFLAGS) ./test/test.o -o $$@ $$($(1).OBJ)
endef

# Run the template over all of our tests
$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

.PHONY: clean_tests run_tests

test/bin:
	@echo " MKDIR   ./test/bin"
	$(Q)mkdir ./test/bin

clean_tests:
	@echo " RM      ./test/*.o"
	$(Q)rm -f ./test/*.o
	@echo " RM      $(TEST_TESTS)"
	$(Q)rm -f $(TEST_TESTS)
	@echo " RM      ./test/bin"
	$(Q)rm -fr ./test/bin

run_tests: ./test/test.o $(TEST_TESTS)
	$(Q)./test/run_tests.sh $(TESTS)

./test/%.o: ./test/%.c
	@echo " CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@


