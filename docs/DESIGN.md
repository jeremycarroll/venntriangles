# Design Considerations

The goal of the program is to find all choices of facial cycle for each face such that
the overall result describes a planar graph that can be drawn with six traingles.

We cover both high level design and comments on lower level issues of wide scope. 
Comments about lower level issues which fit better in the code are in the code.

## High-Level Design

The problem of finding diagrams of 6 Venn triangles is a search problem,
which we address in a top-down fashion. 
The search is divided into three parts:

1. Find a maximal sequence of 6 integers making a 5-degree signature.
1. (the main search) Find 64 facial cycles defining a Venn diagram with this 5-degree signature,
   which satisfies several necessary conditions to be drawable with triangles.
1. Find an edge to corner mapping for this Venn diagram,
   satisfying the condition that every pair of lines cross at most once.

The final step is to write the resulting Venn diagram, including its corners
into a [GraphML](http://graphml.graphdrawing.org/primer/graphml-primer.html) file.

Each of the three steps, involve guessing, and we usually guess badly. That
branch of the search ends in failure and we backtrack to the previous
choicepoint, and make the next guess.

Success is very similar to failure. We get to the end of the search,
satisfying the critieria for this phase. We then call a callback function
that takes us to the next phase (writing the graphml file being the final phase).
After executing the callback function, we backtrack to proceed to the next guess.

In the first and third phases, we structure the search on the call stack, with six
recursive calls, one for each color. Some of the constraints can be evaluated
on each call, some after the final choice.

In the second phase we have a loop with more explicit choice points.

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

## Backtracking, Memory and the Trail

Memory management presents a challenge to most C programs.

In the crucial main phase, our design is to not use any memory on the heap,
and most of the memory used is global memory, defining a single static
solution to the problem. This reflects that at each point of program
execution we are only considering one of the many solutions.

As part of this design, we preallocate global memory for every possible
edge and every possible vertex, even though only some of them are used
in each solution.

In order to reuse this single record of a solution, we record
every assignment to it, or more precisely the previous value
of every assignment, in a trail, so that as the search fails (or succeeds)
followed by backtracking, we can undo all the changes we made 
by reversing the trail.

In the first and third parts of the search, we use the callstack
to implement the depth first search, so naturally local variables
on the stack get allocated while that part of the search is in scope.

In the third phase, the vertex to line mapping, needed in identifying
faulty corner guesses, is stored in the solution memory,
and tracked on the trail. Other aspects are in local memory.

### Memoization

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

### Use of the Heap

During other stages, the use of heap, particularly for strings, 
simplifies the program, since we do not need to decide ahead of time,
how much memory we need. However, rather than carefully balancing
malloc and free calls, we hold to a design where heap memory has limited
life and we free all heap memory at regular points through the program
execution.

## Six Phases

There are hence the following phases:

1. Initialization

   As well as the usual, this includes initializing the global solution space; and memoizing
   several results needed for the main search.

1. Finding 5-degree signatures
1. Finding Venn diagrams (the main search)
1. Assigning corners
1. Writing GraphML
1. Reset of memory structures (for unit tests only)

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
- functions that reset the Scratch area during testing, start with `reset...`


### File Layout Conventions

Most of the files correspond to geometric concepts with both a .c file and .h file.
Sometimes we put more than one geometric concept into one .c file.

We have additional files for the trail concept, for the top level search primitives, for the dihedral group D6,
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
