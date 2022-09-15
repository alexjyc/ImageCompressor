# Compiler flags
CC := gcc
IFLAGS :=  -I/comp/40/build/include -I/usr/sup/cii40/include/cii 
CFLAGS := -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)
LDFLAGS := -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64 
LDLIBS := -l40locality -larith40 -lnetpbm -lcii40 -lm -lrt

includes := $(shell echo *.h)

MAIN := 40image ppmdiff

# Test
TEST := test_prog
TESTFLAGS := $(CFLAGS) -Wno-unused
TESTBUILD := test.o bitpack-test.o bitpack.o formulas-test.o formulas.o

# Prevent folder collision with target
.PHONY: $(MAIN)

%.o: %.c $(includes)
	$(CC) $(CFLAGS) -c $< -o $@

%-test.o: %-test.c
	$(CC) $(TESTFLAGS) -c $< -o $@

all: $(MAIN)

40image: 40image.o compress40.o a2blocked.o a2plain.o uarray2b.o uarray2.o \
         io.o transform.o formulas.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

test: $(TESTBUILD)
	$(CC) $(LDFLAGS) $^ -o $(TEST) $(LDLIBS);
	valgrind ./$(TEST);

ppmdiff: ppmdiff.o a2blocked.o uarray2b.o uarray2.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f *.o $(MAIN)