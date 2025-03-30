
## Backtracking, Initialization and Naming

We have multiple global variables which constitute a scratch area with details
of a single, partial, simple Venn diagrams, of 6 curves, each curve is labelled with one
of six colors, each face is labelled with a subset of these. Each face has a facial cycle,
initial unset, chosen from one of a set of possible facial cycles. There are a few hundred
to choose from, being all possible sequences (understood as a cycle) from 0,..,5 of length 3 to 6, 
where each color occurs at most once.

The goal of the program is to find all choices of facial cycle for each face such that
the overall result describes a planar graph that can be drawn with six traingles.

The approach is to dynamically choose one face, then dynamically choose one facial cycle. 
We then dynamically compute all the consequences of that choice. And repeat. Sometimes we find that 
the consequence is that we cannot proceed. This is called a failure, and we backtrack. We undo 
all the dynamic computations until we reach the most recent facial cycle choice point, we then
make the next choice of facial cycle - if there are none left, we continue backtracking.

We use a large array called the trail to record the dynamic choices so that backtracking
is trivial.

We use the following naming conventions:
- functions that modify the Scratch area before we begin the search start with
  initialize...
- functions that modify the Scratch area during the dynamic search, recording
  the changes on the trail start with dynamic...
- functions that reset the Scratch area during testing, start with reset...

## Guidance about Algorithm And Code Layout


| Geometric Concept  | Files | Notes |
| ------------- | ------------- | ---- |
| Color | color.c, color.h | Edge Label |
| ColorSet | color.c, color.h | Face Label, used as proxy forward reference to FACE |
| Cycle | color.c, color.h | Sequence of Edge colors around a Face |
| CycleSet | color.c, color.h | Possible Sequences of Edge colors around a Face |
| Link in Curve | edge.h | The pointy end of an edge, where it meets a vertex, forward reference to EDGE and POINT |
| Edge | edge.c, edge.h | A directed, labelled side of a face, between two points, one of which is called out as the arrowhead |
| Curve | edge.c, edge.h | a connected sequence of edges with the same label |
| Point | point.c, point.h | A possible oriented point between 4 specific faces, forward reference to EDGE |
| Triangle | point.c, point.h | a closed curve, satisfying various triangle rules in relationship to other triangles |
| Face  | face.c, face.h  | A face of a Venn diagram |

The geometric files are in the sequence above, with each header file including the previous header file,
and each c file including the corresponding header file, so that face.h and face.c include everything.

Some of the functionality that should logically be in face.c is found in search.c to better balance
the files.

color.h includes core.h which defines various constants needed.

## Naming Conventions

Static variables of any scope (function, file, global) are in PascalCase.

Functions of any scope are in camelCase.

Non-static local variables are in camelCase.

Macros are in UPPER_SNAKE_CASE.

## File Layout Conventions

Most of the files correspond to geometric concepts with both a .c file and .h file.
Sometimes we put more than one geometric concept into one .c file.

We have additional files for the trail concept, for the top level search primitives, for the dihedral group D6,
which we use to avoid computing symmetries, without loss of generality, and for utils.

Each .c file has the following layout:

- includes
- global variables: first globally scoped then file scope
- declaration of any file scoped static functions
- externally linked functions in the following sections
    - initialize... functions
    - dyanmic... functions
    - reset... functions
    - other functions (in alphabetical order)
- file scoped static functions
