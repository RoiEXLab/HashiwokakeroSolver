[![Build Status](https://travis-ci.com/RoiEXLab/HashiwokakeroSolver.svg?branch=master&style=flat-square)](https://travis-ci.com/RoiEXLab/HashiwokakeroSolver)

# HashiwokakeroSolver
橋をかけろ - A fast Solver for Hashiwokakero Riddles written in C++

### Usage
The basic usage is as follows
```bash
./SolverMain /path/to/input/file /path/to/output
```

Where the input file must be one of the following formats:

### File Formats

#### Plain format
The plain format is a human-readable format that is pretty intuitive.
I generally looks something like this:
```
1  2

2  3
```
Where the numbers are the Islands to be connected and the position in the file are the actual coordinates.

#### XY Format
The XY Format is far more machine friendly.
It's basically a CSV file with the scheme xcoordinate,ycoordinate,requiredbridges and looks something like this:

```
0,0,1
2,0,2
0,2,2
2,2,3
```
#### Comments
In both formats lines starting with `#` are treated as comments and can be completely ignored.
