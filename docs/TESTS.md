# Test Suite Documentation

This document describes the test suite and includes illustrations for tests that benefit from visual representation.

## 3-Venn Diagram; test_venn3.c

There is only one simple 3 venn diagram:

<img src="../images/Venn3.jpg" alt="Basic 3-Venn Diagram" width="400"/>

Other 3 venn diagrams, have multiple lines going through a single point,
e.g. from the online survey [Ruskey and Weston]

<img src="../images/images/combinatorics-ds5-venn3c-03-04.gif" alt="Two other 3-Venn diagrams" width="400"/>



<img src="../images/Venn3ABC.jpg" alt="3-Venn Diagram with ABC labels" width="400"/>
<img src="../images/Venn3ABCFaceAEdge.jpg" alt="3-Venn Diagram ABC Face A Edge" width="400"/>
<img src="../images/Venn3ABFaceAEdge.jpg" alt="3-Venn Diagram AB Face A Edge" width="400"/>
<img src="../images/Venn3ACB.jpg" alt="3-Venn Diagram with ACB labels" width="400"/>
<img src="../images/Venn3AFaceAEdge.jpg" alt="3-Venn Diagram A Face A Edge" width="400"/>
<img src="../images/Venn3OuterFaceAEdge.jpg" alt="3-Venn Diagram Outer Face A Edge" width="400"/>

This test suite verifies the basic structure of a 3-Venn diagram. Several tests would benefit from illustrations:

### Edge Tests

The following tests verify edge relationships between faces and need illustrations:

1. `testOuterAEdge` - Tests the edge between the outer face and face A
2. `testAFaceAEdge` - Tests the edge between face A and its adjacent faces
3. `testAbFaceAEdge` - Tests the edge between face AB and its adjacent faces
4. `testAbcFaceAEdge` - Tests the edge between face ABC and its adjacent faces

TODO: Add illustrations for these tests showing:
- Face relationships
- Edge directions
- Vertex configurations
- Color assignments

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