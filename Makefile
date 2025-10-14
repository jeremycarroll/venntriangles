# Check for the -Wmissing-variable-declarations flag
GCC_CHECK := $(shell echo | gcc -E -x c - -Wmissing-variable-declarations 2>&1 | grep -o missing-variable-declarations)
ifeq ($(GCC_CHECK),)
    CFLAGS += -Wmissing-variable-declarations
endif

CC          = gcc
CFLAGS      += -g -Wall -Wextra -std=c11 -MMD -Wmissing-prototypes -Wmissing-declarations -Wshadow -fno-common
UNITY_DIR   = ../Unity
TEST_CFLAGS = -I$(UNITY_DIR)/src -I.
TEST_SRC    = test/test_chirotope.c test/test_pco4.c test/test_pco5.c test/test_pco2.c test/test_venn3.c test/test_s6.c test/test_initialize.c  \
              test/test_graphml.c test/test_venn4.c test/test_venn5.c test/test_venn6.c test/test_known_solution.c \
              test/test_main.c
TEST_BIN    = $(TEST_SRC:test/%.c=bin/%)
# Do not include entrypoint.c in the test builds, it contains the main function, which is also in the test files.
SRC         = main.c failure.c color.c cycle.c cycleset.c edge.c log.c vertex.c statistics.c s6.c face.c \
              dynamicface.c utils.c memory.c graphml.c triangles.c engine.c corners.c initialize.c nondeterminism.c \
              innerface.c venn.c save.c alternating.c
TEST_HELPERS = test/helper_for_tests.c
XSRC        = entrypoint.c
HDR         = color.h cycle.h cycleset.h dynamicface.h edge.h statistics.h core.h face.h main.h trail.h \
              s6.h failure.h vertex.h memory.h common.h triangles.h engine.h nondeterminism.h alternating.h
OBJ2        = $(SRC:%.c=objs2/%.o) $(TEST_HELPERS:test/%.c=objs2/%.o)
OBJ3        = $(SRC:%.c=objs3/%.o) $(TEST_HELPERS:test/%.c=objs3/%.o)
OBJ4        = $(SRC:%.c=objs4/%.o) $(TEST_HELPERS:test/%.c=objs4/%.o)
OBJ5        = $(SRC:%.c=objs5/%.o) $(TEST_HELPERS:test/%.c=objs5/%.o)
OBJ6        = $(SRC:%.c=objs6/%.o)
TEST_OBJ6   = $(TEST_HELPERS:test/%.c=objs6/%.o)
XOBJ        = $(XSRC:%.c=objs6/%.o)
DEP         = $(OBJ6:.o=.d) $(OBJ5:.o=.d) $(OBJ4:.o=.d) $(OBJ3:.o=.d) $(OBJ2:.o=.d) $(XOBJ:.o=.d) $(TEST_SRC:test/%.c=bin/%.d)
TARGET      = bin/venn

.SECONDARY: 

# If the user has installed Unity, then we use development mode - also needing clang-format.
UNITY_PRESENT := $(shell test -d $(UNITY_DIR) && echo "yes" || echo "no")

ifeq ($(UNITY_PRESENT),yes)
all: .format $(TARGET) tests
else
all: $(TARGET)
endif

-include $(DEP)

bin/test_%2: objsv/test_%2.o $(UNITY_DIR)/src/unity.c $(OBJ2)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

bin/test_%3: objsv/test_%3.o $(UNITY_DIR)/src/unity.c $(OBJ3)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

bin/test_%4: objsv/test_%4.o $(UNITY_DIR)/src/unity.c $(OBJ4)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

bin/test_%5: objsv/test_%5.o $(UNITY_DIR)/src/unity.c $(OBJ5)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

bin/test_main: objst/test_main.o $(UNITY_DIR)/src/unity.c objs6/main.o
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

bin/test_%: objst/test_%.o $(UNITY_DIR)/src/unity.c $(OBJ6) $(TEST_OBJ6)
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -o $@ $^ -lm

.format: $(SRC) $(HDR) $(TEST_SRC) $(XSRC) $(D6) $(TEST_HELPERS)
	clang-format -i $?
	for f in $?; do \
		if [ $$(tail -c 1 "$$f" | od -An -t x1) \!= "0a" ]; then \
			echo >> "$$f"; \
		fi; \
	done
	touch .format

tests: $(TEST_BIN)
	for i in $^; do echo $$i; bash -c "./$$i 2>&1" | grep -v -e ':PASS$$' -e '^-*$$' -e '^$$' ; done

clean:
	rm -rf bin objs? .format

$(TARGET): $(OBJ6) $(XOBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ6) $(XOBJ) -lm

objsv/test_%2.o: test/test_%2.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=2 -c $< -o $@

objsv/test_%3.o: test/test_%3.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=3 -c $< -o $@

objsv/test_%4.o: test/test_%4.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=4 -c $< -o $@

objsv/test_%5.o: test/test_%5.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=5 -c $< -o $@

objst/%.o: test/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=6 -c $< -o $@

objs2/%.o: test/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=2 -c $< -o $@

objs3/%.o: test/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=3 -c $< -o $@

objs4/%.o: test/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=4 -c $< -o $@

objs5/%.o: test/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=5 -c $< -o $@

objs6/%.o: test/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -DNCOLORS=6 -c $< -o $@

objs2/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DNCOLORS=2 -c $< -o $@

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

