The program is essentially complete. The best feature is that it is very fast.

Two to-do items are:
- graphml output
- appropriate use of log methods to proviode variable levels of output.

Issues to be addressed in refactoring:
- [ ] Modularization: which functions and variables should be in which files and why.
- [ ] Replace monolithic header file with (approximately) one per source file?
- [ ] Order include statements appropriately
- [ ] Better use of -I in the Makefile
- [ ] Use of #include <> vs #include ""
- [ ] naming conventions: when to use camelCase, snake_case, all caps etc.
- [ ] naming conventions when to use prefixes to provide some namespacing
- [ ] ensuring most files are between 100 and 500 lines long
- [ ] removing unused functions
- [ ] removing unused struct fields
- [ ] replacing the EDGE out[] field in struct directedPoint with a single EDGE out field (I never used the second entry)
- [ ] ensuring setupCentralFaces uses the trail, so that we don't need to reinitialize in full_search_callback6 but can simply backtrack
- [ ] ensure toplevel documentation adequately describes the goal and approach
- [ ] rationalize initialization; maybe use more lazy initialization
- [ ] ensure visible_for_testing.h is doing the right thing - i.e. only exposing functions and variables to the test files as appropriate
- [ ] ensure coding conventions are documented
- [ ] ensure no use of // comments.
- [ ] ensure copyright header in every file.
- [x] eliminate GET_COMPRESSED_FACE_POINTER_ENTRY
- [ ] move FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET to color.c
- [ ] delete globals.c
- [ ] improve TRAIL_SIZE, add maxTrail to stats
- [ ] https://stackoverflow.com/questions/2394609/makefile-header-dependencies
- [ ] more clearly separating code that is for testing only.
- [ ] withoutColor - via function? - rename
- [ ] omittingCycleSets - move - rename
- [ ] include order as per https://stackoverflow.com/a/2762596
- [ ] names matching (|dynamic|initialize|reset)(Point|Color|ColorSet|Edge|Face|Cycle|CycleSet|Failure|Trail)[A-Z][a-z]*
- [ ] break up graph into edge and face
- [ ] revisit static/dynamic macros ...


Naming is not quite there yet ... the empty case is for global constants; we need to look at the exceptions
to these too.

