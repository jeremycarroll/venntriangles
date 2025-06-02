# Venn Triangles

Search for Venn diagrams of six triangles.
This is a new re-implementation of the method descibed in [Carroll, 2000](https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/2000/HPL-2000-73.pdf); there are some improvements, giving about a 5,000,000 fold speed up on the earlier version (which is lost).

## Overview

This program searches the space of all monotone simple 6-Venn diagrams looking
for ones that satisfy various necessary conditions for the diagram to be stretchable
into a diagram drawn with six triangles.

The output is as graphml files, each defining a planar graph, labelled to show
18 pseudoline segments in six sets of three.
It is known that many of these are stretchable. 
The problem of stretching the pseudoline segments to make the drawing is not addressed in this program.
It is possible, with several hours work, to do such a stretching by hand, for the easier cases.

## Installation

We require gcc and make. For development, we also 
need [Unity](https://github.com/ThrowTheSwitch/Unity) test framework and clang-format.

There is a hard dependency on 64-bit architectures, verified as:
```
    assert((sizeof(uint64_t) == sizeof(void *)))
```

We have tested on MacOS 15.4.1, Ubuntu, and Amazon Linux / RedHat.

### Mac
```
# required - for gcc and make
xcode-select --install
# optional
brew install clang-format 
git clone git@github.com:ThrowTheSwitch/Unity.git
# required
git clone git@github.com:jeremycarroll/venntriangles.git
cd venntriangles
make
```

### Ubuntu
```
# required
sudo apt-get update
# for gcc and make
sudo apt-get install build-essential
# optional
sudo apt-get install clang-format
git clone git@github.com:ThrowTheSwitch/Unity.git
# required
git clone git@github.com:jeremycarroll/venntriangles.git
cd venntriangles
make
```

### Amazon Linux / RedHat
```
# required
sudo yum update
# for gcc and make
sudo yum groupinstall "Development Tools"
# optional
sudo yum install clang-tools-extra
git clone git@github.com:ThrowTheSwitch/Unity.git
# required
git clone git@github.com:jeremycarroll/venntriangles.git
cd venntriangles
make
```

## Usage

There is no need to run this program more than once: it takes less than 10 minutes, and 60 GB of disk for all the results.

A complete run is:
```
bin/venn -f ../results
```
which creates a new folder `../results` and populates with many files.

To find just the Venn diagram solutions without the variants use:
```
bin/venn -f ../results -n 1 -j 1
```

To find only solutions and variants with specific 
5-face degree sequence, use:
```
bin/venn -f ../results -d 664443
```

Each solution has an output file (ending .txt) in the results folder. 
Each variation of that solution, i.e. the placement of the corners of the triangles,
has an xml file nested within the corresponding subfolder. The xml file is in GraphML
format, and needs further processing ... in particular it hopefully contains an 
arrangement of 18 pseudoline segments which needs to be stretched to give a Venn diagram
of six triangles.

An [example output file](./docs/555444-64-27-005.xml) has a pseudoline segment arrangement
that can be stretched to:

<img src="./images/carroll2000bis.jpg" alt="A 6-Venn Diagram" width="900"/>

## Documentation

### Mathematical Foundations

See [MATH.md](docs/MATH.md) for mathematical concepts, conjectures, and references.

### System Design

See [DESIGN.md](docs/DESIGN.md) for implementation details and design decisions.

### Results

See [RESULTS.md](docs/RESULTS.md) for a summary of the solutions found.

### Test Cases

See [TESTS.md](docs/TESTS.md) for visual documentation of some of the test cases.

## References

See [MATH.md](docs/MATH.md#references).
