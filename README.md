# KTruss

C++17 implementation of k-truss decomposition for the paper *Truss decomposition in massive networks*.

The implementation reads an undirected graph, removes self-loops and duplicate edges, computes each edge's truss value, and reports the maximum truss number `kmax`.

## Build

```bash
make
```

The executable is written to:

```bash
output/ktruss
```

## Run

```bash
./output/ktruss <graph-file> [--output file] [--one-based]
```

Examples:

```bash
./output/ktruss data/data.txt --one-based
./output/ktruss data/amaze.txt
./output/ktruss data/data.txt --one-based --output data.truss
```

The optional output file stores one record per edge:

```text
<source> <target> <truss>
```

## Input Formats

The parser supports these graph formats.

Header edge list:

```text
<vertex_count> <edge_count>
<source> <target>
<source> <target>
...
```

Plain edge list:

```text
<source> <target>
<source> <target>
...
```

Adjacency list:

```text
<vertex_count> <edge_count>
<vertex_id> <degree> <neighbor_1> ... <neighbor_n>
...
```

or:

```text
<vertex_count> <edge_count>
<degree> <neighbor_1> ... <neighbor_n>
...
```

Vertex ids are zero-based by default. Use `--one-based` for one-based input.

## Clean

```bash
make clean      # remove object files
make clean_all  # remove object files and executable
```

## Project Layout

```text
include/TrussDecomposition.h  Public API
src/TrussDecomposition.cpp    Graph parser and truss decomposition
src/main.cpp                  CLI runner
data/                         Small sample inputs
```
