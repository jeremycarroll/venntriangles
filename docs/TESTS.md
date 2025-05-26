# Test Suite Documentation

This document describes the test suite and includes illustrations for tests that benefit from visual representation.

## 3-Venn Diagram; test_venn3.c

A simple 3-Venn diagram is one where each pair of curves intersects exactly once, and no three curves intersect at a single point. There is only one such diagram up to homeomorphism:

<img src="../images/Venn3.jpg" alt="Basic 3-Venn Diagram" width="400"/>

Other 3-Venn diagrams exist, but they violate the simplicity condition by having multiple lines through a single point. For example, from the online survey [Ruskey and Weston], notice that the left one of this pair can be drawn with convex curves (e.g. triangles), and the right one cannot (e.g. the outer face has only two sides;
breaking monotonicity (see [maths](./MATH.md)) preventing convex shapes).

<img src="../images/combinatorics-ds5-venn3c-03-04.gif" alt="Two other 3-Venn diagrams" width="400"/>

This means that compiling the code with `NCOLORS` as 3 allows
us to verify the data structures fairly easily since they are small, and less complicated than with 6 colors.

There are six ways to label the simple 3-Venn diagram with the six permutations of _A_, _B_, and _C_. However, we keep the first fixed arbitrarily leaving us two labellings:

### ABC

<img src="../images/Venn3ABC.jpg" alt="3-Venn Diagram with ABC labels" width="400"/>

### ACB

<img src="../images/Venn3ACB.jpg" alt="3-Venn Diagram with ACB labels" width="400"/>

### Face Adjacency Tests

Both of these labelings (ABC and ACB) have the same adjacency relationships between the faces, which are set up during the [initialization](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L27) phase. In the tests, the initialization does not
get reversed in the `tearDown` function, instead, the initialization is idempotent. The key difference between these two
labelings is the orientation. We orient all curves in a clockwise fashion, so that the central face has cycle _abc_ in the first, and _acb_ in the second. This clockwise orientation is an artifact of our approach to the search, and not fundamental to the
Venn diagram, which does not have the arrows on the curves (or the labels). Conventionally, we read the edges of the outer face counter-clockwise not clockwise. The first picture has the inner face with facial cycle _abc_ and the outer face with facial cycle _acb_. 

We test both orientations of the inner face.

There are several tests such as [testOuterFace](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L61) and [testAbFace](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L79) verifying the basic relationships between the faces, which are the same in both of those diagrams.

### Edge & Vertex Tests

