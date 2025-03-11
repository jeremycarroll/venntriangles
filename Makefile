CC          = gcc
CFLAGS      = -g -Wall -Wextra -std=c11

UNITY_DIR   = ../Unity
TEST_CFLAGS = -I$(UNITY_DIR)/src
TEST_SRC    = test/test_venn6.c test/test_venn4.c test/test_known_solution.c test/test_venn5.c test/test_venn3.c \
              test/test_main.c test/test_initialize.c
TEST_BIN    = $(TEST_SRC:test/%.c=bin/%.u)
SRC         = main.c initialize.c globals.c cycles.c trail.c dynamic.c failure.c colors.c \
              point.c triangles.c debug.c statistics.c search.c
XSRC        = entrypoint.c
HDR	        = venn.h
OBJ3        = $(SRC:%.c=objs3/%.o)
OBJ4        = $(SRC:%.c=objs4/%.o)
OBJ5        = $(SRC:%.c=objs5/%.o)
OBJ6        = $(SRC:%.c=objs6/%.o)
XOBJ	    = $(XSRC:%.c=objs6/%.o)
TARGET      = bin/venn


all: .format $(TARGET) tests

bin/test_venn3.u: test/test_venn3.c $(UNITY_DIR)/src/unity.c $(OBJ3)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCURVES=3 -o $@ $^

bin/test_venn4.u: test/test_venn4.c $(UNITY_DIR)/src/unity.c $(OBJ4)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCURVES=4 -o $@ $^

bin/test_venn5.u: test/test_venn5.c $(UNITY_DIR)/src/unity.c $(OBJ5)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCURVES=5 -o $@ $^

bin/%.u: test/%.c $(UNITY_DIR)/src/unity.c $(OBJ6)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCURVES=6 -o $@ $^


.format: $(SRC) $(HDR) $(TEST_SRC) $(XSRC)
	clang-format -i $?
	for i in $?; do if ! [ $$(tail -c 1 $$i | od -An -t x1) == "0a" ]; then echo >> $$i ; fi; done
	touch .format

tests: $(TEST_BIN)
	for i in $^; do echo $$i; ./$$i | grep -v -e ':PASS$$' -e '^-*$$' ; done

clean:
	rm -f $(TEST_BIN) $(TARGET) $(OBJ6) $(OBJ3) $(XOBJ) $(TEST_BIN) .format

$(TARGET): $(OBJ6) $(XOBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ6) $(XOBJ)

objs%/debug.o: debug.c $(HDR)
		@echo Compiling $<
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCURVES=$(*F) -c $< -o $@

objs3/%.o: %.c  $(HDR)
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCURVES=3 -c $< -o $@


objs4/%.o: %.c  $(HDR)
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCURVES=4 -c $< -o $@


objs5/%.o: %.c  $(HDR)
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCURVES=5 -c $< -o $@

objs6/%.o: %.c  $(HDR)
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCURVES=6 -c $< -o $@

