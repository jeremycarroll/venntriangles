# Mathematical Concepts

We outline the key mathematical concepts and open problems related to Venn triangles.
We switch, without comment, between treating the number of curves in the Venn diagram as N, or 6; 
the reader is left to make the jump as needed.

## Venn Diagrams, Facial Cycles, and FISCs

A family of intersecting simple closed curves (a FISC) is a collection of N Jordan curves in the Euclidean plane, with some inner face, inside all the curves, and with every two curves intersecting finitely. This is a Venn diagram, if for every subset of the curves, there is exactly one face inside those curves, and outside the other curves.

A FISC is a plane graph, with an induced planar graph.

We focus on cases where:
- there are six curves
- they are all triangles
- no three curves meet at any vertex

We use the word _vertex_ as a point of intersection between two curves; _edge_ as the segment of a curve between two adjacent vertices on the curve, _face_ as the enclosed region between some cycle of edges not containing other vertices or edges.

A 6-Venn diagram can be labeled by labelling the curves a,b,c,d,e,f (often in our code 0, 1, 2, 3, 4, 5) and then labelling the faces A, AB, ABC, ABCD, etc. to reflect which curves the face is inside (in our code, a number between 0 and 63 interpretted bitwise). The labelling is arbitrary and is not part of the Venn diagram; we also ignore rotations and reflections of the plane. This labels are known as colors - i.e. each edge has a color, each face has a set of colors.

We use the term _k-face_ for a face lablled by _k_ colors; and _k-edge_ for an edge lying between a _k-face_ and a _(k-1)-face_.

We direct every edge so that the curves are clockwise cycles in the plane.

For much of this discussion we will assume an arbitraty labelling.

### Initial Observations

We observe the following, some are fairly trivial, some are in the literature, ones marked _conjecture_ are neither.

1. Each face has at most one egde from each curve.
1. Each face has at least three edges.
1. (conjecture) Up to isomorphism (see below) the Venn diagram is defined by the facial cycles, 
   i.e. the sequence of 3 to 6 letters from abcdef, giving the labels of the edges in a clockwise progression 
   around each face.
1. There are hence no more than 394⁶⁴ different Venn diagrams.
1. Triangles are convex; a 6 Venn triangle diagram is a FISC of convex curves.
1. Thus, the FISC is monotone.
1. The dual of monotone FISCs have one source (the inner face), and one sink (the outer face).
1. (conjecture) In faces other the inner and outer faces, distinguishing outer edges, (i.e. where in the dual graph 
   the edge is out-going), from inner edges, i.e.(in-coming in the dual), we have:
    - the facial cycle consists of 1 or more inner edges, followed by 1 or more outer edges.
1. If _F_ is edge adjacent to _F'_ at _j_, and _i_, _j_, _k_ is part of the facial cycle of _F_ then _k_, _j_, _i_ is 
   part of the facial cycle of _F'_.
1. If the labels of _F_ and _F'_ differ by one letter _j_ and they are not edge adjacent (at _j_), then _j_ appears in 
   neither facial cycle of _F_ nor _F'_
1. If _F_ is vertex adjacent to _F'_ at a vertex where curve _i_ meets curve _j_ and _i_, _j_ is in the 
   facial cycle of _F_ then _i_, _j_ is in the facial cycle of _F'_
1. If the labels of _F_ and _F'_ differ by two letters _i_, _j_ and _F_ and _F'_ are not vertex adjacent then neither 
   facial cycle include _i_, _j_ nor _j_, _i_
1. At each vertex, one curve, the _primary_, crosses from inside the other curve, the  _secondary_, to
   outside it.
1. Each vertex has four faces meeting at it, the one inside both curves is the inner face.
1. The vertices hence can be labelled with the pair: the primary color, and the secondary color; and the set of colors
   including both the primary and secondary colors, labelling the inner-most face touching the vertex.
1. There are (_N_ - 2)! × _N_ × (_N_ - 1) different labels of that form for a vertex.
1. A pair of triangles, labelled _i_ & _j_ intersect in at most six vertices: and at most three vertices
   in which _i_ is the primary color.
1. A side of one triangle intersects a side of another triangle at most once.
1. (conjecture) For simple, monotone Venn diagrams, the _k-edges_ form a single cycle.
1. For simple, monotone _N_-Venn diagrams, the _N_-face and the 0-face both have face degree _N_.
1. For simple, monotone _N_-Venn diagrams, there are _N_ _N_-edges and _N_ 1-edges.
1. (conjecture) For simple, monotone _N_-Venn diagrams, and 1<_k_<_N_ 
   there are <sub>N</sub>C<sub>k</sub> + <sub>N</sub>C<sub>k-1</sub> k-edges.
1. The sum of the face degrees of the 5-faces is the number of 6-edges plus the number of 5-edges, i.e. 27. 

### Types of Isomorphism

- Venn Diagram Class Isomorphism

(reference unknown), projecting a FISC onto a sphere, then two FISCs are in the same class, if, by continuous transformation of the sphere, one of them can be changed into the other or its mirror image.

- Venn Diagram Isomorphism

