# venntriangles
Search for Venn diagrams of six triangles

## Install

## Running the code

A full run takes about 5s to do the search and 6m in total, most of which is writing the results.
There are just under 60 GB of results. There are 233 different solutions, up to isomorphism of the Venn diagram; 
these are from 223 different isomorphism classes. There are 1,730,260 different variations of these solutions,
which identify which face each of the 18 corners lie in. While these different variations do not have any two lines
crossing, verifying that a) they can be embedded in an arrangement of pseudolines, and b) such an arrangement
is stretchable into six triangles is not in scope. The least defined solution has over half a million variations;
most have less than 3,000; although all have over 100 variations.

# Lemmas used in the code

## For a simple FISC of N Curves, there are 2 ^ N - 2 vertices, and 2 ^ (N + 1) - 4 edges.

By Euler:
```
Euler:
E = F + V - 2

F = 2 ^ N

From simplicity, every edge has two vertices, and every vertix has four edges:
E = 2 V

2.V = 2 ^ N + V - 2

and the result follows.
```

# Miscellaneous Literature

## Venn Diagrams
[Bultena, Bette, Branko Grünbaum, and Frank Ruskey. "Convex drawings of intersecting families of simple closed curves." CCCG. 1999.](https://www.cccg.ca/proceedings/1999/c14.pdf)

## HPL Reports
[Carroll, Jeremy J. "Drawing Venn triangles." HP LABORATORIES TECHNICAL REPORT HPL 73 (2000).](https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/2000/HPL-2000-73.pdf)

