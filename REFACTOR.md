The program is essentially complete. The best feature is that it is very fast.

A to-do items are:
- graphml output

Issues to be addressed in refactoring:
- [x] simplify output, e.g. just use stdout
- [x] move FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET to color.c
- [x] improve TRAIL_SIZE, add maxTrail to stats
- [x] rationalize initialization; maybe use more lazy initialization
- [x] Revist function names
- [x] ensuring most files are between 100 and 500 lines long
- [x] Order of declarations within files
- [ ] ensure toplevel documentation adequately describes the goal and approach
- [ ] ensure coding conventions are documented
- [ ] ensure copyright header in every file.
- [x] Modularization: which functions and variables should be in which files and why.
- [x] Replace monolithic header file with (approximately) one per source file?
- [x] Order include statements appropriately
- [x] Decide how to deal with globals
- [x] Better use of -I in the Makefile
- [x] Use of #include <> vs #include ""
- [x] naming conventions: when to use camelCase, snake_case, all caps etc.
- [x] naming conventions when to use prefixes to provide some namespacing
- [x] removing unused functions
- [x] removing unused struct fields
- [x] replacing the EDGE out[] field in struct directedPoint with a single EDGE out field (I never used the second entry)
- [x] ensuring setupCentralFaces uses the trail, so that we don't need to reinitialize in full_search_callback6 but can simply backtrack
- [x] ensure visible_for_testing.h is doing the right thing - i.e. only exposing functions and variables to the test files as appropriate
- [x] eliminate GET_COMPRESSED_FACE_POINTER_ENTRY
- [x] delete globals.c
- [x] https://stackoverflow.com/questions/2394609/makefile-header-dependencies
- [x] more clearly separating code that is for testing only.
- [x] withoutColor - via function? - rename
- [x] omittingCycleSets - move - rename
- [x] include order as per https://stackoverflow.com/a/2762596
- [x] names matching (|dynamic|initialize|reset)(Point|Color|ColorSet|Edge|Face|Cycle|CycleSet|Failure|Trail)[A-Z][a-z]*
- [x] break up graph into edge and face
- [x] revisit static/dynamic macros ...
- [x] remove logging stuff - unused


Misnamed:
- setCycleLength
- removeFromCycleSetWithTrail
- InitializeCycleSetPairs
- InitializeCycleSetTriples
- InitializeCycleSetSets
- getPoint

globals starting Dynamic ?

Naming is not quite there yet ... the empty case is for global constants; we need to look at the exceptions
to these too.

