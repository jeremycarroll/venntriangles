# Design Considerations

The goal of the program is to find all choices of facial cycle for each face such that
the overall result describes a planar graph that can be drawn with six triangles.

We cover both high level design and comments on lower level issues of wide scope. 
Comments about lower level issues which fit better in the code are in the code.

## High-Level Design

The problem of finding diagrams of 6 Venn triangles is a search problem. 
The search is divided into three parts:

1. Find a maximal sequence of 6 integers making a 5-degree signature.
1. (the main search) Find 64 facial cycles defining a Venn diagram with this 5-degree signature,
   which satisfies several necessary conditions to be drawable with triangles.
1. Find an edge to corner mapping for this Venn diagram,
   satisfying the condition that every pair of lines cross at most once.

The final step is to write the resulting Venn diagram, including its corners
into a [GraphML](http://graphml.graphdrawing.org/primer/graphml-primer.html) file.

We approach this in top-down fashion. Each of the three steps, involve guessing, and we usually guess badly. That
branch of the search ends in failure and we backtrack to the previous
choice point, and make the next guess.

Success is very similar to failure. We get to the end of the search,
satisfying the criteria for this phase. We then proceed to the next phase of the search,
which is based on the results so far.
After executing the next phase, we backtrack, undoing the guesses we have made so far,
and proceed to the next guess.


### Searching for Venn Diagrams

In the main search, at each step we assign a specific facial cycle to a specific face.
At every step in the search we have a set of possible facial cycles for each face.
If this set is empty for any face, then the search has failed and we backtrack to 
the previous choice point.
If this set is a singleton for any face, then we make that choice
and compute all its consequences, which may result in failure, or
in a further assignment in a face with only one remaining facial cycle.

In the main loop, we first choose the next face to assign to, then we choose
the facial cycle for the face. We can backtrack and guess again
over all possible choices for the facial cycle for the chosen face.
Conversely, the choice of face for this iteration
is not backtrackable - the face chosen has to have a facial cycle, we have decided
to guess it now.

## Non-deterministic Engine, Backtracking, Memory and the Trail

Given that the problem is non-deterministic, with three separate non-deterministic subproblems,
we encode them all uniformly as top-down searches, and abandon the usual top-level control flow
of C programs to instead use a non-deterministic engine.
The engine executes a short non-deterministic program,
consisting of a sequence of predicates. Each predicate can be evaluated to either succeed or fail
or create a choice point. Each choice point has a known number of choices. Each choice can either succeed
or fail, continuing to the next choice. When the choices are exhausted the predicate fails.

Success has two flavors: _success-same-predicate_ is a partial success that re-invokes the
current predicate with an incremented _round_ (starting at 0); _success-nest-predicate_ is a full success,
indicating that the engine should move on to the next predicate. It is a run time disaster if the final 
predicate in the program succeeds with _success-nest-predicate_. A constant predicate the _FAILPredicate_ is provided to be the final entry in most programs.

On failure, if the current execution is a choice-point, the next choice (if any) is invoked. Otherwise,
the program backtracks to the previous predicate. If there are no previous predicates then the 
program execution has completed, since the top-down search has been exhausted.

The engine is somewhat motivated by Prolog.

### Forward Backward Predicates

There are several control predicates defined by two boolean functions and a void function.

The first function is a gate, which can fail the predicate; if the gate passes then the
predicate has a simple choice between two options. The first, the forward operation, can: pass, proceeding to the next
predicate; or fail, proceeding directly to the third function. The third function, the backward operation, 
is executed, and then the predicate fails. This allows for inserting code execution into the program execution
as the non-deterministic search proceeds forward or backward.

In Prolog terms, this is:

```
predicate :- gate, (forward; backward, fail).
```

### Memory Management

While each execution step follows normal C memory management (e.g. local stack, static variables, malloc),
the behavior between the predicates and on backtracking is mixed.

1. The main program state, is in a single variable `Faces`. This stores: the facial cycle of each face; 
   the edges around the face; the adjacent faces; the faces that might be adjacent, or might have been adjacent;
   the vertices, both their actual, and possible configurations, etc. This state is tracked on the trail,
   and when the engine backtracks through the normal non-deterministic operation, any changes to this state
   are reversed. Thus the state reflects the currently active choices.
1. Ancillary state: there are a couple of global variables that are similarly tracked with the trail, e.g. `EdgeColorCount`**`State`**
   the number of times each pair of colors are crossing in the current solution. 
1. heap, through malloc. This is temporary memory only, and is freed on each step in the engine. This is 
   useful for temporary strings, and arrays etc. but long term use of the memory is not supported.
1. flags set on the command line, and readable throughout the code, e.g. `TargetFolder`**`Flag`**
1. shared state between the different predicates of the non-deterministic program, not on the trail.
   The engine provides no explicit data flow between the predicates being executed. The data flow is
   implicitly encoded with state variables, such as `GlobalSolutionsFound`**`IPC`** (IPC: inter-predicate communication) which is incremented
   as each solution is found, in the main search predicate, and read in several other predicates
   to support various output related tasks. Notice that the read accesses can occur in predicates both
   before and after the predicate updating the variable. In this particular case, we can log the solution
   number in the later predicates, operating in a forward direction, and log the number of solutions found, 
   in the earlier predicates, operating in a backward direction.
1. Memoized computation. In the initialize phase (which is implemented as the first predicate
   of the non-deterministic program), we compute many reusable structures. 
   During the main search, performance is critical. Hence, any operation
   that gets repeated is precomputed during an initialization phase.
   Many of these correspond to some of the constraints seen in [MATH.md](./MATH.md),
   for example, we precompute the set of all facial cycles that contain
   the sequence _i_, _j_, _k_ for all triples _i_, _j_, _k_. This allows
   the constraint to be applied during the search as a simple bitwise operation
   which is very fast, uses fixed memory, and is quick and easy to add to the trail.

   In some sense, many of the relationships between faces, and possible edges,
   and possible vertices, are also memoized - in the sense that these are all
   precomputed during initialization and then, when needed, simply put into place,
   often just by changing a single pointer.
   
   e.g. for each member of each
   possible facial cycle, we compute: the set of facial cycles, with two entries going in the same direction, 
   which might restrict a vertex adjacent face; and a set of facial cycles, with three entries going in the reverse direction, 
   which might restrict an edge adjacent face.

1. The predicates, such as `InnerFace`**`Predicate`**, which finds  6 integers making a 5-degree signature.

1. A couple of miscellaneous extras, like the `NonDeterministicProgram` itself.
## Seven Phases & Eight Predicates

There are hence the following phases and predicates:

1. Deterministic initialization

   As well as the usual, this includes initializing the global solution space; and memoizing
   several results needed for the main search.

1. Finding 5-degree signatures
1. A deterministic logging step

   With logging the number of results
   for a specific 5-face degree signature
   on the backward execution.

1. Finding Venn diagrams (the main search)

   This selects a solution.

1. A deterministic save step

   Writing the current solution to an appropriate
   file (and including the number of variation on 
   the backward execution)

1. Assigning corners

   Selecting all mappings of the 18 corners
   of the diagram to the possible faces in which 
   they might lie; this determines a variation
   of the solution.

1. Deterministically Writing GraphML
1. Fail - to force exhaustive searching

## Unit Testing

The program was developed using TDD (test driven development),
so while the tests are comprehensive and have good coverage,
there is a somewhat _ad hoc_ selection.

We use [Unity](https://github.com/ThrowTheSwitch/Unity) test framework.
Moreover, while the program uses the value 6 for the number of colors, and the number
of curves in the Venn diagram, we support the values 3, 4 and 5 as well.
Thus some of the tests use these different values: to present simpler tests
that are easier to understand. Since this value is a compile time constant
not a runtime one, the [Makefile](../Makefile) can compile with any of these values
using folders `objs[3456]` to store the corresponding object files.

## Implementation Details

### MEMO and DYNAMIC annotations

In [core.h](../core.h),  _SOLUTION_, _MEMO_, _DYNAMIC_ are defined as empty,
i.e. they are simply comments on global variables.
_SOLUTION_ is used to mark a global variable as one of those that get used
in the solution found during the main search phase. Some fields or 
global variables maybe marked as _MEMO_ 
indicating that those fields are set during the initialization phase, and 
then do not change. Those fields or variables marked as _DYNAMIC_ do get changed
during the main search, and the old values are stored on the trail
to allow for backtracking.

### Source Files and Geometric Concepts


| Geometric Concept  | Files | Notes |
| ------------- | ------------- | ---- |
| Color | color.c, color.h | Edge Label |
| ColorSet | color.c, color.h | Face Label, used as proxy forward reference to FACE |
| Cycle | color.c, color.h | Sequence of Edge colors around a Face |
| CycleSet | color.c, color.h | Possible Sequences of Edge colors around a Face |
| Link in Curve | edge.h | The pointy end of an edge, where it meets a vertex, forward reference to EDGE and POINT |
| Edge | edge.c, edge.h | A directed, labelled side of a face, between two points, one of which is called out as the arrowhead |
| Curve | edge.c, edge.h | a connected sequence of edges with the same label |
| Vertex | vertex.c, vertex.h | A possible oriented vertex between 4 specific faces, forward reference to EDGE |
| Triangle | vertex.c, vertex.h | a closed curve, satisfying various triangle rules in relationship to other triangles |
| Face  | face.c, face.h  | A face of a Venn diagram |

The geometric files are in the sequence above, with each header file including the previous header file,
and each c file including the corresponding header file, so that face.h and face.c include everything.

Some of the functionality that should logically be in face.c is found in search.c to better balance
the files.

color.h includes core.h which defines various constants needed.

### Predicate Files

| Predicate | Files | Notes |
| ------------- | ------------- | ---- |
| Initialize | initialize.c | Initializes solution space and memoizes results |
| InnerFace | innerface.c | Processes inner faces |
| Log | log.c | Logging of inner faces |
| Venn | venn.c | Main search for Venn diagrams |
| Save | save.c | Saves solutions |
| Corners | corners.c | Assigns corners to faces |
| GraphML | graphml.c | Writes GraphML output |
| FAIL | engine.c | Final predicate that always fails |
| SUSPEND | engine.c | Special predicate for testing |

Each predicate file implements a single predicate, with the predicate's name matching the file name.
The header files declare the predicate structure and any helper functions needed by the predicate.
The implementation files contain the predicate's logic and any file-scoped helper functions.

### Naming Conventions

Static variables of any scope (function, file, global) are in PascalCase.

Functions of any scope are in camelCase.

Non-static local variables are in camelCase.

Macros are in UPPER_SNAKE_CASE.

#### Function name prefixes

We use the following naming conventions:

- functions that modify the solution area, or create memoized results, 
  before we begin the search start with `initialize...`
- functions that modify the Scratch area during the dynamic search, recording
  the changes on the trail start with `dynamic...`


### File Layout Conventions

Most of the files correspond to geometric concepts with both a .c file and .h file.
Sometimes we put more than one geometric concept into one .c file.

We have additional files for the trail concept, for the symmetric group S₆, with explicit support for the dihedral group D₆,
which we use to avoid computing symmetries, without loss of generality, and for utils.

Each .c file has the following layout:

- includes
- global variables: first globally scoped then file scope
- declaration of any file scoped static functions
- externally linked functions in the following sections
    - `initialize...` functions
    - `dyanmic...` functions
    - `reset...` functions
    - other functions (in alphabetical order)
- file scoped static functions