As per [Bultena et al.] (following [Hamburger]), two FISCs are isomorphic if, by continuous transformation of the plane,
one of them can be changed into the other or its mirror image.

- Polygon Venn Diagram Isomorphism

Adding the corners of the polygons as vertices in the graph, then two Venn diagrams of polygons are isomorphic if, 
by continuous transformation of the plane, one of them can be changed into the other or its mirror image.

Trivially, these three concepts are defined in order of increasing specificity.

Given a labelling of a Venn diagram, we can apply the natural action of a member of S<sub>n</sub> to give a different labelling.
When considering monotone 6-Venn diagrams up to Venn diagram isomorphism, without loss of generality, the facial cycle for the innermost face is (a, b, c, d, e, f). The action of D<sub>n</sub> preserves this (or reflects it).

### 5-Face Degree Signature

We can take the sequence of integers being the face degree of faces (BCDEF, ACDEF, ABDEF, ABCEF, ABCDF, ABCDE). 
Each of these is an integer between 3 and 6. The sum of these integers is 27.
The action of D<sub>n</sub> induces a permutation on this sequence, while preserving the facial cycle (a, b, c, d, e, f) of the innermost face. Without loss of generality, we can restrict ourselves to cases where 
this sequence is maximal under the action of  D<sub>n</sub>.

However, some sequences of 6 numbers such as 5,4,5,4,5,4 have non-trivial stabilizer. For these, we can extend the sequence 
of 6 face degrees to a sequence of 64 face degrees, by appending the other faces in arbitrary order. Then, still, wlog, we can restrict ourselves to when this longer sequence is maximal under the group action. In practice, this suffices to uniquely label each 6-Venn diagram (of triangles).

### Facial Cycle Signatures

A more thorough way to identify Venn diagrams, and Venn diagrams classes (on both the Euclidean plane and the spherical plane), 
is by identifying a Venn diagram by the facial cycles of every face.

We have a small number of choices for the facial cycle depending on N, i.e. $\sum_{k=3}^N {}_N \mathrm{ C }_k (N-1)! $

We can order these so that the cycle (a b c d e f) is last (one way is shown in the definition of `initializeCycles` in [color.c](../color.c)), hence giving a number for each facial cycle, with a simple clockwise rotation being maximal. 

Given a labelling of a Venn diagram we construct the facial cycle signature being
a sequence of $2^N$ numbers, each being the identifying number of the facial cycle of the corresonding face 
(starting with the outer face). Each member of $S_N$ induces an action on these sequences, permuting both the order (because of the action on the face ids), and the the facial sequence number (because of the action on facial sequences).

The facial cycle of a monotone Venn diagram is then found by applying a permutation to ensure the permutation 
of the outer face is (a b c d e f). Then the facial cycle signature of the Venn diagram is the maximal value of the
facial cycle signature of a labelling of the Venn diagram under the action of $D_N$.

The class facial cycle signature is found similarly by finding the maximal value under the action of $S_N$, allowing
any face to play the role of the outer face, since all faces are equivalent on the sphere.

## Pseudolines and Pseudoline Segments

A further mathematical area is that of pseudolines in the Euclidean plane.

A pseudoline is an unbounded Jordan curve cutting the Euclidean plane into two parts.

A pseudoline arrangement is a set of pseudolines such that every pair intersect exactly once.

A pseudoline arrangement can be stretched if it is isomorphic to an arrangement of straight lines.

A pseudoline segment is a portion of a pseudoline. An arrangement of pseudoline segments is stretchable if the segments can be mapped to straight line segments while preserving all intersection properties. Many arrangements of pseudoline segments are not stretchable, and determining whether a given arrangement is stretchable is computationally difficult.

# Relationship to the Code

The code is based on an exploration of all possible combinations of facial cycles.

These are restricted by the observations about monotone simple Venn diagrams.

W.l.o.g. we restrict ourselves to those cases where the facial cycle of the innermost face is (a, b, c, d,  e, f),
and the 5-face degree signature is maximal.
We discard duplicates where the full sequence of 64 face degrees is non-maximal.

There is a small confusion since the definition of facial cycle signature assumes that the outermost face has facial cycle (a, b, c, d, e, f), which usually requires a relabelling.

The only aspects of the theory of pseudolines that appears in the code are the output files, and the prohibition on pseudoline segments intersecting more than once.

# References

[Bultena, Bette, Branko Grünbaum, and Frank Ruskey. "Convex drawings of intersecting families of simple closed curves." CCCG. 1999.](https://www.cccg.ca/proceedings/1999/c14.pdf)

[Carroll, Jeremy J. "Drawing Venn triangles." HP LABORATORIES TECHNICAL REPORT HPL 73 (2000).](https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/2000/HPL-2000-73.pdf)

Grünbaum, Branko. "The importance of being straight." In Proc. 12th Biannual Intern. Seminar of the Canadian Math. Congress (Vancouver 1969), pp. 243-254. 1970.

[Felsner, Stefan, and Jacob E. Goodman. "Pseudoline arrangements." In Handbook of Discrete and Computational Geometry, pp. 125-157. Chapman and Hall/CRC, 2017.](https://www.csun.edu/~ctoth/Handbook/chap5.pdf)
