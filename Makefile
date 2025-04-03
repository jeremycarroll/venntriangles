CC		  = gcc
CFLAGS	  = -g -Wall -Wextra -std=c11 -MMD -Wmissing-prototypes -Wmissing-declarations -Wshadow
UNITY_DIR   = ../Unity
TEST_CFLAGS = -I$(UNITY_DIR)/src -I.
TEST_SRC	=  test/test_venn4.c test/test_venn5.c test/test_known_solution.c test/test_venn3.c \
			  test/test_d6.c  test/test_venn6.c test/test_initialize.c test/test_main.c 
TEST_BIN	= $(TEST_SRC:test/%.c=bin/%)
# Do not include entrypoint.c in the test builds, it contains the main function, which is also in the test files.
SRC		    = main.c trail.c failure.c color.c edge.c \
			  point.c statistics.c search.c d6.c face.c utils.c memory.c
TEST_HELPERS = test/test_helpers.c
XSRC		= entrypoint.c
HDR			= color.h edge.h statistics.h core.h face.h main.h trail.h \
			  d6.h failure.h point.h search.h memory.h
OBJ3		= $(SRC:%.c=objs3/%.o) $(TEST_HELPERS:test/test_%.c=objs3/test_%.o)
OBJ4		= $(SRC:%.c=objs4/%.o) $(TEST_HELPERS:test/test_%.c=objs4/test_%.o)
OBJ5		= $(SRC:%.c=objs5/%.o) $(TEST_HELPERS:test/test_%.c=objs5/test_%.o)
OBJ6		= $(SRC:%.c=objs6/%.o)
TEST_OBJ6 = $(TEST_HELPERS:test/test_%.c=objs6/test_%.o)
XOBJ		= $(XSRC:%.c=objs6/%.o)
DEP		 = $(OBJ6:.o=.d) $(OBJ5:.o=.d) $(OBJ4:.o=.d) $(OBJ3:.o=.d) $(XOBJ:.o=.d) $(TEST_SRC:test/%.c=bin/%.d)
TARGET	  = bin/venn

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

bin/test_main: objst/test_main.o $(UNITY_DIR)/src/unity.c objs6/main.o
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

bin/test_%: objst/test_%.o $(UNITY_DIR)/src/unity.c $(OBJ6) $(TEST_OBJ6)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^

.format: $(SRC) $(HDR) $(TEST_SRC) $(XSRC) $(D6) $(TEST_HELPERS)
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

objsv/test_venn%.o: test/test_venn%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=$(*F) -c $< -o $@

objst/test_%.o: test/test_%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=6 -c $< -o $@

objs3/test_%.o: test/test_%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=3 -c $< -o $@

objs4/test_%.o: test/test_%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=4 -c $< -o $@

objs5/test_%.o: test/test_%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=5 -c $< -o $@

objs6/test_%.o: test/test_%.c
	@echo Compiling $<
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

