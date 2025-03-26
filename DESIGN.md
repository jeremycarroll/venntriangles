
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
- global variables, and file scope variables that are part of the scratch area 
  have names starting with Scratch...
- functions that modify the Scratch area before we begin the search start with
  initialize...
- functions that modify the Scratch area during the dynamic search, recording
  the changes on the trail start with dynamic...

## Guidance about Algorithm And Code Layout


| Geometric Concept  | Files | Notes |
| ------------- | ------------- | ---- |
| Color | color.c, color.h | Edge Label |
| ColorSet | color.c, color.h | Face Label, used as proxy forward reference to FACE |
| Cycle | color.c, color.h | Sequence of Edges around a Face |
| CycleSet | color.c, color.h | Possible Sequences of Edges around a Face |
| Undirected Point | point.c, point.h | A possible oriented point between 4 specific faces, forward reference to EDGE |
| Directed Point | point.c, point.h | A point on a labelled curve |
| Edge | edge.c, edge.h | A directed, labelled side of a face, between two points |
| Curve | edge.c, edge.h | a connected sequence of edges with the same label |
| Triangle | edge.c, edge.h | a closed curve, satisfying various triangle rules in relationship to other triangles |
| Face  | face.c, face.h  | A face of a Venn diagram |

The geometric files are in the sequence above, with each header file including the previous header file,
and each c file including the corresponding header file, so that face.h and face.c include everything.

color.h includes core.h which defines various constants needed.
