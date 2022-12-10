This is a pool of the most challenging getopt exercises made by students.
Some are from old exercises, some from old exams and some are just self invented.

The goal in each exercise is the same: Get the options based on the synopsis and validate user input.


## exercise 1
```
SYNOPSIS: ./client [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]

optargA: int
optargB: int
optargC: int
```

Additionally:

- If `-d` is set but `-b` is not, then fail
- If `-d` is not set but `-b` is, then accept the input


## exercise 2
```
SYNOPSIS: ./client [-a optargA | {-b optargB | -o} ] -c [optargC] file...

optargA: int [-50,300]
optargB: char [69,71]
optargC: char[8] (exactly!)
```


Additionally:

- accept 8 positional arguments at most (`argc-optind`)


maximum of 8 pos-args

## exercise 3
```
SYNOPSIS: ./client {-a optargA | -b | -c [optargC] } file...

optargA: int
optargC: int
```

Additionally:

- There must be exactly 4 files
- add them all together in reverse of the mentioned order in the string `totalString`
- set a flag if `-b` was set