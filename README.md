The pool of OSUE exercises. Assignments/specifications are maintained here.

Topics include (but are not limited to):
* C programming basics
* Sockets
* Process creation (fork, exec)
* Pipes
* Semaphores
* Shared memory

## Generating `pdf` files

To generate the pdf documents, you must install latex first.

The script `genpdfs.sh` is provided for convenience. It recursively runs latexmk
for all .tex files it finds in the current working directory and all subdirectories.

Install latex on your system with all necessary dependencies and then run the script:
```bash
$ sudo chmod +x ./genpdfs.sh
$ ./genpdfs.sh
```