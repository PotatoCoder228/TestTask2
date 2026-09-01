# Test task 2

## How to build

```bash
#build
make

#run
./generator data/example_input.txt
```

## The principle of the algorithm

The algorithm was based on 3 principles:
1. Extensibility and preservation of intermediate states. Not hardcore coding.
2. Optimization of run-time and memory asymptotics.
3. Optimize the output file size.
4. Correct display in different browsers and platforms.

The program reads the coordinates of non-overlapping rectangles and performs coordinate compression: it collects all unique X and Y coordinates of their edges, sorts them, and uses the distances between adjacent coordinates as the widths of table columns and the heights of table rows.

As a result, the table size depends on the number of rectangles and their boundaries rather than on the absolute coordinate values, while preserving the original proportions and relative positions of the objects.

After compression, each rectangle is converted into a position within the table with the corresponding rowspan, colspan, and a color based on its relative area.
