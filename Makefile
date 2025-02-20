CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -arch arm64 

UNITY_DIR = ../Unity
CFLAGS += -I$(UNITY_DIR)/src
TEST_SRC = test/test_main.c
TEST_OBJ = $(TEST_SRC:.c=.o)
SRC      = main.c initialize.c globals.c cycles.c trail.c dynamic.c failure.c colors.c triangles.c point.c
OBJ      = $(SRC:.c=.o)
XSRC     = entrypoint.c
XOBJ	 = $(XSRC:.c=.o)

test/%: test/%.c $(UNITY_DIR)/src/unity.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^


all: formatheader main tests

formatheader:
	clang-format -i venn.h
	if ! [ $$(tail -c 1 venn.h | od -An -t x1) == "0a" ]; then echo >> venn.h ; fi

tests: test/test_initialize test/test_main test/test_known_solution
	for i in $^; do ./$$i; done

clean:
	rm -f $(TEST_OBJ) $(TEST_BIN) main $(OBJ) $(XOBJ)

main: $(OBJ) $(XOBJ)
	$(CC) $(CFLAGS) -o main $(OBJ) $(XOBJ)

%.o: %.c venn.h
	# Impose formatting.
	clang-format -i $<
	# Fix missing nl at eof.
	if ! [ $$(tail -c 1 $< | od -An -t x1) == "0a" ]; then echo >> $< ; fi
	$(CC) $(CFLAGS) -c $< -o $@