There are four tests to verify the vertices for the 3-Venn case. While we know the color of the 8 faces, we don't know
the nature of the vertices. As becomes more important in the 6-Venn case, during initialization, we memoize the relationships between all the _possible_ vertices and the _possible_ edges and the (2<sup>N</sup>) faces before we start. A possible vertex is [characterized](https://github.com/jeremycarroll/venntriangles/blob/testdocs/vertex.c#L135) by the colors of the outermost face adjacent to the vertex and the two colors of the edges passing through the vertex (neither of which is a face color). For N=3, this gives us 2^(3-2) × 3 × (3-1) = 2 × 3 × 2 = 12 possible vertices. The two edges passing through the vertex are [distinguishable](https://github.com/jeremycarroll/venntriangles/blob/testdocs/vertex.h#L24-L26). One edge, the primary edge, passes from inside the curve of the other edge, the secondary edge, to outside. The secondary edge, conversely, passes from outside the curve of the primary edge, to inside. Thus we have 2<sup>N - 2</sup> × N x (N-1) different possibly vertices. For 3-Venn diagrams, that is 12 possible vertices, i.e. those found in the diagrams _ABC_ and _ACB_ above. In each of the two actual solutions, we use 6 of them because each vertex must be used exactly once, and a 3-Venn diagram has exactly 6 vertices (two for each pair of curves intersecting).

With 6 colors, the number of possible vertices is a fairly modest 480, compared with the 14,446 vertices found in the actual 233 solutions. Each possible vertex appears in multiple solutions. In any of the actual solutions we only use 62 of the 480 possible vertices.

The format, and the body, of the four face and edge tests are the same. Each tests selects one of the faces (the 
[A face](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L134), 
the [AB face](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L144) 
and the [ABC face](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L154)
and the
[outer face](https://github.com/jeremycarroll/venntriangles/blob/testdocs/test/test_venn3.c#L124))
and finds the A edge on that face and the possible vertex 
with the B edge, where the A edge and B edge meet. The direction 
of the edge is clockwise (counterclockwise for the outer face),
and so one or other of the _ABC_ or _ACB_ pictures applies: the one in which the A edge next to the face is heading towards the B edge of that face, rather than away from it. The A edge is either clockwise around that face or not. At the A,B vertex where the edges meet, the A edge is either primary or secondary (as above).

The first three cases are illustrated:

The _ABC_ picture is the one with the a,b vertex on the A-face. The _ACB_ picture has a b,a vertex on the A-face.
<img src="../images/Venn3AFaceAEdge.jpg" alt="3-Venn Diagram A Face A Edge" width="600"/>

The _ACB_ picture is the one with the a,b, vertex on the AB-face.

<img src="../images/Venn3ABFaceAEdge.jpg" alt="3-Venn Diagram AB Face A Edge" width="600"/>

The _ABC_ picture is the one with the a,b, vertex on the ABC-face.

<img src="../images/Venn3ABCFaceAEdge.jpg" alt="3-Venn Diagram ABC Face A Edge" width="600"/>

We put that together in the following table, that motivates
the details of the test cases.

Face | A is Clockwise around face | A is primary at vertex | next Face after vertex
-- | -- | -- | --
A | true | false | AB
AB | true | true | A
ABC | true | true | AC
outer | false | true | B

Notice the counterclockwise orientation for the outer face.

<img src="../images/Venn3OuterFaceAEdge.jpg" alt="3-Venn Diagram Outer Face A Edge" width="600"/>

### Checking backtracking and search

While searching for 3-Venn diagrams is trivial, we have tests to check the function.
Selecting the facial cycle in any face reduces the search space from 2 to 1 for every face,and each of the other 7 faces is set.
A full search finds both solutions _ABC_ and _ACB_, and has 14 forced settings of facial cycles.

## test_venn4.c

There is also only one simple, convex 4-Venn diagram:

<img src="../images/4ellipses.jpg" alt="4-Venn Diagram" width="400"/>

There is one other simple 4-Venn diagram, but this can't be drawn with convex curves: the outer faces has only three sides.

<img src="../images/4nonconvex.jpg" alt="4-Venn Diagram" width="400"/>

## test_venn5.c

This test suite includes tests for the 5-Venn diagram, with existing illustrations for the inversion tests:

### Inversion Tests

The test `testInvert` verifies the behavior of the diagram under label inversion. The following illustrations are available in `images/test_venn5.testInvert/`:

1. `original.pdf` - The original diagram
2. `labels-inverted.pdf` - The diagram with inverted labels
3. `fully-inverted.pdf` - The fully inverted diagram

These illustrations demonstrate how the diagram structure changes under different types of inversions.

## test_known_solution.c

This test suite verifies the Carroll 2000 solution and its properties. It would benefit from illustrations showing:

1. The face relationships
2. The cycles around faces
3. The dihedral group transformations (testDE1, testDE2)

TODO: Add illustrations for these tests.

## test_graphml.c

This test suite verifies the GraphML output format. Future illustrations will be generated from the GraphML files showing:

1. The node/vertex structure
2. The edge relationships
3. The corner assignments
4. The color/label assignments

Note: These illustrations will be generated in a future project using the GraphML output files.

## Future Work

1. Generate illustrations for test_venn3.c edge tests
2. Generate illustrations for test_known_solution.c
3. Create a tool to generate pseudoline diagrams from GraphML files
4. Generate illustrations for test_graphml.c using the GraphML output 

## References

Ruskey, Frank, and Mark Weston. "[Venn diagrams.](https://www.combinatorics.org/files/Surveys/ds5/ds5v3-2005/VennEJC.html)" The electronic journal of combinatorics (2005): DS5-Jun.