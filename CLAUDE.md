# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the main venn binary
make

# Run all tests (requires Unity test framework installed at ../Unity)
make tests

# Clean build artifacts
make clean

# Run a specific test (example for PCO 2-color test)
bin/test_pco2

# Format code (requires clang-format)
touch .format
make .format

# Run the venn program with output to a results directory
bin/venn -f ../results

# Find only solutions and variants with specific 5-face degree sequence
bin/venn -f ../results -d 664443

# Find just the Venn diagram solutions without variants
bin/venn -f ../results -n 1 -j 1
```

## Code Architecture

### Overall Structure

This codebase implements a search algorithm for Venn diagrams of six triangles, as described in the Carroll 2000 paper. The program searches for monotone simple 6-Venn diagrams that satisfy necessary conditions for being stretchable into a diagram drawn with six triangles.

The program follows a non-deterministic search approach, dividing the search into three main parts:
1. Finding a maximal sequence of 6 integers making a 5-face degree signature
2. Finding 64 facial cycles defining a Venn diagram with this signature
3. Finding an edge to corner mapping satisfying the condition that every pair of lines cross at most once

The final output is in GraphML format, defining a planar graph labeled to show 18 pseudoline segments in six sets of three.

### Key Architectural Components

1. **Non-deterministic Engine**: Implements a backtracking search engine that executes a sequence of predicates, used throughout the codebase for searching.

2. **Predicates System**: The search is divided into predicates (e.g., Initialize, InnerFace, Venn, Corners, etc.), each handling a specific phase of the search.

3. **Memory Management & Trail**: The program uses a trail-based approach to track state changes during backtracking, allowing state to be properly restored when the engine backtracks.

4. **Geometric Concepts**: The codebase implements various geometric concepts:
   - Colors (edge labels)
   - ColorSets (face labels)
   - Cycles (sequences of edge colors)
   - Edges (directed, labeled sides of faces)
   - Vertices (oriented meeting points between edges)
   - Faces (regions in the Venn diagram)
   - Triangles (closed curves with the same label)

5. **Partial Cyclic Order**: Implements the Partial Cyclic Order (PCO) system for ensuring proper geometric constraints.

### File Structure

- **Core Geometric Files**: color.c/.h, cycle.c/.h, edge.c/.h, face.c/.h, vertex.c/.h, triangles.c/.h
- **Search Engine**: engine.c/.h, nondeterminism.c/.h
- **Predicates**: initialize.c, innerface.c, venn.c, corners.c, save.c, graphml.c
- **Utility Files**: memory.c/.h, statistics.c/.h, utils.c/.h, failure.c/.h
- **Main Program**: main.c/.h, entrypoint.c
- **Test Files**: test/test_*.c

### Development Notes

The codebase is compiled with different NCOLORS values (2-6), which determines the number of colors/curves in the Venn diagrams being searched. The production version uses NCOLORS=6, but smaller values are used for testing and development.

The project has a hard dependency on 64-bit architectures and has been tested on macOS, Ubuntu, and Amazon Linux/RedHat.