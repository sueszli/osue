Was sind positional arguments?
---
Nachdem alle Command Line Optionen abgearbeitet wurden, zeigt ``optind`` auf das erste positional argument

```c
./hello -a optarg arg1 arg2
// nach getopt()
argc = 5, optind = 3
```