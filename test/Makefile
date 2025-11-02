# This Makefile assumes the top folder has been built
TOP = ..

CORE_DIR      = $(TOP)/core
INCLUDE_DIR   = $(TOP)/include
GENERATED_DIR = $(TOP)/generated
REFLECTC_DIR  = $(TOP)/reflect-c

TEST_DISCORD = racecond rest timeout
TEST_CORE    = websockets queriec

TESTS = $(TEST_DISCORD) $(TEST_GITHUB) $(TEST_CORE)

CFLAGS  = -O0 -g -pthread -Wall \
		  -I$(INCLUDE_DIR) -I$(CORE_DIR) -I$(GENERATED_DIR) -I$(REFLECTC_DIR)
LDFLAGS = -L$(TOP)/lib
LDLIBS  = -ldiscord -lcurl

all: $(TESTS)

echo:
	@ echo -e 'CC: $(CC)\n'
	@ echo -e 'TESTS: $(TESTS)\n'

clean:
	@ rm -f $(TESTS)

.PHONY: all echo clean
