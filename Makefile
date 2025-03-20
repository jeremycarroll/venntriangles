CC          = gcc
CFLAGS      = -g -Wall -Wextra -std=c11 -MMD
UNITY_DIR   = ../Unity
TEST_CFLAGS = -I$(UNITY_DIR)/src
TEST_SRC    = test/test_venn6.c test/test_main.c test/test_venn4.c test/test_known_solution.c test/test_venn3.c \
	          test/test_initialize.c test/test_d6.c  test/test_venn5.c
TEST_BIN    = $(TEST_SRC:test/%.c=bin/%)
# Do not include entrypoint.c in the test builds, it contains the main function, which is also in the test files.
SRC         = main.c initialize.c trail.c dynamic.c failure.c color.c \
	          point.c triangles.c debug.c statistics.c search.c d6.c output.c logging.c
XSRC        = entrypoint.c
HDR	        = venn.h core.h
OBJ3        = $(SRC:%.c=objs3/%.o)
OBJ4        = $(SRC:%.c=objs4/%.o)
OBJ5        = $(SRC:%.c=objs5/%.o)
OBJ6        = $(SRC:%.c=objs6/%.o)
XOBJ	    = $(XSRC:%.c=objs6/%.o)
DEP         = $(OBJ6:.o=.d) $(OBJ5:.o=.d) $(OBJ4:.o=.d) $(OBJ3:.o=.d) $(XOBJ:.o=.d) $(TEST_SRC:test/%.c=bin/%.d)
TARGET      = bin/venn

.SECONDARY: 

all: .format $(TARGET) tests

-include $(DEP)

bin/test_venn3: objsv/test_venn3.o $(UNITY_DIR)/src/unity.c $(OBJ3)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

bin/test_venn4: objsv/test_venn4.o $(UNITY_DIR)/src/unity.c $(OBJ4)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

bin/test_venn5: objsv/test_venn5.o $(UNITY_DIR)/src/unity.c $(OBJ5)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

bin/test_main: objst/test_main.o $(UNITY_DIR)/src/unity.c objs6/main.o objs6/logging.o
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

bin/test_%: objst/test_%.o $(UNITY_DIR)/src/unity.c $(OBJ6)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

.format: $(SRC) $(HDR) $(TEST_SRC) $(XSRC) $(D6)
	clang-format -i $?
	for i in $?; do if ! [ $$(tail -c 1 $$i | od -An -t x1) == "0a" ]; then echo >> $$i ; fi; done
	touch .format

tests: $(TEST_BIN)
	for i in $^; do echo $$i; ./$$i | grep -v -e ':PASS$$' -e '^-*$$' ; done

clean:
	rm -rf bin objs? .format

$(TARGET): $(OBJ6) $(XOBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ6) $(XOBJ)

objs%/debug.o: debug.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=$(*F) -c $< -o $@

objsv/test_venn%.o: test/test_venn%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=$(*F) -c $< -o $@

objst/test_%.o: test/test_%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=6 -c $< -o $@

objs3/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCOLORS=3 -c $< -o $@

objs4/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCOLORS=4 -c $< -o $@

objs5/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCOLORS=5 -c $< -o $@

objs6/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCOLORS=6 -c $< -o $@

