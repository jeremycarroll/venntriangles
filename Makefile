CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11

UNITY_DIR = ../Unity
CFLAGS += -I$(UNITY_DIR)/src
TEST_SRC = test/test_main.c test/test_known_solution.c test/test_initialize.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_BIN = $(TEST_SRC:.c=)
SRC      = main.c initialize.c globals.c cycles.c trail.c dynamic.c failure.c colors.c \
           point.c triangles.c debug.c statistics.c search.c
OBJ      = $(SRC:.c=.o)
OBJ3      = $(SRC:.c=.3.o)
XSRC     = entrypoint.c
XOBJ	 = $(XSRC:.c=.o)


all: formatheader main tests

test/test_venn3: test/test_venn3.c $(UNITY_DIR)/src/unity.c $(OBJ3)
	@# Impose formatting.
	@clang-format -i test/test_venn3.c
	@# Fix missing nl at eof.
	@if ! [ $$(tail -c 1 test/test_venn3.c | od -An -t x1) == "0a" ]; then echo >> test/test_venn3.c ; fi
	$(CC) $(CFLAGS) -DNCURVES=3 -o $@ $^

test/%: test/%.c $(UNITY_DIR)/src/unity.c $(OBJ)
	@# Impose formatting.
	@clang-format -i $<
	@# Fix missing nl at eof.
	@if ! [ $$(tail -c 1 $< | od -An -t x1) == "0a" ]; then echo >> $< ; fi
	$(CC) $(CFLAGS) -o $@ $^

formatheader:
	clang-format -i venn.h
	if ! [ $$(tail -c 1 venn.h | od -An -t x1) == "0a" ]; then echo >> venn.h ; fi

tests: test/test_venn3 test/test_initialize test/test_known_solution test/test_main
	for i in $^; do ./$$i; done

clean:
	rm -f $(TEST_OBJ) $(TEST_BIN) main $(OBJ) $(XOBJ)

main: $(OBJ) $(XOBJ)
	$(CC) $(CFLAGS) -o main $(OBJ) $(XOBJ)

%.3.o: %.c venn.h
	$(CC) $(CFLAGS) -DNCURVES=3 -c $< -o $@


%.o: %.c venn.h
	@# Impose formatting.
	@clang-format -i $<
	@# Fix missing nl at eof.
	@if ! [ $$(tail -c 1 $< | od -An -t x1) == "0a" ]; then echo >> $< ; fi
	$(CC) $(CFLAGS) -c $< -o $@
